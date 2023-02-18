Module.onRuntimeInitialized = function () {
    console.log('asdasd')
    /**
    * Create a property (getter + setter) that OTIO will be able
    * to serialize/deserialize.
    *
    * @param {Object} klass Class to attach the property to.
    * @param {string} name Name of the property to create.
    * @param {any} [required_type] Property type
    */
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
}
