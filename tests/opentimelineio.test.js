const factory = require('../install/opentimelineio');
const { expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {factory.CustomEmbindModule}
 */
let lib;

beforeAll(async () => {
    lib = await factory();
    // debugger
});

function expectError(expectedMessage, callback) {
    try {
        callback();
        throw new Error('Expected function to throw!');
    } catch (err) {
        // console.log(err.is(lib.asm.__cpp_exception))
        expect(err).toBeInstanceOf(WebAssembly.Exception)
        expect(err.message[1]).toEqual(expectedMessage)
    }
}

test('asd', () => {
    const av = new lib.AnyVector();
    av.push_back("a value set from JS!")
    expect(av.get(0)).toEqual("a value set from JS!")
    av.delete()

    const asd = new lib.SerializableObject()
    expect(asd.schema_name()).toEqual("SerializableObject")
    asd.delete()

    expectError("No such file or directory: non existent file", () => {
        lib.SerializableObject.from_json_file("non existent file")
    })

    expectError("JSON parse error while reading: JSON parse error on input string: Invalid value. (line 1, column 0)", () => {
        lib.SerializableObject.from_json_string("aasd")
    })

    expectError("JSON parse error while reading: JSON parse error on input string: The document is empty. (line 1, column 0)", () => {
        lib.SerializableObject.from_json_string("")
    })

    expectError("type mismatch while decoding: Expected a SerializableObject*, found object of type 'opentimelineio::v1_0::AnyDictionary' instead", () => {
        lib.SerializableObject.from_json_string("{}")
    })
})

test('SerializableCollection', () => {
    const so1 = new lib.SerializableObject();
    const so2 = new lib.SerializableObject();
    new lib.SerializableCollection();
    const vec = new lib.SOVector()
    vec.push_back(so1)
    vec.push_back(so2)
    new lib.SerializableCollection("myname", vec);
})

test("Imath", () => {
    console.log(lib.V2d.baseTypeLowest)
})

test("stack algo", () => {
    const track_d = lib.deserialize_json_from_string(`{
        "OTIO_SCHEMA": "Track.1",
        "children": [
            {
                "OTIO_SCHEMA": "Clip.1",
                "effects": [],
                "markers": [],
                "media_reference": null,
                "metadata": {},
                "name": "A",
                "source_range": {
                    "OTIO_SCHEMA": "TimeRange.1",
                    "duration": {
                        "OTIO_SCHEMA": "RationalTime.1",
                        "rate": 24,
                        "value": 50
                    },
                    "start_time": {
                        "OTIO_SCHEMA": "RationalTime.1",
                        "rate": 24,
                        "value": 0.0
                    }
                }
            },
            {
                "OTIO_SCHEMA": "Clip.1",
                "effects": [],
                "markers": [],
                "media_reference": null,
                "metadata": {},
                "name": "B",
                "source_range": {
                    "OTIO_SCHEMA": "TimeRange.1",
                    "duration": {
                        "OTIO_SCHEMA": "RationalTime.1",
                        "rate": 24,
                        "value": 50
                    },
                    "start_time": {
                        "OTIO_SCHEMA": "RationalTime.1",
                        "rate": 24,
                        "value": 0.0
                    }
                }
            },
            {
                "OTIO_SCHEMA": "Clip.1",
                "effects": [],
                "markers": [],
                "media_reference": null,
                "metadata": {},
                "name": "C",
                "source_range": {
                    "OTIO_SCHEMA": "TimeRange.1",
                    "duration": {
                        "OTIO_SCHEMA": "RationalTime.1",
                        "rate": 24,
                        "value": 50
                    },
                    "start_time": {
                        "OTIO_SCHEMA": "RationalTime.1",
                        "rate": 24,
                        "value": 0.0
                    }
                }
            }
        ],
        "effects": [],
        "kind": "Video",
        "markers": [],
        "metadata": {},
        "name": "Sequence1",
        "source_range": null
    }`)

    const children = track_d.find_children()
    console.log(children)
    // https://github.com/AcademySoftwareFoundation/OpenTimelineIO/blob/80d341bfd74b56d2011204fa2d0f4dfe02ff821b/tests/test_stack_algo.py#L449
    // stack = otio.schema.Stack(children=[
    //     self.trackABC,
    //     self.track_d
    // ])
    // flat_track = otio.algorithms.flatten_stack(stack)
    // self.assertJsonEqual(
    //     flat_track[:],
    //     self.trackABC[:]
    // )
})
