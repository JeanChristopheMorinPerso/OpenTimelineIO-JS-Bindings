// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include <emscripten/bind.h>
#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>
#include <opentime/timeTransform.h>
#include <opentimelineio/serializableObject.h>

#include "js_any.h"

namespace ems = emscripten;

EMSCRIPTEN_BINDINGS(js_any)
{
    ems::class_<JSAnyRationalTime>("JSAnyRationalTime")
        .constructor<opentime::RationalTime&>();

    ems::class_<JSAnyTimeRange>("JSAnyTimeRange")
        .constructor<opentime::TimeRange&>();

    ems::class_<JSAnyTimeTransform>("JSAnyTimeTransform")
        .constructor<opentime::TimeTransform&>();

    ems::class_<JSAnySerializableObject>("JSAnySerializableObject")
        .constructor<OTIO_NS::SerializableObject*>();
};
