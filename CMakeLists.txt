project(powerfake)

cmake_minimum_required(VERSION 3.5.1)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

add_library(powerfake STATIC powerfake.cpp powerfake.h)

set(pair_sources powerfake  SymbolAliasMap)
set(powerfake_sources $<JOIN:${pair_sources},.cpp > bind_fakes.cpp)
set(powerfake_headers $<JOIN:${pair_sources},.h >
    nmreader.h  piperead.h
)

add_library(pw_bindfakes STATIC ${powerfake_sources} ${powerfake_headers})
set_property(TARGET pw_bindfakes APPEND PROPERTY COMPILE_DEFINITIONS BIND_FAKES)

add_library(pw_bindfakes_coverage STATIC EXCLUDE_FROM_ALL
    ${powerfake_sources} ${powerfake_headers})
target_compile_options(pw_bindfakes_coverage PRIVATE --coverage -O0 -g)

function(build_bind_fakes wrap_library_target)
    
    add_executable(bind_fakes)
    
    # Add flags to ignore undefined reference to __real_ function calls, which we
    # don't use
    set_property(TARGET bind_fakes APPEND PROPERTY
        LINK_FLAGS "-Wl,--warn-unresolved-symbols -Wl,-z,lazy")
    target_link_libraries(bind_fakes pw_bindfakes
        -Wl,--whole-archive ${wrap_library_target} -Wl,--no-whole-archive)
endfunction(build_bind_fakes)

function(bind_fakes target_name test_lib wrapper_funcs_lib)
    add_custom_command(TARGET ${target_name} PRE_LINK
        COMMAND bind_fakes $<TARGET_FILE:${test_lib}> $<TARGET_FILE:${wrapper_funcs_lib}>)

    # Add powerfake link flags
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY
        LINK_FLAGS @${CMAKE_CURRENT_BINARY_DIR}/powerfake.link_flags)
endfunction(bind_fakes)

add_subdirectory(test EXCLUDE_FROM_ALL)
