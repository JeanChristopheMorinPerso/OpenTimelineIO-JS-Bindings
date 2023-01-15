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
})

test("test_copy_subclass", () => {
    var MyFoo = opentimelineio.SerializableObjectWithMetadata.extend("SerializableObjectWithMetadata", {
        // __construct and __destruct are optional.  They are included
        // in this example for illustration purposes.
        // If you override __construct or __destruct, don't forget to
        // call the parent implementation!
        __construct: function () {
            this.__parent.__construct.call(this);
            opentimelineio.set_type_record(this, 'MyFoo')
        },
        __destruct: function () {
            this.__parent.__destruct.call(this);
        },
        invoke: function () {
            // your code goes here
            console.log('asd')
            console.log(this.metadata)
        },
    })

    opentimelineio.register_serializable_object_type(MyFoo, 'MyFoo', 1)
    const asd = new MyFoo();
    const met = asd.metadata
    met['asdasd'] = 'value';
    asd.set_metadata(met)
    asd.invoke();

    expect(asd.to_json_string(4)).toEqual(`{
    "OTIO_SCHEMA": "MyFoo.1",
    "metadata": {
        "asdasd": "value"
    },
    "name": ""
}`)
})
