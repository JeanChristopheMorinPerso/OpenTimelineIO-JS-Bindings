// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_ANYVECTOR_H
#define JS_ANYVECTOR_H

#include <iostream>
#include <string>
#include <utility>

#include "any/any.hpp"
#include <emscripten/bind.h>
#include <opentimelineio/anyVector.h>

#include "utils.h"

namespace emscripten { namespace internal {

template <>
struct TypeID<OTIO_NS::AnyVector>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct TypeID<const OTIO_NS::AnyVector>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct TypeID<OTIO_NS::AnyVector&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct TypeID<const OTIO_NS::AnyVector&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct BindingType<OTIO_NS::AnyVector>
{
    using ValBinding = BindingType<val>;
    using WireType   = ValBinding::WireType;

    // C++ > JS
    static WireType toWireType(const OTIO_NS::AnyVector& data)
    {
        std::cout << "Entering BindingType<OTIO_NS::AnyVector>::toWireType\n";
        val obj = val::array();
        for (auto element: data)
        {
            obj.call<void>("push", any_to_js(element, true));
        }
        return ValBinding::toWireType(obj);
    }

    // JS > C++
    static OTIO_NS::AnyVector fromWireType(WireType value)
    {
        std::cout << "Entering BindingType<OTIO_NS::AnyVector>::fromWireType\n";
        return js_array_to_cpp(ValBinding::fromWireType(value));
    }
};

}} // namespace emscripten::internal

#endif // JS_ANYVECTOR_H
