const js = require('@eslint/js');

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
                console: 'readonly',
                module: 'readonly',
                require: 'readonly',
                WebAssembly: 'readonly',
            },
        },
        rules: {
            indent: ['error', 4],
            'linebreak-style': ['error', 'unix'],
            quotes: ['error', 'single'],
        },
    },
];
