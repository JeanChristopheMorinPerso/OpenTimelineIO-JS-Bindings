// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

import { SerializableObject, RationalTime, TimeRange, TimeTransform } from "../install/opentimelineio";

type Int = number

/**
 * Create a property (getter + setter) that OTIO will be able
 * to serialize/deserialize.
 *
 * @param klass Class to attach the property to.
 * @param name Name of the property to create.
 * @param required_type Property type (class).
 */
export function serializable_field(klass: SerializableObject, name: string, required_type: any): void

export interface SerializeOptions {
    // Schema version to target.
    schema_version_target?: Map<string, Int>,
    // Number of spaces to use as indentation.
    indent?: Int,
}

/**
 * Serialize an item into a JSON string.
 * 
 * @param item The thing to serialize.
 * @param options Serialization options.
 */
export function serialize_json_to_string(item: RationalTime | TimeRange | TimeTransform, options?: SerializeOptions): string
