import { defineConfig, devices } from '@playwright/test';

const PORT = 4321;

export default defineConfig({
    testDir: './tests/e2e',
    fullyParallel: false,
    retries: process.env.CI ? 1 : 0,
    reporter: process.env.CI ? [['dot'], ['github']] : 'list',
    use: {
        baseURL: `http://localhost:${PORT}`,
        trace: 'retain-on-failure',
    },
    projects: [
        { name: 'chromium', use: { ...devices['Desktop Chrome'] } },
    ],
    webServer: {
        command: `node tests/e2e/preview.mjs`,
        url: `http://localhost:${PORT}/index.html`,
        reuseExistingServer: !process.env.CI,
        env: { E2E_PREVIEW_PORT: String(PORT) },
        timeout: 30000,
    },
});
