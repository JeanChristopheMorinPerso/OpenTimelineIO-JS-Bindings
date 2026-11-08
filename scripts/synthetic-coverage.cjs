const path = require('node:path')

// This is API exercise evidence, not C++ line execution. A shared declaration
// is covered only when every registered signature in every module reached C++.
module.exports = function syntheticCoverage(bindings, root) {
    const files = new Map()
    for (const binding of bindings) {
        if (!binding.source || !binding.source.href.startsWith('cpp/')) {
            continue
        }
        const file = binding.source.file
        if (!files.has(file)) {
            files.set(file, { functions: [], lines: new Map() })
        }
        const entry = files.get(file)
        const line = binding.source.line
        const hit = binding.cppCalls > 0 ? 1 : 0
        const name = `${binding.module}:${binding.name}(${binding.arguments.join(', ')}) -> ${binding.returns}`
        entry.functions.push({ line, name, hit })
        entry.lines.set(line, Math.min(entry.lines.get(line) ?? 1, hit))
    }
    return [...files].sort(([a], [b]) => a.localeCompare(b)).map(([file, entry]) => {
        const lines = [...entry.lines].sort(([a], [b]) => a - b)
        return [
            'TN:bindings_synthetic',
            `SF:${path.join(root, file)}`,
            ...entry.functions.map(fn => `FN:${fn.line},${fn.name}`),
            ...entry.functions.map(fn => `FNDA:${fn.hit},${fn.name}`),
            `FNF:${entry.functions.length}`,
            `FNH:${entry.functions.filter(fn => fn.hit).length}`,
            ...lines.map(([line, hit]) => `DA:${line},${hit}`),
            `LF:${lines.length}`,
            `LH:${lines.filter(([, hit]) => hit).length}`,
            'end_of_record',
        ].join('\n')
    }).join('\n') + '\n'
}
