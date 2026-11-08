const { execFileSync, spawnSync } = require('node:child_process')
const fs = require('node:fs')
const path = require('node:path')
const writeBindingCoverage = require('./binding-coverage.cjs')

/* global __dirname, process */
const root = path.resolve(__dirname, '..')
const coverageDir = path.join(root, 'coverage')
const rawDir = path.join(coverageDir, 'raw')
const llvmBin = path.join(root, 'emsdk/upstream/bin')
const llvmCov = path.join(llvmBin, 'llvm-cov')

// LCOV's genhtml renders the C++ report; LLVM still exports the Wasm data.
execFileSync('genhtml', ['--version'], { stdio: 'inherit' })

// Build and install with BUILD_TYPE=Coverage before running this script.
fs.rmSync(coverageDir, { recursive: true, force: true })
fs.mkdirSync(rawDir, { recursive: true })
const tests = spawnSync(process.execPath, [require.resolve('jest/bin/jest'), '--runInBand'], {
    cwd: root,
    env: { ...process.env, OTIO_COVERAGE_DIR: rawDir },
    stdio: 'inherit',
})
if (tests.error) {
    throw tests.error
}

const profiles = fs.readdirSync(rawDir)
    .filter(name => name.endsWith('.profraw'))
    .map(name => path.join(rawDir, name))
if (profiles.length === 0) {
    throw new Error('No C++ profiles generated. Run make build BUILD_TYPE=Coverage and make install first.')
}
execFileSync(path.join(llvmBin, 'llvm-profdata'), [
    'merge', '-sparse', ...profiles, '-o', path.join(coverageDir, 'coverage.profdata'),
], { stdio: 'inherit' })

// Both modules' profiles contribute; the larger module has the shared mappings.
const coverageArgs = [
    path.join(root, 'install/opentimelineio.wasm'),
    `-instr-profile=${path.join(coverageDir, 'coverage.profdata')}`,
    '-ignore-filename-regex=(/emsdk/|/src/deps/)',
]
const summary = execFileSync(llvmCov, ['report', ...coverageArgs])
fs.writeFileSync(path.join(coverageDir, 'cpp-summary.txt'), summary)
process.stdout.write(summary)
fs.writeFileSync(path.join(coverageDir, 'bindings-summary.txt'),
    execFileSync(llvmCov, ['report', ...coverageArgs, path.join(root, 'src')]))

// LLVM's LCOV export writes "FN:<start>,<name>" without the function end line
// that LCOV uses to check function hits against line hits. Without it, LCOV
// guesses each function's extent from the next function's start, which puts
// the tail of a function inside the lambdas nested in it. LLVM knows the real
// extents, so take them from its JSON export of the same mapping.
//
// Lambdas get no extent: their first line is also a line of the enclosing
// function, so an uncalled lambda sits on a hit line. LCOV skips that line
// for lambdas, but LCOV 2.6 only recognises GCC's "{lambda(" spelling, not
// Clang's "$_N" closures (fixed upstream after 2.6 in commit 5b1dd9c1), and
// would otherwise report them and mark them hit. The pattern is upstream's.
const lambdaName = /(?:^|:)_Z.*(?:Ul\w*?E\d*_|\d\$_\d+)(?:cl[EI]|cv|4_FUN|8__invoke|D[0-2]|C[1-3])/

function addFunctionEndLines(lcov, exported) {
    const extents = new Map()
    for (const fn of exported.data[0].functions) {
        // regions: [startLine, startCol, endLine, endCol, count, fileId, expandedFileId, kind]
        const own = fn.regions.filter(region => region[5] === 0)
        if (own.length > 0) {
            extents.set(`${fn.filenames[0]}\0${fn.name}`, {
                start: Math.min(...own.map(region => region[0])),
                end: Math.max(...own.map(region => region[2])),
            })
        }
    }
    let file
    return lcov.split('\n').map(line => {
        if (line.startsWith('SF:')) {
            file = line.slice(3)
        }
        const match = /^FN:(\d+),(.*)$/.exec(line)
        if (!match || lambdaName.test(match[2])) {
            return line
        }
        const extent = extents.get(`${file}\0${match[2]}`)
        if (extent?.start !== Number(match[1])) {
            throw new Error(`No matching LLVM function record for ${file}:${match[1]} ${match[2]}`)
        }
        return `FN:${extent.start},${extent.end},${match[2]}`
    }).join('\n')
}

