const opentimelineioFactory = require('../../install/opentimelineio');
const { expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {opentimelineioFactory.CustomEmbindModule}
 */
let opentimelineio;


beforeAll(async () => {
    // opentime = await opentimeFactory();
    opentimelineio = await opentimelineioFactory();
});

test("test_serialize_time", () => {
    const rt = new opentimelineio.RationalTime(15, 24);
    const encoded = opentimelineio.serialize_json_to_string(rt);
    const decoded = opentimelineio.deserialize_json_from_string(encoded);
    expect(rt).toEqual(decoded);

    const rt_dur = new opentimelineio.RationalTime(10, 20)
    const tr = new opentimelineio.TimeRange(rt, rt_dur)
    const encoded2 = opentimelineio.serialize_json_to_string(tr)
    const decoded2 = opentimelineio.deserialize_json_from_string(encoded2)
    expect(tr).toEqual(decoded2)

    const tt = new opentimelineio.TimeTransform(rt_dur, 1.5)
    const encoded3 = opentimelineio.serialize_json_to_string(tt)
    const decoded3 = opentimelineio.deserialize_json_from_string(encoded3)
    expect(tt).toEqual(decoded3)
})

test("test_cons", () => {
    const so = new opentimelineio.SerializableObjectWithMetadata()
    const met = so.metadata
    met['foo'] = 'bar'
    expect(met['foo']).toEqual('bar')
    so.set_metadata(met)
    expect(so.metadata['foo']).toEqual('bar')
    so.delete()
})


/**
 * Create a property (getter + setter) that OTIO will be able
 * to serialize/deserialize.
 * @param {Object} klass Class to attach the property to.
 * @param {string} name Name of the property to create.
 * @param {any} [required_type] Property type
 */
function serializable_field(klass, name, required_type) {
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

test("test_subclass", () => {
    // TODO: Document this.
    // Also, the embind docs documents another method.This method is taken from https://github.com/emscripten-core/emscripten/issues/7200#issuecomment-442323087

    // Test newer class syntax
    // Note that the first argument given to extend doesn't matter when using real classes.
    class MyFoo1 extends opentimelineio.SerializableObject.extend('MyFoo1', {}) {
        constructor() {
            super()
            opentimelineio.set_type_record(this, 'MyFoo1')
        }

        invoke() {
            // your code goes here
            console.log(`MyFoo1.invoke() metadata: ${JSON.stringify(this.metadata)}`)
        }

        _ = serializable_field(MyFoo1, 'myprop1')
        _ = serializable_field(MyFoo1, 'myprop2')
    }

    opentimelineio.register_serializable_object_type(MyFoo1, 'MyFoo1', 1)
    let instance1 = new MyFoo1();
    instance1.invoke();

    instance1.myprop1 = '1234'

    console.log('Calling to_json_string')
    expect(instance1.to_json_string(4)).toEqual(`{
    "OTIO_SCHEMA": "MyFoo1.1",
    "myprop1": "1234"
}`)
    // Verify that the C++ object is still alive. If it's not it will crash.
    instance1.to_json_string(4)

    // TODO: Calling to_json_sring a second time raises "RuntimeError: table index is out of bounds"
    // instance1.to_json_string(4)
    instance1.delete();

    // Test using "old" class style. Note that the first argument to extend needs to match
    // the type being extended.
    var MyFoo2 = opentimelineio.SerializableObject.extend("SerializableObject", {
        __construct: function () {
            this.__parent.__construct.call(this);
            opentimelineio.set_type_record(this, 'MyFoo2')
        }
    });

    opentimelineio.register_serializable_object_type(MyFoo2, 'MyFoo2', 1)
    const instance2 = new MyFoo2();

    // Note that object won't be aware of that property because it's not registered (added to
    // the dynamic fields list).
    instance2.anything = 'asd'

    expect(instance2.to_json_string(4)).toEqual(`{
    "OTIO_SCHEMA": "MyFoo2.1"
}`)

    // Verify that the C++ object is still alive. If it's not it will crash.
    instance2.to_json_string(4)
})

test("asd", () => {
    const asd = new opentimelineio.SerializableObject();
    asd.set_metadata({ 'mykey': 'myvalue' })
    console.log(asd.to_json_string(4))
    console.log(asd.to_json_string(4))
    console.log(asd.schema_name())
    console.log(asd.schema_name())
    asd.delete()

    const soMetadata = new opentimelineio.SerializableObjectWithMetadata()
    console.log(soMetadata.to_json_string(4))
    console.log(soMetadata.to_json_string(4))
    soMetadata.delete()

    const soMetadata2 = new opentimelineio.SerializableObjectWithMetadata("myname")
    console.log(soMetadata2.to_json_string(4))
    console.log(soMetadata2.to_json_string(4))
    soMetadata2.delete()

    const soMetadata3 = new opentimelineio.SerializableObjectWithMetadata("myname", { myky: 'myvalue' })
    console.log(soMetadata3.to_json_string(4))
    console.log(soMetadata3.to_json_string(4))
    soMetadata3.delete()
})
