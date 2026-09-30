Species classification
======================

Group flowers into species with a self-organizing map:
:class:`flowerevolver.FlowerClassifier`, trained with
:func:`flowerevolver.train_flower_classifier` and evaluated with
:meth:`flowerevolver.FlowerClassifier.evaluate`. The same feature extraction, training and
evaluation back the C++ and JS/WASM APIs, so what is described here holds for all three.

.. include:: ../source/species-concepts.inc

Example
~~~~~~~

.. code-block:: python

   import flowerevolver as fe

   environment = fe.Environment()            # defaults: humidity 0.5, 20 C, sea level
   flowers = [fe.make_flower(64, 3, p, 1.0) for p in (1, 2, 3, 4, 5, 6, 8, 12) * 5]
   genomes = [flower.to_json() for flower in flowers]

   # 1. extract once: one feature vector per flower
   samples = [fe.extract_flower_features(genome, environment) for genome in genomes]

   # 2. train a 4x3 map, then look at the fit and at how the flowers spread over it
   config = fe.SOMTrainingConfig()
   config.width, config.height, config.epochs = 4, 3, 20
   classifier = fe.train_flower_classifier(samples, environment, config)
   report = classifier.evaluate(samples)
   # report["mean_quantization_error"], report["used_nodes"], report["node_counts"], ...

   # 3. classify: a SpeciesMatch (x, y and distance to the species' prototype), or None for a flower that cannot be classified
   match = classifier.classify(genomes[0], environment)
   matches = classifier.classify_flowers(
       '{"Generation": [' + ",".join(genomes) + "]}", environment)   # one entry per flower

   # 4. save and load
   text = classifier.to_json()
   again = fe.FlowerClassifier.from_json(text)

   # 5. fine-tune on a new batch, in place; it returns a report with a SOMEpoch per epoch
   report = classifier.continue_training(samples, 10, 0.3, 1.0)
   print(report.skipped, [round(e.mean_distance, 3) for e in report.epochs])

   # on_epoch shows progress, and stops the training when it returns False
   classifier.continue_training(samples, 10, 0.3, 1.0, on_epoch=lambda e: print(e.epoch, e.mean_distance))

Splitting a species is the same calls on the flowers of one cell:

.. code-block:: python

   # 6. split the species at (x, y): a small classifier trained on its flowers alone
   x, y = match.x, match.y
   members = [v for v in samples if (m := classifier.classify_features(v)) and (m.x, m.y) == (x, y)]
   sub = fe.train_flower_classifier(members, environment, fe.SOMTrainingConfig(2, 2, 60, 0.5, 1.0))
   # two steps: the main map, then the small map of the cell it chose
   sub_match = sub.classify_features(members[0])

``tools/som.py`` and ``tools/train_som.ipynb`` in the repository wrap this workflow (dataset generation, grid
size exploration, training curves, export of the embedded classifier); see ``tools/README.md``.
