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

test('SOWithMetadata', () => {
    const so = new lib.SerializableObjectWithMetadata("myobject", { "mykey": "myvalue" })
    // Causes a "RuntimeError: memory access out of bounds" error.
    // console.log(so.metadata)
    // console.log(so.metadata.get('mykey'))
    console.log('value of mykey:', so.metadata['mykey'])
    so.metadata['asd'] = 'asdasasd'
    console.log('value of asd:', so.metadata['asd'])
    // console.log('value of asd2:', so.metadata.get('asd2'))
    so.delete()
})
