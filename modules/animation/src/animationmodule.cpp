/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2023 Inviwo Foundation
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

#include <modules/animation/animationmodule.h>

#include <inviwo/core/common/inviwomodule.h>                               // for InviwoModule
#include <inviwo/core/datastructures/camera/camera.h>                      // for mat4
#include <inviwo/core/io/serialization/deserializer.h>                     // for ContainerWrapp...
#include <inviwo/core/io/serialization/serializationexception.h>           // for SerializationE...
#include <inviwo/core/io/serialization/ticpp.h>                            // for TxElement, Ele...
#include <inviwo/core/io/serialization/versionconverter.h>                 // for ElementMatcher
#include <inviwo/core/properties/boolproperty.h>                           // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>                         // for ButtonProperty
#include <inviwo/core/properties/cameraproperty.h>                         // for CameraProperty
#include <inviwo/core/properties/fileproperty.h>                           // for FileProperty
#include <inviwo/core/properties/minmaxproperty.h>                         // for MinMaxProperty
#include <inviwo/core/properties/optionproperty.h>                         // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                        // for OrdinalProperty
#include <inviwo/core/properties/ordinalrefproperty.h>                     // for OrdinalRefProp...
#include <inviwo/core/properties/property.h>                               // for PropertyTraits
#include <inviwo/core/properties/stringproperty.h>                         // for StringProperty
#include <inviwo/core/util/exception.h>                                    // for Exception
#include <inviwo/core/util/foreacharg.h>                                   // for for_each_type
#include <inviwo/core/util/glmmat.h>                                       // for dmat2, dmat3
#include <inviwo/core/util/glmmatext.h>                                    // for mix
#include <inviwo/core/util/glmvec.h>                                       // for dvec2, dvec3
#include <inviwo/core/util/staticstring.h>                                 // for operator+
#include <inviwo/core/util/stringconversion.h>                             // for replaceInString
#include <modules/animation/animationmanager.h>                            // for AnimationManager
#include <modules/animation/animationsupplier.h>                           // for AnimationSupplier
#include <modules/animation/datastructures/animationtime.h>                // for animation
#include <modules/animation/datastructures/basetrack.h>                    // for BaseTrack<>::k...
#include <modules/animation/datastructures/buttonkeyframe.h>               // for ButtonKeyframe
#include <modules/animation/datastructures/buttonkeyframesequence.h>       // for ButtonKeyframe...
#include <modules/animation/datastructures/buttontrack.h>                  // IWYU pragma: keep
#include <modules/animation/datastructures/callbacktrack.h>                // for CallbackTrack
#include <modules/animation/datastructures/camerakeyframe.h>               // for CameraKeyframe
#include <modules/animation/datastructures/cameratrack.h>                  // IWYU pragma: keep
#include <modules/animation/datastructures/controltrack.h>                 // for ControlTrack
#include <modules/animation/datastructures/easing.h>                       // for ease
#include <modules/animation/datastructures/keyframe.h>                     // for operator<
#include <modules/animation/datastructures/keyframesequence.h>             // for operator<
#include <modules/animation/datastructures/propertytrack.h>                // for PropertyTrack
#include <modules/animation/datastructures/valuekeyframe.h>                // for ValueKeyframe
#include <modules/animation/datastructures/valuekeyframesequence.h>        // for KeyframeSequen...
#include <modules/animation/factories/interpolationfactory.h>              // for InterpolationF...
#include <modules/animation/interpolation/cameralinearinterpolation.h>     // for CameraLinearIn...
#include <modules/animation/interpolation/camerasphericalinterpolation.h>  // for CameraSpherica...
#include <modules/animation/interpolation/constantinterpolation.h>         // for ConstantInterp...
#include <modules/animation/interpolation/interpolation.h>                 // for InterpolationT...
#include <modules/animation/interpolation/linearinterpolation.h>           // for LinearInterpol...
#include <modules/animation/workspaceanimations.h>                         // for WorkspaceAnima...

#include <cstddef>     // for size_t
#include <functional>  // for __base
#include <map>         // for map
#include <string>      // for string, basic_...
#include <tuple>       // for tuple
#include <vector>      // for vector

