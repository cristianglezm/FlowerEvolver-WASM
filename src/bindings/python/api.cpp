#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>

#include <fe/FlowerEvolver.hpp>
#include <fe/config.hpp>

namespace nb = nanobind;
using namespace nb::literals;

namespace fe_py{

void bind_api(nb::module_& m){
    m.def("make_flower", &::makeFlower,
        "radius"_a, "num_layers"_a, "P"_a, "bias"_a,
        "Generates a brand new, random flower.\n\n"
        ":param radius: pixel radius, clamped to [4, 256].\n"
        ":param num_layers: clamped to [1, floor(log2(radius))].\n"
        ":param P: controls roughly how many petals the flower can have.\n"
        ":param bias: bias fed into the CPPN alongside radius/angle/layer.\n"
        ":return: a Flower -- .to_json() for the genome, .petals.image for "
        "the render.");

    m.def("make_petals", &::makePetals,
        "radius"_a, "num_layers"_a, "P"_a, "bias"_a,
        "Same as make_flower(), but renders just the petals (no stem).");

    m.def("make_petal_layer", &::makePetalLayer,
        "radius"_a, "num_layers"_a, "P"_a, "bias"_a, "layer"_a,
        "Same as make_flower(), but renders just one petal layer (no stem).\n\n"
        ":param layer: which layer to render, 0-indexed from the outermost.");

    m.def("make_stem", &::makeStem,
        "radius"_a, "num_layers"_a, "P"_a, "bias"_a,
        "Same as make_flower(), but renders just a stem (no petals).");

    m.def("draw_flower", &::drawFlower,
        "dna"_a, "radius"_a, "num_layers"_a, "P"_a, "bias"_a,
        "Renders an existing DNA (e.g. from Flower.dna, or "
        "DNA.from_json()) instead of generating a new one.\n\n"
        ":param dna: the genome to render; needs at least 2 genomes.\n"
        ":param radius: pixel radius, clamped to [4, 256].\n"
        ":param num_layers: clamped to [1, floor(log2(radius))].\n"
        ":param P: controls roughly how many petals the flower can have.\n"
        ":param bias: bias fed into the CPPN alongside radius/angle/layer.\n"
        ":return: the rendered Image.\n"
        ":raises ValueError: if dna has fewer than 2 genomes.");

    m.def("draw_petals", &::drawPetals,
        "dna"_a, "radius"_a, "num_layers"_a, "P"_a, "bias"_a,
        "Same as draw_flower(), but renders just the petals (no stem).");

    m.def("draw_petal_layer", &::drawPetalLayer,
        "dna"_a, "radius"_a, "num_layers"_a, "P"_a, "bias"_a, "layer"_a,
        "Same as draw_flower(), but renders just one petal layer (no stem).\n\n"
        ":param layer: which layer to render, 0-indexed from the outermost.");

    m.def("reproduce", &::reproduce,
        "dna1"_a, "dna2"_a, "radius"_a, "num_layers"_a, "P"_a, "bias"_a,
        "Breeds two genomes into a rendered child Flower. For crossover "
        "only, with no rendering, see DNA.reproduce() instead.\n\n"
        ":param dna1: the father's genome.\n"
        ":param dna2: the mother's genome; must have the same number of "
        "genomes as dna1.\n"
        ":param radius: pixel radius, clamped to [4, 256].\n"
        ":param num_layers: clamped to [1, floor(log2(radius))].\n"
        ":param P: controls roughly how many petals the flower can have.\n"
        ":param bias: bias fed into the CPPN alongside radius/angle/layer.\n"
        ":raises RuntimeError: if dna1 and dna2 don't have the same number "
        "of genomes.");

    m.def("mutate", &::mutate,
        "dna"_a, "radius"_a, "num_layers"_a, "P"_a, "bias"_a,
        "add_node_rate"_a = 0.2f, "add_conn_rate"_a = 0.3f,
        "remove_conn_rate"_a = 0.2f, "perturb_weights_rate"_a = 0.6f,
        "enable_rate"_a = 0.35f, "disable_rate"_a = 0.3f, "act_type_rate"_a = 0.4f,
        "Mutates a copy of dna and renders the result -- dna itself is "
        "left untouched.\n\n"
        ":param dna: genome to mutate; needs at least 2 genomes.\n"
        ":param radius: pixel radius, clamped to [4, 256].\n"
        ":param num_layers: clamped to [1, floor(log2(radius))].\n"
        ":param P: controls roughly how many petals the flower can have.\n"
        ":param bias: bias fed into the CPPN alongside radius/angle/layer.\n"
        ":param add_node_rate: rate to add a new node.\n"
        ":param add_conn_rate: rate to add a new connection.\n"
        ":param remove_conn_rate: rate to remove an existing connection.\n"
        ":param perturb_weights_rate: rate to change a connection's weight.\n"
        ":param enable_rate: rate to enable a currently-disabled gene.\n"
        ":param disable_rate: rate to disable a currently-enabled gene.\n"
        ":param act_type_rate: rate to change a neuron's activation function.\n"
        ":raises ValueError: if dna has fewer than 2 genomes.");

    m.def("make_3d_flower", &::make3DFlower,
        "dna"_a, "radius"_a, "num_layers"_a, "P"_a, "bias"_a,
        "flower_id"_a, "flower_params"_a = "",
        "Generates a 3D flower model.\n\n"
        ":param dna: the genome to render; needs at least 2 genomes.\n"
        ":param radius: pixel radius, clamped to [4, 256].\n"
        ":param num_layers: clamped to [1, floor(log2(radius))].\n"
        ":param P: controls roughly how many petals the flower can have.\n"
        ":param bias: bias fed into the CPPN alongside radius/angle/layer.\n"
        ":param flower_id: a unique string identifying this flower (used "
        "in the model's own group names).\n"
        ":param flower_params: JSON string, e.g. '{\"sex\": 2, "
        "\"useNormals\": true, \"useEmissive\": false}'; \"\" for defaults.\n"
        ":return: the 3D model as a glTF 2.0 JSON string.\n"
        ":raises ValueError: if dna has fewer than 2 genomes, or radius/"
        "num_layers/flower_id are invalid.");

    m.def("get_flower_stats", &::getFlowerStats,
        "genome"_a, "humidity"_a, "temperature"_a, "altitude"_a, "terrain_type"_a,
        "Derives environment-dependent Stats from a flower's genome.\n\n"
        ":param genome: a Flower.to_json()-shaped JSON string.\n"
        ":param humidity: 0.0 to 1.0.\n"
        ":param temperature: degrees, same scale as the resulting Stats' "
        "min_temperature/max_temperature.\n"
        ":param altitude: meters above sea level.\n"
        ":param terrain_type: terrain type id.\n\n"
        ":raises ValueError: if genome doesn't contain a DNA with at least "
        "2 genomes.\n"
        ":raises RuntimeError: if genome is malformed JSON entirely.");

    m.def("get_version", &fe::version,
        "The semver this extension was built from, e.g. \"4.0.0\".");
    m.def("get_commit_hash", &fe::commitHash,
        "The short git commit hash this extension was built from, or "
        "\"unknown\" for a build outside a git checkout.");
    m.def("get_version_string", &fe::versionString,
        "get_version() + \"+\" + get_commit_hash(), e.g. \"4.0.0+a1b2c3d\" "
        "-- the one to put in a bug report.");
}

}
