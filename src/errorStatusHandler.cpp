// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <emscripten/bind.h>

#include <errorStatusHandler.h>
#include <exceptions.h>

namespace ems  = emscripten;
namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;

ErrorStatusHandler::~ErrorStatusHandler() noexcept(false)
{
    if (!otio::is_error(error_status))
    {
        return;
    }

    switch (error_status.outcome)
    {
        case ErrorStatus::NOT_IMPLEMENTED:
            throw NotImplementedError(error_status.details);
        case ErrorStatus::ILLEGAL_INDEX:
            throw IndexError(error_status.details);
        case ErrorStatus::KEY_NOT_FOUND:
            throw KeyError(error_status.details);
        case ErrorStatus::INTERNAL_ERROR:
            throw ValueError(
                std::string("Internal error (aka \"this is a bug\"): ")
                + details());
        case ErrorStatus::UNRESOLVED_OBJECT_REFERENCE:
            throw ValueError(
                "Unresolved object reference while reading: " + details());
        case ErrorStatus::DUPLICATE_OBJECT_REFERENCE:
            throw ValueError(
                "Duplicated object reference while reading: " + details());
        case ErrorStatus::MALFORMED_SCHEMA:
            throw ValueError("Illegal/malformed schema: " + details());
        case ErrorStatus::JSON_PARSE_ERROR:
            throw ValueError("JSON parse error while reading: " + details());
        case ErrorStatus::FILE_OPEN_FAILED:
        case ErrorStatus::FILE_WRITE_FAILED:
            throw IOError(std::string(std::strerror(errno)) + ": " + details());
        case ErrorStatus::SCHEMA_VERSION_UNSUPPORTED:
            throw UnsupportedSchemaError(full_details());
        case ErrorStatus::NOT_A_CHILD_OF:
        case ErrorStatus::NOT_A_CHILD:
        case ErrorStatus::NOT_DESCENDED_FROM:
            throw NotAChildError(full_details());
        case ErrorStatus::CANNOT_COMPUTE_AVAILABLE_RANGE:
            throw CannotComputeAvailableRangeError(full_details());
        case ErrorStatus::OBJECT_CYCLE:
            throw ValueError(
                "Detected SerializableObject cycle while copying/serializing: "
                + details());
        case ErrorStatus::MEDIA_REFERENCES_DO_NOT_CONTAIN_ACTIVE_KEY:
            throw ValueError(
                "The media references do not contain the active key");
        case ErrorStatus::MEDIA_REFERENCES_CONTAIN_EMPTY_KEY:
            throw ValueError("The media references contain an empty key");
        default:
            throw ValueError(full_details());
    }
}

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
ErrorStatusHandler::details()
{
    if (!error_status.object_details)
    {
        return error_status.details;
    }

    return string_printf(
        "%s: %s",
        error_status.details.c_str(),
        ems::val(error_status.object_details)
            .call<ems::val>("toString")
            .as<std::string>()
            .c_str());
}

std::string
ErrorStatusHandler::full_details()
{
    if (!error_status.object_details)
    {
        return error_status.full_description;
    }

    return string_printf(
        "%s: %s",
        error_status.full_description.c_str(),
        ems::val(error_status.object_details)
            .call<ems::val>("toString")
            .as<std::string>()
            .c_str());
}

namespace jsexceptions {
std::string
getExceptionMessage(int exceptionPtr)
{
    return std::string(reinterpret_cast<std::exception*>(exceptionPtr)->what());
}
} // namespace jsexceptions

EMSCRIPTEN_BINDINGS(exceptions)
{
    // TODO: This is not needed if we use -fwasm-exceptions compiler flag.
    // But it's not yet supported in all environments. See https://webassembly.org/roadmap/.
    // ems::function("getExceptionMessage", &jsexceptions::getExceptionMessage);
}
