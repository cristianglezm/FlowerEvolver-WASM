// libFuzzer harness for fe::DNA(JsonBox::Object) -- one level below
// fe::Flower's own constructor (see fuzz_flower_from_json.cpp), and the
// one that actually reaches into EvoAI::Genome's own JSON parsing for
// each element of the "genomes" array. EvoAI is a separate dependency
// this project doesn't control the source of - fuzzing at this boundary
// covers bugs there too, not just in this project's own DNA.cpp.
#include <cstddef>
#include <cstdint>
#include <string>

#include <fe/DNA.hpp>
#include <JsonBox.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
    std::string input(reinterpret_cast<const char*>(data), size);
    try{
        JsonBox::Value v;
        v.loadFromString(input);
        if(v.isObject()){
            fe::DNA dna(v.getObject());
            (void)dna;
        }
    }catch(...){
        // see fuzz_flower_from_json.cpp's own comment: clean exceptions
        // are expected and uninteresting here, ASan/UBSan catch the
        // memory-safety/UB issues this harness actually exists to find.
    }
    return 0;
}
