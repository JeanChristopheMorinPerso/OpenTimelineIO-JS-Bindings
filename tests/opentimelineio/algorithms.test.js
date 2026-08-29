const opentimelineioFactory = require('../../install/opentimelineio')
const { beforeAll, expect, test } = require('@jest/globals')

/**
 * @type {opentimelineioFactory.CustomEmbindModule}
 */
let opentimelineio

beforeAll(async () => {
    opentimelineio = await opentimelineioFactory()
})

test('release_to_schema_version_map returns nested JavaScript objects', () => {
    const versions = opentimelineio.release_to_schema_version_map()

    expect(versions['0.14.0'].Clip).toBe(1n)
    expect(versions['0.15.0'].Clip).toBe(2n)
    expect(versions['0.16.0.dev1'].Marker).toBe(2n)
})

test('neighbors_of returns JavaScript neighbors', () => {
    const first = new opentimelineio.Clip('first')
    const second = new opentimelineio.Clip('second')
    const children = new opentimelineio.ComposableVector()
    children.push_back(first)
    children.push_back(second)
    const track = new opentimelineio.Track('track', children)

    const neighbors = track.neighbors_of(
        first,
        opentimelineio.TrackNeighborGapPolicy.never
    )

    expect(neighbors[0]).toBeNull()
    expect(neighbors[1].name).toBe('second')

    track.delete()
    children.delete()
    first.delete()
    second.delete()
})

test('flatten_stack accepts both a Stack and a TrackVector', () => {
    const stack = new opentimelineio.Stack()
    const flattenedStack = opentimelineio.flatten_stack(stack)
    expect(flattenedStack).toBeInstanceOf(opentimelineio.Track)

    const tracks = new opentimelineio.TrackVector()
    const flattenedTracks = opentimelineio.flatten_stack(tracks)
    expect(flattenedTracks).toBeInstanceOf(opentimelineio.Track)

    flattenedStack.delete()
    flattenedTracks.delete()
    tracks.delete()
    stack.delete()
})
