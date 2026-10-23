// libFuzzer harness for fe::FlowerClassifier(JsonBox::Object): the boundary FEService.loadSOM(),
// Python's FlowerClassifier.from_json() and every saved classifier go through. A loaded classifier
// is also serialized and asked for a match, since a SOM that loads but misbehaves is the dangerous case.
#include <cstddef>
#include <cstdint>
#include <string>

#include <fe/FlowerClassifier.hpp>
#include <JsonBox.h>


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
    std::string input(reinterpret_cast<const char*>(data), size);
    try{
        JsonBox::Value v;
        v.loadFromString(input);
        if(v.isObject()){
            fe::FlowerClassifier classifier(v.getObject());
            (void)classifier.toJson();
            (void)classifier.som.classify(std::vector<double>(fe::FlowerClassifier::inputDim, 0.5));
        }
    }catch(...){
        // clean exceptions are expected, see fuzz_flower_from_json.cpp
    }
    return 0;
}
