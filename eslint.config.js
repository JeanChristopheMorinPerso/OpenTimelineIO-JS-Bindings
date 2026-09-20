const js = require('@eslint/js');
const globals = require('globals');

module.exports = [
    {
        ignores: [
            'deps/**/*',
            'install/**/*',
            'build/**/*',
            'emsdk/**/*',
            'repros/**/*',
        ],
    },
    js.configs.recommended,
    {
        linterOptions: {
            reportUnusedDisableDirectives: false,
        },
        languageOptions: {
            ecmaVersion: 'latest',
            sourceType: 'module',
            globals: {
                ...globals.browser,
                ...globals.es2021,
                ...globals.commonjs,
            },
        },
        rules: {
            indent: ['error', 4],
            'linebreak-style': ['error', 'unix'],
            quotes: ['error', 'single'],
        },
    },
];
