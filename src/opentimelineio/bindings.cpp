// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <opentimelineio/anyDictionary.h>
#include <opentimelineio/serializableCollection.h>
#include <opentimelineio/serializableObject.h>
#include <opentimelineio/unknownSchema.h>

#include "errorStatusHandler.h"

namespace ems = emscripten;
using namespace opentimelineio::OPENTIMELINEIO_VERSION;

namespace emscripten { namespace internal {
// https://github.com/emscripten-core/emscripten/issues/5587#issuecomment-429085470
template <>
void
raw_destructor<SerializableObject>(SerializableObject* ptr)
{
    ptr->possibly_delete();
}

template <>
void
raw_destructor<UnknownSchema>(UnknownSchema* ptr)
{
    ptr->possibly_delete();
}

template <>
void
raw_destructor<SerializableObjectWithMetadata>(
    SerializableObjectWithMetadata* ptr)
{
    ptr->possibly_delete();
}
}} // namespace emscripten::internal

SerializableObjectWithMetadata*
constructSOWithMetadata0()
{
    return new SerializableObjectWithMetadata();
}

SerializableObjectWithMetadata*
constructSOWithMetadata1(std::string name)
{
    return new SerializableObjectWithMetadata(name);
}

SerializableObjectWithMetadata*
constructSOWithMetadata2(std::string name, ems::val metadata)
{
    return new SerializableObjectWithMetadata(name, AnyDictionary());
}

EMSCRIPTEN_BINDINGS(opentimelineio)
{
    ems::class_<SerializableObject>("SerializableObject")
        .constructor<>()
        .function("is_equivalent_to", &SerializableObject::is_equivalent_to)
        .function(
            "clone",
            ems::optional_override([](SerializableObject* so) {
                return so->clone(ErrorStatusHandler());
            }),
            ems::allow_raw_pointers())
        .function(
            "to_json_string",
            ems::optional_override([](SerializableObject* so, int indent) {
                return so->to_json_string(ErrorStatusHandler(), {}, indent);
            }),
            ems::allow_raw_pointers())
        .function(
            "to_json_file",
            ems::optional_override(
                [](SerializableObject* so, std::string file_name, int indent) {
                    return so->to_json_file(
                        file_name,
                        ErrorStatusHandler(),
                        {},
                        indent);
                }),
            ems::allow_raw_pointers())
        .class_function(
            "from_json_string",
            ems::optional_override([](std::string input) {
                return SerializableObject::from_json_string(
                    input,
                    ErrorStatusHandler());
            }),
            ems::allow_raw_pointers())
        .class_function(
            "from_json_file",
            ems::optional_override([](std::string file_name) {
                return SerializableObject::from_json_file(
                    file_name,
                    ErrorStatusHandler());
            }),
            ems::allow_raw_pointers())
        .function("schema_name", &SerializableObject::schema_name)
        .function("schema_version", &SerializableObject::schema_version)
        .property("is_unknown_schema", &SerializableObject::is_unknown_schema);

    ems::class_<UnknownSchema, ems::base<SerializableObject>>("UnknownSchema")
        .constructor<std::string, int>()
        .property("original_schema_name", &UnknownSchema::original_schema_name)
        .property(
            "original_schema_version",
            &UnknownSchema::original_schema_version);

    ems::class_<SerializableObjectWithMetadata, ems::base<SerializableObject>>(
        "SerializableObjectWithMetadata")
        .constructor(&constructSOWithMetadata0, ems::allow_raw_pointers())
        .constructor(&constructSOWithMetadata1, ems::allow_raw_pointers())
        .constructor(&constructSOWithMetadata2, ems::allow_raw_pointers());
}
