/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2023 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef ABUFFER_MAIN_GLSL
#define ABUFFER_MAIN_GLSL

#if (USE_ABUFFER)
// extensions are now added via the shader object (see abuffergl.cpp)
//#extension GL_NV_shader_buffer_load : enable
//#extension GL_NV_shader_buffer_store : enable

// ABuffer includes
#include "abuffer/abuffersort.glsl"
#include "abuffer/abufferlinkedlist.glsl"

void sync(ivec2 coords) {
    int ii = 0;
    bool leaveLoop = false;
    while (!leaveLoop && ii < 1000) {
        if (semaphoreAcquire(coords)) {
            leaveLoop = true;
            memoryBarrier();
            semaphoreRelease(coords);
        }
        ii++;
    };
}

// Reset ABuffer
void resetABuffer(ivec2 coords) {
    // reset stuffs with any first acquired thread
    // ivec2 coords = ivec2(gl_FragCoord.x, gl_FragCoord.y);
    if (semaphoreAcquire(coords)) {
        setPixelCurrentPage(coords, 0);
        setPixelFragCounter(coords, 0);
        memoryBarrier();
        semaphoreRelease(coords);
    }
}

void performFragmentCountOnly(vec2 pixelPos, out float returnVal) {

    ivec2 coords = ivec2(pixelPos);

    if (coords.x >= 0 && coords.y >= 0 && coords.x < SCREEN_WIDTH && coords.y < SCREEN_HEIGHT) {
        int ii = 0;
        bool leaveLoop = false;
        vec4 val1, val2;

        uint curPage = 0;
        uint curFragIdx = 0;

        while (!leaveLoop && ii < 10000) {   // stall-loop
            if (semaphoreAcquire(coords)) {  // acquire semaphore

                curPage = getPixelCurrentPage(coords) * ABUFFER_PAGE_SIZE;  // current page
                curFragIdx = getPixelFragCounter(coords);  // current fragment count per pixel
                uint curFragIdxMod = curFragIdx % ABUFFER_PAGE_SIZE;

                if (curFragIdxMod == 0) {
                    uint newpageidx = (sharedPageCounterAtomicAdd() + 1) * (ABUFFER_PAGE_SIZE);

                    if (newpageidx < (abufferSharedPoolSize)) {
                        sharedPoolSetLink(newpageidx / ABUFFER_PAGE_SIZE,
                                          curPage / ABUFFER_PAGE_SIZE);
                        setPixelCurrentPage(coords, newpageidx / ABUFFER_PAGE_SIZE);
                        curPage = newpageidx;
                    } else {
                        curPage = 0;
                    }
                }

                setPixelFragCounter(coords, curFragIdx + 1);

                leaveLoop = true;
                memoryBarrier();
                returnVal = curFragIdx + 1;
                semaphoreRelease(coords);
            }  // acquire semaphore

            ii++;
        };  // stall-loop
    }
}

// Blend Function
u8vec4 blendABufferArray(uint abNumFrag, uint offset) {

    vec4 bgF = vec4(BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, BACKGROUND_COLOR_A);

    if (abNumFrag == 0) {
        return u8vec4(BACKGROUND_COLOR_R * 255, BACKGROUND_COLOR_G * 255, BACKGROUND_COLOR_B * 255,
                      BACKGROUND_COLOR_A * 255);
    };

    // what to do if it exceeds ABUFFER_SIZE ???
    abNumFrag = clamp((int)abNumFrag, 1, ABUFFER_SIZE);

    vec4 finalColor = vec4(0.0);

    for (int i = 0; i < abNumFrag; i++) {
        u8vec4 frag = ABUFFER_RGBA_ELEMENT(i, offset);
        vec4 fragF = vec4(float(frag.r) / 255.0, float(frag.g) / 255.0, float(frag.b) / 255.0,
                          float(frag.a) / 255.0);

        fragF.rgb = fragF.rgb * fragF.a;

        if ((1.0f - finalColor.a) > 0.0) {
            finalColor = finalColor + fragF * (1.0f - finalColor.a);
        }
    }

    if ((1.0f - finalColor.a) > 0.0) {
        finalColor = finalColor + bgF * (1.0f - finalColor.a);
    }

    return u8vec4(finalColor.r * 255, finalColor.g * 255, finalColor.b * 255, finalColor.a * 255);
}

// This function fills up the fragment color and depth into local memory array
// This local memory is accessed using macros ABUFFER_RGBA_ELEMENT, ABUFFER_EXT_ELEMENT.
// Later sorting and blending is applied on this local memory array

