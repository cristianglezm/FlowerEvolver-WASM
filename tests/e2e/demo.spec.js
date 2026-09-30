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

    const sumSpeciesCounts = async () =>
        (await page.locator('#species-map .species-count').allTextContents()).reduce((a, c) => a + Number(c), 0);

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

    test('species: every flower is classified when created and shows a species badge', async () => {
        const total = await page.locator('#flowers .flower').count();
        expect(total).toBeGreaterThan(0);
        await expect(page.locator('#flowers .flower .species-badge')).toHaveCount(total);
        await expect(page.locator('#flowers .flower .species-badge').first()).not.toHaveText('');
        // the badge says how far the flower is from its species' prototype
        await expect(page.locator('#flowers .flower .species-badge').first()).toHaveAttribute('title', /Distance to the species prototype: \d+\.\d\d/);
    });

    test('species: the Species tab shows a map whose counts add up to the number of flowers', async () => {
        const total = await page.locator('#flowers .flower').count();
        await page.locator('#tab-species').click();

        await expect(page.locator('#species')).toBeVisible();
        await expect(page.locator('#species-status')).toHaveText('');
        const cards = page.locator('#species-map .species-card');
        expect(await cards.count()).toBeGreaterThan(0);
        await expect(cards.first().locator('canvas')).toBeVisible();
        await expect(cards.first().locator('.species-name')).not.toHaveText('');
        expect(await sumSpeciesCounts()).toBe(total);
    });

    test('species: clicking a species lists exactly the flowers in it', async () => {
        const card = page.locator('#species-map .species-card:not(.empty)').first();
        const count = Number(await card.locator('.species-count').textContent());
        const name = await card.locator('.species-name').textContent();

        await card.click();

        await expect(page.locator('#species-flowers .flower')).toHaveCount(count);
        await expect(page.locator('#header-species')).toContainText(name);
        await expect(page.locator('#species-map .species-card.selected')).toHaveCount(1);
    });

    test('species: a flower made while on the Species tab shows up on the map', async () => {
        const before = await sumSpeciesCounts();
        await page.locator('#btn-make').click();
        await expect.poll(sumSpeciesCounts, { timeout: 15000 }).toBe(before + 1);
    });

    test('species: are persisted, so after a reload the badges are there without reclassifying', async () => {
        await page.reload();
        await expect(page.locator('#btn-make')).toBeEnabled({ timeout: 30000 });
        // the controls enable before the saved flowers are rendered
        await expect(page.locator('#flowers .flower').first()).toBeVisible();

        const total = await page.locator('#flowers .flower').count();
        await expect(page.locator('#flowers .flower .species-badge')).toHaveCount(total);
    });

    test('species: classification runs in a worker, so a burst of new flowers does not block the page', async () => {
        await page.locator('#tab-flowers').click();
        expect(page.workers().length).toBeGreaterThan(0);

        const before = await page.locator('#flowers .flower').count();
        for (let i = 0; i < 3; i++) {
            await page.locator('#btn-make').click();
        }
        // each flower is shown as soon as it is drawn, its badge arrives when the worker has classified it
        await expect(page.locator('#flowers .flower')).toHaveCount(before + 3, { timeout: 30000 });
        await expect(page.locator('#flowers .flower .species-badge')).toHaveCount(before + 3, { timeout: 30000 });
    });

    const savedClassifier = () => page.evaluate(() => new Promise((resolve, reject) => {
        const open = indexedDB.open('Flowers');
        open.onerror = () => reject(open.error);
        open.onsuccess = () => {
            const db = open.result;
            if (!db.objectStoreNames.contains('classifier')) { db.close(); return resolve(null); }
            const get = db.transaction('classifier').objectStore('classifier').get('tuned');
            get.onsuccess = () => { db.close(); resolve(get.result ? get.result.json.length : null); };
            get.onerror = () => { db.close(); reject(get.error); };
        };
    }));

    test('species: summary, detail panel with similar species, and hiding empty species', async () => {
        await page.locator('#tab-species').click();
        await expect(page.locator('#species-summary')).toContainText('species in use');
        await expect(page.locator('#species-detail')).toBeHidden();
        await expect(page.locator('#btn-reset-classifier')).toBeHidden(); // the built-in classifier is in use

        const all = await page.locator('#species-map .species-card').count();
        await page.locator('#species-map .species-card:not(.empty)').first().click();
        await expect(page.locator('#species-detail')).toBeVisible();
        await expect(page.locator('#species-detail-meta')).toContainText('flower');
        await expect(page.locator('#species-neighbours button')).toHaveCount(4);

        await page.locator('#species-hide-empty').check();
        await expect.poll(() => page.locator('#species-map .species-card').count()).toBeLessThan(all);
        await expect(page.locator('#species-map .species-card.empty:not(.selected)')).toHaveCount(0);
        await page.locator('#species-hide-empty').uncheck();
        await expect.poll(() => page.locator('#species-map .species-card').count()).toBe(all);
    });

    test('species: a flower of a species can be picked as a parent and bred from the Species tab', async () => {
        await page.locator('#tab-species').click();
        await page.locator('#species-map .species-card:not(.empty)').first().click();
        await expect(page.locator('#species-detail')).toBeVisible();
        await expect(page.locator('#btn-species-parent')).toBeEnabled();

        await page.locator('#btn-species-parent').click();
        await expect(page.locator('#species-parents')).toContainText(/Parent 1: #\d+/);
        await page.locator('#btn-species-parent').click();
        await expect(page.locator('#species-parents')).toContainText(/Parent 2: #\d+/);

        const total = () => page.evaluate(() => new Promise((resolve, reject) => {
            const open = indexedDB.open('Flowers');
            open.onerror = () => reject(open.error);
            open.onsuccess = () => {
                const count = open.result.transaction('descendants').objectStore('descendants').count();
                count.onsuccess = () => resolve(count.result);
            };
        }));
        const before = await total();
        await page.locator('#btn-species-reproduce').click();
        await expect.poll(total, { timeout: 30000 }).toBe(before + 1);
    });

    test('species: a species with enough flowers can be split in sub-species that survive a reload', async () => {
        await page.locator('#tab-flowers').click();
        await page.locator('#tab-species').click();
        // the embedded classifier spreads random flowers over many cells: split the busiest one
        const busiest = () => page.evaluate(() => [...document.querySelectorAll('#species-map .species-card')]
            .map(card => ({ x: card.dataset.x, y: card.dataset.y, n: Number(card.querySelector('.species-count').textContent) }))
            .sort((a, b) => b.n - a.n)[0]);
        const best = await busiest();
        await page.locator(`#species-map .species-card[data-x="${best.x}"][data-y="${best.y}"]`).click();
        await expect(page.locator('#species-detail')).toBeVisible();

        const count = Number((await page.locator('#header-species').textContent()).match(/(\d+) flower/)[1]);
        test.skip(count < 6, 'no species with enough flowers to split in this run');
        await page.locator('#btn-species-split').click();
        // the dialog opens without charts (the empty charts hold sized svgs, so a [hidden] that CSS overrides shows here)
        await expect(page.locator('#split-charts')).toBeHidden();
        await expect(page.locator('#split-status')).toHaveText('');
        await page.locator('#split-width').fill('2');
        await page.locator('#split-height').fill('1');
        await page.locator('#split-epochs').fill('5');
        await page.locator('#btn-split').click();
        await expect(page.locator('#split-status')).toContainText(/Done|need at least/, { timeout: 120000 });
        test.skip(/need at least/.test(await page.locator('#split-status').textContent()), 'too few flowers in this species for the grid');
        // the start of the run plus one point per epoch, and one cell per sub-species in both maps
        await expect(page.locator('#split-qe-chart circle')).toHaveCount(6);
        await expect(page.locator('#split-occ-before i')).toHaveCount(2);
        await expect(page.locator('#split-occ-now i')).toHaveCount(2);
        await page.locator('#btn-split-close').click();
        await page.locator('#btn-split-close').click();

        await expect(page.locator('#subspecies-map .subspecies-card')).toHaveCount(2);
        await expect(page.locator('#species-flowers .species-badge').first()).toContainText(/ \/ [ab]( · atypical)?$/);

        await page.reload();
        await page.locator('#tab-species').click();
        await page.locator(`#species-map .species-card[data-x="${best.x}"][data-y="${best.y}"]`).click();
        await expect(page.locator('#subspecies-map .subspecies-card')).toHaveCount(2);
    });

    test('species: fine-tuning re-classifies every flower and the result is saved', async () => {
        const total = await page.locator('#flowers .flower').count();
        await page.locator('#tab-species').click();
        await page.locator('#btn-open-finetune').click();
        await expect(page.locator('dialog#finetune')).toBeVisible();
        await page.locator('#finetune-epochs').fill('2');
        await page.locator('#btn-finetune').click();

        await expect(page.locator('#btn-finetune')).toBeDisabled();
        await expect(page.locator('#finetune-status')).toContainText('Done.', { timeout: 180000 });
        await expect(page.locator('#btn-finetune')).toBeEnabled();
        // before + one point per epoch, and a heat map for before and now
        await expect(page.locator('#finetune-qe-chart circle')).toHaveCount(3);
        expect(await page.locator('#finetune-occ-before i').count()).toBeGreaterThan(0);
        expect(await page.locator('#finetune-occ-now i').count()).toBe(await page.locator('#finetune-occ-before i').count());
        await page.locator('#btn-finetune-close').click();
        await expect(page.locator('dialog#finetune')).toBeHidden();
        await expect(page.locator('#btn-reset-classifier')).toBeVisible();
        await expect.poll(sumSpeciesCounts, { timeout: 60000 }).toBe(total);
        expect(await savedClassifier()).toBeGreaterThan(1000);
    });

    test('species: the fine-tuned classifier is still used after a reload', async () => {
        await page.reload();
        await expect(page.locator('#btn-make')).toBeEnabled({ timeout: 30000 });
        await expect(page.locator('#flowers .flower').first()).toBeVisible();
        const total = await page.locator('#flowers .flower').count();

        await page.locator('#tab-species').click();
        // same classifier as when they were last classified, so nothing is classified again
        await expect(page.locator('#species-status')).toHaveText('');
        expect(await sumSpeciesCounts()).toBe(total);
        expect(await savedClassifier()).not.toBeNull();
    });

    test('species: resetting goes back to the built-in classifier', async () => {
        // outside the fine-tune dialog: a showModal() dialog would hide the confirmation behind it
        await expect(page.locator('#btn-reset-classifier')).toBeVisible();
        await page.locator('#btn-reset-classifier').click();
        await page.locator('#modalYesNo button', { hasText: 'Yes' }).click();

        await expect(page.locator('#species-status')).toHaveText('Using the built-in classifier.', { timeout: 60000 });
        expect(await savedClassifier()).toBeNull();
        await expect(page.locator('#btn-reset-classifier')).toBeHidden();
        const total = await page.locator('#flowers .flower').count();
        await expect.poll(sumSpeciesCounts, { timeout: 60000 }).toBe(total);
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
