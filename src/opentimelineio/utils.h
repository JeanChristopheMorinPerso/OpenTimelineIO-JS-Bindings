// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_UTILS_H
#define JS_UTILS_H

#include <emscripten/val.h>
#include <opentimelineio/any.h>
#include <opentimelineio/anyDictionary.h>
#include <opentimelineio/anyVector.h>

namespace ems = emscripten;
using namespace opentimelineio::OPENTIMELINEIO_VERSION;

void _build_any_to_js_dispatch_table();

ems::val any_to_js(any const& a, bool top_level);

any js_to_any(ems::val const& item);

template <typename T>
T js_to_cpp(ems::val const& item);

AnyVector js_array_to_cpp(ems::val const& item);

AnyDictionary js_map_to_cpp(ems::val const& item);

/**
 * Macro to register the destructor of TYPE. Emscripten needs to have public
 * destructors which are made available from JS, but OTIO's destructors are private.
 * @param TYPE Type name without namespace.
*/
#define REGISTER_DESTRUCTOR(TYPE)                                              \
    namespace emscripten { namespace internal {                                \
    template <>                                                                \
    void raw_destructor<TYPE>(TYPE * ptr)                                      \
    {                                                                          \
        ptr->possibly_delete();                                                \
    }                                                                          \
    }                                                                          \
    } // namespace emscripten::internal

/**
 * Macro that wraps EMSCRIPTEN_WRAPPER to register a wrapper for subclasses.
 * @param TYPE Type name without a namespace.
 * @param NAME Name to use for the wrapper.
*/
#define REGISTER_WRAPPER(TYPE, NAME)                                           \
    struct NAME : public ems::wrapper<TYPE>                                    \
    {                                                                          \
        EMSCRIPTEN_WRAPPER(NAME);                                              \
    };

#endif // JS_UTILS_H
