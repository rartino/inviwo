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

#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/properties/propertyconverter.h>
#include <inviwo/core/properties/property.h>

namespace inviwo {

PropertyConverterManager::PropertyConverterManager() : identityConverter_("", "") {}

PropertyConverterManager::~PropertyConverterManager() {}

bool PropertyConverterManager::registerObject(PropertyConverter* converter) {
    std::string src = converter->getSourcePropertyClassIdenetifier();
    std::string dst = converter->getDestinationPropertyClassIdenetifier();
    if (canConvert(src, dst)) {
        LogWarn("Property Converter from type " << src << " to type " << dst
                                                << " already registered");
        return false;
    }
    converters_.insert(std::make_pair(std::make_pair(src, dst), converter));
    return true;
}

bool PropertyConverterManager::unRegisterObject(PropertyConverter* converter) {
    auto it = converters_.find(std::make_pair(converter->getSourcePropertyClassIdenetifier(),
                                              converter->getDestinationPropertyClassIdenetifier()));
    if (it != converters_.end()) {
        converters_.erase(it);
        return true;
    } else {
        return false;
    }
}

bool PropertyConverterManager::canConvert(std::string_view srcClassIdentifier,
                                          std::string_view dstClassIdentifier) const {
    return getConverter(srcClassIdentifier, dstClassIdentifier) != nullptr;
}

bool PropertyConverterManager::canConvert(const Property* srcProperty,
                                          const Property* dstProperty) const {
    return getConverter(srcProperty->getClassIdentifier(), dstProperty->getClassIdentifier()) !=
           nullptr;
}

const PropertyConverter* PropertyConverterManager::getConverter(
    std::string_view srcClassIdentifier, std::string_view dstClassIdentifier) const {
    if (srcClassIdentifier == dstClassIdentifier) return &identityConverter_;

    auto converter = converters_.find(
        std::make_pair(std::string{srcClassIdentifier}, std::string{dstClassIdentifier}));
    if (converter != converters_.end()) {
        return converter->second;
    }
    return nullptr;
}

const PropertyConverter* PropertyConverterManager::getConverter(const Property* srcProperty,
                                                                const Property* dstProperty) const {
    return getConverter(srcProperty->getClassIdentifier(), dstProperty->getClassIdentifier());
}

}  // namespace inviwo
