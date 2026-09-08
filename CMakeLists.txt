cmake_minimum_required(VERSION 3.27)

file(STRINGS "${CMAKE_SOURCE_DIR}/package.json" FE_VERSION_LINE
     REGEX "\"version\"[ \t]*:[ \t]*\"[^\"]+\"")
list(GET FE_VERSION_LINE 0 FE_VERSION_LINE)
string(REGEX REPLACE ".*\"version\"[ \t]*:[ \t]*\"([^\"]+)\".*" "\\1"
       FE_VERSION "${FE_VERSION_LINE}")

find_package(Git QUIET)
set(FE_COMMIT_HASH "unknown")
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE FE_COMMIT_HASH_OUT OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET RESULT_VARIABLE FE_GIT_RESULT)
    if(FE_GIT_RESULT EQUAL 0 AND FE_COMMIT_HASH_OUT)
        set(FE_COMMIT_HASH "${FE_COMMIT_HASH_OUT}")
    endif()
endif()

project(FlowerEvolver VERSION ${FE_VERSION})

configure_file("${CMAKE_SOURCE_DIR}/include/fe/config.hpp.template"
                "${CMAKE_BINARY_DIR}/include/fe/config.hpp" @ONLY)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
option(FlowerEvolver_BUILD_STATIC "Build static" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sources shared by every target below (Emscripten module, native static lib,
# native shared lib)
set(FE_CORE_PUBLIC_HEADERS
    "include/fe.hpp"
    "include/fe/Color.hpp"
    "include/fe/Rect.hpp"
    "include/fe/Rect.inl"
    "include/fe/Vector2.hpp"
    "include/fe/Vector2.inl"
    "include/fe/DNA.hpp"
    "include/fe/Flower.hpp"
    "include/fe/Stats.hpp"
    "include/fe/MathUtils.hpp"
    "include/fe/Image.hpp"
    "include/fe/Petals.hpp"
    "include/fe/FlowerEvolver.hpp"
    "include/fe/detail/FlowerEvolver.hpp"
    "include/fe/3D/GLTF/Vertex.hpp"
    "include/fe/3D/GLTF/TextureInfo.hpp"
    "include/fe/3D/GLTF/Material.hpp"
    "include/fe/3D/GLTF/Mesh.hpp"
    "include/fe/3D/GLTF/Node.hpp"
    "include/fe/3D/GLTF/Scene.hpp"
    "include/fe/3D/GLTF/JsonWriter.hpp"
    "include/fe/3D/GLTF.hpp"
    "include/fe/3D/Resources.hpp"
    "include/fe/3D/Vec.hpp"
    "include/fe/3D/Vec.inl"
    "include/fe/3D/FlowerParameters.hpp"
    "include/fe/3D/utils.hpp"
    "include/fe/3D/contourFinder.hpp"
    "include/fe/3D/contourSimplifier.hpp"
    "include/fe/3D/meshGenerator.hpp"
    "include/fe/3D.hpp"
)

set(FE_CORE_SOURCES
    "src/fe/Color.cpp"
    "src/fe/DNA.cpp"
    "src/fe/Flower.cpp"
    "src/fe/Stats.cpp"
    "src/fe/MathUtils.cpp"
    "src/fe/Image.cpp"
    "src/fe/Petals.cpp"
    "src/fe/detail/FlowerEvolver.cpp"
    "src/fe/3D/GLTF/Vertex.cpp"
    "src/fe/3D/GLTF/TextureInfo.cpp"
    "src/fe/3D/GLTF/Material.cpp"
    "src/fe/3D/GLTF/Mesh.cpp"
    "src/fe/3D/GLTF/Node.cpp"
    "src/fe/3D/GLTF/Scene.cpp"
    "src/fe/3D/GLTF/JsonWriter.cpp"
    "src/fe/3D/FlowerParameters.cpp"
    "src/fe/3D/utils.cpp"
    "src/fe/3D/contourFinder.cpp"
    "src/fe/3D/contourSimplifier.cpp"
    "src/fe/3D/meshGenerator.cpp"
)

if(EMSCRIPTEN)
    add_executable(FlowerEvolver)
    target_include_directories(FlowerEvolver PUBLIC "include" "${CMAKE_BINARY_DIR}/include")
    target_sources(FlowerEvolver
        PUBLIC
            ${FE_CORE_PUBLIC_HEADERS}
        PRIVATE
            ${FE_CORE_SOURCES}
            "src/bindings/emscripten.cpp"
    )
    target_compile_definitions(FlowerEvolver PRIVATE FE_STATIC)
    set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/fe/public")
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/fe/public")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    find_package(JsonBox REQUIRED)
    if(JsonBox_FOUND)
        target_include_directories(FlowerEvolver PUBLIC ${JsonBox_INCLUDE_DIR})
        target_link_libraries(FlowerEvolver PUBLIC ${JsonBox_LIBRARIES})
    endif()
    set(EvoAI_BUILD_STATIC TRUE)
    find_package(EvoAI 1 REQUIRED)
    if(EvoAI_FOUND)
        target_include_directories(FlowerEvolver PRIVATE ${EvoAI_INCLUDE_DIR})
        target_link_libraries(FlowerEvolver PRIVATE ${EvoAI_LIBRARIES})
    endif()
	target_compile_options(FlowerEvolver PRIVATE -std=c++17 -Wall -Wextra -Wshadow -fexceptions)
	target_link_options(FlowerEvolver PRIVATE -lembind -fexceptions -sMODULARIZE=1 -sEXPORT_ES6=1 -sSINGLE_FILE=1 -sEXPORT_NAME='fe' -sTOTAL_MEMORY=20MB -sEXPORTED_RUNTIME_METHODS=['HEAPU8'])
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(FlowerEvolver PRIVATE -O3)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(FlowerEvolver PRIVATE -g3 -sASSERTIONS)
    endif()

    add_custom_command(TARGET FlowerEvolver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/types/wasm-module.d.ts"
            "${PROJECT_BINARY_DIR}/fe/public/FlowerEvolver.d.ts"
        COMMENT "copying wasm-module.d.ts next to the built FlowerEvolver.js")

	add_custom_target(dist
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md" "${CMAKE_CURRENT_SOURCE_DIR}/dist/LICENSE.md"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/include/fe/SFML-LICENSE.md" "${CMAKE_CURRENT_SOURCE_DIR}/dist/SFML-LICENSE.md"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/index.html" "${CMAKE_CURRENT_SOURCE_DIR}/dist/index.html"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/build/ts/index.js" "${CMAKE_CURRENT_SOURCE_DIR}/dist/index.js"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/build/ts/index.d.ts" "${CMAKE_CURRENT_SOURCE_DIR}/dist/index.d.ts"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/package.json" "${CMAKE_CURRENT_SOURCE_DIR}/dist/package.json"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${CMAKE_CURRENT_SOURCE_DIR}/dist/README.md"
			COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/types" "${CMAKE_CURRENT_SOURCE_DIR}/dist/types"
			COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_BINARY_DIR}/fe/public/FlowerEvolver.js" "${CMAKE_CURRENT_SOURCE_DIR}/dist/public/FlowerEvolver.js"
			COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_BINARY_DIR}/fe/public/FlowerEvolver.d.ts" "${CMAKE_CURRENT_SOURCE_DIR}/dist/public/FlowerEvolver.d.ts"
			COMMENT "built dist dir..."
			DEPENDS FlowerEvolver)
    
    cmake_policy(SET CMP0177 NEW)
    install(FILES index.html DESTINATION ./public/index.html)
    install(DIRECTORY ${PROJECT_BINARY_DIR}/fe/public DESTINATION .)
