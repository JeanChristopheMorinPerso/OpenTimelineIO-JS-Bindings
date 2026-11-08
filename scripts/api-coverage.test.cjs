const { test } = require('node:test')
const assert = require('node:assert/strict')
const { compare } = require('./api-coverage.cjs')

test('exposure includes inherited properties but respects static placement, setters and namespaces', () => {
    class Base {
        get readonly() { throw new Error('Inventory must not call getters') }
        inherited() {}
    }
    class Clip extends Base {
        static available() {}
        static get DEFAULT_MEDIA_KEY() { throw new Error('Do not read static properties') }
        instanceOnly() {}
    }
    const member = (name, kind, extra = {}) => ({
        group: 'schema', name: `Clip.${name}`, className: 'Clip', member: name, kind, ...extra,
    })
    const result = compare({ rows: [
        member('inherited', 'method'), member('readonly', 'property', { writable: true }),
        member('available', 'static method'), member('instanceOnly', 'static method'),
        member('absent', 'method'), member('__iter__', 'method', { protocol: true }),
        { group: 'bundle', name: 'read_otioz', kind: 'function' },
        member('DEFAULT_MEDIA_KEY', 'static property', { writable: false }),
        { group: 'opentime', name: 'from_frames', kind: 'function' },
    ] }, {
        opentimelineio: { Clip, read_otioz() {} },
        opentime: { RationalTime: { from_frames() {} } },
    })
    assert.deepEqual(result.map(row => row.status), [
        'exposed', 'shape mismatch', 'exposed', 'missing', 'missing',
        'language-specific', 'missing', 'exposed', 'missing',
    ])
    assert.equal(result.at(-1).alternative, 'RationalTime.from_frames')
    assert.match(result[1].note, /no setter/)
    const withNamespace = compare({ rows: [{ group: 'bundle', name: 'read_otioz', kind: 'function' }] },
        { opentimelineio: { bundle: { read_otioz() {} } } })
    assert.equal(withNamespace[0].status, 'exposed')
})

test('flattened nested types and bundle class members resolve in their own namespace', () => {
    class Policy {}
    Policy.never = 0
    class ReadOptions {
        get extract_path() { throw new Error('Do not read properties') }
        set extract_path(value) { void value }
    }
    const rows = compare({ rows: [
        { group: 'core', name: 'Track.NeighborGapPolicy', kind: 'class' },
        { group: 'core', name: 'Track.NeighborGapPolicy.never', className: 'Track.NeighborGapPolicy', member: 'never', kind: 'constant' },
        { group: 'core', name: 'Track.NeighborGapPolicy.absent', className: 'Track.NeighborGapPolicy', member: 'absent', kind: 'constant' },
        { group: 'bundle', name: 'ReadOptions.extract_path', className: 'ReadOptions', member: 'extract_path', kind: 'property', writable: true },
    ] }, { opentimelineio: { TrackNeighborGapPolicy: Policy, bundle: { ReadOptions } } })
    assert.deepEqual(rows.map(row => row.status), ['exposed', 'exposed', 'missing', 'exposed'])
    assert.equal(rows[1].target, 'TrackNeighborGapPolicy.never')
    assert.equal(rows[3].target, 'bundle.ReadOptions.extract_path')
})
