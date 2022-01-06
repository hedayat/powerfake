#! bind_fakes : builds and runs bind_fakes binary for the given libraries
#
# This function builds bind_fakes binary and runs it on code libraries and
# wrap libraries for the given test runner target. It'll provide fixed up wrap
# libraries and required linker flags/script for linking the test runner target,
# so that our fakes work.
#
# There are 2 usage forms: a simple one which receives a single code library
# and a single wrap library, and the full form in which you can have multiple
# code and wrap libraries and also have better control on bind_fakes execution
# with the available options.
#
# Usage:
#
# 1. bind_fakes(target_name test_lib wrapper_funcs_lib)
# 2. bind_fakes(target_name SUBJECT <lib1>... WRAPPERS <wlib1>...
#               [PASSIVE][USE_DEFSYM])
#
# \arg:target_name the name of the test runner binary
# \flag:PASSIVE If bind_fakes should run in passive mode (enabled by default
#         when cross-compiling)
# \flag:USE_DEFSYM Use ld's --defsym instead of modifying wrapper libs/objects
# \flag:VERBOSE Enable verbose logging in bind_fakes
# \group:SUBJECT The test subject, usually main code libraries who will call
#         faked functions specified in WRAPPERS
# \group:WRAPPERS Libraries/objects which call WRAP_FUNCTION/HIDE_FUNCTION macros
#
function(bind_fakes target_name)
    # Argument processing
    set(options PASSIVE USE_DEFSYM VERBOSE)
    set(single_val_args )
    set(multi_val_args SUBJECT WRAPPERS)
    cmake_parse_arguments(PARSE_ARGV 1 BFARGS "${options}"
        "{single_val_args}" "${multi_val_args}")

    set(RUN_OPTIONS)
    if (BFARGS_USE_DEFSYM)
        list(APPEND RUN_OPTIONS "--no-objcopy")
    endif()
    if (BFARGS_VERBOSE)
        list(APPEND RUN_OPTIONS "--verbose")
    endif()
    if (CMAKE_CROSSCOMPILING)
        set(BFARGS_PASSIVE True)
    endif()

    if (NOT BFARGS_SUBJECT AND BFARGS_UNPARSED_ARGUMENTS)
        list(GET BFARGS_UNPARSED_ARGUMENTS 0 BFARGS_SUBJECT)
        list(GET BFARGS_UNPARSED_ARGUMENTS 1 BFARGS_WRAPPERS)
    endif()

    set(main_lib_files)
    foreach(mlib IN LISTS BFARGS_SUBJECT)
        list(APPEND main_lib_files $<TARGET_FILE:${mlib}>)
    endforeach()

    set(wrap_lib_files)
    foreach (wrap_lib IN LISTS BFARGS_WRAPPERS)
        target_link_libraries(${wrap_lib} PowerFake::powerfake)
        list(APPEND wrap_lib_files $<TARGET_FILE:${wrap_lib}>)
    endforeach()

    # Build bind_fakes executable
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
        set(wrap_lib_copies)
        foreach (wrap_lib IN LISTS BFARGS_WRAPPERS)
            set(wcopy_name ${CMAKE_STATIC_LIBRARY_PREFIX}${wrap_lib}_copy${CMAKE_STATIC_LIBRARY_SUFFIX})
            list(APPEND wrap_lib_copies ${wcopy_name})
            add_custom_command(OUTPUT ${wcopy_name}
                DEPENDS ${wrap_lib}
                COMMAND objcopy --remove-section=*wrapper_*_alias_*
                    $<TARGET_FILE:${wrap_lib}> ${wcopy_name} VERBATIM)
            add_custom_target(${wrap_lib}_${target_name} DEPENDS ${wcopy_name})
            add_dependencies(${bind_fakes_tgt} ${wrap_lib}_${target_name})
        endforeach()
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            list(APPEND RUN_OPTIONS "--leading-underscore")
        endif()

        set(bf_wrap_lib "-L ." ${wrap_lib_copies})
    else()
        set(bf_wrap_lib ${BFARGS_WRAPPERS})
    endif()

    target_link_libraries(${bind_fakes_tgt} PowerFake::pw_bindfakes
        -Wl,--whole-archive ${bf_wrap_lib} -Wl,--no-whole-archive
        $<TARGET_PROPERTY:${target_name},LINK_LIBRARIES>)

    # Run bind_fakes on the libs and create final output
    set(link_flags_file ${target_name}.powerfake.link_flags)
    set(link_script_file ${target_name}.powerfake.link_script)
    if (NOT BFARGS_PASSIVE)
        add_custom_command(OUTPUT ${link_flags_file} ${link_script_file}
            DEPENDS ${BFARGS_WRAPPERS}
            COMMAND ${bind_fakes_tgt} --output-prefix ${target_name}.powerfake
                    --symbol-files ${main_lib_files} ${RUN_OPTIONS}
                    --wrapper-files ${wrap_lib_files}
            VERBATIM)
    else()
        set(wrap_syms)
        set(wrap_objcopy_params)
        foreach (wrap_lib IN LISTS BFARGS_WRAPPERS)
            list(APPEND wrap_syms ${wrap_lib}.syms)
            list(APPEND wrap_objcopy_params ${wrap_lib}.syms.objcopy_params)
        endforeach()

        add_custom_command(OUTPUT ${link_flags_file} ${link_script_file}
                main.syms ${wrap_syms} ${wrap_objcopy_params}
            DEPENDS ${BFARGS_WRAPPERS}
            COMMAND nm -po ${main_lib_files} > main.syms
            VERBATIM)

        foreach (wrap_lib IN LISTS BFARGS_WRAPPERS)
            add_custom_command(OUTPUT ${link_flags_file} ${link_script_file}
                    main.syms
                DEPENDS ${BFARGS_WRAPPERS}
                COMMAND nm -po $<TARGET_FILE:${wrap_lib}> > ${wrap_lib}.syms
                APPEND VERBATIM)
        endforeach()

        add_custom_command(OUTPUT ${link_flags_file} ${link_script_file}
                main.syms
            DEPENDS ${BFARGS_WRAPPERS}
            COMMAND ${bind_fakes_tgt} --output-prefix ${target_name}.powerfake
                    --passive ${RUN_OPTIONS}
                    --symbol-files main.syms --wrapper-files ${wrap_syms}
            APPEND VERBATIM)
        foreach (wrap_lib IN LISTS BFARGS_WRAPPERS)
            add_custom_command(OUTPUT ${link_flags_file} ${link_script_file}
                    main.syms
                DEPENDS ${BFARGS_WRAPPERS}
                COMMAND objcopy -p @${wrap_lib}.syms.objcopy_params
                    $<TARGET_FILE:${wrap_lib}>
                APPEND VERBATIM)
        endforeach()
    endif()
    add_custom_target(exec_bind_${target_name}
        DEPENDS ${link_flags_file} ${link_script_file})
    add_dependencies(${target_name} exec_bind_${target_name})

    # Add powerfake link flags
    target_link_options(${target_name} PRIVATE
        @${CMAKE_CURRENT_BINARY_DIR}/${link_flags_file}
        ${CMAKE_CURRENT_BINARY_DIR}/${link_script_file})
    target_link_libraries(${target_name} PowerFake::powerfake)

endfunction()
