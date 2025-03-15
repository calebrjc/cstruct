file(GLOB cstruct_test_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

foreach(test_source ${cstruct_test_sources})
    get_filename_component(test_name ${test_source} NAME_WE)

    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} cstruct)
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/minunit)
    target_compile_options(${test_name} PRIVATE -g -Wall -Wextra --pedantic-errors)

    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
