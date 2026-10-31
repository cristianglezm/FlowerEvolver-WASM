Species classification
======================

Group flowers into species with a self-organizing map: ``fe::FlowerClassifier``, trained and
evaluated with the free functions declared in ``FlowerEvolver.hpp``. The same feature extraction,
training and evaluation back the Python and JS/WASM APIs, so what is described here holds for
all three.

.. include:: species-concepts.inc

Example
~~~~~~~

.. code-block:: cpp

   #include <fe/FlowerEvolver.hpp>
   #include <fe/SOMEvaluation.hpp>

   fe::Environment environment;               // defaults: humidity 0.5, 20 C, sea level
   std::string generationsJson = /* {"Generation": [ {"Flower": {...}}, ... ]} */;
   std::vector<std::string> genomes = /* the same flowers, one {"Flower": {...}} per entry */;

   // 1. extract once: one feature vector per flower (throws for a flower that cannot be extracted)
   std::vector<std::vector<double>> samples;
   for(const auto& genome : genomes){
       samples.push_back(fe::extractFlowerFeatures(genome, environment));
   }

   // 2. train a 4x3 map, then look at the fit and at how the flowers spread over it
   fe::SOMTrainingConfig training;
   training.width = 4;
   training.height = 3;
   training.epochs = 20;
   auto classifier = trainFlowerClassifier(samples, environment, training);
   auto evaluation = fe::evaluateSOM(classifier.som, samples);
   // evaluation.meanQuantizationError, .usedNodes / .totalNodes, .busiestNodeCount, .nodeCounts

   // 3. classify a whole batch: one fe::SpeciesMatch (x, y, distance) or std::nullopt per flower, in input order
   auto species = classifyFlowers(classifier, generationsJson, environment);

   // 4. save and load
   std::stringstream saved;
   classifier.toJson().writeToStream(saved, false, true);
   JsonBox::Value loaded;
   loaded.loadFromString(saved.str());
   fe::FlowerClassifier again(loaded.getObject());

Training further is ``classifier.train(samples, epochs, baseLR, baseRadius, epochOffset, totalEpochs, onEpoch)``, which
returns a :cpp:struct:`fe::SOMTrainingReport` (an :cpp:struct:`fe::SOMEpoch` per epoch, the skipped indices and the wins per
node); ``onEpoch`` is called after every epoch and stops the training when it returns false.
See :cpp:class:`fe::FlowerClassifier`.
