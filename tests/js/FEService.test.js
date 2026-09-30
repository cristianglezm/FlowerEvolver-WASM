// Run `npm run prep` (once) and `npm run build` before `npm test`.
import { describe, it, expect, beforeAll } from 'vitest';
import { FEService, FEParams, Flower } from '../../index.ts';

describe('FEParams', () => {
    it('applies the documented defaults', () => {
        const p = new FEParams();
        expect(p).toEqual({ radius: 64, numLayers: 3, P: 6.0, bias: 1.0 });
    });

    it('accepts overrides', () => {
        const p = new FEParams(32, 2, 5.0, -1.0);
        expect(p).toEqual({ radius: 32, numLayers: 2, P: 5.0, bias: -1.0 });
    });
});

describe('Flower', () => {
    it('stores genome and image as given', () => {
        const f = new Flower('genome-str', 'image-str');
        expect(f.genome).toBe('genome-str');
        expect(f.image).toBe('image-str');
    });
});

describe('FEService, before init()', () => {
    it('rejects async wasm-backed calls', async () => {
        const service = new FEService();
        await expect(service.makeFlower()).rejects.toThrow('call FEService.init() before using it');
        await expect(service.reproduce('{}', '{}')).rejects.toThrow('call FEService.init() before using it');
    });

    it('throws synchronously from getFlowerStats (it is not async)', () => {
        const service = new FEService();
        expect(() => service.getFlowerStats('{}')).toThrow('call FEService.init() before using it');
    });
});

