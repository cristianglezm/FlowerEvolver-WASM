cmake_minimum_required(VERSION 3.27)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/../package.json" FE_VERSION_LINE
     REGEX "\"version\"[ \t]*:[ \t]*\"[^\"]+\"")
list(GET FE_VERSION_LINE 0 FE_VERSION_LINE)
string(REGEX REPLACE ".*\"version\"[ \t]*:[ \t]*\"([^\"]+)\".*" "\\1"
       FE_VERSION "${FE_VERSION_LINE}")

project(FlowerEvolverPython LANGUAGES CXX VERSION ${FE_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(nanobind CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules")

find_package(JsonBox REQUIRED)
set(EvoAI_BUILD_STATIC TRUE)
find_package(EvoAI 1 REQUIRED)
find_package(Git QUIET)
set(FE_COMMIT_HASH "unknown")
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE FE_COMMIT_HASH_OUT OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET RESULT_VARIABLE FE_GIT_RESULT)
    if(FE_GIT_RESULT EQUAL 0 AND FE_COMMIT_HASH_OUT)
        set(FE_COMMIT_HASH "${FE_COMMIT_HASH_OUT}")
    endif()
endif()
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../include/fe/config.hpp.template"
                "${CMAKE_CURRENT_BINARY_DIR}/include/fe/config.hpp" @ONLY)

set(FE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/Color.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/DNA.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/Flower.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/Stats.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/MathUtils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/Image.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/Petals.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/detail/FlowerEvolver.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/GLTF/Vertex.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/GLTF/TextureInfo.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/GLTF/Material.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/GLTF/Mesh.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/GLTF/Node.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/GLTF/Scene.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/GLTF/JsonWriter.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/FlowerParameters.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/utils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/contourFinder.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/contourSimplifier.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/3D/meshGenerator.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/native.cpp"
)

# The nanobind glue 
set(FE_PYTHON_BINDING_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/python/module.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/python/mutation_rates.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/python/dna.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/python/image.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/python/petals.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/python/stats.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/python/flower.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/python/api.cpp"
)

# NB_DOMAIN keeps this module's internal type-caster registrations isolated
# from any other nanobind extension that might end up loaded in the same
# Python process
nanobind_add_module(_core NB_DOMAIN flowerevolver
    ${FE_SOURCES}
    ${FE_PYTHON_BINDING_SOURCES}
)
target_include_directories(_core PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
    ${JsonBox_INCLUDE_DIR}
    ${EvoAI_INCLUDE_DIR}
)
target_link_libraries(_core PRIVATE
    ${JsonBox_LIBRARIES}
    ${EvoAI_LIBRARIES}
)
if(EvoAI_BUILD_STATIC)
    target_compile_definitions(_core PRIVATE EvoAI_BUILD_STATIC)
endif()

target_compile_definitions(_core PRIVATE FE_STATIC)
target_compile_options(_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall -Wextra -Wshadow>
)
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(_core PRIVATE -static-libgcc -static-libstdc++ -static)
endif()

install(TARGETS _core LIBRARY DESTINATION flowerevolver)
