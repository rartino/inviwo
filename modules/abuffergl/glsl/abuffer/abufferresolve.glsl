/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.6b
 *
 * Copyright (c) 2013-2023 Inviwo Foundation
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
 * Main file author: Sathish Kottravel
 *
 *********************************************************************************/

//ABuffer
#include "abuffer.glsl"
#include "utils/sampler2d.glsl"

uniform bool isInputImageGiven;
uniform ImageParameters inputimageParameters;
uniform sampler2D inputimageColor;
uniform sampler2D inputimageDepth;

 //Reset ABuffer
 void resetABuffer() {
 	#if USE_ABUFFER==1	
		//reset stuffs with any first acquired thread
		ivec2 coords = ivec2(gl_FragCoord.x, gl_FragCoord.y);
		if ( semaphoreAcquire(coords) ) {		
			setPixelCurrentPage(coords, 0);
			setPixelFragCounter(coords, 0);
			memoryBarrier();
			semaphoreRelease(coords);
		}
	
	#endif
 }

void main() {

#if USE_ABUFFER==1

#if ABUFFER_DISPNUMFRAGMENTS==1
	//test fragment count
	
	//FragData0 = vec4(gl_FragCoord.x/float(SCREEN_WIDTH), gl_FragCoord.y/float(SCREEN_HEIGHT), 1.0, 1.0);	
	float abNumFrag_=(float)getPixelFragCounter(ivec2(gl_FragCoord.x, gl_FragCoord.y));		
	abNumFrag_ = clamp(abNumFrag_, 0.0, 768.0);
	//packing
	float r=0.0,g = 0.0, b=0.0, a=0.0;
	if (abNumFrag_>=512.0) { r = 256.0; g = 256.0; b = abNumFrag_-512.0; }
	else if (abNumFrag_>=256.0) { r = 256.0; g = abNumFrag_-256.0; b=0.0;}
	else { r = abNumFrag_; g=0.0; b=0.0; }
	FragData0 = vec4(r/256.0, g/256.0, b/256.0, 1.0); 
	return;
#endif
	
	if (isInputImageGiven) {
		vec2 texCoords = gl_FragCoord.xy * inputimageParameters.reciprocalDimensions;
		vec4 col = texture(inputimageColor, texCoords); //color image
		vec3 d = texture(inputimageDepth, texCoords).rgb; //depth image

		if (length(col)>0.0) {			
			u8vec4 abufferFrag = u8vec4(col*255.0);
			abufferFrag = clamp(abufferFrag, u8vec4(0), u8vec4(255));
			buildABufferLinkList(ivec2(gl_FragCoord.x, gl_FragCoord.y),
								 abufferFrag,
								 d.x);
		}
	}

	float abNumFrag=(float)getPixelFragCounter(ivec2(gl_FragCoord.x, gl_FragCoord.y));

	gl_FragDepth = 1;

	if (abNumFrag == 0) return;

	//initialize sorting buffer
	u8vec4 backgroundColor = u8vec4(BACKGROUND_COLOR_R*255.0, BACKGROUND_COLOR_G*255.0, BACKGROUND_COLOR_B*255.0, BACKGROUND_COLOR_A*255.0);
	INIT_ABUFFER_RGBA_ELEMENTS(backgroundColor);
	INIT_ABUFFER_EXT_ELEMENTS(1.0)
	
	//fill, depth sort and blend
	fillArrayAndSortABuffer(ivec2(gl_FragCoord.x, gl_FragCoord.y), true);
	
	//reset & sync
    //resetABuffer(ivec2(gl_FragCoord.x, gl_FragCoord.y));

	
	//result
	u8vec4 val = ABUFFER_RGBA_ELEMENT(0, 0);
	FragData0 = vec4(float(val.r)/255.0, float(val.g)/255.0, float(val.b)/255.0, float(val.a)/255.0);
	gl_FragDepth = ABUFFER_EXT_ELEMENT(0, 0);

#endif
}
