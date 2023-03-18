// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project
#include <cstdio>
#include <memory>
#include <string>

#include <emscripten/bind.h>
#include <opentime/errorStatus.h>
#include <opentime/rationalTime.h>
#include <opentime/timeRange.h>
#include <opentime/timeTransform.h>

#include "common_utils.h"
#include "exceptions.h"

namespace ems = emscripten;
using namespace opentime;

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

namespace {

struct ErrorStatusConverter
{
    operator ErrorStatus*() { return &error_status; }

    ~ErrorStatusConverter() noexcept(false)
    {
        if (is_error(error_status))
        {
            throw ValueError(error_status.details);
        }
    }

    ErrorStatus error_status;
};
} // namespace

template <typename T>
T
_type_checked(ems::val const& rhs, char const* op)
{
    try
    {
        return rhs.as<T>();
    }
    catch (...)
    {
        throw TypeError(string_printf(
            "unsupported operand type(s) for %s: "
            "RationalTime and %s",
            op,
            rhs.typeOf().as<std::string>().c_str()));
    }
}

/**
 * Add a comparison operator function on a JS class.
 * @param TYPE Type of the object.
 * @param NAME Name of the function to add
 * @param OPERATOR C++ operator
*/
#define ADD_COMPARISON_OPERATOR(TYPE, NAME, OPERATOR)                          \
    .function(                                                                 \
        NAME,                                                                  \
        ems::optional_override([](TYPE const& lhs, ems::val const& rhs) {      \
            return lhs OPERATOR _type_checked<TYPE>(rhs, #OPERATOR);           \
        }))

EMSCRIPTEN_BINDINGS(opentime)
{

    ems::enum_<IsDropFrameRate>("IsDropFrameRate")
        .value("ForceNo", IsDropFrameRate::ForceNo)
        .value("ForceYes", IsDropFrameRate::ForceYes)
        .value("InferFromRate", IsDropFrameRate::InferFromRate);

    ems::class_<RationalTime>("RationalTime")
        .constructor<>()
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
        .function(
            "to_timecode",
            ems::optional_override([](RationalTime const& rt) {
                return rt.to_timecode(
                    rt.rate(),
                    IsDropFrameRate::InferFromRate,
                    ErrorStatusConverter());
            }))
        .function(
            "to_timecode",
            ems::optional_override([](RationalTime const& rt, double rate) {
                return rt.to_timecode(
                    rate,
                    IsDropFrameRate::InferFromRate,
                    ErrorStatusConverter());
            }))
        .function(
            "to_timecode",
            ems::optional_override([](RationalTime const& rt,
                                      double              rate,
                                      IsDropFrameRate     drop_frame) {
                return rt.to_timecode(rate, drop_frame, ErrorStatusConverter());
            }))
        .function("to_time_string", &RationalTime::to_time_string)
        .class_function(
            "from_timecode",
            ems::optional_override([](std::string timecode, double rate) {
                return RationalTime::from_timecode(
                    timecode,
                    rate,
                    ErrorStatusConverter());
            }))
        .class_function(
            "from_time_string",
            ems::optional_override([](std::string timecode, double rate) {
                return RationalTime::from_time_string(
                    timecode,
                    rate,
                    ErrorStatusConverter());
            }))
        // clang-format off
        ADD_COMPARISON_OPERATOR(RationalTime, "equal", ==)
        ADD_COMPARISON_OPERATOR(RationalTime, "notEqual", !=)
        ADD_COMPARISON_OPERATOR(RationalTime, "lessThan", <)
        ADD_COMPARISON_OPERATOR(RationalTime, "lessThanOrEqual", <=)
        ADD_COMPARISON_OPERATOR(RationalTime, "greaterThan", >)
        ADD_COMPARISON_OPERATOR(RationalTime, "greaterThanOrEqual", >=)
        ADD_COMPARISON_OPERATOR(RationalTime, "add", +)
        ADD_COMPARISON_OPERATOR(RationalTime, "subtract", -)
        // clang-format on
        .function(
            "compoundAdd",
            ems::optional_override([](RationalTime lhs, ems::val const& rhs) {
                return lhs += _type_checked<RationalTime>(rhs, "+=");
            }))
        .function(
            "compoundSubstract",
            ems::optional_override([](RationalTime lhs, ems::val const& rhs) {
                return lhs -= _type_checked<RationalTime>(rhs, "-=");
            }));

    ADD_TO_STRING_TAG_PROPERTY(RationalTime);

    ems::class_<TimeRange>("TimeRange")
        // TODO: Match Python constructor logic?
        .constructor()
        .constructor<RationalTime>()
        .constructor<RationalTime, RationalTime>()
        .property("start_time", &TimeRange::start_time)
        .property("duration", &TimeRange::duration)
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
            &TimeRange::range_from_start_end_time_inclusive)
        // clang-format off
        ADD_COMPARISON_OPERATOR(TimeRange, "equal", ==)
        ADD_COMPARISON_OPERATOR(TimeRange, "notEqual", !=);
    // clang-format on

    ADD_TO_STRING_TAG_PROPERTY(TimeRange);

    ems::class_<TimeTransform>("TimeTransform")
        .constructor<>()
        .constructor<RationalTime>()
        .constructor<RationalTime, double>()
        .constructor<RationalTime, double, double>()
        .property("offset", &TimeTransform::offset)
        .property("scale", &TimeTransform::scale)
        .property("rate", &TimeTransform::rate)
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
                &TimeTransform::applied_to))
        // clang-format off
        ADD_COMPARISON_OPERATOR(TimeTransform, "equal", ==)
        ADD_COMPARISON_OPERATOR(TimeTransform, "notEqual", !=);
    // clang-format on

    ADD_TO_STRING_TAG_PROPERTY(TimeTransform);
}
