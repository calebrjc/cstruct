cmake_minimum_required(VERSION 3.14)

project(cstructex LANGUAGES C)

set(cstructex_sources main.c)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${cstructex_example_sources})

add_executable(cstructex ${cstructex_sources})
target_sources(cstructex PRIVATE ${cstructex_sources})
target_link_libraries(cstructex cstruct)
