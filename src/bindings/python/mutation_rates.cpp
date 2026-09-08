#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <fe/DNA.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace fe_py{

void bind_mutation_rates(nb::module_& m){
    nb::class_<fe::MutationRates>(m, "MutationRates",
        "Rates controlling how DNA.mutate()/mutate() perturbs a genome. "
        "Every parameter is independent -- a given mutation call can add a "
        "node, add a connection, and perturb weights all at once, each "
        "gated by its own rate below.")
        .def(nb::init<>(), "Default rates: matches the values FlowerEvolver's "
             "own JS/WASM API (FEService.mutate) defaults to.")
        .def(nb::init<float, float, float, float, float, float, float>(),
             "add_node_rate"_a, "add_conn_rate"_a, "remove_conn_rate"_a,
             "perturb_weights_rate"_a, "enable_rate"_a, "disable_rate"_a,
             "act_type_rate"_a)
        .def_rw("add_node_rate", &fe::MutationRates::addNodeRate,
                "Rate to add a new node.")
        .def_rw("add_conn_rate", &fe::MutationRates::addConnRate,
                "Rate to add a new connection.")
        .def_rw("remove_conn_rate", &fe::MutationRates::removeConnRate,
                "Rate to remove an existing connection.")
        .def_rw("perturb_weights_rate", &fe::MutationRates::perturbWeightsRate,
                "Rate to change the weight of a connection.")
        .def_rw("enable_rate", &fe::MutationRates::enableRate,
                "Rate to enable a currently-disabled gene.")
        .def_rw("disable_rate", &fe::MutationRates::disableRate,
                "Rate to disable a currently-enabled gene.")
        .def_rw("act_type_rate", &fe::MutationRates::actTypeRate,
                "Rate to change a neuron's activation function.")
        .def("__repr__", [](const fe::MutationRates& mr){
            return "MutationRates(add_node_rate=" + std::to_string(mr.addNodeRate) +
                   ", add_conn_rate=" + std::to_string(mr.addConnRate) +
                   ", remove_conn_rate=" + std::to_string(mr.removeConnRate) +
                   ", perturb_weights_rate=" + std::to_string(mr.perturbWeightsRate) +
                   ", enable_rate=" + std::to_string(mr.enableRate) +
                   ", disable_rate=" + std::to_string(mr.disableRate) +
                   ", act_type_rate=" + std::to_string(mr.actTypeRate) + ")";
        });
}

}
