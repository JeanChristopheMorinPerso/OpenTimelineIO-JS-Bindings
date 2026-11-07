const { afterAll, expect } = require('@jest/globals')
const path = require('node:path')

/* global process */
afterAll(() => {
    const modules = globalThis.__otioCoverageModules || []
    if (modules.length === 0) {
        throw new Error('No instrumented Wasm modules were loaded')
    }

    const testFile = path.relative(process.cwd(), expect.getState().testPath).replace(/[\\/]/g, '_')
    for (const [index, module] of modules.entries()) {
        const filename = path.join(process.env.OTIO_COVERAGE_DIR, `${testFile}-${process.pid}-${index}.profraw`)
        const pointer = module.stringToNewUTF8(filename)
        try {
            module.___llvm_profile_set_filename(pointer)
            if (module.___llvm_profile_write_file() !== 0) {
                throw new Error(`Failed to write C++ coverage profile: ${filename}`)
            }
        } finally {
            module._free(pointer)
        }
    }
})
