cmake_minimum_required(VERSION 3.27)
project(FlowerEvolverDocs)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/../package.json" FE_VERSION_LINE
     REGEX "\"version\"[ \t]*:[ \t]*\"[^\"]+\"")
list(GET FE_VERSION_LINE 0 FE_VERSION_LINE)
string(REGEX REPLACE ".*\"version\"[ \t]*:[ \t]*\"([^\"]+)\".*" "\\1"
       FE_VERSION "${FE_VERSION_LINE}")

find_package(Doxygen REQUIRED dot)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/conf/doxyfile.in"
                "${CMAKE_CURRENT_BINARY_DIR}/doxyfile" @ONLY)

set(FE_DOXYGEN_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/doxygen/xml/index.xml")
add_custom_command(
    OUTPUT "${FE_DOXYGEN_OUTPUT}"
    COMMAND ${CMAKE_COMMAND} -E echo "-- Doxygen: extracting XML from include/ --"
    COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/doxyfile"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/conf/doxyfile.in"
    COMMENT "Running Doxygen"
)
add_custom_target(doxygen_docs DEPENDS "${FE_DOXYGEN_OUTPUT}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules")
find_package(Sphinx REQUIRED)

set(FE_SPHINX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/sphinx")
set(FE_SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/conf/conf.py.in" "${FE_SPHINX_BUILD_DIR}/conf.py" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/conf/index.rst.in" "${FE_SPHINX_BUILD_DIR}/index.rst" @ONLY)

add_custom_target(copy_rsts
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/source" "${FE_SPHINX_BUILD_DIR}/source"
    COMMENT "Copying docs/source/*.rst"
)

add_custom_target(copy_static
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/conf/_static" "${FE_SPHINX_BUILD_DIR}/_static"
    COMMENT "Copying docs/conf/_static/ (custom.css)"
)

add_custom_target(docs ALL
    COMMAND ${CMAKE_COMMAND} -E echo "- Sphinx: building HTML -"
    COMMAND ${SPHINX_EXECUTABLE} -b html -q
        "${FE_SPHINX_BUILD_DIR}"
        "${FE_SPHINX_HTML_DIR}"
    WORKING_DIRECTORY "${FE_SPHINX_BUILD_DIR}"
    COMMENT "Running Sphinx"
)
add_dependencies(docs copy_rsts copy_static doxygen_docs)
