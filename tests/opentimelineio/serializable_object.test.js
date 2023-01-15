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
    so.metadata['foo'] = 'bar'
    expect(so.metadata['foo']).toEqual('bar')
})
