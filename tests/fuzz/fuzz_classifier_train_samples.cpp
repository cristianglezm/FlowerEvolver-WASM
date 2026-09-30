// libFuzzer harness for FlowerClassifier::train()/evaluate() on caller-supplied feature vectors:
// wrong sizes, NaN/Inf entries, empty batches and arbitrary learning rate, radius, epochOffset and
// totalEpochs, which feed the decay arithmetic (pow, gamma) and the split-decay bookkeeping.
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include <fe/FlowerClassifier.hpp>

namespace{
    double nextValue(FuzzedDataProvider& fdp){
        switch(fdp.ConsumeIntegralInRange<int>(0, 5)){
            case 0: return std::numeric_limits<double>::quiet_NaN();
            case 1: return std::numeric_limits<double>::infinity();
            case 2: return -std::numeric_limits<double>::infinity();
            case 3: return fdp.ConsumeFloatingPoint<double>();
            default: return fdp.ConsumeProbability<double>();
        }
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
    FuzzedDataProvider fdp(data, size);
    try{
        fe::FlowerClassifier classifier(fdp.ConsumeIntegralInRange<std::size_t>(1, 4), fdp.ConsumeIntegralInRange<std::size_t>(1, 4), fe::Environment{});
        std::vector<std::vector<double>> samples(fdp.ConsumeIntegralInRange<std::size_t>(0, 4));
        for(auto& sample : samples){
            // mostly the right length, so the numeric values (not the size check) get exercised
            auto length = fdp.ConsumeBool() ? fe::FlowerClassifier::inputDim : fdp.ConsumeIntegralInRange<std::size_t>(0, fe::FlowerClassifier::inputDim + 2);
            sample.reserve(length);
            for(std::size_t i = 0; i < length; ++i){
                sample.push_back(nextValue(fdp));
            }
        }
        classifier.train(samples, fdp.ConsumeIntegralInRange<std::size_t>(0, 3), nextValue(fdp), nextValue(fdp),
            fdp.ConsumeIntegralInRange<std::size_t>(0, 5), fdp.ConsumeIntegralInRange<std::size_t>(0, 5));
        (void)classifier.evaluate(samples);
    }catch(...){
        // clean exceptions are expected, see fuzz_flower_from_json.cpp
    }
    return 0;
}
