// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

/* global Module */
Module.onRuntimeInitialized = function () {
    function invalidateWrapperOnDestruction(baseClass, wrapperClass) {
        const originalExtend = baseClass.extend

        baseClass.extend = function (...args) {
            const subclass = originalExtend.apply(this, args)
            const originalDestruct = wrapperClass.prototype.__destruct

            wrapperClass.prototype.__destruct = function () {
                originalDestruct.call(this)

                // Embind keeps this pointer valid when delete() is called while
                // C++ still owns the object. Once the C++ destructor reaches
                // this callback, the pointer is stale and must not be reused.
                this.$$.smartPtr = undefined
                this.$$.ptr = undefined
            }

            return subclass
        }
    }

    if (Module.SerializableObject) {
        invalidateWrapperOnDestruction(
            Module.SerializableObject,
            Module.SerializableObjectWrapper
        )
        invalidateWrapperOnDestruction(
            Module.SerializableObjectWithMetadata,
            Module.SerializableObjectWithMetadataWrapper
        )
    }

    Module.serializable_field = function (klass, name, required_type) {
        Object.defineProperty(klass.prototype, name, {
            get() {
                return this._get_dynamic_fields()[name]
            },
            set(value) {
                // TODO: Test for null and undefined?
                if (required_type && value) {
                    if (value instanceof required_type) {
                        throw new Error('TODO: Error message')
                    }
                }
                const fields = this._get_dynamic_fields();
                fields[name] = value;

                this._set_dynamic_fields(fields)
                // console.log(`${this.constructor.name}.set: Dynamic fields: ${this._get_dynamic_fields()}`)
            }
        })
    }

    // TODO: Add to TypeScript definitions.
    Module.serialize_json_to_string = function (item, { schema_version_target = {}, indent = 4 } = {}) {
        let jsitem;
        let func;
        if (item instanceof Module.RationalTime) {
            jsitem = new Module.JSAnyRationalTime(item)
            func = Module._serialize_RationalTime_to_string
        } else if (item instanceof Module.TimeRange) {
            jsitem = new Module.JSAnyTimeRange(item)
            func = Module._serialize_TimeRange_to_string
        } else if (item instanceof Module.TimeTransform) {
            jsitem = new Module.JSAnyTimeTransform(item)
            func = Module._serialize_TimeTransform_to_string
        } else if (item instanceof Module.SerializableObject) {
            jsitem = new Module.JSAnySerializableObject(item)
            func = Module._serialize_SerializableObject_to_string
        } else {
            // TODO: Add a proper error message
            throw new Error('asdasd')
        }

        try {
            return func(jsitem, schema_version_target, indent)
        } finally {
            jsitem.delete()
        }
    }
}
