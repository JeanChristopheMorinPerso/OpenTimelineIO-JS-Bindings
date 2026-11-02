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
    const timeRange = new opentimelineio.TimeRange(
        new opentimelineio.RationalTime(5, 24),
        new opentimelineio.RationalTime(10, 24)
    )

    const marker1 = new opentimelineio.Marker()
    expect(marker1.name).toEqual('')
    expect(marker1.marked_range.start_time.equal(timeRange.start_time)).toBeFalsy()
    const defaultColor1 = marker1.color
    expect(defaultColor1.to_hex()).toBe('#ff0000ff')
    defaultColor1.delete()
    expect(marker1.get_metadata()).toEqual({})
    expect(marker1.comment).toBe('')
    marker1.delete()

    const marker2 = new opentimelineio.Marker('myname')
    expect(marker2.name).toEqual('myname')
    expect(marker2.marked_range.start_time.equal(timeRange.start_time)).toBeFalsy()
    const defaultColor2 = marker2.color
    expect(defaultColor2.to_hex()).toBe('#ff0000ff')
    defaultColor2.delete()
    expect(marker2.get_metadata()).toEqual({})
    marker2.delete()

    const marker3 = new opentimelineio.Marker('mycustomname', timeRange)
    expect(marker3.name).toEqual('mycustomname')
    expect(marker3.marked_range.start_time.equal(timeRange.start_time)).toBeTruthy()
    const defaultColor3 = marker3.color
    expect(defaultColor3.to_hex()).toBe('#ff0000ff')
    defaultColor3.delete()
    expect(marker3.get_metadata()).toEqual({})
    marker3.delete()

    const customColor = new opentimelineio.Color(0.2, 0.4, 0.8, 0.6, 'Custom')
    const marker4 = new opentimelineio.Marker('asd', timeRange, customColor)
    expect(marker4.name).toEqual('asd')
    expect(marker4.marked_range.start_time.equal(timeRange.start_time)).toBeTruthy()
    const returnedColor = marker4.color
    expect(returnedColor.to_rgba_float_list()).toEqual([0.2, 0.4, 0.8, 0.6])
    returnedColor.delete()
    marker4.color = undefined
    expect(marker4.color).toBeUndefined()
    expect(marker4.get_metadata()).toEqual({})
    marker4.delete()

    const marker5 = new opentimelineio.Marker('asd2', timeRange, customColor, { 'some data': 'here' }, 'Review this frame')
    expect(marker5.name).toEqual('asd2')
    expect(marker5.marked_range.start_time.equal(timeRange.start_time)).toBeTruthy()
    expect(JSON.parse(marker5.to_json_string()).color).toEqual({
        OTIO_SCHEMA: 'Color.1', r: 0.2, g: 0.4, b: 0.8, a: 0.6, name: 'Custom'
    })
    expect(marker5.get_metadata()).toEqual({ 'some data': 'here' })
    expect(marker5.comment).toBe('Review this frame')
    marker5.comment = 'Revised comment'

    const encoded = opentimelineio.serialize_json_to_string(marker5)
    const decoded = opentimelineio.SerializableObject.from_json_string(encoded)
    expect(marker5.is_equivalent_to(decoded)).toBeTruthy()
    expect(decoded.comment).toBe('Revised comment')
    const legacyData = JSON.parse(encoded)
    delete legacyData.comment
    const legacy = opentimelineio.SerializableObject.from_json_string(JSON.stringify(legacyData))
    expect(legacy.comment).toBe('')
    legacy.delete()
    marker5.delete()
    decoded.delete()
    customColor.delete()
    timeRange.delete()
})

test.each([
    [1, 'gReEn', [0, 1, 0, 1], 'GREEN'],
    [1, 'CUSTOM_COLOR', [1, 1, 1, 1], 'CUSTOM_COLOR'],
    [2, 'gReEn', [0, 1, 0, 1], 'GREEN'],
    [2, 'CUSTOM_COLOR', [1, 1, 1, 1], 'CUSTOM_COLOR']
])('legacy Marker.%i color %s upgrades and downgrades', (version, name, rgba, downgradedName) => {
    const range = new opentimelineio.TimeRange()
    const data = {
        OTIO_SCHEMA: `Marker.${version}`, name: 'legacy', metadata: {}, color: name,
        [version === 1 ? 'range' : 'marked_range']: JSON.parse(range.to_json_string())
    }
    let marker
    let color
    try {
        marker = opentimelineio.SerializableObject.from_json_string(JSON.stringify(data))
        color = marker.color
        expect(marker.schema_version()).toBe(3)
        expect(color.to_rgba_float_list()).toEqual(rgba)
        const downgraded = opentimelineio.serialize_json_to_string(marker, {
            schema_version_target: { Marker: 2 }
        })
        expect(JSON.parse(downgraded).OTIO_SCHEMA).toBe('Marker.2')
        expect(JSON.parse(downgraded).color).toBe(downgradedName)
    } finally {
        color?.delete()
        marker?.delete()
        range.delete()
    }
})
