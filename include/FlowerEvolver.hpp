#ifndef FLOWER_EVOLVER_HPP
#define FLOWER_EVOLVER_HPP

#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <emscripten.h>

#include <sstream>
#include <chrono>
#include <stdexcept>

#include <EvoAI.hpp>

#include <Flower.hpp>

/// global document access
thread_local const emscripten::val document = emscripten::val::global("document");
/**
 * @brief it will copy the ptr buffer into the canvas with id "canvas"
 * @param ptr std::uint8_t* buffer to copy to canvas
 * @param w int width for the canvas
 * @param h int height for the canvas
 */
void copyToCanvas(std::uint8_t* ptr, int w, int h);
/**
 * @brief make flower, it will paint into the canvas, is up to you to get the image from it.
 * 
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return it will return a stringified Flower.json needed to do reproduce or mutate.
 */
std::string makeFlower(int radius, int numLayers, float P, float bias) noexcept;
/**
 * @brief make flower, it will paint into the canvas, is up to you to get the image from it.
 * 
 * @param flower1 const std::string& stringified Flower.json
 * @param flower2 const std::string& stringified Flower.json
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return it will return a stringified Flower.json needed to do reproduce or mutate.
 */
std::string reproduce(const std::string& flower1, const std::string& flower2, int radius, int numLayers, float P, float bias);
/**
 * @brief make flower, it will paint into the canvas, is up to you to get the image from it.
 * 
 * @param flower1 const std::string& stringified Flower.json
 * @param flower2 const std::string& stringified Flower.json
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @param addNodeRate float rate to determine what it mutates
 * @param addConnRate float rate to determine what it mutates
 * @param removeConnRate float rate to determine what it mutates
 * @param perturbWeightsRate float rate to determine what it mutates
 * @param enableRate float rate to determine what it mutates
 * @param disableRate float rate to determine what it mutates
 * @param actTypeRate float rate to determine what it mutates
 * @return it will return a stringified Flower.json needed to do reproduce or mutate.
 */
std::string mutate(const std::string& original, int radius, int numLayers, float P, float bias, 
					float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate, 
					float enableRate, float disableRate, float actTypeRate);

EMSCRIPTEN_BINDINGS(makeFlower){
    emscripten::function("makeFlower", &makeFlower);
}
EMSCRIPTEN_BINDINGS(reproduce){
    emscripten::function("reproduce", &reproduce);
}
EMSCRIPTEN_BINDINGS(mutate){
    emscripten::function("mutate", &mutate);
}
#endif // FLOWER_EVOLVER_HPP