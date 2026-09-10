/*!
 * @license Apache 2.0
 * @cristianglezm/flower-evolver-wasm
 * Copyright 2023-2026 Cristian Gonzalez <cristian.glez.m@gmail.com>
 */
import type { FEModule } from '@cristianglezm/flower-evolver-wasm/wasm';
declare const fe: () => Promise<FEModule>;
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
/**
 * params to make flowers
 */
export declare class FEParams {
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
    constructor(radius?: number, numLayers?: number, P?: number, bias?: number);
}
/**
 * A simple Flower
 */
export declare class Flower {
    genome: string;
    image: string;
    /**
     * @param genome - stringified json
     * @param image  - formatted in base64
     */
    constructor(genome: string, image: string);
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
export declare class FEService {
    #private;
    constructor();
    /**
     * function to load wasm module.
     */
    init(): Promise<void>;
    /**
     * sets the parameters for the creation of flowers.
     */
    setParams(params: FEParams): void;
    /**
     * gets the currents parameters
     */
    getParams(): FEParams;
    /**
     * it makes a flower.
     * @returns flower
     */
    makeFlower(): Promise<Flower>;
    /**
     * returns a GLTF string of the flower.
     * @returns GLTF file string
     */
    make3DFlower(genome: string, flowerID: string, sex: FlowerSex): Promise<string>;
    /**
     * returns a GLTF string of the flower (with normals and emissive textures).
     * @returns GLTF file string
     */
    makeEmissive3DFlower(genome: string, flowerID: string, sex: FlowerSex): Promise<string>;
    /**
     * returns a GLTF string of the flower, it uses the params inside genome.
     */
    draw3DFlower(genome: string, flowerID: string, sex: FlowerSex): Promise<string>;
    /**
     * returns a GLTF string of the flower (with normals and emissive textures), it uses the params inside genome.
     */
    drawEmissive3DFlower(genome: string, flowerID: string, sex: FlowerSex): Promise<string>;
    /**
     * it makes a flower with no stem.
     * @returns flower, only petals
     */
    makePetals(): Promise<Flower>;
    /**
     * it makes the petal for the specified layer. (no stem)
     * @returns flower, only petal in layer
     */
    makePetalLayer(layer: number): Promise<Flower>;
    /**
     * it makes a simple stem. (no petals)
     * @returns flower, only stem
     */
    makeStem(): Promise<Flower>;
    /**
     * it drawn the flower from the given genome.
     * @param genome - stringified json
     * @returns flower from the given genome.
     */
    drawFlower(genome: string): Promise<Flower>;
    /**
     * it drawns the petals from the given genome. (no stem)
     * @param genome - stringified json
     * @returns flower petals
     */
    drawPetals(genome: string): Promise<Flower>;
    /**
     * it drawns the petal layer from the given genome.(no stem)
     * @param genome - stringified json
     * @param layer - layer to draw
     * @returns flower - petal from specified layer
     */
    drawPetalLayer(genome: string, layer: number): Promise<Flower>;
    /**
     * it makes a child from two flower genomes.
     * @param father - genome for father
     * @param mother - genome for mother
     * @returns flower - child flower
     */
    reproduce(father: string, mother: string): Promise<Flower>;
    /**
     * it mutates the given flower genome.
     * @param original - genome from original flower.
     * @returns flower - the new mutated flower
     */
    mutate(original: string, addNodeRate?: number, addConnRate?: number, removeConnRate?: number, perturbWeightsRate?: number, enableRate?: number, disableRate?: number, actTypeRate?: number): Promise<Flower>;
    /**
     * gets the flower stats.
     * @param genome - stringified json
     * @param humidity - 0.0 to 1.0
     * @param temperature - degrees, same scale as the returned min/maxTemperature
     * @param altitude - meters above sea level
     * @param terrainType - terrain type id
     */
    getFlowerStats(genome: string, humidity?: number, temperature?: number, altitude?: number, terrainType?: number): FlowerStats;
    /**
     * the semver this wasm binary was built from, e.g. "4.0.0".
     * Same value as this package's own package.json "version".
     */
    getVersion(): string;
    /**
     * the short git commit hash this wasm binary was built from, or
     * "unknown" for a build outside a git checkout.
     */
    getCommitHash(): string;
    /**
     * getVersion() + "+" + getCommitHash(), e.g. "4.0.0+a1b2c3d".
     */
    getVersionString(): string;
}
//# sourceMappingURL=index.d.ts.map