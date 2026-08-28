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
    const rt = new opentimelineio.RationalTime(123, 24)
    const tr = new opentimelineio.TimeRange(rt, rt)
    const mr = new opentimelineio.ExternalReference()
    mr.available_range = new opentimelineio.TimeRange(rt, new opentimelineio.RationalTime(10, 24))
    mr.target_url = '/var/tmp/test.mov'

    const clip = new opentimelineio.Clip('myclip1', mr, tr, { 'asd': 'value' })
    expect(clip.name).toEqual('myclip1')
    expect(clip.source_range).toEqual(tr)
    expect(clip.media_reference().is_equivalent_to(mr)).toEqual(true)

    const encoded = opentimelineio.serialize_json_to_string(clip)
    const decoded = opentimelineio.SerializableObject.from_json_string(encoded)
    expect(clip.is_equivalent_to(decoded)).toEqual(true)
    console.log(decoded.get_metadata());
    rt.delete()
    tr.delete()
    mr.delete()
    clip.delete()
    decoded.delete()
})

test('test_find_clips', () => {
    const clip = new opentimelineio.Clip('test_clip')
    clip.delete()
})

test('test_media_references', () => {
    const primary = new opentimelineio.ExternalReference()
    primary.target_url = 'primary.mov'
    const proxy = new opentimelineio.MissingReference()
    const clip = new opentimelineio.Clip('clip')

    clip.set_media_references({ primary, proxy }, 'proxy')

    const references = clip.media_references()
    expect(Object.keys(references).sort()).toEqual(['primary', 'proxy'])
    expect(references.primary.is_equivalent_to(primary)).toBe(true)
    expect(references.proxy.is_equivalent_to(proxy)).toBe(true)
    expect(clip.active_media_reference_key).toBe('proxy')

    primary.delete()
    proxy.delete()
    clip.delete()
})

test('test_optional_image_bounds', () => {
    const reference = new opentimelineio.MissingReference()
    const bounds = new opentimelineio.Box2d(
        new opentimelineio.V2d(1, 2),
        new opentimelineio.V2d(3, 4)
    )

    expect(reference.available_image_bounds).toBeUndefined()
    reference.available_image_bounds = bounds
    const returnedBounds = reference.available_image_bounds
    expect(returnedBounds.equal(bounds)).toBe(true)
    returnedBounds.delete()
    reference.available_image_bounds = undefined
    expect(reference.available_image_bounds).toBeUndefined()

    reference.delete()
    bounds.delete()
})
