# jse_add_spec(<target>
#     [ALIAS alias::target]
#     [INPUT root_spec.json]
#     [OUTPUT generated/spec.hpp]
#     [INCLUDE_DIRS include_dir_1 include_dir_2]
#     [LINK_SPECS dependency::spec ...]
#     [NAMESPACE cxx::namespace]
#     [FUNCTION function_name]
# )
#
# Creates or extends a CMake target carrying JSON spec metadata. If INPUT and
# OUTPUT are provided, generated .hpp/.cpp files are added to the target. The
# generated header exposes jse::embed::<target>::<output_stem>::spec() by default.
include_guard(GLOBAL)

function(_jse_resolve_target OUTPUT_VARIABLE TARGET_NAME)
    get_target_property(_jse_aliased_target "${TARGET_NAME}" ALIASED_TARGET)
    if(_jse_aliased_target)
        set(${OUTPUT_VARIABLE} "${_jse_aliased_target}" PARENT_SCOPE)
    else()
        set(${OUTPUT_VARIABLE} "${TARGET_NAME}" PARENT_SCOPE)
    endif()
endfunction()

function(_jse_target_usage_scope OUTPUT_VARIABLE TARGET_NAME)
    get_target_property(_jse_target_type "${TARGET_NAME}" TYPE)
    if(_jse_target_type STREQUAL "INTERFACE_LIBRARY")
        set(${OUTPUT_VARIABLE} INTERFACE PARENT_SCOPE)
    elseif(_jse_target_type STREQUAL "EXECUTABLE")
        set(${OUTPUT_VARIABLE} PRIVATE PARENT_SCOPE)
    else()
        set(${OUTPUT_VARIABLE} PUBLIC PARENT_SCOPE)
    endif()
endfunction()

function(_jse_append_target_property_unique TARGET_NAME PROPERTY_NAME)
    if(ARGC LESS 3)
        return()
    endif()

    get_target_property(_jse_values "${TARGET_NAME}" "${PROPERTY_NAME}")
    if(NOT _jse_values)
        set(_jse_values)
    endif()

    list(APPEND _jse_values ${ARGN})
    list(REMOVE_DUPLICATES _jse_values)
    set_property(TARGET "${TARGET_NAME}" PROPERTY "${PROPERTY_NAME}" "${_jse_values}")
endfunction()

function(_jse_collect_spec_include_dirs OUTPUT_VARIABLE)
    set(_jse_queue ${ARGN})
    set(_jse_seen)
    set(_jse_result)

    while(_jse_queue)
        list(GET _jse_queue 0 _jse_target)
        list(REMOVE_AT _jse_queue 0)

        if(NOT TARGET "${_jse_target}")
            message(FATAL_ERROR "Unknown JSE spec dependency target: ${_jse_target}")
        endif()

        _jse_resolve_target(_jse_real_target "${_jse_target}")
        if("${_jse_real_target}" IN_LIST _jse_seen)
            continue()
        endif()
        list(APPEND _jse_seen "${_jse_real_target}")

        get_target_property(_jse_include_dirs "${_jse_real_target}" INTERFACE_JSE_SPEC_INCLUDE_DIRS)
        if(_jse_include_dirs)
            list(APPEND _jse_result ${_jse_include_dirs})
        endif()

        get_target_property(_jse_link_specs "${_jse_real_target}" INTERFACE_JSE_LINK_SPECS)
        if(_jse_link_specs)
            list(APPEND _jse_queue ${_jse_link_specs})
        endif()
    endwhile()

    list(REMOVE_DUPLICATES _jse_result)
    set(${OUTPUT_VARIABLE} "${_jse_result}" PARENT_SCOPE)
endfunction()

