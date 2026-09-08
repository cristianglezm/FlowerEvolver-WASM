#!/usr/bin/env node
// Serves the real built demo (dist/) for e2e testing, via Vite's preview
// server.
import { existsSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { preview } from 'vite';

const here = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(here, '../..');
const distDir = path.join(repoRoot, 'dist');
const port = Number(process.env.E2E_PREVIEW_PORT || 4321);

if(!existsSync(path.join(distDir, 'index.html'))){
    console.error(
        'tests/e2e expects a real build at dist/index.html, but none was found.\n' +
        'Run `npm run prep` (once) and `npm run build` first, then re-run the e2e tests.'
    );
    process.exit(1);
}

const server = await preview({
    root: repoRoot,
    build: { outDir: distDir },
    preview: { port, strictPort: true },
});
server.printUrls();
