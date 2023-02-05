// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include "any/any.hpp"
#include "opentime/rationalTime.h"
#include "opentime/timeRange.h"
#include "opentime/timeTransform.h"
#include "opentimelineio/any.h"
#include "opentimelineio/anyVector.h"
#include "opentimelineio/optional.h"
#include "opentimelineio/safely_typed_any.h"
#include "opentimelineio/stringUtils.h"
#include <cstddef>
#include <emscripten.h>
#include <emscripten/val.h>
#include <functional>
#include <typeinfo>

#include "exceptions.h"
#include "js_anyDictionary.h" // Needed to support ems::val(AnyDictionary)
#include "utils.h"

namespace ems = emscripten;

static std::
    map<std::type_info const*, std::function<ems::val(linb::any const&, bool)>>
        _js_cast_dispatch_table;
static std::map<std::string, std::function<ems::val(linb::any const&, bool)>>
    _js_cast_dispatch_table_by_name;

void
_build_any_to_js_dispatch_table()
{
    auto& t = _js_cast_dispatch_table;

    t[&typeid(void)] = [](linb::any const& /* a */, bool) {
        return ems::val::null();
    };
    t[&typeid(bool)] = [](linb::any const& a, bool) {
        return ems::val(OTIO_NS::safely_cast_bool_any(a));
    };
    t[&typeid(int)] = [](linb::any const& a, bool) {
        return ems::val(OTIO_NS::safely_cast_int_any(a));
    };
    t[&typeid(int64_t)] = [](linb::any const& a, bool) {
        return ems::val(OTIO_NS::safely_cast_int64_any(a));
    };
    t[&typeid(uint64_t)] = [](linb::any const& a, bool) {
        return ems::val(OTIO_NS::safely_cast_uint64_any(a));
    };
    t[&typeid(double)] = [](linb::any const& a, bool) {
        return ems::val(OTIO_NS::safely_cast_double_any(a));
    };
    t[&typeid(std::string)] = [](linb::any const& a, bool) {
        return ems::val(OTIO_NS::safely_cast_string_any(a));
    };
    t[&typeid(OTIO_NS::RationalTime)] = [](linb::any const& a, bool) {
        return ems::val(OTIO_NS::safely_cast_rational_time_any(a));
    };
    t[&typeid(OTIO_NS::TimeRange)] = [](linb::any const& a, bool) {
        return ems::val(OTIO_NS::safely_cast_time_range_any(a));
    };
    t[&typeid(OTIO_NS::TimeTransform)] = [](linb::any const& a, bool) {
        return ems::val(OTIO_NS::safely_cast_time_transform_any(a));
    };
    // t[&typeid(SerializableObject::Retainer<>)] = [](any const& a, bool) {
    //     SerializableObject* so = safely_cast_retainer_any(a);
    //     return ems::val(so);
    // };
    // t[&typeid(AnyDictionary*)] = [](any const& a, bool) {
    //     return ems::val(any_cast<AnyDictionary*>(a));
    // };
    // t[&typeid(AnyVector*)] = [](any const& a, bool) {
    //     return ems::val(any_cast<AnyVector*>(a));
    // };

    // t[&typeid(AnyDictionary)] = [](any const& a, bool top_level) {
    //     AnyDictionary& d = temp_safely_cast_any_dictionary_any(a);
    //     if (top_level)
    //     {
    //         auto proxy = new AnyDictionaryProxy;
    //         proxy->fetch_any_dictionary().swap(d);
    //         return py::cast(proxy);
    //     }
    //     else
    //     {
    //         return py::cast(
    //             (AnyDictionaryProxy*) d.get_or_create_mutation_stamp());
    //     }
    // };

    // t[&typeid(AnyVector)] = [](any const& a, bool top_level) {
    //     AnyVector& v = temp_safely_cast_any_vector_any(a);
    //     if (top_level)
    //     {
    //         auto proxy = new AnyVectorProxy;
    //         proxy->fetch_any_vector().swap(v);
    //         return py::cast(proxy);
    //     }
    //     return py::cast((AnyVectorProxy*) v.get_or_create_mutation_stamp());
    // };

    for (auto e: t)
    {
        _js_cast_dispatch_table_by_name[e.first->name()] = e.second;
    }
}

