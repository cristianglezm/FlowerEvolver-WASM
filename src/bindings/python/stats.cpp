#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <fe/Stats.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace fe_py{

void bind_stats(nb::module_& m){
    nb::enum_<fe::Stats::Sex>(m, "Sex")
        .value("MALE", fe::Stats::Sex::Male)
        .value("FEMALE", fe::Stats::Sex::Female)
        .value("BOTH", fe::Stats::Sex::Both);

    nb::class_<fe::Stats::Effects>(m, "Effects",
        "Per-stat effect strengths derived from the genome, roughly -100..100 each.")
        .def(nb::init<>())
        .def_rw("vitality", &fe::Stats::Effects::vitality)
        .def_rw("agility", &fe::Stats::Effects::agility)
        .def_rw("intelligence", &fe::Stats::Effects::intelligence)
        .def_rw("strength", &fe::Stats::Effects::strength)
        .def_rw("luck", &fe::Stats::Effects::luck)
        .def("__repr__", [](const fe::Stats::Effects& e){
            return "Effects(vitality=" + std::to_string(e.vitality) +
                   ", agility=" + std::to_string(e.agility) +
                   ", intelligence=" + std::to_string(e.intelligence) +
                   ", strength=" + std::to_string(e.strength) +
                   ", luck=" + std::to_string(e.luck) + ")";
        });

    nb::class_<fe::Stats>(m, "Stats",
        "Environment-dependent stats derived from a flower's genome - see "
        "get_flower_stats().")
        .def(nb::init<const std::string&, float, int, int, int>(),
             "genome"_a, "humidity"_a, "temperature"_a, "altitude"_a, "terrain_type"_a,
             "Usually created via get_flower_stats() instead of directly.\n\n"
             ":param genome: a Flower.to_json()-shaped JSON string.\n"
             ":param humidity: 0.0 to 1.0.\n"
             ":param temperature: degrees, same scale as min_temperature/"
             "max_temperature below.\n"
             ":param altitude: meters above sea level.\n"
             ":param terrain_type: terrain type id.\n\n"
             ":raises ValueError: if genome doesn't contain a DNA with at "
             "least 2 genomes.\n"
             ":raises RuntimeError: if genome is malformed JSON entirely.")
        .def_rw("sex", &fe::Stats::sex)
        .def_rw("health", &fe::Stats::health)
        .def_rw("stamina", &fe::Stats::stamina)
        .def_rw("min_temperature", &fe::Stats::minTemperature)
        .def_rw("max_temperature", &fe::Stats::maxTemperature)
        .def_rw("maturation_period", &fe::Stats::maturationPeriod)
        .def_rw("toxicity_rate", &fe::Stats::toxicityRate)
        .def_rw("effects", &fe::Stats::effects)
        .def("__repr__", [](const fe::Stats& s){
            return "Stats(health=" + std::to_string(s.health) +
                   ", stamina=" + std::to_string(s.stamina) + ", ...)";
        });
}

}
