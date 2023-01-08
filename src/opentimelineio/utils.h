// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include <emscripten/val.h>
#include <opentimelineio/any.h>
#include <opentimelineio/anyDictionary.h>
#include <opentimelineio/anyVector.h>

namespace ems = emscripten;
using namespace opentimelineio::OPENTIMELINEIO_VERSION;

ems::val any_to_js(any const& a, bool top_level);

any js_to_any(ems::val const& item);

template <typename T>
T js_to_cpp(ems::val const& item);

AnyVector js_array_to_cpp(ems::val const& item);

AnyDictionary js_map_to_cpp(ems::val const& item);
