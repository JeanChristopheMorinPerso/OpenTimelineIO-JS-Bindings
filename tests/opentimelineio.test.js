const factory = require('../install/opentimelineio');
const { expect, test, beforeAll } = require('@jest/globals');

/**
 * @type {factory.CustomEmbindModule}
 */
let lib;

beforeAll(async () => {
    lib = await factory();
});

test('asd', () => {
    const av = new lib.AnyVector();
    av.push_back("a value set from JS!")
    expect(av.get(0)).toEqual("a value set from JS!")
})
