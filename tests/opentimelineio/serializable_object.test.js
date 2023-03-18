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

test('test_serialize_object', () => {
    const so = new opentimelineio.SerializableObject()
    expect(so.to_json_string(4)).toEqual(`{
    "OTIO_SCHEMA": "SerializableObject.1"
}`)
    so.delete()
})

// This can look weird, but serialization and deserialization is part of
// opentimelineio, not opentime.
test('test_serialize_time', () => {
    const rt = new opentimelineio.RationalTime(15, 24);
    const decoded = opentimelineio.deserialize_json_from_string(rt.to_json_string());
    expect(rt).toEqual(decoded);

    const rt_dur = new opentimelineio.RationalTime(10, 20)
    const tr = new opentimelineio.TimeRange(rt, rt_dur)
    const decoded2 = opentimelineio.deserialize_json_from_string(tr.to_json_string())
    expect(tr).toEqual(decoded2)

    const tt = new opentimelineio.TimeTransform(rt_dur, 1.5)
    const decoded3 = opentimelineio.deserialize_json_from_string(tt.to_json_string())
    expect(tt).toEqual(decoded3)
})

test('test_serialize', () => {
    function expectError(expectedMessage, callback) {
        try {
            callback();
            throw new Error('Expected function to throw!');
        } catch (err) {
            expect(err).toBeInstanceOf(WebAssembly.Exception)
            expect(err.message[1]).toEqual(expectedMessage)
        }
    }

    expectError('No such file or directory: non existent file', () => {
        opentimelineio.SerializableObject.from_json_file('non existent file')
    })

    expectError('JSON parse error while reading: JSON parse error on input string: Invalid value. (line 1, column 0)', () => {
        opentimelineio.SerializableObject.from_json_string('aasd')
    })

    expectError('JSON parse error while reading: JSON parse error on input string: The document is empty. (line 1, column 0)', () => {
        opentimelineio.SerializableObject.from_json_string('')
    })

    expectError('type mismatch while decoding: Expected a SerializableObject*, found object of type \'opentimelineio::v1_0::AnyDictionary\' instead', () => {
        opentimelineio.SerializableObject.from_json_string('{}')
    })
})

// TODO: Add more metadata (cover all possible types)
test('test_metadata', () => {
    const so = new opentimelineio.SerializableObjectWithMetadata()
    const met = so.get_metadata()
    met['foo'] = 'bar'
    expect(met['foo']).toEqual('bar')
    so.set_metadata(met)
    expect(so.get_metadata()['foo']).toEqual('bar')
    so.delete()
})

test('test_subclass', () => {
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
        }

        _ = opentimelineio.serializable_field(MyFoo1, 'myprop1') // eslint-disable-line no-undef
        _ = opentimelineio.serializable_field(MyFoo1, 'myprop2') // eslint-disable-line no-undef, no-dupe-class-members
    }

    opentimelineio.register_serializable_object_type(MyFoo1, 'MyFoo1', 1)
    let instance1 = new MyFoo1();
    instance1.invoke();

    instance1.myprop1 = '1234'

    expect(instance1.to_json_string(4)).toEqual(`{
    "OTIO_SCHEMA": "MyFoo1.1",
    "myprop1": "1234"
}`)
    // Verify that the C++ object is still alive. If it's not it will crash.
    instance1.to_json_string(4)

    instance1.delete();

    // Test using "old" class style. Note that the first argument to extend needs to match
    // the type being extended.
    var MyFoo2 = opentimelineio.SerializableObject.extend('SerializableObject', {
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

test('test_constructors', () => {
    const asd = new opentimelineio.SerializableObject();
    // Call twice to make sure it doesn't crash.
    asd.to_json_string(4)
    asd.to_json_string(4)
    asd.schema_name()
    asd.schema_name()
    asd.delete()

    const soMetadata = new opentimelineio.SerializableObjectWithMetadata()
    soMetadata.to_json_string(4)
    soMetadata.to_json_string(4)
    soMetadata.delete()

    const soMetadata2 = new opentimelineio.SerializableObjectWithMetadata('myname')
    soMetadata2.to_json_string(4)
    soMetadata2.to_json_string(4)
    soMetadata2.delete()

    const soMetadata3 = new opentimelineio.SerializableObjectWithMetadata('myname', { myky: 'myvalue' })
    soMetadata3.to_json_string(4)
    soMetadata3.to_json_string(4)
    soMetadata3.delete()
})

test.skip('test_copy_lib', () => {
    const so = new opentimelineio.SerializableObjectWithMetadata('', { 'foo': 'bar' })

    const so_copy = so.clone_otio()

    expect(so.is_equivalent_to(so_copy)).toEqual(true)

    const met = so_copy.get_metadata()
    met['asdasd'] = 'random value'
    so_copy.set_metadata(met)

    expect(so.is_equivalent_to(so_copy)).toEqual(false)
    so.delete()
    so_copy.delete()
})

test('equality', () => {
    const so1 = new opentimelineio.SerializableObject()
    const so2 = new opentimelineio.SerializableObject()
    expect(so1.is_equivalent_to(so2)).toEqual(true)
    so1.delete()
    so2.delete()
})

test('test_truthiness', () => {
    const so = new opentimelineio.SerializableObject()
    expect(so).toBeTruthy()
})

test.skip('test_instancing_without_instancing_support', () => {
    const so1 = new opentimelineio.SerializableObjectWithMetadata()
    const so2 = new opentimelineio.SerializableObjectWithMetadata()
    const met = so1.get_metadata()
    met['child1'] = so2
    met['child2'] = so2
    so1.set_metadata(met)

    const so_copy = so1.clone_otio()

    const met2 = so_copy.get_metadata()
    const so3 = met2['child2']
    so3.set_metadata({ 'sub': 'so3' })

    expect(so1.is_equivalent_to(so_copy)).toBeFalsy()
    so1.delete()
    so2.delete()
    so3.delete()
})

test.skip('test_schema_definition', () => {
    // Define a schema and instantiate it from JS

    // TODO: Re-enable once std::unordered_map is bound.
    // expect(opentimelineio.type_version_map()).not.toContain('Stuff')

    class FakeThing extends opentimelineio.SerializableObject.extend('FakeThing', {}) {
        constructor() {
            super()
            opentimelineio.set_type_record(this, 'Stuff')
        }

        _ = serializable_field(FakeThing, 'foo_two') // eslint-disable-line no-undef
    }

    opentimelineio.register_serializable_object_type(FakeThing, 'Stuff', 1)

    const ft = new FakeThing()

    expect(ft.schema_name()).toEqual('Stuff')
    expect(ft.schema_version()).toEqual(1)

    // TODO: This throw but not for the good reason.
    // It raises: TypeError: Class constructor FakeThing cannot be invoked without 'new'
    // It should raise an UnknownSchemaError.
    expect(() => {
        opentimelineio.instance_from_schema('Stuff', 2, { 'foo': 'bar' })
    }).toThrow()

    // version_map = otio.core.type_version_map()
    // self.assertEqual(version_map["Stuff"], 1)

    // ft = otio.core.instance_from_schema("Stuff", 1, {"foo": "bar"})
    // self.assertEqual(ft._dynamic_fields['foo'], "bar")
})
