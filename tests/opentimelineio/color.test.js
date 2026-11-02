const factory = require('../../install/opentimelineio')
const { beforeAll, expect, test } = require('@jest/globals')

let otio
beforeAll(async () => {
    otio = await factory()
})

test.each([
    [0xff00ff00, '#00ff00ff'],
    [0xffff0000, '#0000ffff'],
    [0x80362cd2, '#d22c3680']
])('packed ABGR %i preserves channel order and alpha', (packed, hex) => {
    const decoded = otio.Color.from_abgr_int(packed)
    const encoded = otio.Color.from_hex(hex)
    try {
        expect(decoded.to_hex()).toBe(hex)
        expect(decoded.to_abgr_int()).toBe(packed)
        expect(encoded.to_abgr_int()).toBe(packed)
    } finally {
        decoded.delete()
        encoded.delete()
    }
})

test('Color exposes RGBA, conversions, and independently owned named colors', () => {
    const color = new otio.Color(0.25, 0.5, 0.75, 0.125, 'custom')
    const hex = otio.Color.from_hex('#12569abc')
    const integers = otio.Color.from_int_list([16, 80, 192, 128], 8)
    const floats = otio.Color.from_float_list([0.1, 0.3, 0.7])
    const red = otio.Color.RED
    const anotherRed = otio.Color.RED
    const transparent = otio.Color.TRANSPARENT

    try {
        expect(color.to_rgba_float_list()).toEqual([0.25, 0.5, 0.75, 0.125])
        expect(color.name).toBe('custom')
        expect(color.to_rgba_int_list()).toEqual([63, 127, 191, 31])
        expect(color.to_rgba_int_list(8)).toEqual([63, 127, 191, 31])
        expect(color.to_rgba_int_list(10)).toEqual([255, 511, 767, 127])
        color.r = 0.5
        color.g = 0.25
        color.b = 1
        color.a = 1
        color.name = 'edited'
        expect(color.to_rgba_float_list()).toEqual([0.5, 0.25, 1, 1])
        expect(color.name).toBe('edited')
        expect(hex.to_hex()).toBe('#12569abc')
        expect(hex.to_rgba_int_list(8)).toEqual([18, 86, 154, 188])
        expect(integers.to_rgba_float_list()).toEqual([16 / 255, 80 / 255, 192 / 255, 128 / 255])
        expect(floats.to_rgba_float_list()).toEqual([0.1, 0.3, 0.7, 1])
        expect(red.equal(anotherRed)).toBe(true)
        red.g = 0.5
        expect(red.equal(anotherRed)).toBe(false)
        expect(anotherRed.to_rgba_float_list()).toEqual([1, 0, 0, 1])
        expect(transparent.to_rgba_float_list()).toEqual([0, 0, 0, 0])
        for (const [name, expected] of [
            ['PINK', [1, 0, 1, 1]],
            ['RED', [1, 0, 0, 1]],
            ['ORANGE', [1, 0.5, 0, 1]],
            ['YELLOW', [1, 1, 0, 1]],
            ['GREEN', [0, 1, 0, 1]],
            ['CYAN', [0, 1, 1, 1]],
            ['BLUE', [0, 0, 1, 1]],
            ['PURPLE', [0.5, 0, 0.5, 1]],
            ['MAGENTA', [1, 0, 1, 1]],
            ['BLACK', [0, 0, 0, 1]],
            ['WHITE', [1, 1, 1, 1]],
            ['TRANSPARENT', [0, 0, 0, 0]]
        ]) {
            const namedColor = otio.Color[name]
            try {
                expect(namedColor.to_rgba_float_list()).toEqual(expected)
                expect(namedColor.name).toBe(name[0] + name.slice(1).toLowerCase())
            } finally {
                namedColor?.delete()
            }
        }
        expect(() => otio.Color.from_float_list([0.1, 0.2])).toThrow()
    } finally {
        for (const value of [color, hex, integers, floats, red, anotherRed, transparent]) {
            value?.delete()
        }
    }
})

test.each(['Item', 'Clip', 'Track'])('%s stores optional colors by value and round-trips them', type => {
    const item = new otio[type]('colored')
    const color = new otio.Color(0.2, 0.4, 0.8, 0.6, 'label')
    let returned
    let decoded
    let decodedColor
    try {
        expect(item.color).toBeUndefined()
        item.color = color
        color.r = 1
        returned = item.color
        expect(returned.to_rgba_float_list()).toEqual([0.2, 0.4, 0.8, 0.6])
        returned.g = 1
        const encoded = item.to_json_string()
        expect(JSON.parse(encoded).color).toEqual({
            OTIO_SCHEMA: 'Color.1', r: 0.2, g: 0.4, b: 0.8, a: 0.6, name: 'label'
        })
        decoded = otio.SerializableObject.from_json_string(encoded)
        decodedColor = decoded.color
        expect(decodedColor.to_rgba_float_list()).toEqual([0.2, 0.4, 0.8, 0.6])
        item.color = undefined
        expect(item.color).toBeUndefined()
        expect(JSON.parse(item.to_json_string()).color).toBeNull()
    } finally {
        decodedColor?.delete()
        decoded?.delete()
        returned?.delete()
        color.delete()
        item.delete()
    }
})

test('Color, V2d, and Box2d survive nested metadata and standalone Color serialization', () => {
    const color = new otio.Color(0.125, 0.375, 0.75, 0.5, 'metadata')
    const point = new otio.V2d(-3, 7)
    const max = new otio.V2d(11, 19)
    const bounds = new otio.Box2d(point, max)
    const owner = new otio.SerializableObjectWithMetadata('owner', {
        nested: [color, { point, bounds }]
    })
    let decoded
    let standalone
    const returned = []
    try {
        const encodedColor = otio.serialize_json_to_string(color, { indent: 0 })
        expect(JSON.parse(encodedColor)).toEqual({
            OTIO_SCHEMA: 'Color.1', r: 0.125, g: 0.375, b: 0.75, a: 0.5, name: 'metadata'
        })
        standalone = otio.deserialize_json_from_string(encodedColor)
        expect(standalone).toBeInstanceOf(otio.Color)
        expect(standalone.to_rgba_float_list()).toEqual([0.125, 0.375, 0.75, 0.5])
        expect(color.name).toBe('metadata')
        decoded = otio.SerializableObject.from_json_string(owner.to_json_string())
        for (const value of [owner, decoded]) {
            const metadata = value.get_metadata().nested
            returned.push(metadata[0], metadata[1].point, metadata[1].bounds)
            expect(metadata[0]).toBeInstanceOf(otio.Color)
            expect(metadata[0].to_rgba_float_list()).toEqual([0.125, 0.375, 0.75, 0.5])
            expect(metadata[1].point.get(0)).toBe(-3)
            expect(metadata[1].point.get(1)).toBe(7)
            expect(metadata[1].bounds.equal(bounds)).toBe(true)
        }
    } finally {
        for (const value of returned) value.delete()
        standalone?.delete()
        decoded?.delete()
        owner.delete()
        bounds.delete()
        max.delete()
        point.delete()
        color.delete()
    }
})
