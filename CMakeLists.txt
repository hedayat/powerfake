#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.13)

set(CMAKE_USER_MAKE_RULES_OVERRIDE "DefaultFlags.cmake")
project(powerfake VERSION 1.0.0
    DESCRIPTION "A tool fake free/non-virtual functions for C++"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

# Project Options
# =============================================================================
option(JUNIT_REPORT "Enable generating junit test report" OFF)
option(TEST_COVERAGE "Enable test coverage report" OFF)
option(ENABLE_FAKEIT "Enable building FakeIt integration samples" OFF)

# Look for dependencies
# =============================================================================
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(GTest)

if(TEST_COVERAGE AND NOT GCOVR_EXEC)
    message(STATUS "Test coverage report enabled")
    find_program(GCOVR_EXEC gcovr)
    if(GCOVR_EXEC)
        message(STATUS "Found gcovr binary: ${GCOVR_EXEC}")
    else()
        message(FATAL_ERROR "Cannot find gcovr binary")
    endif()
endif()

# General compiler settings
# =============================================================================
# Boost.Test needs ... DYN_LINK to link against their libs
add_compile_definitions(BOOST_ALL_DYN_LINK)

# =============================================================================
include(EmbedPowerFake.cmake)

if (BIND_FAKES_ONLY)
    return()
endif()

add_subdirectory(sample)
add_subdirectory(test EXCLUDE_FROM_ALL)

# =============================================================================
install(TARGETS powerfake pw_bindfakes bind_fakes EXPORT PowerFakeTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/powerfake
)

install(FILES powerfake.h DESTINATION include/powerfake)
install(DIRECTORY mocking/ DESTINATION include/powerfake)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PowerFake/PowerFakeConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(EXPORT PowerFakeTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/PowerFake/PowerFakeTargets.cmake"
    NAMESPACE PowerFake::
)
configure_file(cmake/PowerFakeConfig.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/PowerFake/PowerFakeConfig.cmake"
    COPYONLY
)
configure_file(cmake/PowerFakeFunctions.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/PowerFake/PowerFakeFunctions.cmake"
    COPYONLY
)

set(ConfigPackageLocation lib/cmake/PowerFake)
install(EXPORT PowerFakeTargets
    FILE
        PowerFakeTargets.cmake
    NAMESPACE
        PowerFake::
    DESTINATION
        ${ConfigPackageLocation}
)

install(
    FILES
        cmake/PowerFakeConfig.cmake
        cmake/PowerFakeFunctions.cmake
        "${CMAKE_CURRENT_BINARY_DIR}/PowerFake/PowerFakeConfigVersion.cmake"
    DESTINATION
        ${ConfigPackageLocation}
)
