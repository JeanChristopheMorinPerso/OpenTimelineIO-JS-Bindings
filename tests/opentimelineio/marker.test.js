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
    // TODO: The default color is RED in Python. Should we also do that?
    expect(marker1.color).toEqual('GREEN')
    expect(marker1.get_metadata()).toEqual({})
    marker1.delete()

    const marker2 = new opentimelineio.Marker('myname')
    expect(marker2.name).toEqual('myname')
    expect(marker2.marked_range.start_time.equal(timeRange.start_time)).toBeFalsy()
    // TODO: The default color is RED in Python. Should we also do that?
    expect(marker2.color).toEqual('GREEN')
    expect(marker2.get_metadata()).toEqual({})
    marker2.delete()

    const marker3 = new opentimelineio.Marker('mycustomname', timeRange)
    expect(marker3.name).toEqual('mycustomname')
    expect(marker3.marked_range.start_time.equal(timeRange.start_time)).toBeTruthy()
    // TODO: The default color is RED in Python. Should we also do that?
    expect(marker3.color).toEqual('GREEN')
    expect(marker3.get_metadata()).toEqual({})
    marker3.delete()

    const marker4 = new opentimelineio.Marker('asd', timeRange, 'CUSTOM_COLOR')
    expect(marker4.name).toEqual('asd')
    expect(marker4.marked_range.start_time.equal(timeRange.start_time)).toBeTruthy()
    // TODO: The default color is RED in Python. Should we also do that?
    expect(marker4.color).toEqual('CUSTOM_COLOR')
    expect(marker4.get_metadata()).toEqual({})
    marker4.delete()

    const marker5 = new opentimelineio.Marker('asd2', timeRange, 'CUSTOM_COLOR2', { 'some data': 'here' })
    expect(marker5.name).toEqual('asd2')
    expect(marker5.marked_range.start_time.equal(timeRange.start_time)).toBeTruthy()
    // TODO: The default color is RED in Python. Should we also do that?
    expect(marker5.color).toEqual('CUSTOM_COLOR2')
    expect(marker5.get_metadata()).toEqual({ 'some data': 'here' })

    const encoded = opentimelineio.serialize_json_to_string(marker5)
    const decoded = opentimelineio.SerializableObject.from_json_string(encoded)
    expect(marker5.is_equivalent_to(decoded)).toBeTruthy()
    marker5.delete()
    decoded.delete()
})
