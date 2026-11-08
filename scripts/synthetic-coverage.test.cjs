// Regression test for synthetic LCOV semantics: shared declaration lines are
// covered only when every overload in every module reaches C++; conversion
// failures must not count as coverage. This is not part of report generation.
// Run separately with: node --test scripts/synthetic-coverage.test.cjs
const { test } = require('node:test')
const assert = require('node:assert/strict')
const syntheticCoverage = require('./synthetic-coverage.cjs')

test('synthetic lines require every overload and module to reach C++', () => {
    const binding = (module, name, line, cppCalls, calls = cppCalls) => ({
        module, name, source: { file: 'src/bindings.cpp', line, href: 'cpp/src/bindings.cpp.gcov.html' },
        arguments: ['double'], returns: 'Time', cppCalls, calls,
    })
    const output = syntheticCoverage([
        binding('opentime', 'called', 10, 7),
        binding('opentime', 'conversionRejected', 20, 0, 3),
        binding('opentime', 'overloaded', 30, 2),
        { ...binding('opentime', 'overloaded', 30, 0), arguments: ['double', 'double'] },
        binding('opentime', 'bothCalledA', 40, 1),
        binding('opentime', 'bothCalledB', 40, 9),
        binding('opentime', 'moduleSpecific', 50, 1),
        binding('opentimelineio', 'moduleSpecific', 50, 0),
        { ...binding('opentime', 'sdk', 60, 1), source: undefined },
    ], '/repo')
    assert.deepEqual(output.split('\n').filter(line => line.startsWith('DA:')), [
        'DA:10,1', 'DA:20,0', 'DA:30,0', 'DA:40,1', 'DA:50,0',
    ])
    assert.match(output, /FNF:8\nFNH:5/)
    assert.match(output, /LF:5\nLH:2/)
    assert.match(output, /FNDA:0,opentime:conversionRejected/)
    assert.ok(output.includes('FNDA:1,opentime:overloaded(double) -> Time'))
    assert.ok(output.includes('FNDA:0,opentime:overloaded(double, double) -> Time'))
    assert.match(output, /FNDA:0,opentimelineio:moduleSpecific/)
    assert.ok(!output.includes('sdk'))
})
