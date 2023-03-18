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

test('test_constructor', () => {
    const children = [
        new opentimelineio.SerializableObjectWithMetadata('testSO'),
        new opentimelineio.SerializableObject(),
        new opentimelineio.SerializableObject()
    ]

    const sovec = new opentimelineio.SOVector();
    for (const item of children) {
        sovec.push_back(item)
    }

    const sc = new opentimelineio.SerializableCollection('test', sovec, { 'asd': 'dfg' })
    expect(sc.name).toEqual('test')
    expect(sc.length).toEqual(3)
    expect(sc.get_children().size()).toEqual(3)
    // TODO: Test equality of children
    expect(sc.get_metadata()).toEqual({ 'asd': 'dfg' })
    sc.delete()
    sovec.delete()
})

test('test_iterable', () => {
    const children = [
        new opentimelineio.Clip('testClip'),
        new opentimelineio.MissingReference()
    ]

    const sovec = new opentimelineio.SOVector();
    for (const item of children) {
        sovec.push_back(item)
    }

    const sc = new opentimelineio.SerializableCollection('test', sovec, { 'asd': 'dfg' })
    expect(sc.get_children().get(0).is_equivalent_to(children[0])).toEqual(true)

    for (const child of sc) {
        console.log(child)
    }
    sovec.delete()
})

test.skip('test_serialize', () => {
    const children = [
        new opentimelineio.Clip('testClip'),
        new opentimelineio.MissingReference()
    ]

    const sovec = new opentimelineio.SOVector();
    for (const item of children) {
        sovec.push_back(item)
    }

    const sc = new opentimelineio.SerializableCollection('test', sovec, { 'asd': 'dfg' })

    const encoded = opentimelineio.serialize_json_to_string(sc)
    const decoded = opentimelineio.SerializableCollection.from_json_string(encoded)
    console.log(sc)
    console.log(decoded)
    expect(sc.is_equivalent_to(decoded)).toEqual(true)
    sc.delete()
    sovec.delete()
})