void fillFragmentArray(int pageIdx, int abNumFrag) {
    int curPage = pageIdx;
    int ip = 0;
    int fi = 0;

    while (curPage != 0 && ip < 200) {
        int numElem;
        if (ip == 0) {
            numElem = abNumFrag % (ABUFFER_PAGE_SIZE);
            if (numElem == 0) numElem = ABUFFER_PAGE_SIZE;
        } else {
            numElem = ABUFFER_PAGE_SIZE;
        }

        for (int i = 0; i < numElem; i++) {
            if (fi < ABUFFER_SIZE) {
                u8vec4 val1;
                float val2;
                sharedPoolGetValue(curPage + i, val1, val2);
                ABUFFER_RGBA_ELEMENT(fi, 0) = val1;
                ABUFFER_EXT_ELEMENT(fi, 0) = val2;
            }
            fi++;
        }
        curPage = (int)sharedPoolGetLink(curPage / ABUFFER_PAGE_SIZE) * ABUFFER_PAGE_SIZE;
        ip++;
    };
}

// Resolve A-Buffer Link list
void fillArrayAndSortABuffer(vec2 pixelPos, bool blend) {
    ivec2 coords = ivec2(pixelPos);
    if (coords.x >= 0 && coords.y >= 0 && coords.x < SCREEN_WIDTH && coords.y < SCREEN_HEIGHT) {
        ABUFFER_RGBA_ELEMENT(0, 0) = u8vec4(BACKGROUND_COLOR_R * 255, BACKGROUND_COLOR_G * 255,
                                            BACKGROUND_COLOR_B * 255, BACKGROUND_COLOR_A * 255);
        int pageIdx = (int)getPixelCurrentPage(coords) * ABUFFER_PAGE_SIZE;

        if (pageIdx > 0) {
            int abNumFrag = (int)getPixelFragCounter(coords);
            if (abNumFrag > 0) {
                fillFragmentArray(pageIdx, abNumFrag);
                abNumFrag = min(abNumFrag, ABUFFER_SIZE);

                // Sort
                bubbleSort(abNumFrag);

                // Blend
                if (blend) ABUFFER_RGBA_ELEMENT(0, 0) = blendABufferArray(abNumFrag, 0);
            }
        }
    }
}

void buildABufferLinkList(vec2 pixelPos, u8vec4 linkListValue1, float linkListValue2) {

    //<<<Building Link-list break down>>>

    // stall loop - stall until semaphore acquire

    // acquire-semaphore - to make sure single fragment is processed.
    // Also do not perform heavy operation in semaphore acquire
    // This is similar to blocking in other gpgpu paradigms.
    // This blocking is expensive so unblock as soon as possible.

    // get current page - index which is unique (and generated everytime
    // fragment count exceeds page size. Stored usually in a image texture)

    // get fragment count per pixel ( ranges from 0 to ABUFFER_PAGE_SIZE-1.
    // Stored in image texture)

    ivec2 coords = ivec2(pixelPos);

    if (coords.x >= 0 && coords.y >= 0 && coords.x < SCREEN_WIDTH && coords.y < SCREEN_HEIGHT) {
        // Get current page
        uint curPage = 0;
        uint curFragIdx = 0;
        int ii = 0;
        bool leaveLoop = false;
        vec4 val1, val2;

        while (!leaveLoop && ii < 1000) {
            if (semaphoreAcquire(coords)) {
                curPage = getPixelCurrentPage(coords) * ABUFFER_PAGE_SIZE;
                curFragIdx = getPixelFragCounter(coords);
                uint curFragIdxMod = curFragIdx % ABUFFER_PAGE_SIZE;

                if (curFragIdxMod == 0) {
                    uint newpageidx = (sharedPageCounterAtomicAdd() + 1) * (ABUFFER_PAGE_SIZE);

                    if (newpageidx < (abufferSharedPoolSize)) {
                        sharedPoolSetLink(newpageidx / ABUFFER_PAGE_SIZE,
                                          curPage / ABUFFER_PAGE_SIZE);
                        setPixelCurrentPage(coords, newpageidx / ABUFFER_PAGE_SIZE);
                        curPage = newpageidx;
                    } else {
                        curPage = 0;
                    }
                }

                setPixelFragCounter(coords, curFragIdx + 1);
                curFragIdx = curFragIdxMod;

                leaveLoop = true;
                memoryBarrier();
                semaphoreRelease(coords);
            }

            ii++;
        };

        sharedPoolSetValue(curPage + curFragIdx, linkListValue1, linkListValue2);
    }
}

#endif  // USE_ABUFFER

#endif  // ABUFFER_MAIN_GLSL