function(jse_add_spec TARGET_NAME)
    set(options)
    set(one_value_args ALIAS INPUT OUTPUT NAMESPACE FUNCTION)
    set(multi_value_args INCLUDE_DIRS LINK_SPECS)
    cmake_parse_arguments(JSE_SPEC
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    if(JSE_SPEC_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments for jse_add_spec: ${JSE_SPEC_UNPARSED_ARGUMENTS}")
    endif()

    if("${TARGET_NAME}" STREQUAL "")
        message(FATAL_ERROR "jse_add_spec requires a target name")
    endif()

    if((JSE_SPEC_INPUT AND NOT JSE_SPEC_OUTPUT) OR (JSE_SPEC_OUTPUT AND NOT JSE_SPEC_INPUT))
        message(FATAL_ERROR "jse_add_spec requires both INPUT and OUTPUT when generating an embedded spec")
    endif()

    set(_jse_generation_requested OFF)
    if(JSE_SPEC_INPUT)
        set(_jse_generation_requested ON)
    endif()

    if(TARGET "${TARGET_NAME}")
        _jse_resolve_target(_jse_spec_target "${TARGET_NAME}")
    else()
        if("${TARGET_NAME}" MATCHES "::")
            message(FATAL_ERROR
                "Cannot create target ${TARGET_NAME}; CMake target names containing :: must be aliases. "
                "Use jse_add_spec(real_target ALIAS ${TARGET_NAME} ...)."
            )
        endif()

        if(_jse_generation_requested)
            add_library(${TARGET_NAME})
        else()
            add_library(${TARGET_NAME} INTERFACE)
        endif()
        set(_jse_spec_target "${TARGET_NAME}")
    endif()

    get_target_property(_jse_spec_target_type "${_jse_spec_target}" TYPE)
    if(_jse_generation_requested AND "${_jse_spec_target_type}" STREQUAL "INTERFACE_LIBRARY")
        message(FATAL_ERROR
            "jse_add_spec cannot add generated sources to interface target ${_jse_spec_target}. "
            "Create the spec target first with a call that has INPUT and OUTPUT, or use a separate "
            "metadata-only spec target as a LINK_SPECS dependency."
        )
    endif()

    if(JSE_SPEC_ALIAS)
        if(TARGET "${JSE_SPEC_ALIAS}")
            get_target_property(_jse_existing_alias_target "${JSE_SPEC_ALIAS}" ALIASED_TARGET)
            if(NOT "${_jse_existing_alias_target}" STREQUAL "${_jse_spec_target}")
                message(FATAL_ERROR "Target ${JSE_SPEC_ALIAS} already exists and is not an alias for ${_jse_spec_target}")
            endif()
        else()
            add_library(${JSE_SPEC_ALIAS} ALIAS ${_jse_spec_target})
        endif()
    endif()

    _jse_target_usage_scope(_jse_usage_scope "${_jse_spec_target}")

    foreach(_jse_link_spec IN LISTS JSE_SPEC_LINK_SPECS)
        if(NOT TARGET "${_jse_link_spec}")
            message(FATAL_ERROR "Unknown JSE spec dependency target: ${_jse_link_spec}")
        endif()
    endforeach()

    if(JSE_SPEC_LINK_SPECS)
        _jse_append_target_property_unique(${_jse_spec_target} INTERFACE_JSE_LINK_SPECS ${JSE_SPEC_LINK_SPECS})
    endif()

    set(_jse_local_include_dirs ${JSE_SPEC_INCLUDE_DIRS})
    if(JSE_SPEC_INPUT)
        get_filename_component(_jse_input_abs "${JSE_SPEC_INPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        get_filename_component(_jse_input_dir "${_jse_input_abs}" DIRECTORY)
        list(APPEND _jse_local_include_dirs "${_jse_input_dir}")
    endif()

    set(_jse_abs_include_dirs)
    foreach(_jse_include_dir IN LISTS _jse_local_include_dirs)
        get_filename_component(_jse_abs_include_dir "${_jse_include_dir}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND _jse_abs_include_dirs "${_jse_abs_include_dir}")
    endforeach()
    if(_jse_abs_include_dirs)
        list(REMOVE_DUPLICATES _jse_abs_include_dirs)
        _jse_append_target_property_unique(${_jse_spec_target} INTERFACE_JSE_SPEC_INCLUDE_DIRS ${_jse_abs_include_dirs})
    endif()

    if(NOT _jse_generation_requested)
        return()
    endif()

    if(NOT TARGET jse_embed_spec_tool)
        message(FATAL_ERROR "jse_add_spec requires the jse_embed_spec_tool generator target")
    endif()

    get_filename_component(_jse_embed_header "${JSE_SPEC_OUTPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    get_filename_component(_jse_embed_output_dir "${_jse_embed_header}" DIRECTORY)
    get_filename_component(_jse_embed_output_stem "${_jse_embed_header}" NAME_WE)
    set(_jse_embed_source "${_jse_embed_output_dir}/${_jse_embed_output_stem}.cpp")
    string(MAKE_C_IDENTIFIER "${_jse_spec_target}" _jse_embed_target_namespace)
    string(MAKE_C_IDENTIFIER "${_jse_embed_output_stem}" _jse_embed_spec_namespace)

    if(JSE_SPEC_NAMESPACE)
        set(_jse_embed_namespace "${JSE_SPEC_NAMESPACE}")
    else()
        set(_jse_embed_namespace "jse::embed::${_jse_embed_target_namespace}::${_jse_embed_spec_namespace}")
    endif()

    if(JSE_SPEC_FUNCTION)
        set(_jse_embed_function "${JSE_SPEC_FUNCTION}")
    else()
        set(_jse_embed_function "spec")
    endif()

    get_property(_jse_embed_namespaces GLOBAL PROPERTY JSE_EMBED_SPEC_NAMESPACES)
    if("${_jse_embed_namespace}" IN_LIST _jse_embed_namespaces)
        message(FATAL_ERROR "jse_add_spec already has an embedded spec named ${_jse_embed_namespace}")
    endif()
    set_property(GLOBAL APPEND PROPERTY JSE_EMBED_SPEC_NAMESPACES "${_jse_embed_namespace}")

    _jse_collect_spec_include_dirs(_jse_embed_include_dirs "${_jse_spec_target}")

    set(_jse_embed_include_args)
    set(_jse_embed_depends "${_jse_input_abs}")
    foreach(_jse_embed_include_dir IN LISTS _jse_embed_include_dirs)
        list(APPEND _jse_embed_include_args "--include-dir" "${_jse_embed_include_dir}")
        file(GLOB_RECURSE _jse_embed_json_depends
            CONFIGURE_DEPENDS
            "${_jse_embed_include_dir}/*.json"
        )
        list(APPEND _jse_embed_depends ${_jse_embed_json_depends})
    endforeach()
    list(REMOVE_DUPLICATES _jse_embed_depends)

    file(MAKE_DIRECTORY "${_jse_embed_output_dir}")

    add_custom_command(
        OUTPUT
            "${_jse_embed_header}"
            "${_jse_embed_source}"
        COMMAND
            jse_embed_spec_tool
            --input "${_jse_input_abs}"
            --output-header "${_jse_embed_header}"
            --output-source "${_jse_embed_source}"
            --namespace "${_jse_embed_namespace}"
            --function "${_jse_embed_function}"
            ${_jse_embed_include_args}
        DEPENDS
            jse_embed_spec_tool
            ${_jse_embed_depends}
        COMMENT
            "Generating embedded JSON spec ${_jse_embed_output_stem}"
        VERBATIM
    )

    set_source_files_properties(
        "${_jse_embed_header}"
        "${_jse_embed_source}"
        PROPERTIES GENERATED TRUE
    )

    target_sources(${_jse_spec_target} PRIVATE
        "${_jse_embed_source}"
        "${_jse_embed_header}"
    )
    target_include_directories(${_jse_spec_target} ${_jse_usage_scope}
        "$<BUILD_INTERFACE:${_jse_embed_output_dir}>"
    )
    target_link_libraries(${_jse_spec_target} ${_jse_usage_scope} nlohmann_json::nlohmann_json)
    target_compile_features(${_jse_spec_target} ${_jse_usage_scope} cxx_std_17)

    source_group("Generated" FILES "${_jse_embed_source}" "${_jse_embed_header}")

    set(JSE_SPEC_HEADER "${_jse_embed_header}" PARENT_SCOPE)
    set(JSE_SPEC_SOURCE "${_jse_embed_source}" PARENT_SCOPE)
endfunction()
