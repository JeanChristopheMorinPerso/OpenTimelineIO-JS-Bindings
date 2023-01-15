// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

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
#include "utils.h"

namespace ems = emscripten;
using namespace opentimelineio::OPENTIMELINEIO_VERSION;

static std::
    map<std::type_info const*, std::function<ems::val(any const&, bool)>>
        _js_cast_dispatch_table;
static std::map<std::string, std::function<ems::val(any const&, bool)>>
    _js_cast_dispatch_table_by_name;

void
_build_any_to_js_dispatch_table()
{
    auto& t = _js_cast_dispatch_table;

    t[&typeid(void)] = [](any const& /* a */, bool) {
        return ems::val::null();
    };
    t[&typeid(bool)] = [](any const& a, bool) {
        return ems::val(safely_cast_bool_any(a));
    };
    t[&typeid(int)] = [](any const& a, bool) {
        return ems::val(safely_cast_int_any(a));
    };
    t[&typeid(int64_t)] = [](any const& a, bool) {
        return ems::val(safely_cast_int64_any(a));
    };
    t[&typeid(uint64_t)] = [](any const& a, bool) {
        return ems::val(safely_cast_uint64_any(a));
    };
    t[&typeid(double)] = [](any const& a, bool) {
        return ems::val(safely_cast_double_any(a));
    };
    t[&typeid(std::string)] = [](any const& a, bool) {
        return ems::val(safely_cast_string_any(a));
    };
    t[&typeid(RationalTime)] = [](any const& a, bool) {
        return ems::val(safely_cast_rational_time_any(a));
    };
    t[&typeid(TimeRange)] = [](any const& a, bool) {
        return ems::val(safely_cast_time_range_any(a));
    };
    t[&typeid(TimeTransform)] = [](any const& a, bool) {
        return ems::val(safely_cast_time_transform_any(a));
    };
    // t[&typeid(SerializableObject::Retainer<>)] = [](any const& a, bool) {
    //     SerializableObject* so = safely_cast_retainer_any(a);
    //     return ems::val(so);
    // };
    // t[&typeid(AnyDictionary*)] = [](any const& a, bool) {
    //     return ems::val(any_cast<AnyDictionaryProxy*>(a));
    // };
    // t[&typeid(AnyVector*)] = [](any const& a, bool) {
    //     return ems::val(any_cast<AnyVectorProxy*>(a));
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
any_to_js(any const& a, bool top_level)
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
        return ems::val(safely_cast_bool_any(a));
    }
    else if (tInfo == typeid(int))
    {
        return ems::val(safely_cast_int_any(a));
    }
    else if (tInfo == typeid(int64_t))
    {
        return ems::val(safely_cast_int64_any(a));
    }
    else if (tInfo == typeid(int64_t))
    {
        return ems::val(safely_cast_uint64_any(a));
    }
    else if (tInfo == typeid(double))
    {
        return ems::val(safely_cast_double_any(a));
    }
    else if (tInfo == typeid(std::string))
    {
        return ems::val(safely_cast_string_any(a));
    }
    else if (tInfo == typeid(RationalTime))
    {
        return ems::val(safely_cast_rational_time_any(a));
    }
    else if (tInfo == typeid(TimeRange))
    {
        return ems::val(safely_cast_time_range_any(a));
    }
    else if (tInfo == typeid(TimeTransform))
    {
        return ems::val(safely_cast_time_transform_any(a));
    }
    else if (tInfo == typeid(SerializableObject::Retainer<>))
    {
        SerializableObject* so = safely_cast_retainer_any(a);
        return ems::val(so);
    }

    throw ValueError(string_printf(
        "Unable to cast any of type '%s' to JS object",
        type_name_for_error_message(tInfo).c_str()));
}

any
js_to_any(ems::val const& item)
{
    if (item.isNull() || item.isUndefined())
    {
        return any(nullptr);
    }

    if (item.isFalse() || item.isTrue())
    {
        return any(js_to_cpp<bool>(item));
    }

    if (item.isNumber())
    {
        // TODO: How to handle other types of ints? Javascript only has Number...
        // Also, handle floats, double (?).
        return any(js_to_cpp<int32_t>(item));
    }

    if (item.isString())
    {
        return any(js_to_cpp<std::string>(item));
    }

    if (item.isArray())
    {
        return any(js_array_to_cpp(item));
    }

    if (item.typeOf().as<std::string>() == "object")
    {
        return any(js_map_to_cpp(item));
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

AnyVector
js_array_to_cpp(ems::val const& item)
{
    AnyVector av = AnyVector();
    for (auto& it: ems::vecFromJSArray<ems::val>(item))
    {
        av.push_back(js_to_any(it));
    }
    return av;
}

AnyDictionary
js_map_to_cpp(ems::val const& m)
{
    ems::val keys   = ems::val::global("Object").call<ems::val>("entries", m);
    size_t   length = keys["length"].as<size_t>();

    AnyDictionary d = AnyDictionary();

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
