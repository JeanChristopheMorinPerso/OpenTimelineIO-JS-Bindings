/* global Module */

// Each Jest suite can create multiple independent Wasm instances.
globalThis.__otioCoverageModules ??= []
globalThis.__otioCoverageModules.push(Module)
