// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project
#include <utility>

#include "any/any.hpp"
#include <emscripten/bind.h>
#include <opentimelineio/any.h>
#include <opentimelineio/anyVector.h>
#include <opentimelineio/safely_typed_any.h>

namespace ems = emscripten;

EMSCRIPTEN_BINDINGS(opentimelineio_anyVector)
{

    // ems::class_<OTIO_NS::AnyVector>("AnyVector")
    //     .constructor()
    //     .function(
    //         "push_back",
    //         ems::optional_override([](OTIO_NS::AnyVector& a, ems::val item) {
    //             std::string valueType = item.typeof().as<std::string>();
    //             if (valueType == "string")
    //             {
    //                 a.push_back(linb::any(item.as<std::string>()));
    //             }
    //             a.push_back(linb::any(item));
    //             return true;
    //         }))
    //     .function(
    //         "get",
    //         ems::optional_override([](OTIO_NS::AnyVector& a, int index) {
    //             if (std::cmp_less(index, a.size()))
    //             {
    //                 return ems::val(OTIO_NS::safely_cast_string_any(a[index]));
    //             }
    //             return ems::val::undefined();
    //         }))
    //     .function(
    //         "set",
    //         ems::optional_override(
    //             [](OTIO_NS::AnyVector& a, int index, ems::val value) {
    //                 a[index] = linb::any(value);
    //                 return true;
    //             }));

    // .function("resize", &AnyVector::resize)
    // .function("size", &AnyVector::size);
    // .function("get", &internal::VectorAccess<VecType>::get)
    // .function("set", &internal::VectorAccess<VecType>::set);
}
