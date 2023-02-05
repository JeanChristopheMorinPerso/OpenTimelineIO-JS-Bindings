const opentimelineioFactory = require('../../install/opentimelineio');
const { expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {opentimelineioFactory.CustomEmbindModule}
 */
let opentimelineio;


beforeAll(async () => {
    // opentime = await opentimeFactory();
    opentimelineio = await opentimelineioFactory();
    // console.log(opentimelineio.demangle('NSt3__28optionalIN8opentime4v1_09TimeRangeEEE'))
});

test('test_contructors', () => {
    const rt = new opentimelineio.RationalTime(123, 24)
    const tr = new opentimelineio.TimeRange(rt, rt)
    const mr = new opentimelineio.ExternalReference()
    mr.available_range = new opentimelineio.TimeRange(rt, new opentimelineio.RationalTime(10, 24))
    mr.target_url = '/var/tmp/test.mov'

    const clip = new opentimelineio.Clip('myclip1', mr, tr)
    expect(clip.name).toEqual('myclip1')
    expect(clip.source_range).toEqual(tr)
    expect(clip.media_reference().is_equivalent_to(mr)).toEqual(true)

    encoded = opentimelineio.serialize_json_to_string(clip)
    decoded = opentimelineio.Clip.from_json_string(encoded)
    expect(clip.is_equivalent_to(decoded)).toEqual(true)
    rt.delete()
    tr.delete()
    mr.delete()
    clip.delete()
    decoded.delete()
})
