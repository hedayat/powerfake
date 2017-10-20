project(powerfake)

cmake_minimum_required(VERSION 3.5.1)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS core)

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

add_subdirectory(test EXCLUDE_FROM_ALL)

include_directories(${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/boost_test)


function(build_bind_fakes wrap_library_target)
    set(pair_sources powerfake  SymbolAliasMap)
    set(powerfake_sources $<JOIN:${pair_sources},.cpp > bind_fakes.cpp)
    set(powerfake_headers $<JOIN:${pair_sources},.h >
        nmreader.h  piperead.h
    )
    
    add_executable(bind_fakes ${powerfake_sources} ${powerfake_headers})
    
    # Add flags to ignore undefined reference to __real_ function calls, which we
    # don't use
    set_property(TARGET bind_fakes APPEND PROPERTY
        LINK_FLAGS "-Wl,--warn-unresolved-symbols -Wl,-z,lazy")
    set_property(TARGET bind_fakes APPEND PROPERTY COMPILE_DEFINITIONS BIND_FAKES)
    target_link_libraries(bind_fakes
        -Wl,--whole-archive ${wrap_library_target} -Wl,--no-whole-archive)
endfunction(build_bind_fakes)