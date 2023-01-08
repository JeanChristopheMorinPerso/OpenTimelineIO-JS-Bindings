// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project
#include "opentimelineio/any.h"
#include "opentimelineio/anyVector.h"
#include "opentimelineio/optional.h"
#include "opentimelineio/safely_typed_any.h"
#include <emscripten/val.h>

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
    auto                  e     = _js_cast_dispatch_table.find(&tInfo);

    if (e == _js_cast_dispatch_table.end())
    {
        auto backup_e = _js_cast_dispatch_table_by_name.find(tInfo.name());
        if (backup_e != _js_cast_dispatch_table_by_name.end())
        {
            _js_cast_dispatch_table[&tInfo] = backup_e->second;
            e = _js_cast_dispatch_table.find(&tInfo);
        }
    }

    if (e == _js_cast_dispatch_table.end())
    {
        // throw py::value_error(string_printf(
        //     "Unable to cast any of type %s to python object",
        //     type_name_for_error_message(tInfo).c_str()));
        throw "asd";
    }

    return e->second(a, top_level);
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
        return js_array_to_cpp(item);
    }

    if (item.typeOf().as<std::string>() == "object")
    {
        AnyDictionary d = AnyDictionary();
        // TODO: how to handle objects?
    }
    // TODO: Handle all other types.
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
js_map_to_cpp(ems::val const& item)
{}
