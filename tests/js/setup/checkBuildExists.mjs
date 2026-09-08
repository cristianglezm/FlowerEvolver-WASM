import { existsSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const here = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(here, '../../..');
const wasmModule = path.join(repoRoot, 'dist', 'public', 'FlowerEvolver.js');

export async function setup(){
    if(!existsSync(wasmModule)){
        throw new Error(
            'tests/js expects a real wasm build at dist/public/FlowerEvolver.js, but none was found.\n' +
            'Run `npm run prep` (once) and `npm run build` first, then re-run the tests.'
        );
    }
}