else()
    find_package(JsonBox REQUIRED)
    set(EvoAI_BUILD_STATIC TRUE)
    find_package(EvoAI 1 REQUIRED)
    include(GNUInstallDirs)
    set(FE_ANDROID_LIB_SUFFIX "")
    if(CMAKE_SYSTEM_NAME MATCHES "Android")
        set(FE_ANDROID_LIB_SUFFIX "/${CMAKE_ANDROID_ARCH_ABI}")
    endif()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(FE_CONFIG "Shared")
    if(FlowerEvolver_BUILD_STATIC)
        set(FE_CONFIG "Static")
    endif()
    if(NOT FlowerEvolver_BUILD_STATIC)
        add_library(FlowerEvolver SHARED ${FE_CORE_SOURCES} "src/bindings/native.cpp")
        target_compile_definitions(FlowerEvolver PRIVATE FE_EXPORTS)
        set_target_properties(FlowerEvolver PROPERTIES DEBUG_POSTFIX -d)
    else()
        add_library(FlowerEvolver STATIC ${FE_CORE_SOURCES} "src/bindings/native.cpp")
        target_compile_definitions(FlowerEvolver PUBLIC FE_STATIC)
        set_target_properties(FlowerEvolver PROPERTIES
            DEBUG_POSTFIX -s-d
            RELEASE_POSTFIX -s
            MINSIZEREL_POSTFIX -s
        )
    endif()

    if(EvoAI_BUILD_STATIC)
        target_compile_definitions(FlowerEvolver PUBLIC EvoAI_BUILD_STATIC)
    endif()

    add_library(FlowerEvolver::FlowerEvolver ALIAS FlowerEvolver)

    target_include_directories(FlowerEvolver
        PUBLIC
            "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
            "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
            "$<BUILD_INTERFACE:${JsonBox_INCLUDE_DIR}>"
            "$<BUILD_INTERFACE:${EvoAI_INCLUDE_DIR}>"
    )
    target_link_libraries(FlowerEvolver
        PUBLIC
            ${JsonBox_LIBRARIES}
            ${EvoAI_LIBRARIES}
    )
    if(MSVC)
        target_compile_options(FlowerEvolver PRIVATE /W4)
    else()
        target_compile_options(FlowerEvolver PRIVATE -Wall -Wextra -Wshadow)

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            target_compile_options(FlowerEvolver PRIVATE -O3)
        elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(FlowerEvolver PRIVATE -g3)
        endif()
    endif()
    set_target_properties(FlowerEvolver PROPERTIES
        POSITION_INDEPENDENT_CODE ON
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "FlowerEvolver-${PROJECT_VERSION_MAJOR}"
    )

    install(TARGETS FlowerEvolver
        EXPORT FlowerEvolver${FE_CONFIG}Targets
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}${FE_ANDROID_LIB_SUFFIX}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}${FE_ANDROID_LIB_SUFFIX}"
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING
            PATTERN "*.hpp"
            PATTERN "*.inl"
    )
    # the generated header, not the template, consumers never see @FE_VERSION@
    install(FILES "${CMAKE_BINARY_DIR}/include/fe/config.hpp" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/fe")
    if(CMAKE_SYSTEM_NAME MATCHES "Android")
        install(FILES "${CMAKE_SOURCE_DIR}/src/Android.mk" DESTINATION .)
    endif()
    install(FILES "${CMAKE_SOURCE_DIR}/LICENSE.md" "${CMAKE_SOURCE_DIR}/README.md" "${CMAKE_SOURCE_DIR}/include/fe/SFML-LICENSE.md"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/FlowerEvolver"
    )

    if(NOT CMAKE_SYSTEM_NAME MATCHES "Android")
        include(CMakePackageConfigHelpers)

        set(FE_PACKAGE_CONFIG_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/FlowerEvolver")

        install(EXPORT FlowerEvolver${FE_CONFIG}Targets
            FILE FlowerEvolver${FE_CONFIG}Targets.cmake
            NAMESPACE FlowerEvolver::
            DESTINATION "${FE_PACKAGE_CONFIG_DESTINATION}"
        )
        install(FILES
                "${CMAKE_SOURCE_DIR}/cmake/modules/FindEvoAI.cmake"
                "${CMAKE_SOURCE_DIR}/cmake/modules/FindJsonBox.cmake"
            DESTINATION "${FE_PACKAGE_CONFIG_DESTINATION}/modules"
        )

        configure_package_config_file(
            "${CMAKE_SOURCE_DIR}/cmake/FlowerEvolverConfig.cmake.in"
            "${CMAKE_BINARY_DIR}/FlowerEvolverConfig.cmake"
            INSTALL_DESTINATION "${FE_PACKAGE_CONFIG_DESTINATION}"
        )
        write_basic_package_version_file(
            "${CMAKE_BINARY_DIR}/FlowerEvolverConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY SameMajorVersion
        )
        install(FILES
                "${CMAKE_BINARY_DIR}/FlowerEvolverConfig.cmake"
                "${CMAKE_BINARY_DIR}/FlowerEvolverConfigVersion.cmake"
            DESTINATION "${FE_PACKAGE_CONFIG_DESTINATION}"
        )
    endif()
    option(FlowerEvolver_BUILD_TESTS "Build C++ unit tests (Catch2, native only)" OFF)
    if(FlowerEvolver_BUILD_TESTS)
        include(FetchContent)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.7.1
        )
        FetchContent_MakeAvailable(Catch2)
        list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
        enable_testing()
        add_subdirectory(tests/cpp)
    endif()

    option(FlowerEvolver_BUILD_FUZZERS "Build libFuzzer harnesses (Clang only, native only)" OFF)
    if(FlowerEvolver_BUILD_FUZZERS)
        if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            message(FATAL_ERROR "FlowerEvolver_BUILD_FUZZERS requires Clang (got ${CMAKE_CXX_COMPILER_ID}) -- "
                "reconfigure with -DCMAKE_CXX_COMPILER=clang++.")
        endif()
        add_subdirectory(tests/fuzz)
    endif()
    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_PACKAGE_VENDOR "Cristian Gonzalez")
    set(CPACK_PACKAGE_DESCRIPTION "FlowerEvolver-WASM native library: generates 2D/3D flowers from CPPN genomes evolved via EvoAI.")
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://cristianglezm.github.io/FlowerEvolver-WASM")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
    set(FE_CPACK_PLATFORM_NAME "${CMAKE_SYSTEM_NAME}")
    if(CMAKE_SYSTEM_NAME MATCHES "Android")
        set(FE_CPACK_PLATFORM_NAME "Android-${CMAKE_ANDROID_ARCH_ABI}")
    endif()
    if(FlowerEvolver_BUILD_STATIC)
        set(CPACK_PACKAGE_FILE_NAME "FlowerEvolver-${FE_VERSION}-${FE_CPACK_PLATFORM_NAME}-static")
    else()
        set(CPACK_PACKAGE_FILE_NAME "FlowerEvolver-${FE_VERSION}-${FE_CPACK_PLATFORM_NAME}-shared")
    endif()
    include(CPack)
endif()
