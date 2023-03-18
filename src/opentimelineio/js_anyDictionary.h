// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_ANYDICTIONARY_H
#define JS_ANYDICTIONARY_H

#include "any/any.hpp"
#include <emscripten/bind.h>
#include <iostream>
#include <opentimelineio/anyDictionary.h>
#include <string>
#include <utility>

#include "utils.h"

namespace emscripten { namespace internal {

template <>
struct TypeID<OTIO_NS::AnyDictionary>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct TypeID<const OTIO_NS::AnyDictionary>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct TypeID<OTIO_NS::AnyDictionary&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct TypeID<const OTIO_NS::AnyDictionary&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct BindingType<OTIO_NS::AnyDictionary>
{
    using ValBinding = BindingType<val>;
    using WireType   = ValBinding::WireType;

    // C++ > JS
    static WireType toWireType(const OTIO_NS::AnyDictionary& data)
    {
        std::cout
            << "Entering BindingType<OTIO_NS::AnyDictionary>::toWireType\n";
        val obj = val::object();
        for (std::pair<std::string, linb::any> element: data)
        {
            obj.set(element.first, any_to_js(element.second, true));
        }
        return ValBinding::toWireType(obj);
    }

    // JS > C++
    static OTIO_NS::AnyDictionary fromWireType(WireType value)
    {
        std::cout
            << "Entering BindingType<OTIO_NS::AnyDictionary>::fromWireType\n";
        return js_map_to_cpp(ValBinding::fromWireType(value));
    }
};
}} // namespace emscripten::internal

#endif // JS_ANYDICTIONARY_H
