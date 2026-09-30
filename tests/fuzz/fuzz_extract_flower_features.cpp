// libFuzzer harness for fe::extractFlowerFeatures(const std::string& genome,
// const fe::Environment& environment) - mirrors fuzz_stats_from_genome.cpp's
// approach (this function ends up calling fe::Stats internally, plus its own
// JSON parsing + fe::detail::impl_drawPetals on top), splitting the fuzzed
// byte stream into the 4 fe::Environment numbers via FuzzedDataProvider and
// a genome string from whatever bytes are left.
#include <cstddef>
#include <cstdint>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include <fe/FlowerClassifier.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
    FuzzedDataProvider fdp(data, size);

    float humidity = fdp.ConsumeFloatingPoint<float>();
    int temperature = fdp.ConsumeIntegral<int>();
    int altitude = fdp.ConsumeIntegral<int>();
    int terrainType = fdp.ConsumeIntegral<int>();
    fe::Environment environment(humidity, temperature, altitude, terrainType);
    std::string genome = fdp.ConsumeRemainingBytesAsString();

    try{
        auto features = fe::extractFlowerFeatures(genome, environment);
        (void)features;
    }catch(...){
        // see fuzz_flower_from_json.cpp's own comment: clean exceptions
        // are expected and uninteresting here, ASan/UBSan catch the
        // memory-safety/UB issues this harness actually exists to find.
    }
    return 0;
}
