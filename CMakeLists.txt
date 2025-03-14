cmake_minimum_required(VERSION 3.16)

project(
    binpack
    VERSION 0.0.1
    DESCRIPTION "Like the `struct` module from Python, but for C."
    LANGUAGES C)

option(BINPACK_BUILD_TESTS "Build tests" OFF)
option(BINPACK_BUILD_EXAMPLES "Build examples" OFF)

set(bpk_sources include/jcsw/binpack.h src/binpack.c)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${bpk_sources})

add_library(binpack)
target_sources(binpack PRIVATE ${bpk_sources})
target_include_directories(binpack PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

if (BINPACK_BUILD_TESTS)
    # ...
endif ()

if (BINPACK_BUILD_EXAMPLES)
    add_subdirectory(example)
endif ()
