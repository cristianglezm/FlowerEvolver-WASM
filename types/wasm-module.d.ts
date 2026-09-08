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