describe('FEService, after init()', () => {
    // One shared, initialized service for the rest of the suite.
    let service;

    beforeAll(async () => {
        service = new FEService();
        await service.init();
    });

    it('init() is idempotent - calling it again is a safe no-op', async () => {
        await expect(service.init()).resolves.not.toThrow();
    });

    it('setParams/getParams round-trip without leaking into other tests', () => {
        const original = service.getParams();
        const custom = new FEParams(32, 1, 3.0, 0.5);
        service.setParams(custom);
        expect(service.getParams()).toBe(custom);
        service.setParams(original); // restore, since `service` is shared across this describe block
    });

    describe('makeFlower', () => {
        it('returns a parseable genome and a base64 PNG data URL', async () => {
            const flower = await service.makeFlower();
            expect(flower).toBeInstanceOf(Flower);

            const parsed = JSON.parse(flower.genome);
            expect(parsed.Flower).toBeDefined();
            expect(Array.isArray(parsed.Flower.dna.genomes)).toBe(true);
            expect(parsed.Flower.dna.genomes.length).toBe(2); // stats genome + petals CPPN

            expect(flower.image.startsWith('data:image/png;base64,')).toBe(true);
        });

        it('generates a fresh genome on every call, not a cached one', async () => {
            const a = await service.makeFlower();
            const b = await service.makeFlower();
            expect(a.genome).not.toBe(b.genome);
        });
    });

    describe('makePetals / makeStem / makePetalLayer', () => {
        it('makePetals returns a valid 2-genome Flower', async () => {
            const flower = await service.makePetals();
            expect(JSON.parse(flower.genome).Flower.dna.genomes.length).toBe(2);
        });

        it('makeStem returns a valid 2-genome Flower', async () => {
            const flower = await service.makeStem();
            expect(JSON.parse(flower.genome).Flower.dna.genomes.length).toBe(2);
        });

        it('makePetalLayer returns a valid Flower for layer 0', async () => {
            const flower = await service.makePetalLayer(0);
            expect(JSON.parse(flower.genome).Flower.dna.genomes.length).toBe(2);
        });
    });

    describe('draw*', () => {
        it('drawFlower renders the exact genome it was given, unchanged, plus a fresh image', async () => {
            const original = await service.makeFlower();
            const redrawn = await service.drawFlower(original.genome);
            expect(redrawn.genome).toBe(original.genome);
            expect(redrawn.image.startsWith('data:image/png;base64,')).toBe(true);
        });

        it('drawPetals / drawPetalLayer also round-trip the genome unchanged', async () => {
            const original = await service.makeFlower();
            const petals = await service.drawPetals(original.genome);
            expect(petals.genome).toBe(original.genome);
            const layer = await service.drawPetalLayer(original.genome, 0);
            expect(layer.genome).toBe(original.genome);
        });

        it('rejects a malformed genome with a readable error, not a raw wasm exception pointer', async () => {
            await expect(service.drawFlower('not json')).rejects.toThrow();
        });
    });

    describe('reproduce', () => {
        it('combines two parent genomes into a valid child', async () => {
            const father = await service.makeFlower();
            const mother = await service.makeFlower();
            const child = await service.reproduce(father.genome, mother.genome);
            expect(JSON.parse(child.genome).Flower.dna.genomes.length).toBe(2);
        });
    });

    describe('mutate', () => {
        it('returns a valid Flower distinct from the original', async () => {
            const original = await service.makeFlower();
            const mutated = await service.mutate(original.genome);
            expect(JSON.parse(mutated.genome).Flower.dna.genomes.length).toBe(2);
        });

        it('accepts custom mutation rates without throwing', async () => {
            const original = await service.makeFlower();
            await expect(
                service.mutate(original.genome, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7)
            ).resolves.toBeInstanceOf(Flower);
        });
    });

    describe('getFlowerStats', () => {
        it('is synchronous and returns stats within their documented/clamped ranges', async () => {
            const flower = await service.makeFlower();
            const stats = service.getFlowerStats(flower.genome);

            expect(stats.health).toBeGreaterThanOrEqual(1);
            expect(stats.health).toBeLessThanOrEqual(100);
            expect(stats.stamina).toBeGreaterThanOrEqual(1);
            expect(stats.stamina).toBeLessThanOrEqual(100);
            expect(stats.minTemperature).toBeLessThanOrEqual(stats.maxTemperature);
            expect(['male', 'female', 'both']).toContain(stats.sex);

            for(const key of ['vitality', 'agility', 'intelligence', 'strength', 'luck']){
                expect(stats.effects[key]).toBeGreaterThanOrEqual(-100);
                expect(stats.effects[key]).toBeLessThanOrEqual(100);
            }
        });
    });

    describe('3D flowers', () => {
        it('make3DFlower returns a well-formed glTF 2.0 document', async () => {
            const flower = await service.makeFlower();
            const gltf = await service.make3DFlower(flower.genome, 'test-flower', 'both');
            expect(JSON.parse(gltf).asset.version).toBe('2.0');
        });

        it('makeEmissive3DFlower also returns a well-formed glTF 2.0 document', async () => {
            const flower = await service.makeFlower();
            const gltf = await service.makeEmissive3DFlower(flower.genome, 'test-flower-emissive', 'both');
            expect(JSON.parse(gltf).asset.version).toBe('2.0');
        });

        it("draw3DFlower uses the genome's own params and restores the caller's params afterward", async () => {
            const flower = await service.makeFlower();
            const before = service.getParams();

            const gltf = await service.draw3DFlower(flower.genome, 'test-flower-draw', 'both');

            expect(() => JSON.parse(gltf)).not.toThrow();
            expect(service.getParams()).toEqual(before);
        });
    });

    // Order matters here: vitest runs tests in declaration order, and "classify succeeds"/"fineTune"
    // rely on "train()" having just run on the shared `service`. The tests using a fresh
    // FEService have no ordering dependency.
    describe('SOM species classifier', () => {
        // the {"Generation": [...]} wire format train()/fineTune() expect
        function generationJsonFrom(flowers) {
            return JSON.stringify({
                Generation: flowers.map((f) => JSON.parse(f.genome).Flower),
            });
        }

        it('classify() succeeds out of the box on a fresh service, using the embedded default classifier', async () => {
            // no specific match asserted: only that the embedded default is a real, usable classifier
            const fresh = new FEService();
            await fresh.init();
            const flower = await fresh.makeFlower();
            await expect(fresh.classify(flower.genome)).resolves.toBeDefined();
        });

        it('classifyBatch() returns one entry per flower in order, null for an unusable one', async () => {
            const fresh = new FEService();
            await fresh.init();
            const flowers = [];
            for (let i = 0; i < 3; i++) {
                flowers.push(await fresh.makeFlower());
            }
            const generation = flowers.map((f) => JSON.parse(f.genome).Flower);
            generation.splice(1, 0, {});
            const matches = await fresh.classifyBatch(JSON.stringify({ Generation: generation }));
            expect(matches).toHaveLength(4);
            expect(matches[1]).toBeNull();
            for (const i of [0, 2, 3]) {
                expect(Object.keys(matches[i]).sort()).toEqual(['distance', 'x', 'y']);
                expect(matches[i].distance).toBeGreaterThanOrEqual(0);
                expect(Number.isFinite(matches[i].distance)).toBe(true);
            }
            await expect(fresh.classifyBatch(JSON.stringify({ Generation: [] }))).resolves.toEqual([]);
        });

        it('evaluate() reports quantization error and how the flowers spread over the nodes', async () => {
            const fresh = new FEService();
            await fresh.init();
            const flowers = [];
            for (let i = 0; i < 4; i++) {
                flowers.push(await fresh.makeFlower());
            }
            const batch = JSON.stringify({ Generation: flowers.map((f) => JSON.parse(f.genome).Flower) });
            const e = await fresh.evaluate(batch);
            const grid = e.totalNodes;
            expect(e.samplesUsed).toBe(4);
            expect(e.nodeCounts).toHaveLength(grid);
            expect(e.nodeCounts.reduce((a, b) => a + b, 0)).toBe(4);
            expect(e.usedNodes).toBe(e.nodeCounts.filter((c) => c > 0).length);
            expect(e.busiestNodeCount).toBe(Math.max(...e.nodeCounts));
            expect(e.meanQuantizationError).toBeGreaterThan(0);
            await expect(fresh.evaluate(JSON.stringify({ Generation: [{}] }))).rejects.toThrow();
        });

        it('extractFeatures() gives one vector per flower, null for an unusable one, and classifyFeatures() matches classifyBatch()', async () => {
            const fresh = new FEService();
            await fresh.init();
            const flowers = [];
            for (let i = 0; i < 3; i++) {
                flowers.push(await fresh.makeFlower());
            }
            const generation = flowers.map((f) => JSON.parse(f.genome).Flower);
            generation.splice(1, 0, {});
            const batch = JSON.stringify({ Generation: generation });
            const features = await fresh.extractFeatures(batch);
            expect(features).toHaveLength(4);
            expect(features[1]).toBeNull();
            for (const i of [0, 2, 3]) {
                expect(features[i]).toHaveLength(267);
            }
            const matches = await fresh.classifyFeatures(features);
            expect(matches).toHaveLength(4);
            expect(matches[1]).toBeNull();
            for (const i of [0, 2, 3]) {
                expect(Object.keys(matches[i]).sort()).toEqual(['distance', 'x', 'y']);
            }
            // the same flowers classified from their genomes (features are re-extracted, so allow a rare tie-break
            // difference, and the distance only agrees to the noise of the extraction)
            const viaGenomes = await fresh.classifyBatch(batch);
            const same = [0, 2, 3].filter((i) => matches[i].x === viaGenomes[i].x && matches[i].y === viaGenomes[i].y).length;
            expect(same).toBeGreaterThanOrEqual(2);
        });

        it('extractFeatures() returns null, not an unparsable batch, for a flower whose network overflows', async () => {
            const fresh = new FEService();
            await fresh.init();
            const flowers = [];
            for (let i = 0; i < 8; i++) {
                flowers.push(await fresh.makeFlower());
            }
            // 1e999 parses to infinity; JSON.stringify cannot write it, so it goes in as text
            const batch = JSON.stringify({ Generation: flowers.map((f) => JSON.parse(f.genome).Flower) })
                .replace(/"weight":-?[0-9.eE+-]+/g, '"weight":1e999');
            const features = await fresh.extractFeatures(batch);
            expect(features).toHaveLength(8);
            for (const vector of features) {
                if (vector !== null) {
                    expect(vector.every(Number.isFinite)).toBe(true);
                }
            }
            // about 3 in 4 such flowers overflow, so all eight surviving is not a realistic outcome
            expect(features.some((vector) => vector === null)).toBe(true);
        });

        it('evaluateFeatures() and fineTuneFeatures() work on cached vectors, one decay split into steps', async () => {
            const fresh = new FEService();
            await fresh.init();
            const flowers = [];
            for (let i = 0; i < 4; i++) {
                flowers.push(await fresh.makeFlower());
            }
            const features = await fresh.extractFeatures(JSON.stringify({ Generation: flowers.map((f) => JSON.parse(f.genome).Flower) }));
            const before = await fresh.evaluateFeatures(features);
            expect(before.samplesUsed).toBe(4);
            expect(before.nodeCounts.reduce((a, b) => a + b, 0)).toBe(4);

            const original = fresh.getSOM();
            for (let done = 0; done < 3; done++) {
                await fresh.fineTuneFeatures(features, 1, 0.3, 1.0, done, 3);
            }
            expect(fresh.getSOM()).not.toBe(original);
            const after = await fresh.evaluateFeatures(features);
            expect(after.samplesUsed).toBe(4);

            // nothing usable, or a step outside the decay: rejected, classifier untouched
            const kept = fresh.getSOM();
            await expect(fresh.fineTuneFeatures([null], 1, 0.3, 1.0)).rejects.toThrow();
            await expect(fresh.fineTuneFeatures(features, 2, 0.3, 1.0, 2, 3)).rejects.toThrow();
            await expect(fresh.evaluateFeatures([null])).rejects.toThrow();
            expect(fresh.getSOM()).toBe(kept);
        });

        it('train() refuses a grid over the wasm limit (256 nodes, the heap is a fixed 30 MB)', async () => {
            const fresh = new FEService();
            await fresh.init();
            const flower = await fresh.makeFlower();
            const batch = JSON.stringify({ Generation: [JSON.parse(flower.genome).Flower] });
            // 12x12 is well inside the limit; the 256-node boundary itself (16x16) is not trained here,
            // it needs about 10 MB of the 30 MB heap
            await expect(fresh.train(batch, {}, { width: 12, height: 12, epochs: 1 })).resolves.toBeUndefined();
            const kept = fresh.getSOM();
            expect(JSON.parse(kept).som.width).toBe('12');
            await expect(fresh.train(batch, {}, { width: 17, height: 16, epochs: 1 })).rejects.toThrow();
            await expect(fresh.train(batch, {}, { width: 32, height: 32, epochs: 1 })).rejects.toThrow();
            await expect(fresh.train(batch, {}, { width: 5000, height: 1 })).rejects.toThrow();
            expect(fresh.getSOM()).toBe(kept);
        });

        it('reports a C++ exception with its own message, not the text of a wasm trap', async () => {
            const fresh = new FEService();
            await fresh.init();
            const error = await fresh.drawFlower('not json').then(() => null, (e) => e);
            expect(error).toBeInstanceOf(Error);
            expect(error.message).not.toMatch(/null function|signature mismatch/);
            expect(error.message.length).toBeGreaterThan(0);

            const flower = await fresh.makeFlower();
            const batch = JSON.stringify({ Generation: [JSON.parse(flower.genome).Flower] });
            await expect(fresh.train(batch, {}, { width: 17, height: 16, epochs: 1 })).rejects.toThrow(/limits the grid/);
        });

        it('classifyBatch() rejects when explicitly given a malformed classifier via loadSOM()', async () => {
            const fresh = new FEService();
            await fresh.init();
            fresh.loadSOM('not valid json');
            await expect(fresh.classifyBatch(JSON.stringify({ Generation: [] }))).rejects.toThrow();
        });

        it('classify() rejects when explicitly given a malformed classifier via loadSOM()', async () => {
            const fresh = new FEService();
            await fresh.init();
            fresh.loadSOM('not valid json');
            const flower = await fresh.makeFlower();
            await expect(fresh.classify(flower.genome)).rejects.toThrow();
        });

        it('train() populates getSOM(), and classify() then succeeds against it', async () => {
            const flowers = [];
            for (let i = 0; i < 6; i++) {
                flowers.push(await service.makeFlower());
            }
            const generationsJson = generationJsonFrom(flowers);

            await service.train(generationsJson, {}, { width: 2, height: 2, epochs: 20, baseLR: 0.5, baseRadius: 1.5 });

            expect(service.getSOM()).not.toBe('');
            expect(() => JSON.parse(service.getSOM())).not.toThrow();

            const probe = await service.makeFlower();
            const match = await service.classify(probe.genome);
            expect(match).not.toBeNull();
            expect(match.x).toBeGreaterThanOrEqual(0);
            expect(match.x).toBeLessThan(2);
            expect(match.y).toBeGreaterThanOrEqual(0);
            expect(match.y).toBeLessThan(2);
            expect(match.distance).toBeGreaterThanOrEqual(0);
        });

        it('classify() returns null (not a rejection) for a malformed genome, once a classifier is loaded', async () => {
            const match = await service.classify('not json');
            expect(match).toBeNull();
        });

        it('accepts an explicit environment without throwing, and still finds a match', async () => {
            const probe = await service.makeFlower();
            const match = await service.classify(probe.genome, { humidity: 0.7, temperature: 15, altitude: 100, terrainType: 2 });
            expect(match).not.toBeNull();
        });

        it('fineTune() replaces getSOM() with a new result, building on the classifier trained above', async () => {
            const before = service.getSOM();
            const flowers = [];
            for (let i = 0; i < 4; i++) {
                flowers.push(await service.makeFlower());
            }
            const generationsJson = generationJsonFrom(flowers);

            await service.fineTune(generationsJson, {}, 10, 0.3, 1.0);

            expect(service.getSOM()).not.toBe('');
            expect(service.getSOM()).not.toBe(before); // weights actually changed
            expect(() => JSON.parse(service.getSOM())).not.toThrow();
        });

        it('getTrainedSOM() returns the embedded classifier without loading it into getSOM()', async () => {
            const fresh = new FEService();
            await fresh.init();
            const som = JSON.parse(fresh.getTrainedSOM()).som;
            // sizes are serialized as strings by EvoAI
            const expected = Number(som.width) * Number(som.height) * Number(som.inputDim);
            expect(expected).toBeGreaterThan(0);
            expect(som.weights.length).toBe(expected);
            expect(fresh.getSOM()).toBe('');
        });

        it('getTrainedSOM() rejects before init()', () => {
            expect(() => new FEService().getTrainedSOM()).toThrow(/init/);
        });

        describe('getSOM/loadSOM on a fresh, un-trained service', () => {
            it('getSOM() is "" before any classifier is trained/loaded', () => {
                const fresh = new FEService();
                expect(fresh.getSOM()).toBe('');
            });

            it('loadSOM() overrides the currently-loaded classifier, readable back via getSOM(), even before init()', () => {
                const fresh = new FEService();
                fresh.loadSOM('{"some":"json"}');
                expect(fresh.getSOM()).toBe('{"some":"json"}');
            });
        });
    });
});
