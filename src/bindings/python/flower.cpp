#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <fe/Flower.hpp>

#include "json_helpers.hpp"

namespace nb = nanobind;
using namespace nb::literals;

namespace fe_py{

void bind_flower(nb::module_& m){
    nb::class_<fe::Flower>(m, "Flower",
        "A complete flower: its genome (dna) and rendered image (petals). "
        "Usually created via make_flower()/reproduce()/mutate() rather than "
        "constructed directly.")
        .def(nb::init<>())
        .def_rw("dna", &fe::Flower::dna)
        .def_rw("petals", &fe::Flower::petals)
        .def("to_json", [](const fe::Flower& flower){
                JsonBox::Value v;
                v["Flower"] = flower.toJson();
                return jsonToString(v);
            },
            "Serializes to the same {\"Flower\": {\"dna\": ..., \"petals\": "
            "...}} JSON shape used throughout this project's ecosystem "
            "(the JS/WASM API, the native desktop app, Generation.json/"
            "Session.json's own per-flower entries) -- round-trips with "
            "Flower.from_json(), and interoperates with genomes saved by "
            "any of those.")
        .def_static("from_json", [](const std::string& s){
                JsonBox::Value v;
                v.loadFromString(s);
                if(v["Flower"]["dna"].isNull()){
                    throw std::invalid_argument("error, invalid flower, could not parse data.");
                }
                return fe::Flower(v["Flower"].getObject());
            }, "json_str"_a,
            "Inverse of to_json() - also accepts genomes produced by the "
            "JS/WASM API or the native desktop app, since they share this "
            "same wire format.\n\n"
            ":raises ValueError: if json_str doesn't contain a "
            "\"Flower\"/\"dna\" with at least 2 genomes.\n"
            ":raises RuntimeError: if json_str is malformed JSON entirely.")
        .def("__repr__", [](const fe::Flower& f){
            return "Flower(dna=DNA(size=" + std::to_string(f.dna.size()) + "))";
        });
}

}
