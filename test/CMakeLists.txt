#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

include_directories(${CMAKE_SOURCE_DIR})

if(JUNIT_REPORT)
    message(STATUS "Test results output: ${CMAKE_CURRENT_BINARY_DIR}/test_log.xml")
    set(TEST_LOG_PARAMS --log_sink=${CMAKE_CURRENT_BINARY_DIR}/test_log.xml
            --log_format=JUNIT)
endif()

# Test Internal Dependencies
# =============================================================================
# BindFakes library, with coverage info included
add_library(bindfakes_core_coverage STATIC
    ${bindfakes_core_sources} ${bindfakes_core_headers})
target_compile_options(bindfakes_core_coverage PRIVATE --coverage -O0 -g)
target_link_libraries(bindfakes_core_coverage PUBLIC Boost::boost)
set_property(TARGET bindfakes_core_coverage APPEND PROPERTY
    COMPILE_DEFINITIONS BIND_FAKES)

# Create a sample library, to test symbol processing in powerfake
add_library(sample_lib STATIC sample.cpp)

# Test sources
# =============================================================================
set(test_sources test_main.cpp powerfake_test.cpp)

# Normal test runner
# =============================================================================
add_executable(test_runner ${test_sources})
target_link_libraries(test_runner pw_bindfakes Boost::unit_test_framework)

# Normal test target
add_custom_target(test_normal
    COMMAND test_runner ${TEST_LOG_PARAMS} --log_level=test_suite --color_output
        -- --sample-lib $<TARGET_FILE:sample_lib>
    DEPENDS test_runner)

# Test runner with test coverage report
# =============================================================================
add_executable(test_runner_coverage ${test_sources})
target_link_libraries(test_runner_coverage bindfakes_core_coverage --coverage
   Boost::unit_test_framework)

# Coverage test target
add_custom_target(test_coverage
    COMMAND test_runner_coverage ${TEST_LOG_PARAMS} --log_level=test_suite
        --color_output
        -- --sample-lib $<TARGET_FILE:sample_lib>
    COMMAND gcovr -e ${CMAKE_SOURCE_DIR}/third_party -e ${CMAKE_BINARY_DIR}
        -r ${CMAKE_SOURCE_DIR}
    DEPENDS test_runner_coverage)


# Define test target
# =============================================================================
if(TEST_COVERAGE)
    add_custom_target(test DEPENDS test_coverage)
else()
    add_custom_target(test DEPENDS test_normal)
endif()
