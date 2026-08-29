// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_ANYDICTIONARY_H
#define JS_ANYDICTIONARY_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

#include "any/any.hpp"
#include <emscripten/bind.h>
#include <opentimelineio/anyDictionary.h>

#include "utils.h"

EMSCRIPTEN_DECLARE_VAL_TYPE(SchemaVersionMap);

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
    static WireType
    toWireType(const OTIO_NS::AnyDictionary& data, rvp::default_tag)
    {
        std::cout
            << "Entering BindingType<OTIO_NS::AnyDictionary>::toWireType\n";
        val obj = val::object();
        for (std::pair<std::string, linb::any> element: data)
        {
            obj.set(element.first, any_to_js(element.second, true));
        }
        return ValBinding::toWireType(obj, rvp::default_tag{});
    }

    // JS > C++
    static OTIO_NS::AnyDictionary fromWireType(WireType value)
    {
        std::cout
            << "Entering BindingType<OTIO_NS::AnyDictionary>::fromWireType\n";
        return js_map_to_cpp(ValBinding::fromWireType(value));
    }
};

// unordered_map
template <>
struct TypeID<std::unordered_map<std::string, int64_t>>
{
    static constexpr TYPEID get()
    {
        return LightTypeID<SchemaVersionMap>::get();
    }
};

template <>
struct TypeID<const std::unordered_map<std::string, int64_t>>
{
    static constexpr TYPEID get()
    {
        return LightTypeID<SchemaVersionMap>::get();
    }
};

template <>
struct TypeID<std::unordered_map<std::string, int64_t>&>
{
    static constexpr TYPEID get()
    {
        return LightTypeID<SchemaVersionMap>::get();
    }
};

template <>
struct TypeID<const std::unordered_map<std::string, int64_t>&>
{
    static constexpr TYPEID get()
    {
        return LightTypeID<SchemaVersionMap>::get();
    }
};

template <>
struct BindingType<std::unordered_map<std::string, int64_t>>
{
    using Map        = std::unordered_map<std::string, int64_t>;
    using ValBinding = BindingType<SchemaVersionMap>;
    using WireType   = ValBinding::WireType;

    // TypeID makes JavaScript use the val converter, so BindingType must use
    // the same wire representation. Otherwise the Emval handle is mistaken
    // for a Map pointer, which only appeared to work when low memory was zero.
    static WireType toWireType(Map const& data, rvp::default_tag policy)
    {
        SchemaVersionMap obj(val::object());
        for (auto const& [key, value]: data)
        {
            obj.set(key, static_cast<int>(value));
        }
        return ValBinding::toWireType(std::move(obj), policy);
    }

    static Map fromWireType(WireType value)
    {
        Map result;
        val entries = val::global("Object").call<val>(
            "entries",
            ValBinding::fromWireType(value));
        for (size_t i = 0; i < entries["length"].as<size_t>(); ++i)
        {
            result[entries[i][0].as<std::string>()] = entries[i][1].as<int>();
        }
        return result;
    }
};

}} // namespace emscripten::internal

#endif // JS_ANYDICTIONARY_H
