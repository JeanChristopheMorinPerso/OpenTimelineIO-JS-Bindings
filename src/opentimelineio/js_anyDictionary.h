// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_ANYDICTIONARY_H
#define JS_ANYDICTIONARY_H

#include <emscripten/bind.h>
#include <iostream>
#include <opentimelineio/any.h>
#include <opentimelineio/anyDictionary.h>
#include <string>
#include <utility>

#include "utils.h"

using namespace opentimelineio::OPENTIMELINEIO_VERSION;

namespace emscripten { namespace internal {

template <>
struct TypeID<AnyDictionary>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct TypeID<const AnyDictionary>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct TypeID<AnyDictionary&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct TypeID<const AnyDictionary&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <>
struct BindingType<AnyDictionary>
{
    using ValBinding = BindingType<val>;
    using WireType   = ValBinding::WireType;

    // C++ > JS
    static WireType toWireType(const AnyDictionary& data)
    {
        std::cout << "Entering BindingType<AnyDictionary>::toWireType\n";
        val obj = val::object();
        for (std::pair<std::string, any> element: data)
        {
            obj.set(element.first, any_to_js(element.second, true));
        }
        return ValBinding::toWireType(obj);
    }

    // JS > C++
    static AnyDictionary fromWireType(WireType value)
    {
        std::cout << "Entering BindingType,AnyDictionary>::fromWireType\n";
        return js_map_to_cpp(ValBinding::fromWireType(value));
    }
};
}} // namespace emscripten::internal

#endif // JS_ANYDICTIONARY_H
