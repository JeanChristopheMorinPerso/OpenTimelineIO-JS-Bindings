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

    item.source_range = undefined
    expect(item.source_range).toBeUndefined()

    decoded.delete()
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
    const effect1 = new opentimelineio.Effect('effect1')
    const effect2 = new opentimelineio.Effect('effect2')
    vec.push_back(effect1)
    vec.push_back(effect2)

    const item = new opentimelineio.Item(
        'my item',
        undefined,
        vec
    )

    const effects = item.get_effects()
    expect(Array.from(effects, effect => effect.name)).toEqual([
        'effect1',
        'effect2'
    ])

    const emptyItem = new opentimelineio.Item()
    expect(Array.from(emptyItem.get_effects())).toEqual([])

    emptyItem.delete()
    item.delete()
    vec.delete()
    effect1.delete()
    effect2.delete()
})

test('test_markers_iteration', () => {
    const effects = new opentimelineio.EffectVector()
    const markers = new opentimelineio.MarkerVector()
    const marker1 = new opentimelineio.Marker('marker1')
    const marker2 = new opentimelineio.Marker('marker2')
    markers.push_back(marker1)
    markers.push_back(marker2)

    const item = new opentimelineio.Item(
        'my item',
        undefined,
        effects,
        markers
    )

    expect(Array.from(item.get_markers(), marker => marker.name)).toEqual([
        'marker1',
        'marker2'
    ])

    const emptyItem = new opentimelineio.Item()
    expect(Array.from(emptyItem.get_markers())).toEqual([])

    emptyItem.delete()
    item.delete()
    effects.delete()
    markers.delete()
    marker1.delete()
    marker2.delete()
})
