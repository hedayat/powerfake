function(bind_fakes target_name test_lib wrapper_funcs_lib)
    if (NOT BIND_FAKES_LINKED)
        set(BIND_FAKES_LINKED 1 PARENT_SCOPE)
        target_link_libraries(bind_fakes pw_bindfakes
            -Wl,--whole-archive ${wrapper_funcs_lib} -Wl,--no-whole-archive
            $<TARGET_PROPERTY:${target_name},LINK_LIBRARIES>)
    endif (NOT BIND_FAKES_LINKED)

    add_custom_command(TARGET ${target_name} PRE_LINK
        COMMAND bind_fakes $<TARGET_FILE:${test_lib}>
                $<TARGET_FILE:${wrapper_funcs_lib}>)

    # Add powerfake link flags
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY
        LINK_FLAGS @${CMAKE_CURRENT_BINARY_DIR}/powerfake.link_flags)
endfunction(bind_fakes)
