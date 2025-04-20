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
 * @brief make petals (will only draw the petals), it will paint into the canvas, is up to you to get the image from it.
 * 
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return it will return a stringified Flower.json needed to do reproduce or mutate.
 */
std::string makePetals(int radius, int numLayers, float P, float bias) noexcept;
/**
 * @brief make petal layer (will only draw the petal layer), it will paint into the canvas, is up to you to get the image from it.
 * 
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @param layer int layer layer to paint
 * @return it will return a stringified Flower.json needed to do reproduce or mutate.
 */
std::string makePetalLayer(int radius, int numLayers, float P, float bias, int layer) noexcept;
/**
 * @brief make stem (will only draw the stem), it will paint into the canvas, is up to you to get the image from it.
 * 
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 * @return it will return a stringified Flower.json needed to do reproduce or mutate.
 */
std::string makeStem(int radius, int numLayers, float P, float bias) noexcept;
/**
 * @brief it will paint the given flower into the canvas, is up to you to get the image from it.
 * 
 * @param flower const std::string& stringified Flower.json
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 */
void drawFlower(const std::string& flower, int radius, int numLayers, float P, float bias);
/**
 * @brief it will paint the given flower petals(no stem) into the canvas, is up to you to get the image from it.
 * 
 * @param flower const std::string& stringified Flower.json
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 */
void drawPetals(const std::string& flower, int radius, int numLayers, float P, float bias);
/**
 * @brief it will paint the given flower petal layer(no stem) into the canvas, is up to you to get the image from it.
 * 
 * @param flower const std::string& stringified Flower.json
 * @param radius int radius for the flower
 * @param numLayers int how many layers it will have
 * @param P float P parameter it controls how many petals the flower can have.
 * @param bias float bias
 */
void drawPetalLayer(const std::string& flower, int radius, int numLayers, float P, float bias, int layer);
/**
 * @brief it makes a child from two flower genomes, it will paint into the canvas, is up to you to get the image from it.
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
 * @brief it makes flower from original (mutating it), it will paint into the canvas, is up to you to get the image from it.
 * 
 * @param original const std::string& stringified Flower.json to mutate
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
 * @return it will return a stringified Flower.json needed to do reproduce, mutate or drawFlower.
 */
std::string mutate(const std::string& original, int radius, int numLayers, float P, float bias, 
					float addNodeRate, float addConnRate, float removeConnRate, float perturbWeightsRate, 
					float enableRate, float disableRate, float actTypeRate);
/**
 * @brief gets the exception message
 * @param exceptionPtr std::exception*
 */
std::string getExceptionMessage(int exceptionPtr);

EMSCRIPTEN_BINDINGS(makeFlower){
    emscripten::function("makeFlower", &makeFlower);
}
EMSCRIPTEN_BINDINGS(makePetals){
    emscripten::function("makePetals", &makePetals);
}
EMSCRIPTEN_BINDINGS(makePetalLayer){
    emscripten::function("makePetalLayer", &makePetalLayer);
}
EMSCRIPTEN_BINDINGS(makeStem){
    emscripten::function("makeStem", &makeStem);
}
EMSCRIPTEN_BINDINGS(drawFlower){
    emscripten::function("drawFlower", &drawFlower);
}
EMSCRIPTEN_BINDINGS(drawPetals){
    emscripten::function("drawPetals", &drawPetals);
}
EMSCRIPTEN_BINDINGS(drawPetalLayer){
    emscripten::function("drawPetalLayer", &drawPetalLayer);
}
EMSCRIPTEN_BINDINGS(reproduce){
    emscripten::function("reproduce", &reproduce);
}
EMSCRIPTEN_BINDINGS(mutate){
    emscripten::function("mutate", &mutate);
}
EMSCRIPTEN_BINDINGS(getExceptionMessage) {
    emscripten::function("getExceptionMessage", &getExceptionMessage);
};
#endif // FLOWER_EVOLVER_HPP