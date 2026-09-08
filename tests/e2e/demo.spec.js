// Real end-to-end suite: navigates the actual built demo (dist/index.html,
// served by tests/e2e/static-server.mjs - see playwright.config.js) and
// drives it exactly like a person would, through the real wasm module. No
// mocking of FEService, the wasm module, or IndexedDB.
//
// Tests run serial and share one page/browser context on purpose: the demo
// persists everything in one IndexedDB ("Flowers"), and later tests
// (reproduce, mutate, clear) build on flowers created by earlier ones -
// that's the actual workflow the app supports, so the suite exercises it
// the same way.
import { test, expect } from '@playwright/test';

test.describe.serial('FlowerEvolver-WASM demo', () => {
    let page;

    test.beforeAll(async ({ browser }) => {
        page = await browser.newPage();
        await page.goto('/index.html');
    });

    test.afterAll(async () => {
        await page.close();
    });

    test('loads and enables the wasm-dependent controls once the module is ready', async () => {
        await expect(page).toHaveTitle('FlowerEvolver-WASM Demo');
        await expect(page.locator('#title')).toHaveText('FlowerEvolver-WASM');

        const makeBtn = page.locator('#btn-make');
        await expect(makeBtn).toBeEnabled({ timeout: 30000 });
        await expect(makeBtn).toHaveText('Make Flower');
    });

    test('shows a validation modal when reproducing without two flowers selected', async () => {
        await page.locator('#btn-reproduce').click();
        await expect(page.locator('#modal .modal-message')).toHaveText('Please select two flowers to reproduce.');
        await page.locator('#modal button', { hasText: 'Ok' }).click();
        await expect(page.locator('#modal')).toHaveCount(0);
    });

    test('creates a flower via Make Flower', async () => {
        await page.locator('#btn-make').click();

        const card = page.locator('#flowers .flower').first();
        await expect(card).toBeVisible();
        const img = card.locator('.flower-visual img');
        await expect(img).toHaveAttribute('src', /^data:image\/png;base64,/);
    });

    test('reproduces two selected flowers into a child', async () => {
        // need a second parent.
        await page.locator('#btn-make').click();
        await expect(page.locator('#flowers .flower')).toHaveCount(2);

        const cardsBefore = page.locator('#flowers .flower');
        await cardsBefore.nth(0).getByRole('button', { name: 'Select' }).click();
        await cardsBefore.nth(1).getByRole('button', { name: 'Select' }).click();

        await page.locator('#btn-reproduce').click();

        await expect(page.locator('#flowers .flower')).toHaveCount(3);
    });

    test('mutates a flower into a new one', async () => {
        const before = await page.locator('#flowers .flower').count();
        await page.locator('#flowers .flower').first().getByRole('button', { name: 'Mutate' }).click();
        await expect(page.locator('#flowers .flower')).toHaveCount(before + 1);
    });

    test('downloads the genome and image files for a flower', async () => {
        const card = page.locator('#flowers .flower').first();

        const [genomeDownload] = await Promise.all([
            page.waitForEvent('download'),
            card.getByRole('button', { name: 'Download Genome' }).click(),
        ]);
        expect(genomeDownload.suggestedFilename()).toMatch(/^flower_\d+\.json$/);

        const [imageDownload] = await Promise.all([
            page.waitForEvent('download'),
            card.getByRole('button', { name: 'Download Image' }).click(),
        ]);
        expect(imageDownload.suggestedFilename()).toMatch(/^flower_\d+\.png$/);
    });

    test('toggles a flower through 2D -> 3D -> emissive 3D -> back to 2D', async () => {
        const card = page.locator('#flowers .flower').first();
        const toggleBtn = card.getByRole('button', { name: /^Show (3D|E3D|2D)$/ });
        const visual = card.locator('.flower-visual');

        await expect(visual.locator('img')).toBeVisible();

        await toggleBtn.click(); // -> 3d
        await expect(toggleBtn).toHaveText('Show E3D');
        await expect(visual.locator('canvas')).toBeVisible({ timeout: 15000 });

        await toggleBtn.click(); // -> e3d
        await expect(toggleBtn).toHaveText('Show 2D');
        await expect(visual.locator('canvas')).toBeVisible({ timeout: 15000 });

        await toggleBtn.click(); // -> back to 2d
        await expect(toggleBtn).toHaveText('Show 3D');
        await expect(visual.locator('img')).toBeVisible();
    });

    test('clears all flowers after confirming the Yes/No modal', async () => {
        await page.locator('#btn-clear').click();
        await expect(page.locator('#modalYesNo .modal-message')).toHaveText(
            'This will delete all data permanently. Are you sure?'
        );

        await page.locator('#modalYesNo button', { hasText: 'Yes' }).click();

        await expect(page.locator('#flowers .flower')).toHaveCount(0);
    });
});
