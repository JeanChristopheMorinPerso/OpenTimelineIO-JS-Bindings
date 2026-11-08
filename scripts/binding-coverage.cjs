const fs = require('node:fs')
const path = require('node:path')
const { execFileSync } = require('node:child_process')

// Keep JavaScript entry-point evidence separate from LLVM function coverage:
// C++ may call an implementation without entering any JavaScript binding.
module.exports = function writeBindingCoverage(coverageDir) {
    const rows = new Map()
    const rawDir = path.join(coverageDir, 'raw')
    const root = path.dirname(coverageDir)
    const sources = new Map()
    // LCOV supplies the files already rendered by genhtml. Registration stacks
    // supply the precise overload location; LLVM resolves their Wasm addresses.
    const coveredFiles = new Set(fs.readFileSync(path.join(coverageDir, 'cpp.lcov'), 'utf8')
        .split('\n').filter(line => line.startsWith('SF:')).map(line => line.slice(3)))
    // Jest formats Wasm offsets as one-based decimal columns; Node uses hex.
    const stackAddresses = stack => [...stack.matchAll(/wasm:\/\/wasm\/[^)]+?(?:wasm-function\[\d+\]:(0x[\da-f]+)|:1:(\d+))\)/g)]
        .map(match => match[1] || `0x${(Number(match[2]) - 1).toString(16)}`)
    for (const file of fs.readdirSync(rawDir).filter(file => file.endsWith('.bindings.json'))) {
        const data = JSON.parse(fs.readFileSync(path.join(rawDir, file), 'utf8'))
        const addresses = [...new Set(data.bindings.flatMap(binding => stackAddresses(binding.registrationStack)))]
        const locations = new Map()
        if (addresses.length) {
            const output = execFileSync(path.join(root, 'emsdk/upstream/bin/llvm-symbolizer'), [
                `--obj=${path.join(root, 'install', `${data.module}.wasm`)}`, '--output-style=JSON',
            ], { input: addresses.join('\n') + '\n', encoding: 'utf8', maxBuffer: 16 * 1024 * 1024 })
            for (const line of output.trim().split('\n')) {
                const resolved = JSON.parse(line)
                locations.set(resolved.Address, resolved.Symbol)
            }
        }
        for (const binding of data.bindings) {
            const key = JSON.stringify([data.module, binding.name, binding.arguments, binding.returns])
            const frames = stackAddresses(binding.registrationStack)
                .flatMap(address => locations.get(address) || [])
            const source = frames.find(frame => frame.Line > 0 && coveredFiles.has(frame.FileName))
                || frames.find(frame => frame.Line > 0 && fs.existsSync(frame.FileName))
            if (!rows.has(key)) {
                const { registrationStack, ...signature } = binding
                void registrationStack
                let sourceLink
                if (source) {
                    const relative = path.relative(root, source.FileName)
                    if (coveredFiles.has(source.FileName)) {
                        sourceLink = `cpp/${relative}.gcov.html#L${source.Line}`
                    } else {
                        // SDK-generated bindings have no LCOV page; include their
                        // exact source snapshot rather than guess a project line.
                        const id = sources.size
                        if (!sources.has(source.FileName)) {
                            sources.set(source.FileName, { id, relative })
                        }
                        sourceLink = `binding-sources/${sources.get(source.FileName).id}.html#L${source.Line}`
                    }
                }
                rows.set(key, { ...signature, source: source && { file: path.relative(root, source.FileName), line: source.Line, href: sourceLink }, module: data.module, calls: 0, cppCalls: 0, completed: 0, threw: 0, argumentCounts: {}, tests: new Set() })
            }
            const row = rows.get(key)
            for (const counter of ['calls', 'cppCalls', 'completed', 'threw']) {
                row[counter] += binding[counter]
            }
            for (const [arity, count] of Object.entries(binding.argumentCounts)) {
                row.argumentCounts[arity] = (row.argumentCounts[arity] || 0) + count
            }
            if (binding.calls > 0) {
                row.tests.add(data.testFile)
            }
        }
    }
    const bindings = [...rows.values()].sort((a, b) => a.module.localeCompare(b.module) || a.name.localeCompare(b.name) || a.arguments.length - b.arguments.length)
        .map(row => ({ ...row, tests: [...row.tests].sort() }))
    const entered = bindings.filter(row => row.calls > 0).length
    const reached = bindings.filter(row => row.cppCalls > 0).length
    fs.writeFileSync(path.join(coverageDir, 'bindings.json'), JSON.stringify(bindings, null, 2))
    const escape = text => String(text).replace(/[&<>"']/g, char => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', '\'': '&#39;' })[char])
    if (sources.size) {
        fs.mkdirSync(path.join(coverageDir, 'binding-sources'), { recursive: true })
        for (const [file, source] of sources) {
            const lines = fs.readFileSync(file, 'utf8').split('\n')
                .map((line, index) => `<div id="L${index + 1}"><a href="#L${index + 1}">${index + 1}</a> ${escape(line)}</div>`).join('\n')
            fs.writeFileSync(path.join(coverageDir, 'binding-sources', `${source.id}.html`), `<!doctype html><html><head><meta charset="utf-8"><title>${escape(source.relative)}</title><style>pre{font:13px monospace;white-space:pre-wrap}div:target{background:#fff1ab}a{color:#64748b}</style></head><body><a href="../bindings.html">Binding report</a><h2>${escape(source.relative)}</h2><p>SDK registration source snapshot; no LCOV coverage is available for this file.</p><pre>${lines}</pre></body></html>`)
        }
    }
    const body = bindings.map(row => `<tr class="${row.cppCalls ? 'hit' : 'miss'}"><td>${escape(row.module)}</td><td><code>${row.source ? `<a href="${escape(row.source.href)}">${escape(row.name)}(${row.arguments.map(escape).join(', ')})</a>` : `${escape(row.name)}(${row.arguments.map(escape).join(', ')})`}</code><small>→ ${escape(row.returns)}</small><small>${row.source ? `${escape(row.source.file)}:${row.source.line}` : 'Source location unavailable'}</small></td><td>${row.arguments.length}<small>${Object.entries(row.argumentCounts).map(([arity, count]) => `${arity} supplied: ${count} calls`).join('<br>')}</small></td><td>${row.calls}</td><td>${row.cppCalls}</td><td>${row.completed}</td><td>${row.threw}</td><td>${row.tests.map(escape).join('<br>') || '—'}</td></tr>`).join('\n')
    fs.writeFileSync(path.join(coverageDir, 'bindings.html'), `<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>JavaScript binding overload coverage</title>
<style>body{font:14px system-ui,sans-serif;color:#263342;margin:32px;background:#fafbfc}h1{font-size:26px}p{max-width:1000px;line-height:1.6}a{color:#155bb5}table{border-collapse:collapse;background:white;width:100%;font-size:13px}th{background:#edf1f5;text-align:left}th,td{padding:10px;border-bottom:1px solid #dfe5eb;vertical-align:top}td:nth-child(n+3):nth-child(-n+7){text-align:right;font-variant-numeric:tabular-nums}small{display:block;color:#67717e;margin-top:5px}.miss td:first-child{border-left:4px solid #c77c77}.hit td:first-child{border-left:4px solid #499676}code{white-space:normal}header{margin-bottom:24px}</style></head>
<body><header><h1>JavaScript binding overload coverage</h1>
<p><strong>${entered} / ${bindings.length}</strong> registered signatures entered from JavaScript · <strong>${reached} / ${bindings.length}</strong> reached their C++ invoker.</p>
<p>Each row is a registered Embind method, free function, or constructor signature, scoped to its module. C++ invoker calls occur after argument conversion. Completed means the call returned normally; thrown calls can include conversion errors or C++ exceptions. These counts do not prove result assertions or all argument values were tested.</p>
<p>Signature links open the binding registration line in the LCOV source report. SDK-generated registrations link to a source snapshot instead. These are declaration locations, not inferred upstream implementation locations.</p>
<p><a href="cpp/index.html">C++ source, function and branch coverage</a> · <a href="javascript/lcov-report/index.html">Generated JavaScript coverage</a> · <a href="bindings.json">Download overload evidence</a></p>
<p>Property accessors, handwritten JavaScript wrappers and JavaScript subclass constructors are not enumerated by this invoker factory. Invalid argument counts rejected by the overload selector do not enter a registered signature. Test files identify where calls occurred, not which assertions validated them. No upstream function hit is inferred to be a JavaScript call.</p></header>
<table><thead><tr><th>Module</th><th>Registered signature</th><th>Args</th><th>JS entries</th><th>C++ invoker</th><th>Completed</th><th>Threw</th><th>Calling test files</th></tr></thead><tbody>${body}</tbody></table></body></html>`)
    return { registered: bindings.length, entered, reached }
}
