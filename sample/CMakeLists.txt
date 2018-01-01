include_directories(${CMAKE_SOURCE_DIR})

# include(${CMAKE_SOURCE_DIR}/third_party/powerfake/EmbedPowerFake.cmake)

# Use the above include() command in separate projects instead of the below block
###################
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
add_executable(bind_fakes ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
# Add flags to ignore undefined reference to __real_ function calls, which we
# don't use
set_property(TARGET bind_fakes APPEND PROPERTY
    LINK_FLAGS "-Wl,--warn-unresolved-symbols -Wl,-z,lazy")
target_link_libraries(bind_fakes pw_bindfakes)
###################

add_library(corelib functions.cpp functions.h SampleClass.cpp SampleClass.h)

set(test_sources faked.cpp)
add_library(wrap_lib STATIC wrap.cpp)
# TODO: (oldubuntu) can be replaced by OBJECT type, which is illegal in U 16.04
# cmake which allows it in add_executable & add_library only)

# Test runner build
add_executable(samples ${test_sources})
target_link_libraries(samples wrap_lib corelib powerfake)
bind_fakes(samples corelib wrap_lib)
