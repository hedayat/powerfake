include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party/boost_test powerfake)

set(test_sources test_main.cpp global_fixture.cpp FlowsQueryHelper_test.cpp
  QueryManager_test.cpp FlowInserter_test.cpp DBSetup.cpp FlowForwarder_test.cpp
  Configuration_test.cpp FlowReader_test.cpp powerfake/powerfake.cpp
  powerfake/powerfake_test.cpp powerfake/SymbolAliasMap.cpp)
add_library(wrap_lib STATIC wrap.cpp)
# TODO: (oldubuntu) can be replaced by OBJECT type, which is illegal in U 16.04
# cmake which allows it in add_executable & add_library only)

# Create a sample library, to test symbol processing in powerfake
add_library(sample_lib STATIC powerfake/sample.cpp)
add_custom_target(copy_sample_lib ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:sample_lib> ${CMAKE_SOURCE_DIR}/test/powerfake/)

# Power Fake
add_executable(bind_fakes powerfake/bind_fakes.cpp
    powerfake/powerfake.cpp powerfake/SymbolAliasMap.cpp)
# Add flags to ignore undefined reference to __real_ function calls, which we
# don't use
set_property(TARGET bind_fakes APPEND PROPERTY
    LINK_FLAGS "-Wl,--warn-unresolved-symbols -Wl,-z,lazy")
set_property(TARGET bind_fakes APPEND PROPERTY COMPILE_DEFINITIONS BIND_FAKES)
target_link_libraries(bind_fakes
    -Wl,--whole-archive wrap_lib -Wl,--no-whole-archive)

# Test runner build
add_executable(test_runner ${test_sources})
add_custom_command(TARGET test_runner PRE_LINK
    COMMAND bind_fakes $<TARGET_FILE:corelib> $<TARGET_FILE:wrap_lib>)
target_link_libraries(test_runner corelib wrap_lib ${PostgreSQL_LIBRARIES}
    ${Boost_LIBRARIES} ${POCO_LIBRARIES})

# Normal test target
add_custom_target(test_normal
    COMMAND test_runner --log_level=test_suite --color_output
        -- --test-dir ${CMAKE_SOURCE_DIR}/test/
    DEPENDS test_runner copy_sample_lib)


# Test coverage build
add_executable(test_runner_coverage ${test_sources})
add_custom_command(TARGET test_runner_coverage PRE_LINK
    COMMAND bind_fakes $<TARGET_FILE:corelib_coverage> $<TARGET_FILE:wrap_lib>)
target_link_libraries(test_runner_coverage corelib_coverage wrap_lib --coverage
  ${PostgreSQL_LIBRARIES} ${Boost_LIBRARIES} ${POCO_LIBRARIES})

# Coverage test target
add_custom_target(test_coverage
    COMMAND test_runner_coverage --log_level=test_suite --color_output
        -- --test-dir ${CMAKE_SOURCE_DIR}/test/
    COMMAND ${CMAKE_SOURCE_DIR}/third_party/gcovr -e "/.*third_party\\/.*/" -r ${CMAKE_SOURCE_DIR}
    DEPENDS test_runner_coverage copy_sample_lib)


# Add powerfake link flags
set_property(TARGET test_runner test_runner_coverage APPEND_STRING PROPERTY
    LINK_FLAGS @${CMAKE_CURRENT_BINARY_DIR}/powerfake.link_flags)

# Define check target (NOTE: 'test' target is reserved by CMake)
set(ENABLE_COVERAGE "$ENV{TEST_COVERAGE_REPORT}$ENV{CI}")
if (ENABLE_COVERAGE)
    add_custom_target(check DEPENDS test_coverage)
else (ENABLE_COVERAGE)
    add_custom_target(check DEPENDS test_normal)
endif (ENABLE_COVERAGE)
