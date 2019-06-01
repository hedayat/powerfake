function(bind_fakes target_name test_lib wrapper_funcs_lib)
    target_link_libraries(${wrapper_funcs_lib} PowerFake::powerfake)

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
    set(bind_fakes_tgt bind_fakes_${target_name})
    add_executable(${bind_fakes_tgt} EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
    # Remove __real_ & __wrap_ functions to prevent undefined reference errors
    # These are not used by bind_fakes
    set_property(TARGET ${bind_fakes_tgt} APPEND PROPERTY
        LINK_FLAGS "-Wl,--gc-sections")
    target_link_libraries(${bind_fakes_tgt} PowerFake::pw_bindfakes
        -Wl,--whole-archive ${wrapper_funcs_lib} -Wl,--no-whole-archive
        $<TARGET_PROPERTY:${target_name},LINK_LIBRARIES>)

    add_custom_command(TARGET ${target_name} PRE_LINK
        COMMAND ${bind_fakes_tgt} $<TARGET_FILE:${test_lib}>
                $<TARGET_FILE:${wrapper_funcs_lib}>)

    # Add powerfake link flags
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY
        LINK_FLAGS @${CMAKE_CURRENT_BINARY_DIR}/powerfake.link_flags)
    target_link_libraries(${target_name} PowerFake::powerfake)
endfunction(bind_fakes)
