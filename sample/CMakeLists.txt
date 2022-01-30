#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/mocking)

if (ENABLE_FAKEIT)
    include_directories(../third_party/FakeIt/include
        ../third_party/FakeIt/config/standalone)
    add_compile_definitions(ENABLE_FAKEIT)
endif ()

# include(${CMAKE_SOURCE_DIR}/third_party/powerfake/EmbedPowerFake.cmake)
# Use the above include() command in separate projects

add_library(corelib STATIC functions.cpp functions.h SampleClass.cpp SampleClass.h)

set(test_sources faked.cpp)
add_library(wrap_lib STATIC wrap.cpp)

# Test runner build
add_executable(samples ${test_sources})
target_link_libraries(samples wrap_lib corelib)
bind_fakes(samples SUBJECT corelib WRAPPERS wrap_lib STANDALONE CACHE TIMING)

# Can also use simple invocation form with no options:
#bind_fakes(samples corelib wrap_lib)

if (GTest_FOUND)
    add_compile_definitions(ENABLE_GMOCK)
    target_link_libraries(samples GTest::gmock)
endif ()
