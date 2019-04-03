cmake_minimum_required(VERSION 3.10)
project(hfetch-test-app)
set(CTEST_ENVIRONMENT "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/")
set(ENV{LD_PRELOAD} "ENV{LD_PRELOAD} ${CMAKE_BINARY_DIR}")

#Execute basic examples
set(examples montage_analysis wrf_analysis montage_analysis_pfs wrf_analysis_pfs)
foreach (example ${examples})
    add_executable (${example} ${example}.cpp ../util.h)
    add_dependencies(${example} hfetch)
    if(EXISTS ${CMAKE_SOURCE_DIR}/test/apps/${example}.csv)
        add_custom_command(TARGET ${example} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/test/apps/${example}.csv $<TARGET_FILE_DIR:${example}>)
    endif()
    target_include_directories(${example} PRIVATE "${CMAKE_BINARY_DIR}/")
    set_target_properties(${example} PROPERTIES ENV TEST_ENV="LD_PRELOAD=-L${CMAKE_BINARY_DIR}/libhfetch.so")
    target_link_libraries(${example} ${LIB_FLAGS} -L${CMAKE_BINARY_DIR}/ -lhfetch)
    set_target_properties (${example} PROPERTIES FOLDER test/apps)
    set (test_parameters "")
    add_test(NAME ${example} COMMAND "$<TARGET_FILE:${example}>" ${test_parameters})
    set_tests_properties(${example} PROPERTIES ENV TEST_ENV="LD_PRELOAD=${CMAKE_BINARY_DIR}/libhfetch.so")
    set_tests_properties(${example} PROPERTIES WILL_FAIL true)
endforeach()