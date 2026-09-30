What it is
~~~~~~~~~~

A *species* is a cell of a self-organizing map (SOM). Every flower is turned into a
**feature vector** of ``FlowerClassifier::inputDim`` (267) numbers in [0, 1]: a 16x16 silhouette
of the petals (alpha channel of a petals-only render, max-pooled down) followed by 11 fixed-bounds
normalized stats. The map is a ``width`` x ``height`` grid of prototype vectors trained on a
population of flowers; a flower's species is the ``(x, y)`` of the prototype closest to its vector,
and the ``distance`` from the vector to that prototype says how typical the flower is of its species.
The distance has no absolute scale: compare it with the distances of other flowers (for example the
ones above the mean plus two standard deviations of all of them are the atypical ones).
A flower that cannot be extracted (malformed genome, DNA without enough usable outputs) has no
species: ``null``/``None``/``std::nullopt``, never an exception, so batch results always line up
with their input.

The classifier also records the *environment* (humidity, temperature, altitude, terrain) it was trained
under. That is documentation only: it is not checked when classifying, because the game varies the
environment per flower on purpose. The environment you pass when classifying is the one the flower's stats
are computed in.

A classifier embedded in the library (``get_trained_som()`` in Python, ``getTrainedSOM()`` in JS,
``fe::resources::trainedSOM`` in C++) is used whenever you do not supply one. It is a plain JSON text, so you
can save, version and load your own with ``toJson()``/``to_json()``/``getSOM()`` and
``FlowerClassifier(...)``/``from_json()``/``loadSOM()``.

Extract once
~~~~~~~~~~~~

Rendering a flower is the expensive part (about 47 ms natively, more in wasm), the map lookup is not.
When you classify, evaluate or fine-tune the same flowers more than once, extract their vectors once
(``extractFlowerFeatures``, ``extract_flower_features``, ``FEService.extractFeatures``) and pass the vectors to
the ``*Features`` functions. This is a recommendation, not a requirement: every function also accepts genomes and
extracts for you. A feature vector is about 1.4 KB as JSON against about 17 KB for a genome, so holding the vectors
of a whole collection for the length of a run is cheap; do not store them with the flowers.

Extraction is not bit-for-bit reproducible between calls: for some random genomes the raw render differs
(random number use inside the network construction). After the 16x16 downsample it hardly matters: on 300 random
flowers the species cell never changed, and a map trained on one extraction fits a second extraction as well as the first
(quantization error 2.0956 against 2.0972). Do not compare vectors for equality.

A genome whose network overflows (a weight of ``1e999`` in the JSON, for example) gives NaN values in its vector
instead of an error. Training and classification skip such a vector. If you use vectors yourself, filter them with
``isfinite``. The wasm ``extractFeatures`` returns ``null`` for that flower because JSON cannot carry NaN.

Training
~~~~~~~~

Training takes feature vectors (or a ``{"Generation": [...]}`` batch, which it extracts for you one flower at a time) and a
configuration: grid size, epochs, ``baseLR`` and ``baseRadius``. The defaults (5x5, 100 epochs, 0.5, 2.5) are placeholders, not
tuned. Both the learning rate and the radius decay exponentially from their base value to a floor (0.01 and 0.5) over the
epochs. Samples with the wrong length or non-finite values are skipped, their indices are in the report; training throws if none is usable.

Training returns a report: one entry per epoch with its index, learning rate, radius and ``meanDistance``, the mean distance of
the samples to their species while the epoch ran (free, but the prototypes move during the epoch, so it differs a little from the
quantization error measured afterwards), plus the skipped indices and the samples won by each node in the last epoch. A callback
called after every epoch can show progress or stop the training by returning false.

To report progress without a callback, or when each call has to return (a page that must repaint, for example), split one training over several calls: pass the same
``baseLR``, ``baseRadius`` and ``totalEpochs`` every time and advance ``epochOffset`` by the epochs already done. The
decay then matches a single call. The weights only match to rounding, because each call reshuffles the samples. A call
without them starts a new decay at its own epoch 0, which is what fine-tuning on a new batch wants.

Judging a map
~~~~~~~~~~~~~

Evaluate with the same vectors you trained on and with some you held out. Read **two** numbers:

* the mean quantization error (distance from a flower to its prototype), and
* how the flowers spread over the grid: nodes used, the busiest node, the count per node.

The error can stay flat while the load is still moving over the nodes: on one run the error sat at about 0.83 while the
used nodes kept rising from 328 to 411 of 512. Judge convergence by both.

In an experiment with 1,600 random flowers (1,200 for training, 400 held out, 150 epochs) the held-out error fell steadily
with the grid size, from 2.40 at 4x4 to 1.47 at 16x16, with no elbow, while the gap between training and held-out error grew
(0.08 at 6x6, 0.20 at 16x16): large grids memorize a small population. The busiest node's share of
the flowers fell from 31% (4x4) to 5% (16x16), and the share of nodes used by the held-out flowers from 84% to 54%. A 6x6 grid is a
reasonable small default and 8x6 to 10x10 a reasonable main one, for a population of this size. The experiment used flowers
made with the project's own generator, not evolved lineages or varied environments.

The population decides the species. The mix of petal counts matters most: a dataset with many one-petal (circular) flowers
produces many circular species and few others. Build the training population from the shapes you want told apart
(random, reproduced and mutated flowers, the strange shapes too) and look at the busiest node before you trust a map.

Fine-tuning
~~~~~~~~~~~

Fine-tuning continues training an existing classifier on a new batch. It moves the prototypes toward the new flowers; it does
**not** create species. The grid is fixed, so it spreads the new flowers over the existing cells (on 80 flowers the used cells rose from
23 to 30 of 48). To split one species, train a second, small classifier on that species' flowers alone (below), or train a bigger grid.

Splitting a species
~~~~~~~~~~~~~~~~~~~

A species the main map puts together can hide differences that matter (flowers that look alike but heal or poison, say). Keep the main
map as it is and give that one cell a small map of its own: take the feature vectors of the flowers whose species is that cell, train a
classifier of a few cells on them, and classify in two steps, the main map first and then the small map of the cell it chose. A small
grid (2x2 to 3x3) is enough and needs about three flowers per cell, or it only memorizes the flowers it saw. The small map belongs to
one main classifier: when you fine-tune or replace the main one, the cells move and a small map made for the old one no longer
applies (keep it next to the fingerprint of the main classifier it was made for).

The demo page does this from the Species tab ("Split this species"): it initializes each sub-species prototype from one of the species'
own flowers, trains without wrapping around the grid's edges, stores the small map in the browser next to the main classifier and
classifies new flowers in two steps. It uses only the calls described in this guide.

Limits
~~~~~~

Native and Python callers set their own limits: memory grows with the grid (training peaks at about 40 KB per node natively,
a 16x32 grid about 20 MB, because the serialized weights exist several times at once), and the caller decides how many flowers
to extract at a time. The wasm build has a fixed 30 MB heap that is shared with the 3D generation, so training there is
refused above 256 nodes (``width * height``, and each side at most 256). Loading a classifier is much cheaper than training one.
