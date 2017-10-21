set(POWERFAKE_DIR ${CMAKE_CURRENT_LIST_DIR})

add_library(powerfake STATIC ${POWERFAKE_DIR}/powerfake.cpp
    ${POWERFAKE_DIR}/powerfake.h)

function(build_bind_fakes wrap_library_target)
    
    set(pair_sources ${POWERFAKE_DIR}/powerfake ${POWERFAKE_DIR}/SymbolAliasMap)
    set(powerfake_sources $<JOIN:${pair_sources},.cpp >
        ${POWERFAKE_DIR}/bind_fakes.cpp)
    set(powerfake_headers $<JOIN:${pair_sources},.h >
        ${POWERFAKE_DIR}/nmreader.h  ${POWERFAKE_DIR}/piperead.h
    )

    add_executable(bind_fakes ${powerfake_sources} ${powerfake_headers})
    set_property(TARGET bind_fakes APPEND PROPERTY
        COMPILE_DEFINITIONS BIND_FAKES)
    
    # Add flags to ignore undefined reference to __real_ function calls, which we
    # don't use
    set_property(TARGET bind_fakes APPEND PROPERTY
        LINK_FLAGS "-Wl,--warn-unresolved-symbols -Wl,-z,lazy")
    target_link_libraries(bind_fakes
        -Wl,--whole-archive ${wrap_library_target} -Wl,--no-whole-archive)
endfunction(build_bind_fakes)

function(bind_fakes target_name test_lib wrapper_funcs_lib)
    add_custom_command(TARGET ${target_name} PRE_LINK
        COMMAND bind_fakes $<TARGET_FILE:${test_lib}>
                $<TARGET_FILE:${wrapper_funcs_lib}>)

    # Add powerfake link flags
    set_property(TARGET ${target_name} APPEND_STRING PROPERTY
        LINK_FLAGS @${CMAKE_CURRENT_BINARY_DIR}/powerfake.link_flags)
endfunction(bind_fakes)
