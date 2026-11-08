// API exposure is separate from execution coverage: no tests or instrumentation
// are needed, and a matching name never proves overload or behavioral parity.
const fs = require('node:fs')
const path = require('node:path')
const { execFileSync } = require('node:child_process')
const { createHash } = require('node:crypto')

/* global __dirname, process */
const root = path.resolve(__dirname, '..')
const output = path.join(root, 'api-coverage')
const upstream = path.join(root, 'deps/OpenTimelineIO')

function descriptor(object, name) {
    for (let current = object; current && current !== Object.prototype && current !== Function.prototype;
        current = Object.getPrototypeOf(current)) {
        const found = Object.getOwnPropertyDescriptor(current, name)
        if (found) return found
    }
}

function resolve(module, dotted) {
    let value = module
    for (const name of dotted ? dotted.split('.') : []) {
        const found = descriptor(value, name)
        if (!found || !('value' in found)) return undefined
        value = found.value
    }
    return value
}

// OTIO's nested types have explicit, flattened names in this JS binding.
const typeNames = {
    'Track.Kind': 'TrackKind',
    'Track.NeighborGapPolicy': 'TrackNeighborGapPolicy',
    'Transition.Type': 'TransitionType',
    'ImageSequenceReference.MissingFramePolicy': 'MissingFramePolicy',
}

function compare(reference, modules) {
    return reference.rows.map(row => {
        const moduleName = row.group === 'opentime' ? 'opentime' : 'opentimelineio'
        const module = modules[moduleName]
        const type = row.className || (row.kind === 'class' ? row.name : undefined)
        const mappedType = typeNames[type] || type
        const target = (row.group === 'bundle' ? 'bundle.' : '')
            + (row.className ? `${mappedType}.${row.member}` : mappedType || row.name)
        let found
        if (row.className) {
            const klass = resolve(module, (row.group === 'bundle' ? 'bundle.' : '') + mappedType)
            found = row.kind === 'method' || row.kind === 'property'
                ? descriptor(klass?.prototype, row.member) : descriptor(klass, row.member)
        } else {
            found = descriptor(resolve(module, target.split('.').slice(0, -1).join('.')),
                target.split('.').at(-1))
        }
        let status = 'missing'
        let note = 'No matching JavaScript export'
        if (found) {
            const callable = typeof found.value === 'function'
            const matches = ['class', 'function', 'method', 'static method'].includes(row.kind)
                ? callable : row.kind.endsWith('property') ? Boolean(found.get) : !callable
            status = matches ? 'exposed' : 'shape mismatch'
            note = matches ? 'Name and member kind match; signatures and behavior are not checked'
                : `Expected ${row.kind}; JavaScript exports a different member kind`
            if (matches && typeNames[type]) note = `Explicit type-name mapping: ${target}; signatures and behavior are not checked`
            if (matches && row.writable === true && !found.set) {
                status = 'shape mismatch'
                note = 'Python property is writable; JavaScript property has no setter'
            }
        }
        let alternative = row.name === 'register_type' ? 'register_serializable_object_type' : undefined
        if (row.group === 'opentime' && row.kind === 'function') {
            const klass = row.name.startsWith('range_') ? 'TimeRange' : 'RationalTime'
            alternative = `${klass}.${row.name}`
            if (descriptor(module[klass]?.prototype, row.name)) alternative = `${klass}.prototype.${row.name}`
        }
        if (status === 'missing' && alternative && resolve(module, alternative)) {
            note = `Alternative location: ${alternative}; not counted as matching Python's API`
        } else alternative = undefined
        if (row.protocol) {
            status = 'language-specific'
            note = 'Python protocol; excluded from exposure totals (JavaScript uses different conventions)'
        }
        return { ...row, module: moduleName, target, status, note, alternative }
    })
}

async function main() {
    const git = (...args) => execFileSync('git', ['-C', upstream, ...args], { encoding: 'utf8' }).trim()
    const revision = git('rev-parse', 'HEAD')
    if (git('status', '--porcelain', '--untracked-files=no')) {
        throw new Error('The OTIO submodule is modified. Commit or restore it before creating a pinned reference.')
    }
    for (const name of ['opentime', 'opentimelineio']) {
        if (!fs.existsSync(path.join(root, 'install', `${name}.js`))) {
            throw new Error('Build and install the JavaScript bindings first: make build && make install')
        }
    }
    fs.mkdirSync(output, { recursive: true })
    const exporter = path.join(root, 'scripts/api-reference.py')
    const hash = file => createHash('sha256').update(fs.readFileSync(file)).digest('hex')
    const cache = path.join(output, `reference-${revision}-${hash(exporter).slice(0, 12)}.json`)
    if (!fs.existsSync(cache)) {
        // Build from this checkout, never a released PyPI wheel. A reference is
        // cached by upstream commit, independently of the JS build/report.
        const environment = path.join(output, 'python')
        const python = path.join(environment, 'bin/python')
        execFileSync('python3', ['-m', 'venv', environment], { stdio: 'inherit' })
        execFileSync(python, ['-m', 'pip', 'install', '--force-reinstall', '--no-cache-dir', upstream], { stdio: 'inherit' })
        const data = JSON.parse(execFileSync(python, [exporter], {
            encoding: 'utf8', maxBuffer: 16 * 1024 * 1024,
        }))
        fs.writeFileSync(cache, JSON.stringify({ ...data, revision }, null, 2))
    }
    const reference = JSON.parse(fs.readFileSync(cache, 'utf8'))
    if (reference.revision !== revision) throw new Error('Reference revision mismatch')
    const modules = {}
    for (const name of ['opentime', 'opentimelineio']) {
        modules[name] = await require(path.join(root, 'install', `${name}.js`))()
    }
    const rows = compare(reference, modules)
    const counted = rows.filter(row => !row.protocol)
    const exposed = counted.filter(row => row.status === 'exposed').length
    const report = {
        revision, generatedAt: new Date().toISOString(), referenceVersion: reference.version,
        javascriptArtifacts: Object.fromEntries(['opentime', 'opentimelineio'].map(name => [name,
            Object.fromEntries(['js', 'wasm'].map(extension => [extension, hash(path.join(root, 'install', `${name}.${extension}`))])),
        ])),
        summary: { total: counted.length, exposed, missing: counted.filter(row => row.status === 'missing').length,
            mismatched: counted.filter(row => row.status === 'shape mismatch').length }, rows,
    }
    fs.writeFileSync(path.join(output, 'api-surface.json'), JSON.stringify(report, null, 2))
    const template = fs.readFileSync(path.join(root, 'scripts/api-coverage.html'), 'utf8')
    fs.writeFileSync(path.join(output, 'index.html'), template.replace('/* API_DATA */null',
        JSON.stringify(report).replace(/</g, '\\u003c')))
    console.log(`API exposure: ${exposed}/${counted.length}; ${report.summary.missing} missing; ${report.summary.mismatched} shape mismatches`)
    console.log(`Report: ${path.join(output, 'index.html')}`)
}

module.exports = { compare }
if (require.main === module) main().catch(error => { console.error(error.message); process.exitCode = 1 })
