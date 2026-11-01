const opentimelineioFactory = require('../../install/opentimelineio');
const { expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {opentimelineioFactory.CustomEmbindModule}
 */
let opentimelineio;

beforeAll(async () => {
    opentimelineio = await opentimelineioFactory();
});

test('Imath default constructors match Python zero initialization', () => {
    const vector = new opentimelineio.V2d()
    const origin = new opentimelineio.V2d(0, 0)
    const scalar = new opentimelineio.V2d(12)
    const minimum = new opentimelineio.V2d(-3, 7)
    const maximum = new opentimelineio.V2d(11, 19)
    const box = new opentimelineio.Box2d()
    const originBox = new opentimelineio.Box2d(origin)
    const bounds = new opentimelineio.Box2d(minimum, maximum)
    const center = bounds.center()

    try {
        expect([vector.get(0), vector.get(1)]).toEqual([0, 0])
        expect([scalar.get(0), scalar.get(1)]).toEqual([12, 12])
        expect(box.equal(originBox)).toBe(true)
        expect(box.intersects(originBox)).toBe(true)
        expect([center.get(0), center.get(1)]).toEqual([4, 13])
    } finally {
        for (const value of [vector, origin, scalar, minimum, maximum, box, originBox, bounds, center]) {
            value.delete()
        }
    }
})

test('test_composable_vector_iteration', () => {
    const vector = new opentimelineio.ComposableVector()
    const composable1 = new opentimelineio.Composable('composable1')
    const composable2 = new opentimelineio.Composable('composable2')

    expect(Array.from(vector)).toEqual([])

    vector.push_back(composable1)
    vector.push_back(composable2)
    expect(Array.from(vector, composable => composable.name)).toEqual([
        'composable1',
        'composable2'
    ])

    vector.delete()
    composable1.delete()
    composable2.delete()
})

test('test_track_vector_iteration', () => {
    const vector = new opentimelineio.TrackVector()
    const track1 = new opentimelineio.Track('track1')
    const track2 = new opentimelineio.Track('track2')

    expect(Array.from(vector)).toEqual([])

    vector.push_back(track1)
    vector.push_back(track2)
    expect(Array.from(vector, track => track.name)).toEqual([
        'track1',
        'track2'
    ])

    vector.delete()
    track1.delete()
    track2.delete()
})