const lcovFile = path.join(coverageDir, 'cpp.lcov')
fs.writeFileSync(lcovFile, addFunctionEndLines(
    execFileSync(llvmCov, ['export', ...coverageArgs, '-format=lcov'], { encoding: 'utf8' }),
    JSON.parse(execFileSync(llvmCov, [
        'export', ...coverageArgs, '-format=text', '-skip-expansions', '-skip-branches',
    ], { encoding: 'utf8', maxBuffer: 1 << 30 })),
))

// Clang records a switch case's branch on the case label: taken = the case
// count, not taken = the switch count minus the case count. LCOV expects
// branches on the switch line, as GCC emits them, so an unexecuted case label
// with a sibling case taken is an "inconsistent" line to it. Nothing is wrong
// with the data, so accept exactly that: a branch on an unexecuted line that
// is a case label. Any other inconsistency fails the run.
const inconsistency = /^genhtml: WARNING: \(inconsistent\) "(.+)":(\d+): (.*)$/
function isSwitchCaseLabel(file, line) {
    const source = fs.readFileSync(file, 'utf8').split('\n')[line - 1] ?? ''
    return /^\s*(case\b|default\s*:)/.test(source)
}

const genhtml = spawnSync('genhtml', [lcovFile,
    '--output-directory', path.join(coverageDir, 'cpp'),
    '--prefix', root, '--title', 'OpenTimelineIO C++ coverage',
    '--branch-coverage', '--demangle-cpp', '--legend', '--precision', '2',
    '--rc', 'derive_function_end_line=0',
    '--ignore-errors', 'inconsistent', '--rc', 'max_message_count=0',
], { cwd: root, encoding: 'utf8', stdio: ['ignore', 'inherit', 'pipe'] })
const caseLabels = new Set()
const unexpected = []
let accepted = false
for (const line of genhtml.stderr.split('\n')) {
    const match = inconsistency.exec(line)
    if (!match) {
        // Tab-indented lines are LCOV's hints about the preceding message.
        if (line && !(accepted && line.startsWith('\t'))) {
            console.error(line)
        }
        continue
    }
    accepted = match[3].startsWith('line is not hit but at least one branch on line has been evaluated')
        && isSwitchCaseLabel(match[1], Number(match[2]))
    if (accepted) {
        caseLabels.add(`${match[1]}:${match[2]}`)
    } else {
        unexpected.push(line)
    }
}
if (genhtml.status !== 0 || unexpected.length > 0) {
    throw new Error([
        ...(genhtml.status !== 0 ? [`genhtml exited with ${genhtml.status ?? genhtml.signal}`] : []),
        ...(unexpected.length > 0 ? ['Unexpected coverage data inconsistencies:', ...unexpected] : []),
    ].join('\n'))
}
console.log(`Accepted ${caseLabels.size} unexecuted switch case labels carrying Clang's case branch counts.`)

const bindings = writeBindingCoverage(coverageDir)
console.log(`Binding overloads: ${bindings.entered}/${bindings.registered} entered, ${bindings.reached}/${bindings.registered} reached C++`)
console.log(`Binding report: ${path.join(coverageDir, 'bindings.html')}`)
console.log(`C++ report: ${path.join(coverageDir, 'cpp/index.html')}`)
console.log(`JavaScript report: ${path.join(coverageDir, 'javascript/lcov-report/index.html')}`)
// Partial reports are still useful, but must not hide a failed test run.
process.exitCode = tests.status ?? 1