ems::val
any_to_js(linb::any const& a, bool top_level)
{
    std::type_info const& tInfo = a.type();

    // TODO: Figure out why the table raises a "memory access out of bounds" error.
    // Wild guess: it has to de defined in a header like we had to do for AnyDictionary?
    // auto                  e     = _js_cast_dispatch_table.find(&tInfo);

    if (tInfo == typeid(void))
    {
        return ems::val::null();
    }
    else if (tInfo == typeid(bool))
    {
        return ems::val(OTIO_NS::safely_cast_bool_any(a));
    }
    else if (tInfo == typeid(int))
    {
        return ems::val(OTIO_NS::safely_cast_int_any(a));
    }
    else if (tInfo == typeid(int64_t))
    {
        return ems::val(OTIO_NS::safely_cast_int64_any(a));
    }
    else if (tInfo == typeid(int64_t))
    {
        return ems::val(OTIO_NS::safely_cast_uint64_any(a));
    }
    else if (tInfo == typeid(double))
    {
        return ems::val(OTIO_NS::safely_cast_double_any(a));
    }
    else if (tInfo == typeid(std::string))
    {
        return ems::val(OTIO_NS::safely_cast_string_any(a));
    }
    else if (tInfo == typeid(OTIO_NS::RationalTime))
    {
        return ems::val(OTIO_NS::safely_cast_rational_time_any(a));
    }
    else if (tInfo == typeid(OTIO_NS::TimeRange))
    {
        return ems::val(OTIO_NS::safely_cast_time_range_any(a));
    }
    else if (tInfo == typeid(OTIO_NS::TimeTransform))
    {
        return ems::val(OTIO_NS::safely_cast_time_transform_any(a));
    }
    else if (tInfo == typeid(OTIO_NS::AnyDictionary*))
    {
        OTIO_NS::AnyDictionary* d = linb::any_cast<OTIO_NS::AnyDictionary*>(a);
        return ems::val(d);
    }
    else if (tInfo == typeid(OTIO_NS::SerializableObject::Retainer<>))
    {
        OTIO_NS::SerializableObject* so = OTIO_NS::safely_cast_retainer_any(a);
        return ems::val(so);
    }
    else if (tInfo == typeid(OTIO_NS::AnyDictionary))
    {
        OTIO_NS::AnyDictionary& d =
            OTIO_NS::temp_safely_cast_any_dictionary_any(a);
        return ems::val(d);
    }

    throw ValueError(string_printf(
        "Unable to cast any of type '%s' to JS object",
        OTIO_NS::type_name_for_error_message(tInfo).c_str()));
}

// https://emscripten.org/docs/api_reference/emscripten.h.html?highlight=em_js#c.EM_JS
// clang-format off
// Get the real type of a JS value. "typeof" returns object in most cases,
// which is useless. Be careful though as the return value has to be freed manually!
EM_JS(char*, get_real_js_type, (ems::EM_VAL handle), {
    var value = Emval.toValue(handle);
    var name = value.constructor.name;

    var lengthBytes = lengthBytesUTF8(name)+1;
    var stringOnWasmHeap = _malloc(lengthBytes);
    stringToUTF8(name, stringOnWasmHeap, lengthBytes);
    return stringOnWasmHeap;
});
// clang-format on

