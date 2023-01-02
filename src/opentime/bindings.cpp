// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include "functional"
#include "opentime/rationalTime.h"
#include "opentime/timeRange.h"
#include "opentime/timeTransform.h"
#include <cstdio>
#include <emscripten/bind.h>
#include <memory>
#include <string>

namespace ems = emscripten;
using namespace opentime;

template <typename... Args>
std::string
string_printf(char const* format, Args... args)
{
    char   buffer[4096];
    size_t size = snprintf(buffer, sizeof(buffer), format, args...) + 1;
    if (size < sizeof(buffer))
    {
        return std::string(buffer);
    }

    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format, args...);
    return std::string(buf.get());
}

std::string
opentime_js_string(RationalTime rt)
{
    return string_printf(
        "RationalTime(value=%g, rate=%g)",
        rt.value(),
        rt.rate());
}

EMSCRIPTEN_BINDINGS(opentime)
{

    ems::class_<RationalTime>("RationalTime")
        .constructor()
        .constructor<double>()
        .constructor<double, double>()
        .function("is_invalid_time", &RationalTime::is_invalid_time)
        .property("value", &RationalTime::value)
        .property("rate", &RationalTime::rate)
        .function("toString", &opentime_js_string)
        .function("valueOf", ems::optional_override([](RationalTime& rt) {
                      // This is what RationalTime class does internally when comparing
                      // rational times.
                      return rt.value() / rt.rate();
                  }))
        .function(
            "toPrimitive",
            ems::optional_override([](RationalTime& rt, std::string hint) {
                // This is what RationalTime class does internally when comparing
                // rational times.
                return rt.value() / rt.rate();
            }))
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
                [](RationalTime& self, std::string timecode, double rate) {
                    // TODO: How to handle the error?
                    return RationalTime::from_timecode(timecode, rate);
                }))
        .function(
            "from_time_string",
            ems::optional_override(
                [](RationalTime& self, std::string timecode, double rate) {
                    // TODO: How to handle the error?
                    return RationalTime::from_time_string(timecode, rate);
                }));

    ems::class_<TimeRange>("TimeRange")
        // TODO: Match Python constructor logic?
        .constructor()
        .constructor<RationalTime>()
        .constructor<RationalTime, RationalTime>()
        .property("start_time", &TimeRange::start_time)
        .property("duration", &TimeRange::duration)
        .function("toString", ems::optional_override([](TimeRange& tr) {
                      return string_printf(
                          "TimeRange(start_time=%s, end_time=%s)",
                          opentime_js_string(tr.start_time()).c_str(),
                          opentime_js_string(tr.duration()).c_str());
                  }))
        // TODO: Implement valueOf and toPrimitive to support comparisons
        // .function("valueOf", ems::optional_override([](TimeRange& rt) {
        //               todo
        //           }))
        // .function(
        //     "toPrimitive",
        //     ems::optional_override([](TimeRange& rt, std::string hint) {
        //         todo
        //     }))
        .function("end_time_inclusive", &TimeRange::end_time_inclusive)
        .function("duration_extended_by", &TimeRange::duration_extended_by)
        .function("extended_by", &TimeRange::extended_by)
        .function(
            "clamped",
            ems::select_overload<RationalTime(RationalTime) const>(
                &TimeRange::clamped))
        .function(
            "clamped",
            ems::select_overload<TimeRange(TimeRange) const>(
                &TimeRange::clamped))
        .function(
            "contains",
            ems::select_overload<bool(RationalTime) const>(
                &TimeRange::contains))
        .function(
            "contains",
            ems::select_overload<bool(TimeRange, double) const>(
                &TimeRange::contains))
        .function(
            "overlaps",
            ems::select_overload<bool(RationalTime) const>(
                &TimeRange::overlaps))
        .function(
            "overlaps",
            ems::select_overload<bool(TimeRange, double) const>(
                &TimeRange::overlaps))
        .function(
            "before",
            ems::select_overload<bool(RationalTime, double) const>(
                &TimeRange::before))
        .function(
            "before",
            ems::select_overload<bool(TimeRange, double) const>(
                &TimeRange::before))
        .function("meets", &TimeRange::meets)
        .function(
            "begins",
            ems::select_overload<bool(RationalTime, double) const>(
                &TimeRange::begins))
        .function(
            "begins",
            ems::select_overload<bool(TimeRange, double) const>(
                &TimeRange::begins))
        .function(
            "finishes",
            ems::select_overload<bool(RationalTime, double) const>(
                &TimeRange::finishes))
        .function(
            "finishes",
            ems::select_overload<bool(TimeRange, double) const>(
                &TimeRange::finishes))
        .function("intersects", &TimeRange::intersects)
        .class_function(
            "range_from_start_end_time",
            &TimeRange::range_from_start_end_time)
        .class_function(
            "range_from_start_end_time_inclusive",
            &TimeRange::range_from_start_end_time_inclusive);

    ems::class_<TimeTransform>("TimeTransform")
        .constructor<RationalTime, double, double>()
        .property("offset", &TimeTransform::offset)
        .property("scale", &TimeTransform::scale)
        .property("rate", &TimeTransform::rate)
        .function("toString", ems::optional_override([](TimeTransform& tt) {
                      return string_printf(
                          "TimeTransform(offset=%s, scale=%g, rate=%g)",
                          opentime_js_string(tt.offset()).c_str(),
                          tt.scale(),
                          tt.rate());
                  }))
        // TODO: Implement valueOf and toPrimitive to support comparisons
        .function(
            "applied_to",
            ems::select_overload<TimeRange(TimeRange) const>(
                &TimeTransform::applied_to))
        .function(
            "applied_to",
            ems::select_overload<TimeTransform(TimeTransform) const>(
                &TimeTransform::applied_to))
        .function(
            "applied_to",
            ems::select_overload<RationalTime(RationalTime) const>(
                &TimeTransform::applied_to));
}
