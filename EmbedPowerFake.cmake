#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

set(POWERFAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

include(${POWERFAKE_DIR}/cmake/PowerFakeFunctions.cmake)

add_library(powerfake STATIC ${POWERFAKE_DIR}/powerfake.cpp
    ${POWERFAKE_DIR}/powerfake.h)
target_link_libraries(powerfake PUBLIC Boost::boost)
target_include_directories(powerfake PUBLIC
    $<BUILD_INTERFACE:${POWERFAKE_DIR}/third_party/ctti/include>)
add_library(PowerFake::powerfake ALIAS powerfake)

set(pair_sources ${POWERFAKE_DIR}/powerfake ${POWERFAKE_DIR}/SymbolAliasMap
    ${POWERFAKE_DIR}/NMSymbolReader ${POWERFAKE_DIR}/Reader
    ${POWERFAKE_DIR}/ParseUtils)
set(bindfakes_core_sources $<JOIN:${pair_sources},.cpp >.cpp)
set(bindfakes_core_headers $<JOIN:${pair_sources},.h >.h)

add_library(pw_bindfakes STATIC ${POWERFAKE_DIR}/bind_fakes.cpp
    ${bindfakes_core_sources} ${bindfakes_core_headers})
set_property(TARGET pw_bindfakes APPEND PROPERTY COMPILE_DEFINITIONS BIND_FAKES)
target_link_libraries(pw_bindfakes PUBLIC Boost::boost)
target_include_directories(pw_bindfakes PUBLIC
    $<BUILD_INTERFACE:${POWERFAKE_DIR}/third_party/ctti/include>)
add_library(PowerFake::pw_bindfakes ALIAS pw_bindfakes)

if (MINGW)
    set(CMAKE_CROSSCOMPILING_EMULATOR "wine")
endif()

add_executable(bind_fakes EXCLUDE_FROM_ALL)
target_link_libraries(bind_fakes PowerFake::pw_bindfakes)
