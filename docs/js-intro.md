# FlowerEvolver-WASM - JS/WASM API

- **FEService** module - `import { FEService, FEParams, Flower } from '@cristianglezm/flower-evolver-wasm'`. Start here; this is what most consumers want.
- **wasm-module** module - `import fe from '@cristianglezm/flower-evolver-wasm/wasm'`. The raw Emscripten module `FEService` wraps - only needed for advanced/low-level use.

For the native C++ API, see the [C++ docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/cpp/) instead. For native Python bindings, see the [Python docs](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/python/).

## Species classification

`FEService` groups flowers into species with a self-organizing map: a species is a cell `x`, `y` of the map, found with its `distance` to the
species' prototype (how typical the flower is of it, with no scale of its own: compare it with other flowers), or `null` for a
flower that cannot be classified. An 12x8 classifier is embedded in the wasm binary and used until you train, fine-tune or load
another one. How the map works, how to judge one and what fine-tuning can and cannot do is explained in the
[species classification guide](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/cpp/source/species.html) of the C++ docs;
the same code runs under all three APIs.

```ts
import { FEService } from '@cristianglezm/flower-evolver-wasm';

const service = new FEService();
await service.init();

// a {"Generation": [{"Flower": ...}, ...]} batch, the format used to move a collection between apps
const batch = JSON.stringify({ Generation: genomes.map((genome) => JSON.parse(genome).Flower) });

// extract once: rendering is the slow part, and wasm calls keep no state, so keep the vectors for the
// length of a run and hand them to the *Features calls (about 1.4 KB each against ~17 KB for a genome)
const features = await service.extractFeatures(batch);   // number[] | null per flower, in order

const species = await service.classifyFeatures(features); // { x, y, distance } | null per flower
const fit = await service.evaluateFeatures(features);     // meanQuantizationError, usedNodes, nodeCounts, ...

await service.train(batch, {}, { width: 6, height: 6, epochs: 100 });  // a fresh classifier, now the current one
const saved = service.getSOM();                           // JSON text, '' while the embedded one is in use
service.loadSOM(saved);                                   // restore it, even before init()

// fine-tune with progress: one decay split over several calls, one epoch per call so the page can repaint
const epochs = 10;
for (let done = 0; done < epochs; done++) {
    await service.fineTuneFeatures(features, 1, 0.3, 1.0, done, epochs);
}
```

Things to know:

- `classify()`, `classifyBatch()` and `evaluate()` take genomes and extract for you; use the `*Features` calls when you
  classify, evaluate or fine-tune the same flowers more than once.
- `extractFeatures()` returns `null` for a flower that cannot be extracted, including a genome whose network overflows (it would
  otherwise produce a NaN, which JSON cannot carry). Drop the `null`s or pass them through: the other calls accept them.
- Training happens in wasm, which has a fixed 30 MB heap shared with the 3D generation: `train()` is refused above 256 nodes
  (`width * height`, and each side at most 256). Loading a classifier is much cheaper than training one.
- Every wasm call is synchronous on the thread it runs on. For a page, run the species work in a Web Worker with its own
  `FEService` (the demo page does: [`index.html`](https://github.com/cristianglezm/FlowerEvolver-WASM/blob/master/index.html)
  builds one from a Blob URL, queues the jobs and stamps each result with the classifier that produced it, because the
  classifier can change while a job waits).
