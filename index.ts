/*!
 * @license Apache 2.0
 * @cristianglezm/flower-evolver-wasm
 * Copyright 2023-2026 Cristian Gonzalez <cristian.glez.m@gmail.com>
 */

// @ts-expect-error - nothing real sits here until dist/ is assembled
import feUntyped from './public/FlowerEvolver.js';
import type { FEModule } from '@cristianglezm/flower-evolver-wasm/wasm';

const fe = feUntyped as unknown as () => Promise<FEModule>;
export default fe;

/** Biological sex assigned to a generated flower. */
export type FlowerSex = 'male' | 'female' | 'both';

/** Per-stat effect strengths derived from the genome, range roughly -100..100. */
export interface FlowerEffects {
    vitality: number;
    agility: number;
    intelligence: number;
    strength: number;
    luck: number;
}

/** Environment-dependent stats derived from a flower's genome (see include/fe/Stats.hpp). */
export interface FlowerStats {
    sex: FlowerSex;
    health: number;
    stamina: number;
    minTemperature: number;
    maxTemperature: number;
    maturationPeriod: number;
    toxicityRate: number;
    effects: FlowerEffects;
}

/** Shape the wasm module's make/draw/reproduce/mutate calls return before unwrapping into a Flower. */
interface FlowerResultJson {
    genome: string;
    image: string;
}

/** fe::FlowerParameters' JS-facing subset -- internal to make3DFlower/makeEmissive3DFlower below. */
interface Fe3DFlowerParams {
    sex: 0 | 1 | 2;
    useNormals: boolean;
    useEmissive: boolean;
}

/**
 * params to make flowers
 */
export class FEParams {
    radius: number;
    numLayers: number;
    P: number;
    bias: number;
    /**
     * @param radius - radius of the flower (default: 64)
     * @param numLayers - how many layers it will have. (default: 3)
     * @param P - P parameter (default: 6.0)
     * @param bias - bias (default: 1.0)
     */
    constructor(radius = 64, numLayers = 3, P = 6.0, bias = 1.0) {
        this.radius = radius;
        this.numLayers = numLayers;
        this.P = P;
        this.bias = bias;
    }
}

/**
 * A simple Flower
 */
export class Flower {
    genome: string;
    image: string;
    /**
     * @param genome - stringified json
     * @param image  - formatted in base64
     */
    constructor(genome: string, image: string) {
        this.genome = genome;
        this.image = image;
    }
}

/**
 * simple wrapper around fe wasm module.
 *
 * @example
 * ```ts
 * let fes = new FEService();
 * await fes.init(); // must resolve before any other call
 * // make a flower with default params
 * let flowers = new Array();
 * flowers.push(await fes.makeFlower());
 * // radius, numLayers, P, bias
 * fes.setParams(new FEParams(128, 2, 5.0, -1.0));
 * // make a flower with new params.
 * flowers.push(await fes.makeFlower());
 * flowers.push(await fes.reproduce(flowers[0].genome, flowers[1].genome));
 * // mutate child using default mutation rates.
 * flowers.push(await fes.mutate(flowers[2].genome))
 * ```
 */
export class FEService {
    #fe: FEModule | null = null;
    #params: FEParams;

    constructor() {
        this.#params = new FEParams();
    }

    /**
     * function to load wasm module.
     */
    async init(): Promise<void> {
        if (this.#fe) {
            return;
        }
        try {
            this.#fe = await fe();
        } catch (e) {
            console.error("Error loading WASM module:", e);
            throw e;
        }
    }

    /**
     * sets the parameters for the creation of flowers.
     */
    setParams(params: FEParams): void {
        this.#params = params;
    }

    /**
     * gets the currents parameters
     */
    getParams(): FEParams {
        return this.#params;
    }

    /**
     * it makes a flower.
     * @returns flower
     */
    async makeFlower(): Promise<Flower> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result: FlowerResultJson = JSON.parse(fe.makeFlower(p.radius, p.numLayers, p.P, p.bias));
            return new Flower(result.genome, result.image);
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * returns a GLTF string of the flower.
     * @returns GLTF file string
     */
    async #make3DFlower(genome: string, flowerID: string, flowerParams: Fe3DFlowerParams): Promise<string> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            return fe.make3DFlower(genome, p.radius, p.numLayers, p.P, p.bias, flowerID, JSON.stringify(flowerParams));
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * returns a GLTF string of the flower.
     * @returns GLTF file string
     */
    async make3DFlower(genome: string, flowerID: string, sex: FlowerSex): Promise<string> {
        const flowerParams: Fe3DFlowerParams = {
            sex: sex === "male" ? 0 : sex === "female" ? 1 : 2,
            useNormals: true,
            useEmissive: false,
        };
        return this.#make3DFlower(genome, flowerID, flowerParams);
    }

    /**
     * returns a GLTF string of the flower (with normals and emissive textures).
     * @returns GLTF file string
     */
    async makeEmissive3DFlower(genome: string, flowerID: string, sex: FlowerSex): Promise<string> {
        const flowerParams: Fe3DFlowerParams = {
            sex: sex === "male" ? 0 : sex === "female" ? 1 : 2,
            useNormals: true,
            useEmissive: true,
        };
        return this.#make3DFlower(genome, flowerID, flowerParams);
    }

