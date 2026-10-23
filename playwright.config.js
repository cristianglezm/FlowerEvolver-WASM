import { defineConfig, devices } from '@playwright/test';

const PORT = 4321;
const isCI = !!process.env.CI;

export default defineConfig({
    testDir: './tests/e2e',
    fullyParallel: false,
    retries: isCI ? 1 : 0,
    reporter: isCI ? [['dot'], ['github']] : 'list',
    use: {
        baseURL: `http://localhost:${PORT}`,
        trace: 'retain-on-failure',
        screenshot: 'only-on-failure',
        video: 'retry-with-video',
    },
    projects: [
        { name: 'chromium', use: { ...devices['Desktop Chrome'] } },
//    { name: 'firefox',  use: { ...devices['Desktop Firefox'] } },
//    { name: 'webkit',   use: { ...devices['Desktop Safari'] } },
    ],
    webServer: {
        command: `node tests/e2e/preview.mjs`,
        url: `http://localhost:${PORT}/index.html`,
        reuseExistingServer: !isCI,
        env: { E2E_PREVIEW_PORT: String(PORT) },
        timeout: 30000,
    },
});
