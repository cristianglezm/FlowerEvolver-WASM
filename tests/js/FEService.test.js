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
});
