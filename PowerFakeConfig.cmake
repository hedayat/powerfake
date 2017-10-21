include(CMakeFindDependencyMacro)
find_dependency(Boost)

include("${CMAKE_CURRENT_LIST_DIR}"/PowerFakeTargets.cmake)

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
