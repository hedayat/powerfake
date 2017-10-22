cmake_minimum_required(VERSION 3.5.1)

project(powerfake VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

find_package(Boost REQUIRED)

link_directories(${Boost_LIBRARY_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

# =============================================================================
if(CMAKE_COMPILER_IS_GNUCXX)
    set(EXTRA_FLAGS "-pthread -Wall -Wextra -Woverloaded-virtual")

    # Add some security flags
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG(-fstack-protector-strong STRONG_STACK_PROTECT)
    if (STRONG_STACK_PROTECT)
        set(EXTRA_FLAGS "${EXTRA_FLAGS} -fstack-protector-strong")
    endif(STRONG_STACK_PROTECT)

    if (NOT CMAKE_BUILD_TYPE)
        set(EXTRA_FLAGS "${EXTRA_FLAGS} -O2 -g")
    endif (NOT CMAKE_BUILD_TYPE)

    if(NOT CMAKE_BUILD_TYPE MATCHES "Debug")
        set(EXTRA_FLAGS "${EXTRA_FLAGS} -Wp,-D_FORTIFY_SOURCE=2")
        set(EXTRA_FLAGS "${EXTRA_FLAGS} -mtune=native")
    endif(NOT CMAKE_BUILD_TYPE MATCHES "Debug")

endif(CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} $ENV{CXXFLAGS} ${EXTRA_FLAGS}")

# =============================================================================

include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/boost_test)

include(EmbedPowerFake.cmake)

add_library(pw_bindfakes_coverage STATIC EXCLUDE_FROM_ALL
    ${powerfake_sources} ${powerfake_headers})
target_compile_options(pw_bindfakes_coverage PRIVATE --coverage -O0 -g)
set_property(TARGET pw_bindfakes_coverage APPEND PROPERTY
    COMPILE_DEFINITIONS BIND_FAKES)

add_subdirectory(test EXCLUDE_FROM_ALL)

# =============================================================================
install(TARGETS powerfake pw_bindfakes EXPORT PowerFakeTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES powerfake.h DESTINATION include)

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
