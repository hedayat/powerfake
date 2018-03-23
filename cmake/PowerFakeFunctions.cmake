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
        set(bf_wrap_lib ${wrapper_funcs_lib}_copy)
        add_library(${bf_wrap_lib} STATIC
            $<TARGET_PROPERTY:${wrapper_funcs_lib},SOURCES>)
        target_compile_definitions(${bf_wrap_lib} PRIVATE BIND_FAKES)
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(RUN_OPTIONS "--leading-underscore")
        endif()
    else (MINGW)
        set(bf_wrap_lib ${wrapper_funcs_lib})
    endif (MINGW)

    target_link_libraries(${bind_fakes_tgt} PowerFake::pw_bindfakes
        -Wl,--whole-archive ${bf_wrap_lib} -Wl,--no-whole-archive
        $<TARGET_PROPERTY:${target_name},LINK_LIBRARIES>)

    if (NOT CMAKE_CROSSCOMPILING)
        add_custom_command(OUTPUT powerfake.link_flags
            DEPENDS ${wrapper_funcs_lib}
            COMMAND ${bind_fakes_tgt} ${RUN_OPTIONS} ${ARGV3}
                    $<TARGET_FILE:${test_lib}> $<TARGET_FILE:${wrapper_funcs_lib}>)
    else ()
        add_custom_command(OUTPUT powerfake.link_flags
            DEPENDS ${wrapper_funcs_lib}
            COMMAND nm -po $<TARGET_FILE:${test_lib}> > main.syms
            COMMAND nm -po $<TARGET_FILE:${wrapper_funcs_lib}> > wrap.syms
            COMMAND ${bind_fakes_tgt} ${RUN_OPTIONS} ${ARGV3} --passive main.syms wrap.syms
            COMMAND objcopy @wrap.syms.objcopy_params $<TARGET_FILE:${wrapper_funcs_lib}>)
    endif ()
    add_custom_target(exec_bind_${target_name} DEPENDS powerfake.link_flags)
    add_dependencies(${target_name} exec_bind_${target_name})

    # Add powerfake link flags
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY
        LINK_FLAGS @${CMAKE_CURRENT_BINARY_DIR}/powerfake.link_flags)
    target_link_libraries(${target_name} PowerFake::powerfake)
endfunction(bind_fakes)
