# SOM classifier training tools

A standalone development tool for training and evaluating the SOM species
classifier -- **not part of the runtime**, similar in spirit to `blender/`.
You install `flowerevolver` like any other consumer of this project, then
run this against your own flower collection.

## Install

From the repo root, build/install the Python package first (see the main
README for the full native-dependency setup):

```bash
# requirements has flowerevolver==4.1.0
# if developing flowerevoler use
# npm run prep:native && npm run build:native && npm run build:python
pip install -r tools/requirements.txt
```

## Usage

```bash
cd tools
jupyter notebook train_som.ipynb
```

Walks through, in order:

1. loading your flowers (a real exported collection, or a synthetic
   population for a quick pipeline smoke test if you don't have one yet),
2. extracting features once, up front,
3. **exploring grid sizes** -- don't skip this, the right size depends on
   how varied your particular population is, see the notebook's own note,
4. checking node utilization for a candidate size,
5. training the real thing with a quantization-error training curve,
6. a qualitative look at what each grid node actually learned,
7. exporting the result as `include/fe/Resources.hpp`, ready to embed as
   `fe::resources::trainedSOM`.

### Command line

`som.py` covers the same workflow without Jupyter:

```bash
python som.py --generate-dataset 4000 --out dataset.json --cache   # also writes dataset-cache.json
python som.py --train --dataset dataset.json --cache --out fc.json --width 8 --height 6 --epochs 200
python som.py --finetune --load-model fc.json --dataset dataset.json --cache --out fc2.json
python som.py --classify flower.json --load-model fc.json
python som.py --export fc.json --out ../include/fe/Resources.hpp --force
```

Training and fine-tuning print a progress line per `--report-every` epochs with
the quantization error and how many nodes are used, and end with a summary.
Read both numbers: the error can flatten while the load is still moving over
the nodes. `--out` refuses to overwrite an existing file without `--force`.
`--seed` only seeds the render parameters of `--generate-dataset`, not the
genomes, so keep the file if you need the same dataset again.

`--weight-decimals N` (0 to 17) rounds the weights `--train`, `--finetune` and
`--export` write, which makes the file much shorter: for the embedded 12x8
classifier 4 decimals write about a quarter of the text (105 KB of ~438 KB) and
classify the same samples into the same nodes. A weight within half a unit of
the last decimal of 0 or 1 is written as exactly `0` or `1`. The rounding is
absolute, so it would flatten weights of the order of 0.0001 at 4 decimals:
choose it for the scale of your features (they are in [0, 1] here). Only the
file changes, the classifier keeps its exact weights, and a file loaded back
holds the rounded ones (in Python, `classifier.weight_decimals` is the same
setting). Leave it off for a classifier you keep training; it needs a JsonBox
that writes the shortest text of a double.

**Feature cache.** Rendering the flowers is most of the time of a run (about 47 ms each, so 4,000 flowers take over three
minutes) and it is the same every time for a dataset that does not change. `--cache` keeps the extracted vectors in a file and
reuses them, so a second run, or a run with other hyper-parameters, starts training at once. Without a value it is the
dataset's name with `-cache` added (`dataset.json` -> `dataset-cache.json`); `--cache FILE` names it. With `--generate-dataset` it
is written along with the dataset. Flowers are matched by their content, not their position, so you can add, remove and reorder
flowers in the dataset and only the new ones are rendered; a flower that cannot be extracted is remembered too. A cache belongs
to one environment (`--humidity`, `--temperature`, `--altitude`, `--terrain-type`) and one feature layout: when either differs
the cache is rebuilt, never reused, and a file that is not a feature cache is refused rather than overwritten. A long extraction
saves its progress every 500 flowers. Extraction is not bit-for-bit reproducible, so a cached vector can differ in the last digits
from a fresh one; the species cell does not change. `som_trainer.cache.FeatureCache` is the same cache for scripts and notebooks.

**Parallel rendering.** The flowers are independent, so `--workers N` renders them in N processes (`--workers 0`: one per core;
the default, 1, is no pool): 4,000 flowers at half a second to a second each take over half an hour in one process, and about
that divided by N with a pool. It applies to `--train`, `--finetune` and `--generate-dataset --cache`; with a cache only the
flowers that are not in it are rendered, and the samples are the same, in the same order, as one process makes. Threads cannot
do it: EvoAI draws its random numbers from one shared generator without a lock, and the binding holds the GIL while it
renders. See `som_trainer/parallel.py`. In a notebook it is `workers=` of `som.extract_samples()`.

### Choosing a dataset

The population decides the species, more than any hyperparameter does. Before training a map you intend to keep:

- **Count the one-petal flowers.** A dataset with many circular flowers (one petal, `P` near 1) makes a map with many
  circular species and few others. `--generate-dataset` draws `P` from `(1, 2, 3, 4, 5, 6, 8, 12)` with a random sign
  (a `P` of about +-360 only gives a circle with spines) and keeps the layer count within what the radius can show; for your own
  collections, cap or drop the circular ones.
- **Mix the sources**: random flowers, reproduced and mutated ones, and the strange shapes you want told apart. The
  grid experiment behind the numbers below used only random flowers from the generator, so it says nothing about
  evolved lineages or varied environments; measure those yourself.
- **Hold some flowers out** and read the error on them as well: a large grid memorizes a small population.

### Reading the results

Read the quantization error **and** the node utilization together (both are printed while training). The error can stay flat while
the load is still moving over the nodes: on one run it sat at about 0.83 while the used nodes rose from 328 to 411 of 512.
Check the busiest node too; if one cell holds a large share of the flowers the map is not telling them apart.

With 1,600 random flowers (1,200 for training, 400 held out, 150 epochs, learning rate 0.5, radius `max(width, height) / 2`):

| grid | held-out error | held-out nodes used | busiest node | JSON |
|---|---|---|---|---|
| 4x4 | 2.404 | 84% | 30.9% | 89 KB |
| 6x6 | 2.005 | 86% | 14.0% | 201 KB |
| 8x6 | 1.936 | 83% | 11.5% | 269 KB |
| 8x8 | 1.844 | 78% | 13.5% | 359 KB |
| 10x10 | 1.726 | 75% | 6.5% | 562 KB |
| 16x16 | 1.469 | 54% | 5.0% | 1444 KB |

The error keeps falling with size (no elbow) while the gap between training and held-out error grows (0.08 at 6x6, 0.20 at 16x16).
6x6 is a reasonable small embedded default and 8x6 to 10x10 a reasonable main one for a population of this size. The embedded
classifier is an 12x8 map. The wasm build trains at most 256 nodes. See the
[species classification guide](https://cristianglezm.github.io/FlowerEvolver-WASM/docs/python/species.html) for the rest
(extract once, fine-tuning, limits).

## `som_trainer/`

- **`dataset.py`** -- loading a real exported flower collection (the
  `{"Generation": [...]}` format `FlowerEvolver-frontend`'s exporter
  already produces, or a directory of individually-saved genome files), or
  generating a synthetic population with `make_flower()` for a quick
  pipeline test with no real data on hand.
- **`evaluation.py`** -- quantization error, node utilization/cluster
  size, and a real (not independently-restarted) training curve via
  `FlowerClassifier.continue_training()`.
- **`visualize.py`** -- matplotlib: a grid of learned prototype
  silhouettes (the qualitative "do neighboring nodes look like similar
  flowers" check), a utilization heatmap, a training curve, and a
  grid-size comparison plot.
- **`training.py`** -- `train_with_reports()`: one `continue_training()` call for the whole decay that calls you back every few
  epochs with `classifier.evaluate(samples)` (through the `on_epoch` callback); `som.py` and
  `evaluation.training_curve_one_decay()` are built on it. `evaluation.training_curve()` is the older curve that restarts the
  decay at every checkpoint; prefer `training_curve_one_decay(fe.FlowerClassifier(w, h, env), samples, epochs, lr, radius, every)`
  to see how one run converges.

`continue_training()` returns a `SOMTrainingReport` (`epochs`: a `SOMEpoch` per epoch with `epoch`, `learning_rate`, `radius` and
`mean_distance`; `skipped`: the indices of the samples it could not use; `node_counts`: the wins per node of the last epoch) and takes
an `on_epoch(epoch)` callback that can stop the training by returning `False`. `evaluation.epoch_curve(report)` turns the report into a
point per epoch for `visualize.plot_training_curve()`, for free; `mean_distance` is measured while the prototypes move, so it
differs a little from the error `training_curve_one_decay()` evaluates after the epoch (a full pass per checkpoint).
- **`cache.py`** -- `FeatureCache`: a disk cache of extracted feature vectors keyed by flower, used by `som.py --cache`.
- **`parallel.py`** -- renders flowers in worker processes (`som.py --workers`), see above.
- **`export.py`** -- writes a trained classifier's JSON into
  `include/fe/Resources.hpp` as `fe::resources::trainedSOM`.

Nothing here duplicates the SOM algorithm itself -- it's all just
`flowerevolver`'s own `extract_flower_features()`/`train_flower_classifier()`/
`FlowerClassifier` underneath, plus plain numpy/matplotlib on top.
