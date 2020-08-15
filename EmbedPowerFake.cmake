#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

set(POWERFAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

include(${POWERFAKE_DIR}/cmake/PowerFakeFunctions.cmake)

add_library(powerfake STATIC ${POWERFAKE_DIR}/powerfake.cpp
    ${POWERFAKE_DIR}/powerfake.h)
target_link_libraries(powerfake PUBLIC Boost::boost)
add_library(PowerFake::powerfake ALIAS powerfake)

set(pair_sources ${POWERFAKE_DIR}/powerfake ${POWERFAKE_DIR}/SymbolAliasMap
    ${POWERFAKE_DIR}/NMSymbolReader ${POWERFAKE_DIR}/Reader)
set(bindfakes_core_sources $<JOIN:${pair_sources},.cpp >)
set(bindfakes_core_headers $<JOIN:${pair_sources},.h >)

add_library(pw_bindfakes STATIC ${POWERFAKE_DIR}/bind_fakes.cpp
    ${bindfakes_core_sources} ${bindfakes_core_headers})
set_property(TARGET pw_bindfakes APPEND PROPERTY COMPILE_DEFINITIONS BIND_FAKES)
target_link_libraries(pw_bindfakes PUBLIC Boost::boost)
add_library(PowerFake::pw_bindfakes ALIAS pw_bindfakes)