linb::any
js_to_any(ems::val const& item)
{
    std::string typ = item.typeof().as<std::string>();

    if (item.isNull() || item.isUndefined())
    {
        return linb::any(nullptr);
    }

    if (item.isFalse() || item.isTrue())
    {
        return linb::any(js_to_cpp<bool>(item));
    }

    if (item.isNumber())
    {
        // TODO: How to handle other types of ints? Javascript only has Number...
        // Also, handle floats, double (?).
        return linb::any(js_to_cpp<int32_t>(item));
    }

    if (item.isString())
    {
        return linb::any(js_to_cpp<std::string>(item));
    }

    if (item.isArray())
    {
        return linb::any(js_array_to_cpp(item));
    }

    char*       rawType = get_real_js_type(item.as_handle());
    std::string jsType  = std::string(rawType);
    free(rawType);

    if (jsType == "RationalTime")
    {
        OTIO_NS::RationalTime rt = item.as<OTIO_NS::RationalTime>();
        return linb::any(rt);
    }

    if (jsType == "TimeRange")
    {
        OTIO_NS::TimeRange tr = item.as<OTIO_NS::TimeRange>();
        return linb::any(tr);
    }

    if (jsType == "TimeTransform")
    {
        OTIO_NS::TimeTransform tt = item.as<OTIO_NS::TimeTransform>();
        return linb::any(tt);
    }

    if (item.typeOf().as<std::string>() == "object")
    {
        return linb::any(js_map_to_cpp(item));
    }

    throw TypeError(
        "Unsupported value type: " + item.typeof().as<std::string>());
}

template <typename T>
T
js_to_cpp(ems::val const& item)
{
    return item.as<T>();
};

OTIO_NS::AnyVector
js_array_to_cpp(ems::val const& item)
{
    OTIO_NS::AnyVector av = OTIO_NS::AnyVector();
    for (auto& it: ems::vecFromJSArray<ems::val>(item))
    {
        av.push_back(js_to_any(it));
    }
    return av;
}

OTIO_NS::AnyDictionary
js_map_to_cpp(ems::val const& m)
{
    ems::val keys   = ems::val::global("Object").call<ems::val>("entries", m);
    size_t   length = keys["length"].as<size_t>();

    OTIO_NS::AnyDictionary d = OTIO_NS::AnyDictionary();

    for (size_t i = 0; i < length; ++i)
    {
        if (!keys[i][0].isString())
        {

            throw ValueError(
                "Keys must be of type string, not "
                + m[i][0].typeof().as<std::string>());
        }

        d[keys[i][0].as<std::string>()] = js_to_any(keys[i][1]);
    }

    return d;
}

struct KeepaliveMonitor
{
    OTIO_NS::SerializableObject* _so;
    ems::val                     _keep_alive;

    KeepaliveMonitor(OTIO_NS::SerializableObject* so)
        : _so(so)
    {
        // printf("Constructing KeepaliveMonitor for %s\n", typeid(*so).name());
    }

    void monitor()
    {
        // printf("KeepaliveMonitor::monitor\n");
        if (_so->current_ref_count() > 1)
        {
            // printf("KeepaliveMonitor::monitor: current_ref_count > 1\n");
            if (!_keep_alive)
            {
                // printf(
                //     "KeepaliveMonitor::monitor: _keep_alive is empty, setting it\n");
                _keep_alive = ems::val(_so);
                // printf(
                //     "KeepaliveMonitor::monitor: Successfully set _keep_alive\n");
            }
        }
        else
        {
            // printf("KeepaliveMonitor::monitor: current_ref_count < 1\n");
            // Note that ems::val works with ! only. SO double negate to get the truthy value.
            if (!!_keep_alive)
            {
                // printf(
                //     "KeepaliveMonitor::monitor: _keep_alive is truthy, trying to set to to undefined (clearing)\n");
                _keep_alive =
                    ems::val::undefined(); // this could cause destruction
            }
        }
        // printf("KeepaliveMonitor::monitor: end\n");
    }
};

void
install_external_keepalive_monitor(
    OTIO_NS::SerializableObject* so,
    bool                         apply_now)
{
    KeepaliveMonitor m{ so };
    using namespace std::placeholders;
    // printf(
    //     "Install external keep alive for %p: apply now is %d\n",
    //     so,
    //     apply_now);
    so->install_external_keepalive_monitor(
        std::bind(&KeepaliveMonitor::monitor, m),
        apply_now);
}
