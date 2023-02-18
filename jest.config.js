/** @type {import('jest').Config} */
const config = {
    testMatch: [
        '<rootDir>/tests/**'
    ]
};

/* global process */
if (process.env.CI) {
    config.reporters = [['github-actions', { silent: false }], 'summary']
} else {
    config.reporters = ['default']
}

module.exports = config;
