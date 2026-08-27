const opentimelineioFactory = require('../../install/opentimelineio');
const { expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {opentimelineioFactory.CustomEmbindModule}
 */
let opentimelineio;

beforeAll(async () => {
    opentimelineio = await opentimelineioFactory();
});

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
