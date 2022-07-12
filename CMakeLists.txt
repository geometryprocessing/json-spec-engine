# Detects whether this is a top-level project
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(JSE_TOPLEVEL_PROJECT OFF)
else()
    set(JSE_TOPLEVEL_PROJECT ON)
endif()

# Check required CMake version
set(REQUIRED_CMAKE_VERSION "3.14.0")
if(JSE_TOPLEVEL_PROJECT)
    cmake_minimum_required(VERSION ${REQUIRED_CMAKE_VERSION})
else()
    # Don't use cmake_minimum_required here to avoid implicitly overriding parent policies
    if(${CMAKE_VERSION} VERSION_LESS ${REQUIRED_CMAKE_VERSION})
        message(FATAL_ERROR "CMake required version to build JSE is ${REQUIRED_CMAKE_VERSION}")
    endif()
endif()

# Include user-provided default options if available. We do that before the main
# `project()` so that we can define the C/C++ compilers from the option file.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JSEOptions.cmake)
    message(STATUS "Using local options file: ${CMAKE_CURRENT_SOURCE_DIR}/JSEOptions.cmake")
    include(${CMAKE_CURRENT_SOURCE_DIR}/JSEOptions.cmake)
endif()

################################################################################

project(JSE
        DESCRIPTION "JSON Specs Engine"
        LANGUAGES CXX)

# JSE options
option(JSE_WITH_SANITIZERS    "Enable sanitizers in compilation targets"   OFF)
# Misc.
option(JSE_WITH_TESTS         "Build unit-tests" ${JSE_TOPLEVEL_PROJECT})

include(CMakeDependentOption)

# Set default minimum C++ standard
if(JSE_TOPLEVEL_PROJECT)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

### Configuration
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/jse/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/recipes/")

# Color output
include(jse_use_colors)

# IPC Toolkit utils
include(jse_prepend_current_path)
include(jse_set_source_group)

# Sort projects inside the solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Generate position independent code by default
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

################################################################################
# JSE Library
################################################################################

# Add an empty library and fill in the list of sources in `src/CMakeLists.txt`.
add_library(jse)
add_library(jse::jse ALIAS jse)

add_subdirectory(src)

# Public include directory for JSE
target_include_directories(jse PUBLIC ${PROJECT_SOURCE_DIR}/src)

################################################################################
# Dependencies
################################################################################

# Extra warnings
include(jse_warnings)
target_link_libraries(jse PRIVATE jse::warnings)

# Sanitizers
if(JSE_WITH_SANITIZERS)
    include(sanitizers)
    add_sanitizers(jse)
endif()


# Json (MIT)
include(json)
target_link_libraries(jse PUBLIC nlohmann::json)

################################################################################
# Compiler options
################################################################################

# Use C++17
target_compile_features(jse PUBLIC cxx_std_17)

################################################################################
# Tests
################################################################################

# Compile extras only if this is a top-level project
if(JSE_WITH_TESTS)
    # Unit tests
    include(CTest)
    enable_testing()

    # Include Catch2 and provide function `catch_discover_tests` to register tests.
    include(catch2)
    FetchContent_GetProperties(catch2)
    include("${catch2_SOURCE_DIR}/contrib/Catch.cmake")

    add_subdirectory(tests)
endif()
