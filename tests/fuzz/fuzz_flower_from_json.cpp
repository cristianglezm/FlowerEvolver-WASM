// libFuzzer harness for fe::Flower(JsonBox::Object) -- the exact boundary
// Flower::from_json() (Python), the JS bindings' parseDna()+Flower
// construction, and the CLI's -lf/-repr all funnel attacker-controlled
// bytes through eventually. See tests/fuzz/README.md for how to build and
// run this.
#include <cstddef>
#include <cstdint>
#include <string>

#include <fe/Flower.hpp>
#include <JsonBox.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size){
    std::string input(reinterpret_cast<const char*>(data), size);
    try{
        JsonBox::Value v;
        v.loadFromString(input);
        if(v.isObject()){
            fe::Flower flower(v.getObject());
            (void)flower;
        }
    }catch(...){
        // Clean C++ exceptions (malformed JSON, JsonBox parsing errors,
        // missing keys turning into a thrown getObject() on a non-object
        // Value, etc.) are expected and not what this harness is looking
        // for - ASan/UBSan abort the process outright for the
        // memory-safety/UB issues that actually matter, independently of
        // this catch block, so nothing needs re-throwing or inspecting
        // here.
    }
    return 0;
}
