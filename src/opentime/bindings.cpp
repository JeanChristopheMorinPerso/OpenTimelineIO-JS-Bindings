// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include "opentime/rationalTime.h"
#include "opentime/timeRange.h"
#include "opentime/timeTransform.h"
#include <emscripten/bind.h>

namespace ems = emscripten;
using namespace opentime;

EMSCRIPTEN_BINDINGS(opentime)
{

    ems::class_<RationalTime>("RationalTime")
        .constructor()
        .constructor<double>()
        .constructor<double, double>()
        .function("is_invalid_time", &RationalTime::is_invalid_time)
        .property("value", &RationalTime::value)
        .property("rate", &RationalTime::rate)
        .function(
            "rescaled_to",
            ems::select_overload<RationalTime(double) const>(
                &RationalTime::rescaled_to))
        .function(
            "rescaled_to",
            ems::select_overload<RationalTime(RationalTime) const>(
                &RationalTime::rescaled_to))
        .function(
            "value_rescaled_to",
            ems::select_overload<double(double) const>(
                &RationalTime::value_rescaled_to))
        .function(
            "value_rescaled_to",
            ems::select_overload<double(RationalTime) const>(
                &RationalTime::value_rescaled_to))
        .function("almost_equal", &RationalTime::almost_equal)
        .class_function(
            "duration_from_start_end_time",
            &RationalTime::duration_from_start_end_time)
        .class_function(
            "duration_from_start_end_time_inclusive",
            &RationalTime::duration_from_start_end_time_inclusive)
        .class_function(
            "is_valid_timecode_rate",
            &RationalTime::is_valid_timecode_rate)
        .class_function(
            "nearest_valid_timecode_rate",
            &RationalTime::nearest_valid_timecode_rate)
        .class_function("from_frames", &RationalTime::from_frames)
        // TODO: Variable arguments
        .class_function(
            "from_seconds",
            ems::select_overload<RationalTime(double, double)>(
                &RationalTime::from_seconds))
        .class_function(
            "from_seconds",
            ems::select_overload<RationalTime(double)>(
                &RationalTime::from_seconds))
        // TODO: Variable arguments
        .function(
            "to_frames",
            ems::select_overload<int(void) const>(&RationalTime::to_frames))
        .function(
            "to_frames",
            ems::select_overload<int(double) const>(&RationalTime::to_frames))
        .function("to_seconds", &RationalTime::to_seconds)
        // TODO: Optional parameters
        // TODO: How to handle the error argument?
        // .function(
        //     "to_timecode",
        //     ems::select_overload<std::string(void)>(&RationalTime::to_timecode))
        .function("to_time_string", &RationalTime::to_time_string)
        .function(
            "from_timecode",
            ems::optional_override(
                [](RationalTime& seld, std::string timecode, double rate) {
                    // TODO: How to handle the error?
                    return RationalTime::from_timecode(timecode, rate);
                }))
        .function(
            "from_time_string",
            ems::optional_override(
                [](RationalTime& seld, std::string timecode, double rate) {
                    // TODO: How to handle the error?
                    return RationalTime::from_time_string(timecode, rate);
                }));

    ems::class_<TimeRange>("TimeRange").constructor<RationalTime>();

    ems::class_<TimeTransform>("TimeTransform");
}
