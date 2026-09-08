#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace fe_py{
    void bind_mutation_rates(nb::module_& m);
    void bind_dna(nb::module_& m);
    void bind_image(nb::module_& m);
    void bind_petals(nb::module_& m);
    void bind_stats(nb::module_& m);
    void bind_flower(nb::module_& m);
    void bind_api(nb::module_& m);
}

NB_MODULE(_core, m){
    m.doc() = "Native Python bindings for FlowerEvolver-WASM's C++ core -- "
              "generates 2D/3D flowers from CPPN genomes evolved via EvoAI.";

    fe_py::bind_mutation_rates(m);
    fe_py::bind_dna(m);
    fe_py::bind_image(m);
    fe_py::bind_petals(m);   // depends on Image (Petals.image) and its own Type enum
    fe_py::bind_stats(m);    // depends on nothing else bound here directly
    fe_py::bind_flower(m);   // depends on DNA and Petals (Flower.dna / Flower.petals)
    fe_py::bind_api(m);      // depends on DNA, Flower, Image, Stats (free function signatures)
}
