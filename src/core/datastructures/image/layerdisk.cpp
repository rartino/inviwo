/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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

#include <inviwo/core/datastructures/image/layerdisk.h>

namespace inviwo {

LayerDisk::LayerDisk(size2_t dimensions, const DataFormatBase* format, LayerType type,
                     const SwizzleMask& swizzleMask, InterpolationType interpolation,
                     const Wrapping2D& wrapping)
    : LayerRepresentation(type, format)
    , DiskRepresentation<LayerRepresentation, LayerDisk>()
    , dimensions_(dimensions)
    , swizzleMask_(swizzleMask)
    , interpolation_{interpolation}
    , wrapping_{wrapping} {}

LayerDisk::LayerDisk(std::string url, size2_t dimensions, const DataFormatBase* format,
                     LayerType type, const SwizzleMask& swizzleMask,
                     InterpolationType interpolation, const Wrapping2D& wrapping)
    : LayerRepresentation(type, format)
    , DiskRepresentation<LayerRepresentation, LayerDisk>(url)
    , dimensions_(dimensions)
    , swizzleMask_(swizzleMask)
    , interpolation_{interpolation}
    , wrapping_{wrapping} {}

LayerDisk::~LayerDisk() = default;

LayerDisk* LayerDisk::clone() const { return new LayerDisk(*this); }

void LayerDisk::setDimensions(size2_t dimensions) { dimensions_ = dimensions; }

const size2_t& LayerDisk::getDimensions() const { return dimensions_; }

bool LayerDisk::copyRepresentationsTo(LayerRepresentation*) const { return false; }

void LayerDisk::updateDataFormat(const DataFormatBase* format) { setDataFormat(format); }

std::type_index LayerDisk::getTypeIndex() const { return std::type_index(typeid(LayerDisk)); }

void LayerDisk::setSwizzleMask(const SwizzleMask& mask) { swizzleMask_ = mask; }

SwizzleMask LayerDisk::getSwizzleMask() const { return swizzleMask_; }

void LayerDisk::setInterpolation(InterpolationType interpolation) {
    interpolation_ = interpolation;
}

InterpolationType LayerDisk::getInterpolation() const { return interpolation_; }

void LayerDisk::setWrapping(const Wrapping2D& wrapping) { wrapping_ = wrapping; }

Wrapping2D LayerDisk::getWrapping() const { return wrapping_; }

}  // namespace inviwo
