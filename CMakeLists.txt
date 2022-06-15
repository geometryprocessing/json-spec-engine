# Detects whether this is a top-level project
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
    set(SJV_TOPLEVEL_PROJECT OFF)
else()
    set(SJV_TOPLEVEL_PROJECT ON)
endif()

# Check required CMake version
set(REQUIRED_CMAKE_VERSION "3.14.0")
if(SJV_TOPLEVEL_PROJECT)
    cmake_minimum_required(VERSION ${REQUIRED_CMAKE_VERSION})
else()
    # Don't use cmake_minimum_required here to avoid implicitly overriding parent policies
    if(${CMAKE_VERSION} VERSION_LESS ${REQUIRED_CMAKE_VERSION})
        message(FATAL_ERROR "CMake required version to build SJV is ${REQUIRED_CMAKE_VERSION}")
    endif()
endif()

# Include user-provided default options if available. We do that before the main
# `project()` so that we can define the C/C++ compilers from the option file.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/SJVOptions.cmake)
    message(STATUS "Using local options file: ${CMAKE_CURRENT_SOURCE_DIR}/SJVOptions.cmake")
    include(${CMAKE_CURRENT_SOURCE_DIR}/SJVOptions.cmake)
endif()

################################################################################

project(SJV
        DESCRIPTION "Easy-to-use wrapper for linear solver"
        LANGUAGES CXX)

# SJV options
option(SJV_WITH_SANITIZERS    "Enable sanitizers in compilation targets"   OFF)
# Sanitizer options
option(SJV_SANITIZE_ADDRESS   "Sanitize Address"                           OFF)
option(SJV_SANITIZE_MEMORY    "Sanitize Memory"                            OFF)
option(SJV_SANITIZE_THREAD    "Sanitize Thread"                            OFF)
option(SJV_SANITIZE_UNDEFINED "Sanitize Undefined"                         OFF)
# Misc.
option(SJV_WITH_TESTS         "Build unit-tests" ${SJV_TOPLEVEL_PROJECT})

include(CMakeDependentOption)

# Set default minimum C++ standard
if(SJV_TOPLEVEL_PROJECT)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif()

### Configuration
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/sjv/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/recipes/")
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/find/")

# Color output
include(sjv_use_colors)

# IPC Toolkit utils
include(sjv_prepend_current_path)
include(sjv_set_source_group)

# Sort projects inside the solution
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Generate position independent code by default
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

################################################################################
# SJV Library
################################################################################

# Add an empty library and fill in the list of sources in `src/CMakeLists.txt`.
add_library(sjv)
add_library(sjv::sjv ALIAS sjv)

add_subdirectory(src)

# Public include directory for SJV
target_include_directories(sjv PUBLIC ${PROJECT_SOURCE_DIR}/src)

################################################################################
# Definitions
################################################################################

if(SJV_LARGE_INDEX)
    target_compile_definitions(sjv PUBLIC -DSJV_LARGE_INDEX)
endif()

# No limit yay
target_compile_definitions(sjv PUBLIC -DEIGEN_STACK_ALLOCATION_LIMIT=0)

# 8MB
# target_compile_definitions(sjv PUBLIC -DEIGEN_STACK_ALLOCATION_LIMIT=8388608)

################################################################################
# Dependencies
################################################################################

# Extra warnings
include(sjv_warnings)
target_link_libraries(sjv PRIVATE sjv::warnings)

# Sanitizers
if(SJV_WITH_SANITIZERS)
    include(sanitizers)
    add_sanitizers(sjv)
endif()

# include(eigen)
# target_link_libraries(SJV PUBLIC Eigen3::Eigen)

# Json (MIT)
include(json)
target_link_libraries(sjv PUBLIC nlohmann::json)

################################################################################
# Compiler options
################################################################################

# Use C++17
target_compile_features(sjv PUBLIC cxx_std_17)

################################################################################
# Tests
################################################################################

# Compile extras only if this is a top-level project
if(SJV_WITH_TESTS)
    # Unit tests
    include(CTest)
    enable_testing()

    # Include Catch2 and provide function `catch_discover_tests` to register tests.
    include(catch2)
    FetchContent_GetProperties(catch2)
    include("${catch2_SOURCE_DIR}/contrib/Catch.cmake")

    add_subdirectory(tests)
endif()