    /**
     * returns a GLTF string of the flower, it uses the params inside genome.
     */
    async draw3DFlower(genome: string, flowerID: string, sex: FlowerSex): Promise<string> {
        const genJson = JSON.parse(genome);
        const oldParams = this.#params;
        const params = genJson.Flower.petals;
        this.setParams(new FEParams(params.radius, params.numLayers, params.P, params.bias));
        try {
            return await this.make3DFlower(genome, flowerID, sex);
        } finally {
            this.setParams(oldParams);
        }
    }

    /**
     * returns a GLTF string of the flower (with normals and emissive textures), it uses the params inside genome.
     */
    async drawEmissive3DFlower(genome: string, flowerID: string, sex: FlowerSex): Promise<string> {
        const genJson = JSON.parse(genome);
        const oldParams = this.#params;
        const params = genJson.Flower.petals;
        this.setParams(new FEParams(params.radius, params.numLayers, params.P, params.bias));
        try {
            return await this.makeEmissive3DFlower(genome, flowerID, sex);
        } finally {
            this.setParams(oldParams);
        }
    }

    /**
     * it makes a flower with no stem.
     * @returns flower, only petals
     */
    async makePetals(): Promise<Flower> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result: FlowerResultJson = JSON.parse(fe.makePetals(p.radius, p.numLayers, p.P, p.bias));
            return new Flower(result.genome, result.image);
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * it makes the petal for the specified layer. (no stem)
     * @returns flower, only petal in layer
     */
    async makePetalLayer(layer: number): Promise<Flower> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result: FlowerResultJson = JSON.parse(fe.makePetalLayer(p.radius, p.numLayers, p.P, p.bias, layer));
            return new Flower(result.genome, result.image);
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * it makes a simple stem. (no petals)
     * @returns flower, only stem
     */
    async makeStem(): Promise<Flower> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            const result: FlowerResultJson = JSON.parse(fe.makeStem(this.#params.radius, 1, 1.0, 6.0));
            return new Flower(result.genome, result.image);
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * it drawn the flower from the given genome.
     * @param genome - stringified json
     * @returns flower from the given genome.
     */
    async drawFlower(genome: string): Promise<Flower> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const image = fe.drawFlower(genome, p.radius, p.numLayers, p.P, p.bias);
            return new Flower(genome, image);
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * it drawns the petals from the given genome. (no stem)
     * @param genome - stringified json
     * @returns flower petals
     */
    async drawPetals(genome: string): Promise<Flower> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const image = fe.drawPetals(genome, p.radius, p.numLayers, p.P, p.bias);
            return new Flower(genome, image);
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * it drawns the petal layer from the given genome.(no stem)
     * @param genome - stringified json
     * @param layer - layer to draw
     * @returns flower - petal from specified layer
     */
    async drawPetalLayer(genome: string, layer: number): Promise<Flower> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const image = fe.drawPetalLayer(genome, p.radius, p.numLayers, p.P, p.bias, layer);
            return new Flower(genome, image);
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * it makes a child from two flower genomes.
     * @param father - genome for father
     * @param mother - genome for mother
     * @returns flower - child flower
     */
    async reproduce(father: string, mother: string): Promise<Flower> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result: FlowerResultJson = JSON.parse(fe.reproduce(father, mother, p.radius, p.numLayers, p.P, p.bias));
            return new Flower(result.genome, result.image);
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * it mutates the given flower genome.
     * @param original - genome from original flower.
     * @returns flower - the new mutated flower
     */
    async mutate(
        original: string,
        addNodeRate = 0.2,
        addConnRate = 0.3,
        removeConnRate = 0.2,
        perturbWeightsRate = 0.6,
        enableRate = 0.35,
        disableRate = 0.3,
        actTypeRate = 0.4
    ): Promise<Flower> {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result: FlowerResultJson = JSON.parse(fe.mutate(original, p.radius, p.numLayers, p.P, p.bias,
                addNodeRate, addConnRate, removeConnRate, perturbWeightsRate,
                enableRate, disableRate, actTypeRate));
            return new Flower(result.genome, result.image);
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * gets the flower stats.
     * @param genome - stringified json
     * @param humidity - 0.0 to 1.0
     * @param temperature - degrees, same scale as the returned min/maxTemperature
     * @param altitude - meters above sea level
     * @param terrainType - terrain type id
     */
    getFlowerStats(genome: string, humidity = 0.5, temperature = 20, altitude = 0, terrainType = 0): FlowerStats {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            const json = fe.getFlowerStats(genome, humidity, temperature, altitude, terrainType);
            return JSON.parse(json).stats;
        } catch (e) {
            throw new Error(fe.getExceptionMessage(e as number));
        }
    }

    /**
     * the semver this wasm binary was built from, e.g. "4.0.0".
     * Same value as this package's own package.json "version".
     */
    getVersion(): string {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        return this.#fe.getVersion();
    }

    /**
     * the short git commit hash this wasm binary was built from, or
     * "unknown" for a build outside a git checkout.
     */
    getCommitHash(): string {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        return this.#fe.getCommitHash();
    }

    /**
     * getVersion() + "+" + getCommitHash(), e.g. "4.0.0+a1b2c3d".
     */
    getVersionString(): string {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        return this.#fe.getVersionString();
    }
}
