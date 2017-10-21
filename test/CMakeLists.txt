include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/boost_test)

set(test_sources test_main.cpp powerfake_test.cpp)

# Create a sample library, to test symbol processing in powerfake
add_library(sample_lib STATIC sample.cpp)

# Test runner build
add_executable(test_runner ${test_sources})
target_link_libraries(test_runner pw_bindfakes ${Boost_LIBRARIES})

# Normal test target
add_custom_target(test_normal
    COMMAND test_runner --log_level=test_suite --color_output
        -- --sample-lib $<TARGET_FILE:sample_lib>
    DEPENDS test_runner)


# Test coverage build
add_executable(test_runner_coverage ${test_sources})
target_link_libraries(test_runner_coverage pw_bindfakes_coverage --coverage
  ${Boost_LIBRARIES})

# Coverage test target
add_custom_target(test_coverage
    COMMAND test_runner_coverage --log_level=test_suite --color_output
        -- --sample-lib $<TARGET_FILE:sample_lib>
    COMMAND ${CMAKE_SOURCE_DIR}/third_party/gcovr -e "/.*third_party\\/.*/" -r ${CMAKE_SOURCE_DIR}
    DEPENDS test_runner_coverage)


# Define check target (NOTE: 'test' target is reserved by CMake)
set(ENABLE_COVERAGE "$ENV{TEST_COVERAGE_REPORT}$ENV{CI}")
if (ENABLE_COVERAGE)
    add_custom_target(check DEPENDS test_coverage)
else (ENABLE_COVERAGE)
    add_custom_target(check DEPENDS test_normal)
endif (ENABLE_COVERAGE)
