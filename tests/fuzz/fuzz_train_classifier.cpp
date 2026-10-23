// libFuzzer harness for trainFlowerClassifier(generationsJson, environment, config): the
// {"Generation": [...]} batch path every binding trains from (JSON parse, per-flower extraction,
// skip-and-log policy, SOM training). The grid and epochs are kept small so a valid seed stays fast;
// the numbers FuzzedDataProvider hands out are otherwise unconstrained, validate() has to reject them.
// Run with -max_len large enough for a few genomes (see tests/fuzz/README.md).
#include <cstddef>
#include <cstdint>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include <fe/FlowerEvolver.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
    FuzzedDataProvider fdp(data, size);
    try{
        fe::Environment environment(fdp.ConsumeFloatingPoint<float>(), fdp.ConsumeIntegral<int>(),
            fdp.ConsumeIntegral<int>(), fdp.ConsumeIntegral<int>());
        fe::SOMTrainingConfig training(fdp.ConsumeIntegralInRange<std::size_t>(1, 4), fdp.ConsumeIntegralInRange<std::size_t>(1, 4),
            fdp.ConsumeIntegralInRange<std::size_t>(1, 2), fdp.ConsumeFloatingPoint<double>(), fdp.ConsumeFloatingPoint<double>());
        auto classifier = trainFlowerClassifier(fdp.ConsumeRemainingBytesAsString(), environment, training);
        (void)classifier;
    }catch(...){
        // clean exceptions are expected, see fuzz_flower_from_json.cpp
    }
    return 0;
}