#include <glm/common.hpp>        // for clamp, max, min
#include <glm/gtc/type_ptr.hpp>  // for value_ptr
#include <glm/mat2x2.hpp>        // for operator+
#include <glm/mat3x3.hpp>        // for operator+
#include <glm/mat4x4.hpp>        // for operator+
#include <glm/vec2.hpp>          // for operator!=
#include <glm/vec3.hpp>          // for operator+, ope...
#include <glm/vec4.hpp>          // for operator+, ope...

namespace inviwo {
class InviwoApplication;

namespace animation {
class DemoController;
class MainAnimation;
}  // namespace animation

namespace {

template <typename PropertyType,
          typename Keyframe = animation::ValueKeyframe<typename PropertyType::value_type>,
          typename KeyframeSeq = animation::KeyframeSequenceTyped<Keyframe>>
auto propTrackRegHelper(AnimationModule& am) {
    using namespace animation;
    // Register PropertyTrack and the KeyFrame it should use
    am.registerTrack<PropertyTrack<PropertyType, Keyframe, KeyframeSeq>>();
    am.registerPropertyTrackConnection(
        PropertyTraits<PropertyType>::classIdentifier(),
        PropertyTrack<PropertyType, Keyframe, KeyframeSeq>::classIdentifier());
}

template <typename PropertyType, typename Interpolation>
auto interpolationRegHelper(AnimationModule& am) {
    using namespace animation;
    // No need to add existing interpolation method. Will produce a warning if adding a duplicate
    if (!am.getAnimationManager().getInterpolationFactory().hasKey(
            Interpolation::classIdentifier())) {
        am.registerInterpolation<Interpolation>();
    }
}

struct OrdinalReghelper {
    template <typename T>
    auto operator()(AnimationModule& am) {
        using namespace animation;
        using PropertyType = OrdinalProperty<T>;
        using ValueType = typename PropertyType::value_type;
        propTrackRegHelper<PropertyType>(am);
        interpolationRegHelper<PropertyType, LinearInterpolation<ValueKeyframe<ValueType>>>(am);
        interpolationRegHelper<PropertyType, ConstantInterpolation<ValueKeyframe<ValueType>>>(am);
        using PropertyRefType = OrdinalRefProperty<T>;
        propTrackRegHelper<PropertyRefType>(am);
        interpolationRegHelper<PropertyRefType, LinearInterpolation<ValueKeyframe<ValueType>>>(am);
        interpolationRegHelper<PropertyRefType, ConstantInterpolation<ValueKeyframe<ValueType>>>(
            am);
    }
};

struct MinMaxReghelper {
    template <typename T>
    auto operator()(AnimationModule& am) {
        using namespace animation;
        using PropertyType = MinMaxProperty<T>;
        using ValueType = typename PropertyType::value_type;
        propTrackRegHelper<PropertyType>(am);
        interpolationRegHelper<PropertyType, LinearInterpolation<ValueKeyframe<ValueType>>>(am);
        interpolationRegHelper<PropertyType, ConstantInterpolation<ValueKeyframe<ValueType>>>(am);
    }
};

struct OptionReghelper {
    template <typename T>
    auto operator()(AnimationModule& am) {
        using namespace animation;
        using PropertyType = OptionProperty<T>;
        using ValueType = typename PropertyType::value_type;
        propTrackRegHelper<PropertyType>(am);
        interpolationRegHelper<PropertyType, ConstantInterpolation<ValueKeyframe<ValueType>>>(am);
    }
};

struct ConstantInterpolationReghelper {
    template <typename PropertyType>
    auto operator()(AnimationModule& am) {
        using namespace animation;
        propTrackRegHelper<PropertyType>(am);
        using ValueType = typename PropertyType::value_type;
        interpolationRegHelper<PropertyType, ConstantInterpolation<ValueKeyframe<ValueType>>>(am);
    }
};

}  // namespace

AnimationModule::AnimationModule(InviwoApplication* app)
    : InviwoModule(app, "Animation")
    , animation::AnimationSupplier(manager_)
    , manager_(app)
    , animations_(app, manager_, *this)
    , demoController_(app) {

    using namespace animation;

    // Register Ordinal properties
    using Types = std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4,
                             dmat2, dmat3, dmat4, int, ivec2, ivec3, ivec4, unsigned int, uvec2,
                             uvec3, uvec4, size_t, size2_t, size3_t, size4_t>;
    util::for_each_type<Types>{}(OrdinalReghelper{}, *this);

    // Register MinMaxProperties
    using ScalarTypes = std::tuple<float, double, int, unsigned int, size_t>;
    util::for_each_type<ScalarTypes>{}(MinMaxReghelper{}, *this);

    // Register properties that should not interpolate
    // Todo: Add MultiFileProperty when we can deal with vector<T> data in animation
    util::for_each_type<std::tuple<BoolProperty, FileProperty, StringProperty>>{}(
        ConstantInterpolationReghelper{}, *this);
    util::for_each_type<ScalarTypes>{}(OptionReghelper{}, *this);
    util::for_each_type<std::tuple<std::string>>{}(OptionReghelper{}, *this);

    // Camera property
    propTrackRegHelper<CameraProperty, CameraKeyframe>(*this);
    interpolationRegHelper<CameraProperty, CameraSphericalInterpolation>(*this);
    interpolationRegHelper<CameraProperty, CameraLinearInterpolation>(*this);

    propTrackRegHelper<ButtonProperty, ButtonKeyframe, ButtonKeyframeSequence>(*this);

    // Todo: Add support for TransferFunctionProperty (special interpolation)

    registerTrack<CallbackTrack>();
    registerTrack<ControlTrack>();
}

AnimationModule::~AnimationModule() {
    // need to call that here since we will have to delete the manager before we call the destructor
    // of AnimationSupplier. Other modules that implement AnimationSupplier will not have this
    // problem.
    unRegisterAll();
}

int AnimationModule::getVersion() const { return 3; }

std::unique_ptr<VersionConverter> AnimationModule::getConverter(int version) const {
    return std::make_unique<Converter>(version);
}

animation::WorkspaceAnimations& AnimationModule::getWorkspaceAnimations() { return animations_; }

const animation::WorkspaceAnimations& AnimationModule::getWorkspaceAnimations() const {
    return animations_;
}

animation::MainAnimation& AnimationModule::getMainAnimation() {
    return animations_.getMainAnimation();
}

const animation::MainAnimation& AnimationModule::getMainAnimation() const {
    return animations_.getMainAnimation();
}

animation::AnimationManager& AnimationModule::getAnimationManager() { return manager_; }

const animation::AnimationManager& AnimationModule::getAnimationManager() const { return manager_; }

animation::DemoController& AnimationModule::getDemoController() { return demoController_; }

const animation::DemoController& AnimationModule::getDemoController() const {
    return demoController_;
}

bool AnimationModule::Converter::convert(TxElement* root) {
    using namespace xml;
    std::vector<ElementMatcher> selector{{ElementMatcher{"Animation", {}}},
                                         {ElementMatcher{"tracks", {}}},
                                         {ElementMatcher{"track", {}}}};
    bool res = false;
    switch (version_) {
        case 0: {
            xml::visitMatchingNodes(root, selector, [&res](TxElement* n) {
                auto attr = n->GetAttribute("type");
                replaceInString(attr, "org.inviwo.animation.PropertyTrack.for. ",
                                "org.inviwo.animation.PropertyTrack.for.");

                n->SetAttribute("type", attr);
                res |= true;
            });
            [[fallthrough]];
        }
        case 1: {
            auto aelm = xml::getMatchingElements(root, "Animation");
            if (!aelm.empty()) {
                TxElement list("Animations");
                const auto& e = aelm.front();
                TxElement name("name");
                name.SetAttribute("content", "Animation 1");
                list.InsertEndChild(*e)->InsertEndChild(name);
                root->RemoveChild(e);

                root->InsertEndChild(list);
                res |= true;
            }
            [[fallthrough]];
        }
        case 2: {
            auto ac = xml::getMatchingElements(root, "AnimationController");
            if (!ac.empty()) {
                res |= xml::changeAttribute(ac.front(),
                                            {xml::Kind::property("org.inviwo.CompositeProperty"),
                                             xml::Kind::property("org.inviwo.DoubleProperty")},
                                            "type", "org.inviwo.DoubleProperty",
                                            "org.inviwo.DoubleRefProperty");
            }
            [[fallthrough]];
        }
        default:
            // No changes
            break;
    }
    return res;
}

}  // namespace inviwo
