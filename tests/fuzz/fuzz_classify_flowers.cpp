// libFuzzer harness for classifyFlowers(classifier, generationsJson, environment) against the
// embedded classifier: the batch JSON walk and per-flower extraction, where a bad flower must come
// back as std::nullopt and never as an exception or a crash.
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include <fe/FlowerEvolver.hpp>
#include <fe/Resources.hpp>

namespace{
    const fe::FlowerClassifier& embeddedClassifier(){
        static const fe::FlowerClassifier classifier = []{
            JsonBox::Value v;
            v.loadFromString(fe::resources::trainedSOM);
            return fe::FlowerClassifier(v.getObject());
        }();
        return classifier;
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
    FuzzedDataProvider fdp(data, size);
    try{
        fe::Environment environment(fdp.ConsumeFloatingPoint<float>(), fdp.ConsumeIntegral<int>(),
            fdp.ConsumeIntegral<int>(), fdp.ConsumeIntegral<int>());
        auto matches = classifyFlowers(embeddedClassifier(), fdp.ConsumeRemainingBytesAsString(), environment);
        (void)matches;
    }catch(...){
        // clean exceptions are expected, see fuzz_flower_from_json.cpp
    }
    return 0;
}
