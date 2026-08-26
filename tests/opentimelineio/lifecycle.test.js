const opentimelineioFactory = require('../../install/opentimelineio')
const { beforeAll, expect, test } = require('@jest/globals')

/**
 * @type {opentimelineioFactory.CustomEmbindModule}
 */
let opentimelineio

beforeAll(async () => {
    opentimelineio = await opentimelineioFactory()
})

test('delete synchronously destroys a JavaScript-derived C++ wrapper', () => {
    // Subclassing is essential to this test. Embind only preserves pointers and
    // sends __destruct notifications for C++ wrappers created by JS subclasses;
    // ordinary C++ instances follow a different deletion path.
    class JavaScriptProbe extends opentimelineio.SerializableObject.extend(
        'JavaScriptProbe',
        {}
    ) {}

    const before = opentimelineio._serializable_object_wrapper_destructor_count()
    const probe = new JavaScriptProbe()

    probe.delete()

    expect(probe.isDeleted()).toBe(true)
    expect(() => probe.schema_name()).toThrow(/deleted object/)
    expect(opentimelineio._serializable_object_wrapper_destructor_count()).toBe(
        before + 1
    )
})

test('a C++ owner keeps a JavaScript-derived wrapper alive only while needed', () => {
    class RetainedJavaScriptProbe extends opentimelineio.SerializableObject.extend(
        'RetainedJavaScriptProbe',
        {}
    ) {}

    const before = opentimelineio._serializable_object_wrapper_destructor_count()
    const probe = new RetainedJavaScriptProbe()
    const children = new opentimelineio.SOVector()
    children.push_back(probe)
    // SerializableCollection copies these raw pointers into C++ Retainers, so
    // the collection—not the temporary SOVector—becomes the probe's C++ owner.
    const collection = new opentimelineio.SerializableCollection('owner', children)

    probe.delete()
    // delete() released our reference, but the collection still owns the C++
    // object. Embind must keep the JavaScript subclass usable for C++ callbacks.
    expect(probe.isDeleted()).toBe(false)
    expect(probe._get_dynamic_fields()).toEqual({})
    expect(opentimelineio._serializable_object_wrapper_destructor_count()).toBe(
        before
    )

    children.delete()
    // Destroying the collection releases its C++ Retainer. This is the final
    // owner, so the probe's C++ destructor must run now.
    collection.delete()

    expect(probe.isDeleted()).toBe(true)
    expect(() => probe.schema_name()).toThrow(/deleted object/)
    expect(opentimelineio._serializable_object_wrapper_destructor_count()).toBe(
        before + 1
    )
})

test('actual destruction invalidates a metadata wrapper', () => {
    class MetadataProbe extends opentimelineio.SerializableObjectWithMetadata.extend(
        'MetadataProbe',
        {}
    ) {}

    const probe = new MetadataProbe()
    probe.name = 'probe'
    const children = new opentimelineio.SOVector()
    children.push_back(probe)
    const collection = new opentimelineio.SerializableCollection('owner', children)

    probe.delete()
    // The collection's C++ retainer postpones actual destruction and handle
    // invalidation until collection.delete() below.
    expect(probe.isDeleted()).toBe(false)
    expect(probe.name).toBe('probe')

    children.delete()
    collection.delete()

    expect(probe.isDeleted()).toBe(true)
    expect(() => probe.schema_name()).toThrow(/deleted object/)
})

test('destroyed wrapper addresses can be reused', () => {
    class ReusedAddressProbe extends opentimelineio.SerializableObject.extend(
        'ReusedAddressProbe',
        {}
    ) {}

    const before = opentimelineio._serializable_object_wrapper_destructor_count()

    for (let i = 0; i < 100; ++i) {
        const probe = new ReusedAddressProbe()
        probe.delete()
        expect(probe.isDeleted()).toBe(true)
    }

    expect(opentimelineio._serializable_object_wrapper_destructor_count()).toBe(
        before + 100
    )
})
