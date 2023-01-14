// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include <emscripten/bind.h>
#include <opentimelineio/any.h>
#include <opentimelineio/anyDictionary.h>

#include "utils.h"

using namespace opentimelineio::OPENTIMELINEIO_VERSION;

namespace emscripten { namespace internal {

template <>
struct BindingType<AnyDictionary>
{
    using ValBinding = BindingType<val>;
    using WireType   = ValBinding::WireType;

    // C++ > JS
    static WireType toWireType(const AnyDictionary& data)
    {
        val obj = val::object();
        for (std::pair<std::string, any> element: data)
        {
            obj.set(element.first, element.second);
        }
        return ValBinding::toWireType(obj);
    }

    // JS > C++
    static AnyDictionary fromWireType(WireType value)
    {
        return js_map_to_cpp(ValBinding::fromWireType(value));
    }
};
}} // namespace emscripten::internal
