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
/** fe::Environment's JS-facing shape. Omitted fields are defaulted on the C++ side. */
export interface SOMEnvironment {
    humidity?: number;
    temperature?: number;
    altitude?: number;
    terrainType?: number;
}
/** fe::SOMTrainingConfig's JS-facing shape. Omitted fields are defaulted on the C++ side;
 * the defaults are untuned placeholders, see tools/ for evaluating real ones. */
/**
 * where a flower lands on a classifier's grid: the species `(x, y)` and `distance`, how far the
 * flower's feature vector is from that node's prototype (euclidean, 0 for a flower identical to the
 * prototype). The distance has no absolute scale: compare it with the distances of other flowers to
 * find the atypical ones.
 */
export interface SpeciesMatch {
    x: number;
    y: number;
    distance: number;
}
export interface SOMTrainingConfig {
    width?: number;
    height?: number;
    epochs?: number;
    baseLR?: number;
    baseRadius?: number;
}
/** fe::SOMEvaluation, see FEService.evaluate(). */
export interface SOMEvaluation {
    meanQuantizationError: number;
    samplesUsed: number;
    samplesSkipped: number;
    totalNodes: number;
    usedNodes: number;
    fractionUsed: number;
    busiestNodeCount: number;
    samplesPerNodeMean: number;
    nodeCounts: number[];
}
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
     * classifies a flower against the current SOM species classifier: the one
     * embedded in the wasm binary until train()/fineTune()/loadSOM() replace it.
     * @param genome - stringified json
     * @param environment - conditions this flower's stats are computed under;
     * need not match the one the classifier was trained under (see fe::Environment).
     * @returns the best-matching node and the distance to it, or null if the
     * genome is malformed or its DNA is unusable (not an error; a malformed
     * classifierJson/environment still rejects).
     */
    classify(genome: string, environment?: SOMEnvironment): Promise<SpeciesMatch | null>;
    /**
     * measures how well the current classifier fits a batch of flowers: mean
     * quantization error plus how the flowers spread over the nodes. Look at
     * both when judging convergence, the error can look flat while the load
     * is still being redistributed.
     * @param generationsJson - stringified `{"Generation": [...]}`.
     * @param environment - see classify().
     * @throws if no flower in the batch is usable.
     */
    evaluate(generationsJson: string, environment?: SOMEnvironment): Promise<SOMEvaluation>;
    /**
     * extracts the feature vector of every flower of a batch, once. Rendering is the
     * expensive part of classifying, evaluating or fine-tuning and wasm calls keep no
     * state, so a caller running several steps over the same flowers (e.g. one
     * fine-tuning epoch per call, to show progress) extracts once and passes the vectors
     * (about 1.4 KB as JSON against ~17 KB for a genome) to classifyFeatures()/
     * evaluateFeatures()/fineTuneFeatures(). Hold them for the length of such a run,
     * not with the stored flowers.
     * @param generationsJson - stringified `{"Generation": [...]}`.
     * @param environment - see classify(); the vectors depend on it.
     * @returns one entry per input flower, in order: its vector, or null if the genome is malformed/unusable.
     */
    extractFeatures(generationsJson: string, environment?: SOMEnvironment): Promise<Array<number[] | null>>;
    /**
     * classifies feature vectors from extractFeatures() with the current classifier, no rendering.
     * @returns one entry per input entry: the match, or null for a null/unusable vector.
     */
    classifyFeatures(features: Array<number[] | null>): Promise<Array<SpeciesMatch | null>>;
    /**
     * evaluate() over feature vectors from extractFeatures() (null entries are dropped).
     * @throws if no vector is usable.
     */
    evaluateFeatures(features: Array<number[] | null>): Promise<SOMEvaluation>;
    /**
     * fineTune() over feature vectors from extractFeatures() (null entries are dropped);
     * replaces the current classifier like fineTune() does.
     * To report progress run ONE decay in steps: call it once per epoch (or per few
     * epochs) with the same baseLR/baseRadius/totalEpochs and epochOffset = epochs done so far.
     * @param epochs - epochs to run in this call.
     * @param epochOffset - epochs of the decay already done by earlier calls, 0 for the first.
     * @param totalEpochs - epochs of the whole decay, 0 means epochOffset + epochs.
     */
    fineTuneFeatures(features: Array<number[] | null>, epochs: number, baseLR: number, baseRadius: number, epochOffset?: number, totalEpochs?: number): Promise<void>;
    /**
     * classifies a whole batch of flowers in one wasm call (the classifier is parsed
     * once, unlike classify() per flower), e.g. to migrate a saved collection.
     * @param generationsJson - stringified `{"Generation": [...]}`, the same
     * shape train()/fineTune() take.
     * @param environment - see classify().
     * @returns one entry per input flower, in order: the match, or null if that
     * flower's genome is malformed/unusable (not an error).
     */
    classifyBatch(generationsJson: string, environment?: SOMEnvironment): Promise<Array<SpeciesMatch | null>>;
    /**
     * trains a fresh SOM species classifier from a whole batch of flower genomes
     * in one wasm call and makes it the current one (see getSOM()).
     * @param generationsJson - stringified {"Generation": [{"dna":...,"petals":...}, ...]}
     * (the wire format used to move a flower collection between apps); entries
     * that fail feature extraction are skipped and logged, not an error.
     * @param environment - conditions this batch was grown under.
     * @param training - grid size + hyperparameters.
     */
    train(generationsJson: string, environment?: SOMEnvironment, training?: SOMTrainingConfig): Promise<void>;
    /**
     * fine-tunes the current classifier (the embedded default, or one trained or
     * loaded earlier) on a new batch and makes the result the current one (see getSOM()).
     * @param generationsJson - stringified {"Generation": [...]}, see train().
     * @param environment - conditions THIS batch was grown under -- not
     * necessarily what the currently-loaded classifier was originally
     * trained under (see fe::Environment).
     * @param epochs - how many additional epochs to train for.
     * @param baseLR - starting learning rate for this fine-tuning run.
     * @param baseRadius - starting neighborhood radius for this run.
     */
    fineTune(generationsJson: string, environment?: SOMEnvironment, epochs?: number, baseLR?: number, baseRadius?: number): Promise<void>;
    /**
     * the current classifier's JSON, for persisting a trained or fine-tuned result.
     * Pure JS state, so it works before init(). "" until train()/fineTune()/loadSOM()
     * has been called, i.e. while the embedded classifier is in use.
     * @see loadSOM to restore a previously-saved one.
     */
    getSOM(): string;
    /**
     * replaces the current classifier with one supplied directly, e.g. saved earlier
     * via getSOM() or fetched as its own asset. Pure JS state: the JSON is not parsed
     * or validated until the next classify()/fineTune(), so this works before init().
     * @param classifierJson - stringified FlowerClassifier.toJson()
     */
    loadSOM(classifierJson: string): void;
    /**
     * the classifier embedded in the wasm binary, as-is -- what classify()
     * uses until train()/fineTune()/loadSOM() replace it (see getSOM()).
     */
    getTrainedSOM(): string;
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