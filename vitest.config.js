import { defineConfig } from 'vitest/config';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));

export default defineConfig({
    resolve: {
        alias: [
            // index.ts imports the wasm module via a relative path.
            {
                find: './public/FlowerEvolver.js',
                replacement: path.resolve(here, 'dist/public/FlowerEvolver.js'),
            },
        ],
    },
    test: {
        include: ['tests/js/**/*.test.js'],
        environment: 'node',
        globalSetup: ['./tests/js/setup/checkBuildExists.mjs'],
        testTimeout: 30000, // wasm init + flower generation can take a moment on first call
    },
});
