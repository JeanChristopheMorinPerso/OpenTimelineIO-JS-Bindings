// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#ifndef JS_ANY_H
#define JS_ANY_H

#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>
#include <opentime/timeTransform.h>
#include <opentimelineio/any.h>
#include <opentimelineio/safely_typed_any.h>
#include <opentimelineio/serializableObject.h>

struct JSAnyRationalTime
{
    JSAnyRationalTime() {}
    JSAnyRationalTime(opentime::RationalTime& value)
        : a(OTIO_NS::create_safely_typed_any(std::move(value)))
    {}

    linb::any a;
};

struct JSAnyTimeRange
{
    JSAnyTimeRange() {}
    JSAnyTimeRange(opentime::TimeRange& value)
        : a(OTIO_NS::create_safely_typed_any(std::move(value)))
    {}

    linb::any a;
};

struct JSAnyTimeTransform
{
    JSAnyTimeTransform() {}
    JSAnyTimeTransform(opentime::TimeTransform& value)
        : a(OTIO_NS::create_safely_typed_any(std::move(value)))
    {}

    linb::any a;
};

struct JSAnySerializableObject
{
    JSAnySerializableObject() {}
    JSAnySerializableObject(OTIO_NS::SerializableObject* value)
        : a(OTIO_NS::create_safely_typed_any(value))
    {}

    linb::any a;
};

#endif
