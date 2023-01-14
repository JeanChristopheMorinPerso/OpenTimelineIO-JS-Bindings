// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <opentime/timeRange.h>
#include <opentimelineio/anyDictionary.h>
#include <opentimelineio/marker.h>
#include <opentimelineio/serializableCollection.h>
#include <opentimelineio/serializableObject.h>
#include <opentimelineio/unknownSchema.h>

#include "errorStatusHandler.h"
#include "js_anyDictionary.h"
#include "utils.h"

namespace ems = emscripten;
using namespace opentimelineio::OPENTIMELINEIO_VERSION;

REGISTER_DESTRUCTOR(SerializableObject);
REGISTER_DESTRUCTOR(UnknownSchema);
REGISTER_DESTRUCTOR(SerializableObjectWithMetadata);

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
        .constructor<>()
        .constructor<std::string>()
        .constructor(
            ems::optional_override([](std::string name, ems::val metadata) {
                AnyDictionary d = js_map_to_cpp(metadata);
                return new SerializableObjectWithMetadata(name, d);
            }))
        .property(
            "name",
            &SerializableObjectWithMetadata::name,
            &SerializableObjectWithMetadata::set_name)
        .property(
            "metadata",
            // TODO: Should we instead return the reference? AFAIK we can't
            // to ems::select_overload<AnyDictionary&() noexcept>
            // Also, how will we override metadata? For example so.metadata?
            ems::select_overload<AnyDictionary() const noexcept>(
                &SerializableObjectWithMetadata::metadata));
}
