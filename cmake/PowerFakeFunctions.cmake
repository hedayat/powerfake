function(bind_fakes target_name test_lib wrapper_funcs_lib)
    target_link_libraries(${wrapper_funcs_lib} PowerFake::powerfake)

    set(CMAKE_CROSSCOMPILING_EMULATOR "wine")

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
    set(bind_fakes_tgt bind_fakes_${target_name})
    add_executable(${bind_fakes_tgt} EXCLUDE_FROM_ALL
        ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
    # Remove __real_ & __wrap_ functions to prevent undefined reference errors
    # These are not used by bind_fakes
    set_property(TARGET ${bind_fakes_tgt} APPEND PROPERTY
        LINK_FLAGS "-Wl,--gc-sections")
    if (MINGW)
        # Create a duplicate target with BIND_FAKES flag set, as --gc-sections
        # doesn't remove unused __wrap_ functions created by WRAP_FUNCTION()
        # macros under MINGW
        set(bf_wrap_lib ${CMAKE_STATIC_LIBRARY_PREFIX}${wrapper_funcs_lib}_copy${CMAKE_STATIC_LIBRARY_SUFFIX})
        add_custom_command(OUTPUT ${bf_wrap_lib}
            DEPENDS ${wrapper_funcs_lib}
            COMMAND objcopy --remove-section=*wrapper_*_alias_*
                $<TARGET_FILE:${wrapper_funcs_lib}> ${bf_wrap_lib})
        add_custom_target(wrap_lib_copy_${target_name} DEPENDS ${bf_wrap_lib})
        add_dependencies(${bind_fakes_tgt} wrap_lib_copy_${target_name})
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(RUN_OPTIONS "--leading-underscore")
        endif()

        set(bf_wrap_lib "-L. ${bf_wrap_lib}")
    else (MINGW)
        set(bf_wrap_lib ${wrapper_funcs_lib})
    endif (MINGW)

    target_link_libraries(${bind_fakes_tgt} PowerFake::pw_bindfakes
        -Wl,--whole-archive ${bf_wrap_lib} -Wl,--no-whole-archive
        $<TARGET_PROPERTY:${target_name},LINK_LIBRARIES>)

    set(link_flags_file ${target_name}.powerfake.link_flags)
    set(link_script_file ${target_name}.powerfake.link_script)
    if (NOT CMAKE_CROSSCOMPILING)
        add_custom_command(OUTPUT ${link_flags_file} ${link_script_file}
            DEPENDS ${wrapper_funcs_lib}
            COMMAND ${bind_fakes_tgt} ${RUN_OPTIONS} ${ARGV3}
                    $<TARGET_FILE:${test_lib}> $<TARGET_FILE:${wrapper_funcs_lib}>
            COMMAND ${CMAKE_COMMAND} -E rename powerfake.link_flags ${link_flags_file}
            COMMAND ${CMAKE_COMMAND} -E rename powerfake.link_script ${link_script_file})
    else ()
        add_custom_command(OUTPUT ${link_flags_file} ${link_script_file}
            DEPENDS ${wrapper_funcs_lib}
            COMMAND nm -po $<TARGET_FILE:${test_lib}> > main.syms
            COMMAND nm -po $<TARGET_FILE:${wrapper_funcs_lib}> > wrap.syms
            COMMAND ${bind_fakes_tgt} ${RUN_OPTIONS} ${ARGV3} --passive main.syms wrap.syms
            COMMAND objcopy @wrap.syms.objcopy_params $<TARGET_FILE:${wrapper_funcs_lib}>
            COMMAND ${CMAKE_COMMAND} -E rename powerfake.link_flags ${link_flags_file}
            COMMAND ${CMAKE_COMMAND} -E rename powerfake.link_script ${link_script_file})
    endif ()
    add_custom_target(exec_bind_${target_name}
        DEPENDS ${link_flags_file} ${link_script_file})
    add_dependencies(${target_name} exec_bind_${target_name})

    # Add powerfake link flags
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY
        LINK_FLAGS "@${CMAKE_CURRENT_BINARY_DIR}/${link_flags_file} ${CMAKE_CURRENT_BINARY_DIR}/${link_script_file}")
    target_link_libraries(${target_name} PowerFake::powerfake)
endfunction(bind_fakes)
