/*!
 * @license Apache 2.0
 * @cristianglezm/flower-evolver-wasm
 * Copyright 2023-2026 Cristian Gonzalez <cristian.glez.m@gmail.com>
 */
// @ts-expect-error - nothing real sits here until dist/ is assembled
import feUntyped from './public/FlowerEvolver.js';
const fe = feUntyped;
export default fe;
export class FEParams {
    radius;
    numLayers;
    P;
    bias;
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
    genome;
    image;
    /**
     * @param genome - stringified json
     * @param image  - formatted in base64
     */
    constructor(genome, image) {
        this.genome = genome;
        this.image = image;
    }
}
/**
 * The text of something the wasm module threw. A C++ exception arrives as a pointer
 * (older Emscripten) or as an object holding it in `excPtr` (newer Emscripten), and
 * getExceptionMessage() needs the pointer: handed anything else, such as a RuntimeError
 * from a wasm trap, it traps itself and the caller sees "null function or function
 * signature mismatch" instead of the real message.
 */
function errorMessage(fe, e) {
    const ptr = typeof e === 'number' ? e : e?.excPtr;
    if (typeof ptr === 'number') {
        try {
            const message = fe.getExceptionMessage(ptr);
            return Array.isArray(message) ? message.filter(Boolean).join(': ') : String(message);
        }
        catch {
            // fall through to what JS has
        }
    }
    return e instanceof Error ? e.message : String(e);
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
    #fe = null;
    #params;
    // "" means the classifier embedded in the wasm binary (the wasm boundary uses the same convention)
    #classifierJson = '';
    constructor() {
        this.#params = new FEParams();
    }
    /**
     * function to load wasm module.
     */
    async init() {
        if (this.#fe) {
            return;
        }
        try {
            this.#fe = await fe();
        }
        catch (e) {
            console.error("Error loading WASM module:", e);
            throw e;
        }
    }
    /**
     * sets the parameters for the creation of flowers.
     */
    setParams(params) {
        this.#params = params;
    }
    /**
     * gets the currents parameters
     */
    getParams() {
        return this.#params;
    }
    /**
     * it makes a flower.
     * @returns flower
     */
    async makeFlower() {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result = JSON.parse(fe.makeFlower(p.radius, p.numLayers, p.P, p.bias));
            return new Flower(result.genome, result.image);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * returns a GLTF string of the flower.
     * @returns GLTF file string
     */
    async #make3DFlower(genome, flowerID, flowerParams) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            return fe.make3DFlower(genome, p.radius, p.numLayers, p.P, p.bias, flowerID, JSON.stringify(flowerParams));
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * returns a GLTF string of the flower.
     * @returns GLTF file string
     */
    async make3DFlower(genome, flowerID, sex) {
        const flowerParams = {
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
    async makeEmissive3DFlower(genome, flowerID, sex) {
        const flowerParams = {
            sex: sex === "male" ? 0 : sex === "female" ? 1 : 2,
            useNormals: true,
            useEmissive: true,
        };
        return this.#make3DFlower(genome, flowerID, flowerParams);
    }
    /**
     * returns a GLTF string of the flower, it uses the params inside genome.
     */
    async draw3DFlower(genome, flowerID, sex) {
        const genJson = JSON.parse(genome);
        const oldParams = this.#params;
        const params = genJson.Flower.petals;
        this.setParams(new FEParams(params.radius, params.numLayers, params.P, params.bias));
        try {
            return await this.make3DFlower(genome, flowerID, sex);
        }
        finally {
            this.setParams(oldParams);
        }
    }
    /**
     * returns a GLTF string of the flower (with normals and emissive textures), it uses the params inside genome.
     */
    async drawEmissive3DFlower(genome, flowerID, sex) {
        const genJson = JSON.parse(genome);
        const oldParams = this.#params;
        const params = genJson.Flower.petals;
        this.setParams(new FEParams(params.radius, params.numLayers, params.P, params.bias));
        try {
            return await this.makeEmissive3DFlower(genome, flowerID, sex);
        }
        finally {
            this.setParams(oldParams);
        }
    }
    /**
     * it makes a flower with no stem.
     * @returns flower, only petals
     */
    async makePetals() {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result = JSON.parse(fe.makePetals(p.radius, p.numLayers, p.P, p.bias));
            return new Flower(result.genome, result.image);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * it makes the petal for the specified layer. (no stem)
     * @returns flower, only petal in layer
     */
    async makePetalLayer(layer) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result = JSON.parse(fe.makePetalLayer(p.radius, p.numLayers, p.P, p.bias, layer));
            return new Flower(result.genome, result.image);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * it makes a simple stem. (no petals)
     * @returns flower, only stem
     */
    async makeStem() {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            const result = JSON.parse(fe.makeStem(this.#params.radius, 1, 1.0, 6.0));
            return new Flower(result.genome, result.image);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * it drawn the flower from the given genome.
     * @param genome - stringified json
     * @returns flower from the given genome.
     */
    async drawFlower(genome) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const image = fe.drawFlower(genome, p.radius, p.numLayers, p.P, p.bias);
            return new Flower(genome, image);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * it drawns the petals from the given genome. (no stem)
     * @param genome - stringified json
     * @returns flower petals
     */
    async drawPetals(genome) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const image = fe.drawPetals(genome, p.radius, p.numLayers, p.P, p.bias);
            return new Flower(genome, image);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * it drawns the petal layer from the given genome.(no stem)
     * @param genome - stringified json
     * @param layer - layer to draw
     * @returns flower - petal from specified layer
     */
    async drawPetalLayer(genome, layer) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const image = fe.drawPetalLayer(genome, p.radius, p.numLayers, p.P, p.bias, layer);
            return new Flower(genome, image);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * it makes a child from two flower genomes.
     * @param father - genome for father
     * @param mother - genome for mother
     * @returns flower - child flower
     */
    async reproduce(father, mother) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result = JSON.parse(fe.reproduce(father, mother, p.radius, p.numLayers, p.P, p.bias));
            return new Flower(result.genome, result.image);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * it mutates the given flower genome.
     * @param original - genome from original flower.
     * @returns flower - the new mutated flower
     */
    async mutate(original, addNodeRate = 0.2, addConnRate = 0.3, removeConnRate = 0.2, perturbWeightsRate = 0.6, enableRate = 0.35, disableRate = 0.3, actTypeRate = 0.4) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        const p = this.#params;
        try {
            const result = JSON.parse(fe.mutate(original, p.radius, p.numLayers, p.P, p.bias, addNodeRate, addConnRate, removeConnRate, perturbWeightsRate, enableRate, disableRate, actTypeRate));
            return new Flower(result.genome, result.image);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
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
    getFlowerStats(genome, humidity = 0.5, temperature = 20, altitude = 0, terrainType = 0) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            const json = fe.getFlowerStats(genome, humidity, temperature, altitude, terrainType);
            return JSON.parse(json).stats;
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
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
    async classify(genome, environment = {}) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            const result = JSON.parse(fe.classifyFlower(this.#classifierJson, genome, JSON.stringify(environment)));
            return result.match;
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * measures how well the current classifier fits a batch of flowers: mean
     * quantization error plus how the flowers spread over the nodes. Look at
     * both when judging convergence, the error can look flat while the load
     * is still being redistributed.
     * @param generationsJson - stringified `{"Generation": [...]}`.
     * @param environment - see classify().
     * @throws if no flower in the batch is usable.
     */
    async evaluate(generationsJson, environment = {}) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            return JSON.parse(fe.evaluateClassifier(this.#classifierJson, generationsJson, JSON.stringify(environment)));
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
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
    async extractFeatures(generationsJson, environment = {}) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            return JSON.parse(fe.extractFeatures(generationsJson, JSON.stringify(environment))).features;
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * classifies feature vectors from extractFeatures() with the current classifier, no rendering.
     * @returns one entry per input entry: the match, or null for a null/unusable vector.
     */
    async classifyFeatures(features) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            return JSON.parse(fe.classifyFeatures(this.#classifierJson, JSON.stringify(features))).matches;
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * evaluate() over feature vectors from extractFeatures() (null entries are dropped).
     * @throws if no vector is usable.
     */
    async evaluateFeatures(features) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            return JSON.parse(fe.evaluateFeatures(this.#classifierJson, JSON.stringify(features)));
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * fineTune() over feature vectors from extractFeatures() (null entries are dropped);
     * replaces the current classifier like fineTune() does.
     * To report progress run ONE decay in steps: call it once per epoch (or per few
     * epochs) with the same baseLR/baseRadius/totalEpochs and epochOffset = epochs done so far.
     * @param epochs - epochs to run in this call.
     * @param epochOffset - epochs of the decay already done by earlier calls, 0 for the first.
     * @param totalEpochs - epochs of the whole decay, 0 means epochOffset + epochs.
     */
    async fineTuneFeatures(features, epochs, baseLR, baseRadius, epochOffset = 0, totalEpochs = 0) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            this.#classifierJson = fe.fineTuneFeatures(this.#classifierJson, JSON.stringify(features), epochs, baseLR, baseRadius, epochOffset, totalEpochs);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * classifies a whole batch of flowers in one wasm call (the classifier is parsed
     * once, unlike classify() per flower), e.g. to migrate a saved collection.
     * @param generationsJson - stringified `{"Generation": [...]}`, the same
     * shape train()/fineTune() take.
     * @param environment - see classify().
     * @returns one entry per input flower, in order: the match, or null if that
     * flower's genome is malformed/unusable (not an error).
     */
    async classifyBatch(generationsJson, environment = {}) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            const result = JSON.parse(fe.classifyFlowers(this.#classifierJson, generationsJson, JSON.stringify(environment)));
            return result.matches;
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * trains a fresh SOM species classifier from a whole batch of flower genomes
     * in one wasm call and makes it the current one (see getSOM()).
     * @param generationsJson - stringified {"Generation": [{"dna":...,"petals":...}, ...]}
     * (the wire format used to move a flower collection between apps); entries
     * that fail feature extraction are skipped and logged, not an error.
     * @param environment - conditions this batch was grown under.
     * @param training - grid size + hyperparameters.
     */
    async train(generationsJson, environment = {}, training = {}) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            this.#classifierJson = fe.trainFlowerClassifier(generationsJson, JSON.stringify(environment), JSON.stringify(training));
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
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
    async fineTune(generationsJson, environment = {}, epochs = 50, baseLR = 0.3, baseRadius = 1.0) {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        const fe = this.#fe;
        try {
            this.#classifierJson = fe.fineTune(this.#classifierJson, generationsJson, JSON.stringify(environment), epochs, baseLR, baseRadius);
        }
        catch (e) {
            throw new Error(errorMessage(fe, e));
        }
    }
    /**
     * the current classifier's JSON, for persisting a trained or fine-tuned result.
     * Pure JS state, so it works before init(). "" until train()/fineTune()/loadSOM()
     * has been called, i.e. while the embedded classifier is in use.
     * @see loadSOM to restore a previously-saved one.
     */
    getSOM() {
        return this.#classifierJson;
    }
    /**
     * replaces the current classifier with one supplied directly, e.g. saved earlier
     * via getSOM() or fetched as its own asset. Pure JS state: the JSON is not parsed
     * or validated until the next classify()/fineTune(), so this works before init().
     * @param classifierJson - stringified FlowerClassifier.toJson()
     */
    loadSOM(classifierJson) {
        this.#classifierJson = classifierJson;
    }
    /**
     * the classifier embedded in the wasm binary, as-is -- what classify()
     * uses until train()/fineTune()/loadSOM() replace it (see getSOM()).
     */
    getTrainedSOM() {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        return this.#fe.getTrainedSOM();
    }
    /**
     * the semver this wasm binary was built from, e.g. "4.0.0".
     * Same value as this package's own package.json "version".
     */
    getVersion() {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        return this.#fe.getVersion();
    }
    /**
     * the short git commit hash this wasm binary was built from, or
     * "unknown" for a build outside a git checkout.
     */
    getCommitHash() {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        return this.#fe.getCommitHash();
    }
    /**
     * getVersion() + "+" + getCommitHash(), e.g. "4.0.0+a1b2c3d".
     */
    getVersionString() {
        if (!this.#fe) {
            throw new Error("call FEService.init() before using it");
        }
        return this.#fe.getVersionString();
    }
}
//# sourceMappingURL=index.js.map