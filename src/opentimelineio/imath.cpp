// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project
#include <ImathBox.h>
#include <ImathVec.h>
#include <cstdio>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <memory>
#include <string>

#include "exceptions.h"

namespace ems = emscripten;

template <typename... Args>
std::string
string_printf(char const* format, Args... args)
{
    char   buffer[4096];
    size_t size = std::snprintf(buffer, sizeof(buffer), format, args...) + 1;
    if (size < sizeof(buffer))
    {
        return std::string(buffer);
    }

    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format, args...);
    return std::string(buf.get());
}

template <typename CLASS>
CLASS
_type_checked(ems::val const& rhs, char const* op)
{
    try
    {
        return rhs.as<CLASS>();
    }
    catch (...)
    {
        throw TypeError(string_printf(
            "Unsupported operand type(s) for %s: "
            "%s and %s",
            typeid(CLASS).name(),
            op,
            rhs.typeof().as<std::string>().c_str()));
    }
}

EMSCRIPTEN_BINDINGS(imath)
{
    ems::class_<Imath::V2d>("V2d")
        .constructor<>()
        .constructor<double>()
        .constructor<double, double>()
        // TODO: How to bind this?
        // .property(
        //     "x",
        //     ems::optional_override([](Imath::V2d& v2d) { return v2d.x; }),
        //     ems::optional_override(
        //         [](Imath::V2d& v2d, double value) { return v2d.x = value; }));
        .function(
            "get",
            ems::optional_override(
                [](Imath::V2d const& v, size_t i) { return v[i]; }))
        .function(
            "equal",
            ems::optional_override(
                [](Imath::V2d const& lhs, ems::val const& rhs) {
                    return lhs == _type_checked<Imath::V2d>(rhs, "==");
                }))
        .function(
            "notEqual",
            ems::optional_override(
                [](Imath::V2d const& lhs, ems::val const& rhs) {
                    return lhs != _type_checked<Imath::V2d>(rhs, "!=");
                }))
        .function(
            "xor",
            ems::optional_override(
                [](Imath::V2d const& lhs, ems::val const& rhs) {
                    return lhs ^ _type_checked<Imath::V2d>(rhs, "^");
                }))
        .function(
            "mod",
            ems::optional_override(
                [](Imath::V2d const& lhs, ems::val const& rhs) {
                    return lhs % _type_checked<Imath::V2d>(rhs, "%");
                }))
        .function(
            "sub",
            ems::optional_override(
                [](Imath::V2d const& lhs, ems::val const& rhs) {
                    return lhs - _type_checked<Imath::V2d>(rhs, "-");
                }))
        .function(
            "add",
            ems::optional_override(
                [](Imath::V2d const& lhs, ems::val const& rhs) {
                    return lhs + _type_checked<Imath::V2d>(rhs, "+");
                }))
        .function(
            "mult",
            ems::optional_override(
                [](Imath::V2d const& lhs, ems::val const& rhs) {
                    return lhs * _type_checked<Imath::V2d>(rhs, "*");
                }))
        .function(
            "div",
            ems::optional_override(
                [](Imath::V2d const& lhs, ems::val const& rhs) {
                    return lhs / _type_checked<Imath::V2d>(rhs, "/");
                }))
        .function(
            "equalWithAbsError",
            ems::optional_override(
                [](Imath::V2d* v1, Imath::V2d const& v2, double e) {
                    return v1->equalWithAbsError(v2, e);
                }),
            ems::allow_raw_pointers())
        .function(
            "equalWithRelError",
            ems::optional_override(
                [](Imath::V2d* v1, Imath::V2d const& v2, double e) {
                    return v1->equalWithRelError(v2, e);
                }),
            ems::allow_raw_pointers())
        .function(
            "dot",
            ems::optional_override([](Imath::V2d* v1, Imath::V2d const& v2) {
                return v1->dot(v2);
            }),
            ems::allow_raw_pointers())
        .function(
            "cross",
            ems::optional_override([](Imath::V2d* v1, Imath::V2d const& v2) {
                return v1->cross(v2);
            }),
            ems::allow_raw_pointers())
        .function("length", &Imath::V2d::length)
        .function("length2", &Imath::V2d::length2)
        .function("normalize", &Imath::V2d::normalize)
        .function("normalizeExc", &Imath::V2d::normalizeExc)
        .function("normalizeNonNull", &Imath::V2d::normalizeNonNull)
        .function("normalized", &Imath::V2d::normalized)
        .function("normalizedExc", &Imath::V2d::normalizedExc)
        .function("normalizedNonNull", &Imath::V2d::normalizedNonNull)
        // TODO: Not sure if this value is valid in JS...
        .class_function("baseTypeLowest", &Imath::V2d::baseTypeLowest)
        .class_function("baseTypeMax", &Imath::V2d::baseTypeMax)
        .class_function("baseTypeEpsilon", &Imath::V2d::baseTypeEpsilon)
        .class_function("dimensions", &Imath::V2d::dimensions);

    ems::class_<Imath::Box2d>("Box2d")
        .constructor<>()
        .constructor<Imath::V2d>()
        .constructor<Imath::V2d, Imath::V2d>()
        .function(
            "equal",
            ems::optional_override([](Imath::Box2d lhs, ems::val const& rhs) {
                return lhs == _type_checked<Imath::Box2d>(rhs, "==");
            }))
        .function(
            "notEqual",
            ems::optional_override([](Imath::Box2d lhs, ems::val const& rhs) {
                return lhs != _type_checked<Imath::Box2d>(rhs, "!=");
            }))
        .function("center", &Imath::Box2d::center)
        .function(
            "extendBy",
            ems::select_overload<void(Imath::V2d const&)>(
                &Imath::Box2d::extendBy))
        .function(
            "extendBy",
            ems::select_overload<void(Imath::Box2d const&)>(
                &Imath::Box2d::extendBy))
        .function(
            "intersects",
            ems::optional_override(
                [](Imath::Box2d* box, Imath::V2d const& point) {
                    return box->intersects(point);
                }),
            ems::allow_raw_pointers())
        .function(
            "intersects",
            ems::optional_override(
                [](Imath::Box2d* box, Imath::Box2d const& rhs) {
                    return box->intersects(rhs);
                }),
            ems::allow_raw_pointers());
}
