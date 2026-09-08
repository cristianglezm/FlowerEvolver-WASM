// libFuzzer harness for fe::Stats(const std::string& genome, float
// humidity, int temperature, int altitude, int terrainType) - the one
// entry point in this set that takes numeric params alongside the genome
// string, not just JSON. Splits the fuzzed byte stream into those 4
// numbers (via FuzzedDataProvider, LLVM's own header-only helper for
// exactly this - "structured" fuzzing instead of guessing at a manual
// byte layout) plus a genome string from whatever bytes are left, so the
// same run can turn up either a malformed-JSON bug (like the other two
// harnesses) or a bad-numeric-input bug (huge/negative/NaN humidity,
// INT_MIN altitude, ...) - resource-exhaustion and integer-overflow
// territory that the two JSON-only harnesses can't reach at all, since
// they never vary these.
#include <cstddef>
#include <cstdint>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include <fe/Stats.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
    FuzzedDataProvider fdp(data, size);

    float humidity = fdp.ConsumeFloatingPoint<float>();
    int temperature = fdp.ConsumeIntegral<int>();
    int altitude = fdp.ConsumeIntegral<int>();
    int terrainType = fdp.ConsumeIntegral<int>();
    std::string genome = fdp.ConsumeRemainingBytesAsString();

    try{
        fe::Stats stats(genome, humidity, temperature, altitude, terrainType);
        (void)stats;
    }catch(...){
        // see fuzz_flower_from_json.cpp's own comment: clean exceptions
        // are expected and uninteresting here, ASan/UBSan catch the
        // memory-safety/UB issues this harness actually exists to find.
    }
    return 0;
}
