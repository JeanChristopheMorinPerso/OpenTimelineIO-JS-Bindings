// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_OPTIONAL_H
#define JS_OPTIONAL_H

#include <optional>

#include <emscripten/bind.h>
#include <emscripten/val.h>

namespace emscripten { namespace internal {

template <typename T>
struct TypeID<std::optional<T>>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <typename T>
struct TypeID<const std::optional<T>>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <typename T>
struct TypeID<std::optional<T>&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <typename T>
struct TypeID<std::optional<T>&&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <typename T>
struct TypeID<const std::optional<T>&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <typename T>
struct BindingType<std::optional<T>>
{
    using ValBinding = BindingType<val>;
    using WireType   = ValBinding::WireType;

    // C++ > JS
    static WireType toWireType(std::optional<T> const& opt)
    {
        if (opt.has_value())
        {
            return ValBinding::toWireType(val(opt.value()));
        }
        return ValBinding::toWireType(val::null());
    }

    // JS > C++
    static std::optional<T> fromWireType(WireType value)
    {
        val convertedVal = ValBinding::fromWireType(value);
        if (convertedVal.isNull() || convertedVal.isUndefined())
        {
            return std::nullopt;
        }
        return std::make_optional<T>(convertedVal.as<T>());
    }
};

}} // namespace emscripten::internal

#endif // JS_OPTIONAL_H
