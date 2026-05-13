# jse_add_embedded_spec(
#     INPUT root_spec.json
#     OUTPUT generated/spec.hpp
#     INCLUDE_DIRS include_dir_1 include_dir_2
# )
#
# Creates the fixed jse::embed library target containing generated .hpp/.cpp
# files. The generated jse::embed::spec() function returns a lazily parsed,
# include-expanded nlohmann::json spec.
function(jse_add_embedded_spec)
    set(options)
    set(one_value_args INPUT OUTPUT)
    set(multi_value_args INCLUDE_DIRS)
    cmake_parse_arguments(JSE_EMBED
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )

    if(JSE_EMBED_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown arguments for jse_add_embedded_spec: ${JSE_EMBED_UNPARSED_ARGUMENTS}")
    endif()

    if(TARGET jse_embed)
        message(FATAL_ERROR "jse_add_embedded_spec creates fixed target jse::embed and can only be called once")
    endif()

    if(NOT JSE_EMBED_INPUT)
        message(FATAL_ERROR "jse_add_embedded_spec requires INPUT")
    endif()

    if(NOT JSE_EMBED_OUTPUT)
        message(FATAL_ERROR "jse_add_embedded_spec requires OUTPUT")
    endif()

    if(NOT TARGET jse_embed_spec_tool)
        message(FATAL_ERROR "jse_add_embedded_spec requires the jse_embed_spec_tool generator target")
    endif()

    get_filename_component(_jse_embed_input "${JSE_EMBED_INPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    get_filename_component(_jse_embed_header "${JSE_EMBED_OUTPUT}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    get_filename_component(_jse_embed_output_dir "${_jse_embed_header}" DIRECTORY)
    get_filename_component(_jse_embed_output_stem "${_jse_embed_header}" NAME_WE)
    set(_jse_embed_source "${_jse_embed_output_dir}/${_jse_embed_output_stem}.cpp")

    get_filename_component(_jse_embed_input_dir "${_jse_embed_input}" DIRECTORY)
    set(_jse_embed_include_dirs "${_jse_embed_input_dir}" ${JSE_EMBED_INCLUDE_DIRS})
    set(_jse_embed_abs_include_dirs)
    set(_jse_embed_include_args)
    set(_jse_embed_depends "${_jse_embed_input}")

    foreach(_jse_embed_include_dir IN LISTS _jse_embed_include_dirs)
        get_filename_component(_jse_embed_abs_include_dir "${_jse_embed_include_dir}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND _jse_embed_abs_include_dirs "${_jse_embed_abs_include_dir}")
    endforeach()

    list(REMOVE_DUPLICATES _jse_embed_abs_include_dirs)

    foreach(_jse_embed_abs_include_dir IN LISTS _jse_embed_abs_include_dirs)
        list(APPEND _jse_embed_include_args "--include-dir" "${_jse_embed_abs_include_dir}")
        file(GLOB_RECURSE _jse_embed_json_depends
            CONFIGURE_DEPENDS
            "${_jse_embed_abs_include_dir}/*.json"
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
            --input "${_jse_embed_input}"
            --output-header "${_jse_embed_header}"
            --output-source "${_jse_embed_source}"
            --namespace "jse::embed"
            --function "spec"
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

    add_library(jse_embed
        "${_jse_embed_source}"
        "${_jse_embed_header}"
    )
    add_library(jse::embed ALIAS jse_embed)

    target_include_directories(jse_embed PUBLIC
        "$<BUILD_INTERFACE:${_jse_embed_output_dir}>"
    )
    target_link_libraries(jse_embed PUBLIC nlohmann_json::nlohmann_json)
    target_compile_features(jse_embed PUBLIC cxx_std_17)
    source_group("Generated" FILES "${_jse_embed_source}" "${_jse_embed_header}")

    set(JSE_EMBED_HEADER "${_jse_embed_header}" PARENT_SCOPE)
    set(JSE_EMBED_SOURCE "${_jse_embed_source}" PARENT_SCOPE)
endfunction()
