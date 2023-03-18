const opentimelineioFactory = require('../../install/opentimelineio');
const { expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {opentimelineioFactory.CustomEmbindModule}
 */
let opentimelineio;


beforeAll(async () => {
    opentimelineio = await opentimelineioFactory();
});

test('test_contructors', () => {
    const tr = new opentimelineio.TimeRange(new opentimelineio.RationalTime(0, 1), new opentimelineio.RationalTime(10, 1))
    const item = new opentimelineio.Item('foo', tr)

    expect(item.source_range).toEqual(tr)
    expect(item.name).toEqual('foo')

    const encoded = opentimelineio.serialize_json_to_string(item)
    const decoded = opentimelineio.SerializableObject.from_json_string(encoded)
    expect(item.is_equivalent_to(decoded)).toBeTruthy()

    item.delete()
    tr.delete()
})

test('test_copy_arguments', () => {
    const tr = new opentimelineio.TimeRange(new opentimelineio.RationalTime(0, 1), new opentimelineio.RationalTime(10, 1))

    // TODO: This doesn't work. We need to pass a EffectVector and MarkerVector...
    // Should look into supporting real arrays.
    // const item = new opentimelineio.Item('foo', tr, effects, markers, true, metadata)
    // item.delete()
    tr.delete()
})

test('test_effects', () => {
    const vec = new opentimelineio.EffectVector();
    vec.push_back(new opentimelineio.Effect('effect1'))
    vec.push_back(new opentimelineio.Effect('effect2'))

    const item = new opentimelineio.Item(
        'my item',
        null,
        vec
    )

    const effects = item.get_effects()
    console.log(effects)
    console.log(effects.length)
    console.log(effects.at(-1).name)
    item.delete()
    vec.delete()
})
