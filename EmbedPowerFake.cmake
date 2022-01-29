#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

set(POWERFAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

include(${POWERFAKE_DIR}/cmake/PowerFakeFunctions.cmake)

if (NOT BIND_FAKES_ONLY)
    add_library(powerfake STATIC ${POWERFAKE_DIR}/powerfake.cpp
        ${POWERFAKE_DIR}/powerfake.h)
    target_link_libraries(powerfake PUBLIC Boost::boost)
    add_library(PowerFake::powerfake ALIAS powerfake)
endif()

set(pair_sources ${POWERFAKE_DIR}/powerfake ${POWERFAKE_DIR}/SymbolAliasMap
    ${POWERFAKE_DIR}/NMSymbolReader ${POWERFAKE_DIR}/Reader
    ${POWERFAKE_DIR}/ParseUtils)
set(bindfakes_core_sources $<JOIN:${pair_sources},.cpp >.cpp)
set(bindfakes_core_headers $<JOIN:${pair_sources},.h >.h)

add_library(pw_bindfakes STATIC ${POWERFAKE_DIR}/bind_fakes.cpp
    ${bindfakes_core_sources} ${bindfakes_core_headers})
set_property(TARGET pw_bindfakes APPEND PROPERTY COMPILE_DEFINITIONS BIND_FAKES)
target_link_libraries(pw_bindfakes PUBLIC Boost::boost)
add_library(PowerFake::pw_bindfakes ALIAS pw_bindfakes)

if (MINGW)
    set(CMAKE_CROSSCOMPILING_EMULATOR "wine")
endif()

# Build standalone bind_fakes executable
# -----------------------------------------------------------------------------
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_executable(bind_fakes ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
target_link_libraries(bind_fakes PowerFake::pw_bindfakes)

if (CMAKE_CROSSCOMPILING)
    include(ExternalProject)
    ExternalProject_Add(host_bind_fakes PREFIX host_bind_fakes
        SOURCE_DIR ${POWERFAKE_DIR}
        BINARY_DIR host_bind_fakes/build
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DBIND_FAKES_ONLY=ON
        INSTALL_COMMAND "" EXCLUDE_FROM_ALL ON)

    # Add an imported library for date
    add_executable(native_bind_fakes IMPORTED)
    add_dependencies(native_bind_fakes host_bind_fakes)
    set_target_properties(native_bind_fakes PROPERTIES
        IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/host_bind_fakes/build/bind_fakes)
    add_executable(PowerFake::bind_fakes ALIAS native_bind_fakes)
else()
    add_executable(PowerFake::bind_fakes ALIAS bind_fakes)
endif()
