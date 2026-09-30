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

include(FetchContent)

set(FE_DEPENDENCIES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../.devops/dependencies.env")
if(NOT EXISTS "${FE_DEPENDENCIES_FILE}")
    message(FATAL_ERROR "${FE_DEPENDENCIES_FILE} is missing: it pins the versions of JsonBox and EvoAI that are fetched")
endif()
file(STRINGS "${FE_DEPENDENCIES_FILE}" _fe_pins REGEX "^(JSONBOX|EVOAI)_REF=")
foreach(_pin IN LISTS _fe_pins)
    if(_pin MATCHES "^([A-Z]+)_REF=(.+)$")
        # (not in the condition of the next if(): ${CMAKE_MATCH_1} is expanded before the match is made)
        set(_pin_name "${CMAKE_MATCH_1}")
        set(_pin_value "${CMAKE_MATCH_2}")
        if(NOT DEFINED FE_${_pin_name}_REF)
            set(FE_${_pin_name}_REF "${_pin_value}")
        endif()
    endif()
endforeach()
foreach(_name JSONBOX EVOAI)
    if(NOT FE_${_name}_REF)
        message(FATAL_ERROR "${FE_DEPENDENCIES_FILE} has to pin ${_name}_REF")
    endif()
endforeach()

function(fe_keep_out_of_the_wheel directory)
    set_property(DIRECTORY "${directory}" PROPERTY EXCLUDE_FROM_ALL TRUE)
endfunction()

find_package(JsonBox QUIET)
if(NOT JsonBox_FOUND)
    message(STATUS "JsonBox not found locally -- fetching and building from source (pass -Ccmake.define.JsonBox_ROOT=/path/to/install to use a local build instead)")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    message(STATUS "JsonBox ${FE_JSONBOX_REF}")
    FetchContent_Declare(JsonBox
        GIT_REPOSITORY https://github.com/cristianglezm/JsonBox.git
        GIT_TAG ${FE_JSONBOX_REF}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JsonBox)
    fe_keep_out_of_the_wheel("${jsonbox_SOURCE_DIR}")
    set(JsonBox_INCLUDE_DIR "${jsonbox_SOURCE_DIR}/include")
    set(JsonBox_LIBRARIES JsonBox)
endif()

set(EvoAI_BUILD_STATIC TRUE)
find_package(EvoAI 1 QUIET)
if(NOT EvoAI_FOUND)
    message(STATUS "EvoAI not found locally -- fetching and building from source (pass -Ccmake.define.EvoAI_ROOT=/path/to/install to use a local build instead)")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(EvoAI_BUILD_STATIC TRUE CACHE BOOL "" FORCE)
    message(STATUS "EvoAI ${FE_EVOAI_REF}")
    FetchContent_Declare(EvoAI
        GIT_REPOSITORY https://github.com/cristianglezm/EvoAI.git
        GIT_TAG ${FE_EVOAI_REF}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(EvoAI)
    fe_keep_out_of_the_wheel("${evoai_SOURCE_DIR}")
    set(EvoAI_INCLUDE_DIR "${EvoAI_SOURCE_DIR}/include")
    set(EvoAI_LIBRARIES EvoAI)
endif()
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
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/Environment.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/SOMTrainingConfig.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/SOMEvaluation.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/fe/FlowerClassifier.cpp"
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
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/bindings/python/som.cpp"
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
