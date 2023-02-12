/** @type {import('jest').Config} */
const config = {
    testMatch: [
        "<rootDir>/tests/**"
    ]
};

if (process.env.CI) {
    config.reporters = [['github-actions', { silent: false }], 'summary']
} else {
    config.reporters = ['default']
}

module.exports = config;
