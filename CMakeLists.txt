cmake_minimum_required(VERSION 3.27)

project(FlowerEvolver VERSION 1.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

option(FE_EXPORT "ES6 module export for (vue, react, angular, etc)" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

if(EMSCRIPTEN)
    add_executable(FlowerEvolver)
    target_include_directories(FlowerEvolver PUBLIC "include")
    target_sources(FlowerEvolver
        PUBLIC
            "include/SFML/Graphics/Color.hpp"
            "include/SFML/Graphics/Rect.hpp"
            "include/SFML/Graphics/Rect.inl"
            "include/SFML/System/Vector2.hpp"
            "include/SFML/System/Vector2.inl"
            "include/DNA.hpp"
            "include/Flower.hpp"
            "include/FlowerEvolver.hpp"
	        "include/MathUtils.hpp"
            "include/Petals.hpp"
        PRIVATE
            "src/SFML/Graphics/Color.cpp"
            "src/DNA.cpp"
            "src/Flower.cpp"
            "src/FlowerEvolver.cpp"
            "src/MathUtils.cpp"
            "src/Petals.cpp"
    )
    set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/public")
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/public")
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
    SET(CMAKE_CXX_STANDARD 17)
    SET(CMAKE_CXX_STANDARD_REQUIRED ON)
    SET(CMAKE_CXX_EXTENSIONS OFF)
	if(FE_EXPORT)
		target_compile_options(FlowerEvolver PRIVATE -std=c++17 -Wall -Wextra -Wshadow -pthread -fexceptions)
		target_link_options(FlowerEvolver PRIVATE -lembind -sPTHREAD_POOL_SIZE=4 -fexceptions --embind-emit-tsd FlowerEvolver.d.ts -sMODULARIZE=1 -sEXPORT_ES6=1 -sSINGLE_FILE=1 -sEXPORT_NAME='fe' -sTOTAL_MEMORY=2MB)
	else()
		target_compile_options(FlowerEvolver PRIVATE -std=c++17 -Wall -Wextra -Wshadow -pthread -fexceptions -sSTRICT=1)
		target_link_options(FlowerEvolver PRIVATE -lembind -sPTHREAD_POOL_SIZE=4 -fexceptions -sTOTAL_MEMORY=2MB)
	endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(FlowerEvolver PRIVATE -O3)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(FlowerEvolver PRIVATE -g3 -sASSERTIONS)
    endif()

    if(FE_EXPORT)
	    add_custom_target(dist
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md" "${CMAKE_CURRENT_SOURCE_DIR}/dist/LICENSE.md"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/include/SFML/license.md" "${CMAKE_CURRENT_SOURCE_DIR}/dist/SFML-LICENSE.md"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/index.js" "${CMAKE_CURRENT_SOURCE_DIR}/dist/index.js"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/package.json" "${CMAKE_CURRENT_SOURCE_DIR}/dist/package.json"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${CMAKE_CURRENT_SOURCE_DIR}/dist/README.md"
			COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_BINARY_DIR}/public/FlowerEvolver.js" "${CMAKE_CURRENT_SOURCE_DIR}/dist/public/FlowerEvolver.mjs"
			COMMAND ${CMAKE_COMMAND} -E copy "${PROJECT_BINARY_DIR}/public/FlowerEvolver.d.ts" "${CMAKE_CURRENT_SOURCE_DIR}/dist/public/FlowerEvolver.d.ts"
			COMMENT "built dist dir..."
			DEPENDS FlowerEvolver)
    else()
	    add_custom_target(dist
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md" "${CMAKE_CURRENT_SOURCE_DIR}/dist/LICENSE.md"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/include/SFML/license.md" "${CMAKE_CURRENT_SOURCE_DIR}/dist/SFML-LICENSE.md"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/index.html" "${CMAKE_CURRENT_SOURCE_DIR}/dist/index.html"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/index.js" "${CMAKE_CURRENT_SOURCE_DIR}/dist/index.js"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/package.json" "${CMAKE_CURRENT_SOURCE_DIR}/dist/package.json"
			COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${CMAKE_CURRENT_SOURCE_DIR}/dist/README.md"
			COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_BINARY_DIR}/public" "${CMAKE_CURRENT_SOURCE_DIR}/dist/public/"
			COMMENT "built dist dir..."
			DEPENDS FlowerEvolver)
    endif()

    install(FILES index.html DESTINATION ./public/index.html)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/public DESTINATION .)
else()
    message(FATAL_ERROR "This is only intended for Emscripten compilation")
endif()
