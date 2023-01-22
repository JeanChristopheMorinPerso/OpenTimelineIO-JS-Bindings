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
    expect(sc.children.size()).toEqual(3)
    // TODO: Test equality of children
    expect(sc.get_metadata()).toEqual({ 'asd': 'dfg' })
    sc.delete()
})

test('test_iterable', () => {
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
    expect(sc.children.get(0)).toEqual(children[0])
})
