// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#pragma once

#include <opentimelineio/errorStatus.h>

namespace otio = opentimelineio::OPENTIMELINEIO_VERSION;

struct ErrorStatusHandler
{
    operator otio::ErrorStatus*() { return &error_status; }

    ~ErrorStatusHandler() noexcept(false);

    std::string details();
    std::string full_details();

    otio::ErrorStatus error_status;
};
