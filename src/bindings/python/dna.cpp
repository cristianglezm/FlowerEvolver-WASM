#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <fe/DNA.hpp>

#include "json_helpers.hpp"

namespace nb = nanobind;
using namespace nb::literals;

namespace fe_py{

// fe::DNA wraps a private std::vector<EvoAI::Genome>. EvoAI::Genome has
// no Python binding of its own, so DNA::add()/remove()/operator[] are
// deliberately NOT bound here - handing back a raw EvoAI::Genome
// reference to Python would be a type nanobind has nothing to do with.
// From Python, DNA is an opaque, sized container: you can measure it
// (size()/__len__), read/write its aggregate fitness, mutate it in
// place, cross two of them, and round-trip it to/from JSON, but you
// can't reach into individual genomes directly - that matches how the
// rest of this API already treats DNA (make_flower()/reproduce()/
// mutate() are the only ways most code should ever get one).
void bind_dna(nb::module_& m){
    nb::class_<fe::DNA>(m, "DNA",
        "The genome underlying a Flower - a small collection of CPPN "
        "genomes (a stats genome plus another for petals), evolved "
        "via EvoAI. Most code creates these indirectly, through make_flower()/"
        "reproduce()/mutate() rather than by hand. Individual genomes "
        "aren't exposed to Python (EvoAI::Genome has no binding of its "
        "own) -- treat DNA as an opaque, sized container.")
        .def(nb::init<>(), "Starts empty (size() == 0).")
        .def(nb::init<const fe::DNA&>(), "other"_a, "Deep copy of another DNA.")
        .def("size", &fe::DNA::size, "Number of genomes this DNA holds.")
        .def("__len__", &fe::DNA::size)
        .def("clear", &fe::DNA::clear, "Removes every genome, leaving size() == 0.")
        .def("get_fitness", &fe::DNA::getFitness,
             "Reads back the fitness set via set_fitness() (from genome[0]); "
             "0.0 if this DNA is empty.")
        .def("set_fitness", &fe::DNA::setFitness, "fitness"_a,
             "Applies the same fitness value to every genome this DNA holds.")
        .def("mutate", &fe::DNA::mutate, "rates"_a,
             "Mutates every existing genome in place (topology and "
             "weights), according to `rates`. This never adds or removes "
             "genomes -- size() before and after is always identical; "
             "what changes is each genome's internal structure, not how "
             "many there are.")
        .def_static("reproduce", &fe::DNA::reproduce, "dna1"_a, "dna2"_a,
             "Combines two DNA into a child DNA - crossover only, no "
             "rendering (compare to the top-level reproduce(), which also "
             "renders the result into a Flower). Both DNA must have the "
             "same number of genomes.\n\n"
             ":raises RuntimeError: if dna1 and dna2 don't have the same "
             "number of genomes.")
        .def_static("distance", &fe::DNA::distance, "dna1"_a, "dna2"_a,
             "Genetic distance between two DNA (0.0 for identical genomes). "
             "Both DNA must have the same number of genomes.\n\n"
             ":raises RuntimeError: if dna1 and dna2 don't have the same "
             "number of genomes.")
        .def("to_json", [](const fe::DNA& dna){
                return jsonToString(dna.toJson());
            },
            "Serializes to a compact JSON string -- this DNA's own genomes "
            "array, not wrapped in the outer {\"Flower\": ...} shape "
            "Flower.to_json() produces (a bare DNA isn't a complete, "
            "renderable Flower on its own). Round-trips with DNA.from_json().")
        .def_static("from_json", [](const std::string& s){
                return fe::DNA(parseJsonObject(s, "dna"));
            }, "json_str"_a,
            "Inverse of to_json().\n\n"
            ":raises ValueError: if json_str doesn't parse as a JSON object.\n"
            ":raises RuntimeError: if json_str is malformed JSON entirely.")
        .def("__repr__", [](const fe::DNA& dna){
            return "DNA(size=" + std::to_string(dna.size()) + ")";
        });
}

}
