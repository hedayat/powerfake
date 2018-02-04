#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

set(POWERFAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

include(${POWERFAKE_DIR}/cmake/PowerFakeFunctions.cmake)

add_library(powerfake STATIC ${POWERFAKE_DIR}/powerfake.cpp
    ${POWERFAKE_DIR}/powerfake.h)
add_library(PowerFake::powerfake ALIAS powerfake)

set(pair_sources ${POWERFAKE_DIR}/powerfake ${POWERFAKE_DIR}/SymbolAliasMap
    ${POWERFAKE_DIR}/NMSymbolReader ${POWERFAKE_DIR}/Reader)
set(powerfake_sources $<JOIN:${pair_sources},.cpp >
    ${POWERFAKE_DIR}/bind_fakes.cpp)
set(powerfake_headers $<JOIN:${pair_sources},.h >)

add_library(pw_bindfakes STATIC ${powerfake_sources} ${powerfake_headers})
set_property(TARGET pw_bindfakes APPEND PROPERTY COMPILE_DEFINITIONS BIND_FAKES)
add_library(PowerFake::pw_bindfakes ALIAS pw_bindfakes)

if (NOT ${PROJECT_NAME} STREQUAL "powerfake")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
    add_executable(bind_fakes ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
    # Add flags to ignore undefined reference to __real_ function calls, which we
    # don't use
    set_property(TARGET bind_fakes APPEND PROPERTY
        LINK_FLAGS "-Wl,--gc-sections")
    target_link_libraries(bind_fakes PowerFake::pw_bindfakes)
endif (NOT ${PROJECT_NAME} STREQUAL "powerfake")
