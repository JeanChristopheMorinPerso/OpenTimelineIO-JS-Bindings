// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_OPTIONAL_H
#define JS_OPTIONAL_H

#include "nonstd/optional.hpp"
#include <emscripten/bind.h>

namespace emscripten { namespace internal {

template <typename T>
struct TypeID<nonstd::optional<T>>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <typename T>
struct TypeID<const nonstd::optional<T>>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <typename T>
struct TypeID<nonstd::optional<T>&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <typename T>
struct TypeID<const nonstd::optional<T>&>
{
    static constexpr TYPEID get() { return LightTypeID<val>::get(); }
};

template <typename T>
struct BindingType<nonstd::optional<T>>
{
    using ValBinding = BindingType<val>;
    using WireType   = ValBinding::WireType;

    // C++ > JS
    static WireType toWireType(nonstd::optional<T> const& opt)
    {
        if (opt.has_value())
        {
            return ValBinding::toWireType(val(opt.value()));
        }
        return ValBinding::toWireType(val::null());
    }

    // JS > C++
    static nonstd::optional<T> fromWireType(WireType value)
    {
        val convertedVal = ValBinding::fromWireType(value);
        if (convertedVal.isNull() || convertedVal.isUndefined())
        {
            return nonstd::nullopt;
        }
        return nonstd::optional<T>{ convertedVal.as<T>() };
    }
};

}} // namespace emscripten::internal

#endif // JS_OPTIONAL_H
