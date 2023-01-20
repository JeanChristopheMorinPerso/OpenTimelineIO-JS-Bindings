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

test('test_construct', () => {
    const composable1 = new opentimelineio.Composable()
    expect(composable1.name).toEqual('')
    expect(composable1.get_metadata()).toEqual({})
    composable1.delete()

    const composable2 = new opentimelineio.Composable('custom name')
    expect(composable2.name).toEqual('custom name')
    expect(composable2.get_metadata()).toEqual({})
    composable2.delete()

    const composable3 = new opentimelineio.Composable('test', { 'foo': 'bar' })
    expect(composable3.name).toEqual('test')
    expect(composable3.get_metadata()).toEqual({ 'foo': 'bar' })
    composable3.delete()
})

test('test_serialize1', () => {
    const composable = new opentimelineio.Composable('test', { 'foo': 'bar' })
    const encoded = opentimelineio.serialize_json_to_string(composable)
    // const encoded = composable.to_json_string(4)
    // const decoded = opentimelineio.deserialize_json_from_string(encoded)
    const decoded = opentimelineio.SerializableObject.from_json_string(encoded)
    expect(composable.is_equivalent_to(decoded)).toBeTruthy()
    composable.delete()

    decoded.delete()
})

test.skip('test_serialize2', () => {
    const composable = new opentimelineio.Composable('test', { 'foo': 'bar' })
    const encoded = composable.to_json_string(4)
    const decoded = opentimelineio.deserialize_json_from_string(encoded)
    // TODO: This is crashing. The C++ object behind decoded gets deleted. Not too sure
    // but I think the object stored in ems::val needs to be a managing_ptr?
    expect(composable.is_equivalent_to(decoded)).toBeTruthy()
    composable.delete()
    decoded.delete()
})

test('test_metadata', () => {
    const composable = new opentimelineio.Composable()
    const met = composable.get_metadata()
    met['foo'] = 'bar'
    composable.set_metadata(met)

    const encoded = composable.to_json_string()
    const decoded = opentimelineio.SerializableObject.from_json_string(encoded)
    // TODO: Test equality operator
    expect(composable.get_metadata()['foo']).toEqual(decoded.get_metadata()['foo'])

    composable.delete()
    decoded.delete()
})