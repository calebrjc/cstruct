cmake_minimum_required(VERSION 3.16)

project(
    cstruct
    VERSION 0.0.1
    DESCRIPTION "Like the `struct` module in Python, but for C."
    LANGUAGES C)

option(CSTRUCT_DEV "Enable development features" OFF)
option(CSTRUCT_BUILD_TESTS "Build tests" OFF)
option(CSTRUCT_BUILD_EXAMPLES "Build examples" OFF)

if (CSTRUCT_DEV)
    set(CSTRUCT_BUILD_TESTS ON)
    set(CSTRUCT_BUILD_EXAMPLES ON)
endif ()

set(cstruct_sources src/cstruct.h src/cstruct.c)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${cstruct_sources})

add_library(cstruct)
target_sources(cstruct PRIVATE ${cstruct_sources})
target_include_directories(cstruct PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

if (CSTRUCT_DEV)
    target_compile_options(cstruct PRIVATE -g -Wall -Wextra --pedantic-errors)
endif ()

if (CSTRUCT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()

if (CSTRUCT_BUILD_EXAMPLES)
    add_subdirectory(example)
endif ()
