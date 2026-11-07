/** @type {import('jest').Config} */
const config = {
    testMatch: [
        '<rootDir>/tests/**/*.test.js'
    ]
};

/* global process */
if (process.env.CI) {
    config.reporters = [['github-actions', { silent: false }], 'summary']
} else {
    config.reporters = ['default']
}

if (process.env.OTIO_COVERAGE_DIR) {
    config.setupFilesAfterEnv = ['<rootDir>/tests/coverage-setup.cjs']
    config.collectCoverage = true
    config.coverageProvider = 'v8'
    config.collectCoverageFrom = ['install/*.js']
    config.coverageDirectory = '<rootDir>/coverage/javascript'
    config.coverageReporters = ['json-summary', 'lcov', 'text-summary']
}

module.exports = config;
