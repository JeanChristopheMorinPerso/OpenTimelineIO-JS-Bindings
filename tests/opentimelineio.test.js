const factory = require('../install/opentimelineio');
const { expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {factory.CustomEmbindModule}
 */
let lib;

beforeAll(async () => {
    lib = await factory();
});

function expectError(expectedMessage, callback) {
    try {
        callback();
        throw new Error('Expected function to throw!');
    } catch (err) {
        expect(err).toBeInstanceOf(WebAssembly.Exception)
        expect(err.message[1]).toEqual(expectedMessage)
    }
}

test('asd', () => {
    const av = new lib.AnyVector();
    av.push_back("a value set from JS!")
    expect(av.get(0)).toEqual("a value set from JS!")

    expectError("failed to open file for reading", () => {
        lib.SerializableObject.from_json_file("non existent file")
    })

    expectError("JSON parse error", () => {
        lib.SerializableObject.from_json_string("invalid string")
    })
})
