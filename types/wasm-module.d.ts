declare module '@cristianglezm/flower-evolver-wasm/wasm' {
  /** Factory from the compiled Emscripten module; resolves once WASM is ready. */
  export default function fe(): Promise<FEModule>;

  export interface FEModule {
    /** @returns stringified {"Flower": {"dna": {"genomes": [...]}, "petals": {...}}}, feed into reproduce/mutate/draw* */
    makeFlower(radius: number, numLayers: number, P: number, bias: number): string;
    /** flowerParamsJson: stringified fe::FlowerParameters, e.g. {sex, useNormals, useEmissive}; default "" */
    make3DFlower(
      genome: string, radius: number, numLayers: number, P: number, bias: number,
      flowerId: string, flowerParamsJson?: string
    ): string;
    makePetals(radius: number, numLayers: number, P: number, bias: number): string;
    makePetalLayer(radius: number, numLayers: number, P: number, bias: number, layer: number): string;
    makeStem(radius: number, numLayers: number, P: number, bias: number): string;

    /** @returns a base64-encoded PNG data URL of the rendered genome. */
    drawFlower(genome: string, radius: number, numLayers: number, P: number, bias: number): string;
    /** @returns a base64-encoded PNG data URL of the rendered genome's petals (no stem). */
    drawPetals(genome: string, radius: number, numLayers: number, P: number, bias: number): string;
    /** @returns a base64-encoded PNG data URL of the rendered genome's petal layer (no stem). */
    drawPetalLayer(genome: string, radius: number, numLayers: number, P: number, bias: number, layer: number): string;

    reproduce(flower1: string, flower2: string, radius: number, numLayers: number, P: number, bias: number): string;
    mutate(
      original: string, radius: number, numLayers: number, P: number, bias: number,
      addNodeRate: number, addConnRate: number, removeConnRate: number, perturbWeightsRate: number,
      enableRate: number, disableRate: number, actTypeRate: number
    ): string;

    /** @returns a JSON string shaped `{ stats: FlowerStats }` -- FEService unwraps `.stats` for you. */
    getFlowerStats(genome: string, humidity: number, temperature: number, altitude: number, terrainType: number): string;

    /**
     * @param classifierJson stringified FlowerClassifier.toJson(), or "" to use the classifier embedded in this wasm binary.
     * @param environmentJson stringified fe::Environment, e.g. {"humidity":0.5,"temperature":20,"altitude":0,"terrainType":0}
     * @returns a JSON string shaped `{ match: { x, y, distance } }` (a SpeciesMatch as in index.ts), or `{ match: null }` if genome is malformed/unusable -- not an error.
     */
    classifyFlower(classifierJson: string, genome: string, environmentJson: string): string;
    /**
     * Classifies every flower of a {"Generation": [...]} batch in one call (classifier parsed once).
     * @param classifierJson stringified FlowerClassifier.toJson(), or "" to use the classifier embedded in this wasm binary.
     * @param environmentJson stringified fe::Environment used for every flower.
     * @returns a JSON string shaped `{ matches: (SpeciesMatch | null)[] }`, one entry per input flower in order; null as in classifyFlower().
     */
    classifyFlowers(classifierJson: string, generationsJson: string, environmentJson: string): string;
    /**
     * Measures how well a classifier fits a {"Generation": [...]} batch.
     * @param classifierJson stringified FlowerClassifier.toJson(), or "" to use the classifier embedded in this wasm binary.
     * @param environmentJson stringified fe::Environment used for every flower.
     * @returns a JSON string shaped like SOMEvaluation in index.ts. Throws if no flower in the batch is usable.
     */
    evaluateClassifier(classifierJson: string, generationsJson: string, environmentJson: string): string;
    /**
     * Extracts every flower's feature vector once (rendering is the expensive part), so that
     * the feature-based functions below can run without rendering the flowers again.
     * @returns a JSON string shaped `{ features: (number[] | null)[] }`, one entry per flower in order; null for a flower that can't be extracted.
     */
    extractFeatures(generationsJson: string, environmentJson: string): string;
    /**
     * @param classifierJson stringified FlowerClassifier.toJson(), or "" for the embedded classifier.
     * @param featuresJson stringified `(number[] | null)[]`, as returned in `features` by extractFeatures().
     * @returns a JSON string shaped `{ matches: (SpeciesMatch | null)[] }`, one entry per input entry; null for a null or unusable vector.
     */
    classifyFeatures(classifierJson: string, featuresJson: string): string;
    /**
     * Like evaluateClassifier() over already-extracted vectors (null entries are dropped). Throws if none is usable.
     */
    evaluateFeatures(classifierJson: string, featuresJson: string): string;
    /**
     * Fine-tunes on already-extracted vectors (null entries are dropped) and returns the new classifier JSON.
     * Calling it once per epoch with a growing epochOffset (and the same totalEpochs) is one decay, so the caller can show progress.
     * @param epochOffset epochs of the decay already done by earlier calls; 0 starts a decay.
     * @param totalEpochs epochs of the whole decay; 0 means epochOffset + epochs.
     */
    fineTuneFeatures(classifierJson: string, featuresJson: string, epochs: number, baseLR: number, baseRadius: number, epochOffset: number, totalEpochs: number): string;
    /**
     * Trains a fresh classifier from a whole {"Generation": [...]} batch (one call across the boundary).
     * @param trainingConfigJson stringified fe::SOMTrainingConfig, e.g. {"width":5,"height":5,"epochs":100,"baseLR":0.5,"baseRadius":2.5}
     * @returns the trained classifier's JSON directly (see FlowerClassifier.toJson()) -- NOT wrapped in an envelope object.
     */
    trainFlowerClassifier(generationsJson: string, environmentJson: string, trainingConfigJson: string): string;
    /**
     * Fine-tunes an existing classifier further, in place, on a new {"Generation": [...]} batch.
     * @param classifierJson stringified FlowerClassifier.toJson(), or "" to start from the classifier embedded in this wasm binary.
     * @param environmentJson conditions THIS batch was grown under -- not necessarily what classifierJson was originally trained under.
     * @returns the updated classifier's JSON directly -- NOT wrapped in an envelope object.
     */
    fineTune(
      classifierJson: string, generationsJson: string, environmentJson: string,
      epochs: number, baseLR: number, baseRadius: number
    ): string;
    /** @returns the classifier embedded in this wasm binary, as-is (fe::resources::trainedSOM). */
    getTrainedSOM(): string;

    /** @returns the semver this wasm binary was built from, e.g. "4.0.0" -- mirrors fe::version(). */
    getVersion(): string;
    /** @returns the short git commit hash this wasm binary was built from, or "unknown" -- mirrors fe::commitHash(). */
    getCommitHash(): string;
    /** @returns getVersion() + "+" + getCommitHash(), e.g. "4.0.0+a1b2c3d" -- mirrors fe::versionString(). */
    getVersionString(): string;
    /** @param exceptionPtr the value embind hands you in a catch block around any of the calls above. */
    getExceptionMessage(exceptionPtr: number): string;
  }
}
