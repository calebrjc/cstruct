cmake_minimum_required(VERSION 3.14)

project(binpack-example LANGUAGES C)

set(binpack_example_sources main.c)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${binpack_example_sources})

add_executable(binpack-example ${binpack_example_sources})
target_sources(binpack-example PRIVATE ${binpack_example_sources})
target_link_libraries(binpack-example binpack)
