#  Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#             http://www.boost.org/LICENSE_1_0.txt)

include_directories(${CMAKE_SOURCE_DIR})

# include(${CMAKE_SOURCE_DIR}/third_party/powerfake/EmbedPowerFake.cmake)
# Use the above include() command in separate projects

add_library(corelib STATIC functions.cpp functions.h SampleClass.cpp SampleClass.h)

set(test_sources faked.cpp)
add_library(wrap_lib STATIC wrap.cpp)
# TODO: (oldubuntu) can be replaced by OBJECT type, which is illegal in U 16.04
# cmake which allows it in add_executable & add_library only)

# Test runner build
add_executable(samples ${test_sources})
target_link_libraries(samples wrap_lib corelib)
bind_fakes(samples corelib wrap_lib)
