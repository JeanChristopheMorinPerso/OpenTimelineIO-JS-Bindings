// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project
#include <stdexcept>

struct OTIOException : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct NotImplementedError : public OTIOException
{
    using OTIOException::OTIOException;
};

struct IndexError : public OTIOException
{
    using OTIOException::OTIOException;
};

struct KeyError : public OTIOException
{
    using OTIOException::OTIOException;
};

struct ValueError : public OTIOException
{
    using OTIOException::OTIOException;
};

struct TypeError : public OTIOException
{
    using OTIOException::OTIOException;
};

struct IOError : public OTIOException
{
    using OTIOException::OTIOException;
};

struct NotAChildError : public OTIOException
{
    using OTIOException::OTIOException;
};

struct UnsupportedSchemaError : public OTIOException
{
    using OTIOException::OTIOException;
};

struct CannotComputeAvailableRangeError : public OTIOException
{
    using OTIOException::OTIOException;
};
