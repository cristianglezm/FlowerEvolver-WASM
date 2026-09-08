#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <fe/Petals.hpp>

namespace nb = nanobind;

namespace fe_py{

void bind_petals(nb::module_& m){
    nb::enum_<fe::Petals::Type>(m, "PetalsType",
        "Which part of a flower to render - passed to some of the "
        "lower-level make_*()/draw_*() calls.")
        .value("TRUNK", fe::Petals::Type::Trunk, "Just the stem, no petals.")
        .value("PETALS", fe::Petals::Type::Petals, "Just the petals, no stem.")
        .value("TRUNK_AND_PETALS", fe::Petals::Type::TrunkAndPetals, "Both -- a complete flower.");

    nb::class_<fe::Petals>(m, "Petals",
        "The rendered-image half of a Flower (Flower.petals) - the "
        "generation parameters that produced it, plus the resulting image.")
        .def(nb::init<>(), "Defaults: radius=64, num_layers=3, P=6.0, bias=1.0.")
        .def(nb::init<int, int, float, float>(),
             nb::arg("radius"), nb::arg("num_layers"), nb::arg("P"), nb::arg("bias"),
             "radius is clamped to [4, 256]; num_layers is clamped to "
             "[1, floor(log2(radius))].")
        .def_rw("image", &fe::Petals::image)
        .def_rw("bias", &fe::Petals::bias)
        .def_rw("P", &fe::Petals::P)
        .def_rw("radius", &fe::Petals::radius)
        .def_rw("num_layers", &fe::Petals::numLayers)
        .def_rw("has_bloom", &fe::Petals::hasBloom)
        .def("__repr__", [](const fe::Petals& p){
            return "Petals(radius=" + std::to_string(p.radius) +
                   ", num_layers=" + std::to_string(p.numLayers) +
                   ", P=" + std::to_string(p.P) +
                   ", bias=" + std::to_string(p.bias) + ")";
        });
}

}
