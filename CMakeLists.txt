# **************************************************************************** #
# This file is part of the JCDP build system. It provides the main CMake
# entry point, which generates all necessary sources and builds the JCDP
# libraries and executables.
# **************************************************************************** #

cmake_minimum_required(VERSION 3.25.0)
cmake_policy(VERSION 3.25.0)

# **************************************************************************** #
# Project information
# **************************************************************************** #

# Project description
set(JCDP_PROJECT_DESCRIPTION "Jacobian Chaining via Dynamic Programming")

# Project version
set(JCDP_VER_MAJOR 0)
set(JCDP_VER_MINOR 1)
set(JCDP_VER_PATCH 0)
set(JCDP_VERSION "${JCDP_VER_MAJOR}.${JCDP_VER_MINOR}.${JCDP_VER_PATCH}")

project(JCDP
  LANGUAGES C CXX
  DESCRIPTION ${JCDP_PROJECT_DESCRIPTION}
  VERSION ${JCDP_VERSION})

# **************************************************************************** #
# General setup
# **************************************************************************** #
# Require at least C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library / Archive output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin/")

# JCDP folders & files
set(JCDP_TEMPLATE_DIR "${PROJECT_SOURCE_DIR}/additionals/templates")
set(JCDP_CMAKE_DIR "${PROJECT_SOURCE_DIR}/additionals/cmake")
set(JCDP_IWYU_DIR "${PROJECT_SOURCE_DIR}/additionals/iwyu")
set(JCDP_DAG_DIR "${PROJECT_SOURCE_DIR}/dags")
set(JCDP_CONFIG_DIR "${PROJECT_SOURCE_DIR}/additionals/configs")

# Include CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${JCDP_CMAKE_DIR}")

# Print title
include(print_helper)
print_title("${JCDP_PROJECT_DESCRIPTION}" EMPTY_BEFORE EMPTY_AFTER)

# **************************************************************************** #
# CMake options
# **************************************************************************** #
option(JCDP_STATIC_LIBS
  "Whether to create a static JCDP library." ON)
option(JCDP_SHARED_LIBS
  "Whether to create a shared JCDP library." ON)
option(JCDP_USE_OPENMP
  "Whether to use OpenMP." ON)
# option(JCDP_BUILD_TESTS
#   "Whether to build test programs." ON)
# option(JCDP_EXAMPLES_TARGETS
#   "Whether to create targets to run the examples." ON)
# option(JCDP_BUILD_USERGUIDE
#   "Whether to build the user guide." OFF)
# option(JCDP_BUILD_DOXYGEN
#   "Whether to build the Doxygen documentation." OFF)

# **************************************************************************** #
# Include some modules (intrinsics, PIC, sanitation, etc.)
# **************************************************************************** #
include(build_types)
include(cxx_standard_library)
include(enable_intrinsics)
include(enable_position_independent_code)
include(enable_sanitation)

# **************************************************************************** #
# Find necessary packages
# **************************************************************************** #
# Find GTest
# if(JCDP_BUILD_TESTS)
#   find_package(GTest REQUIRED)
# endif()

# # Find LaTeX
# if(JCDP_BUILD_USERGUIDE)
#   find_package(LATEX REQUIRED)
# endif()

# # Find Doxygen
# if(JCDP_BUILD_DOXYGEN)
#   find_package(Doxygen REQUIRED dot OPTIONAL_COMPONENTS dia mscgen)
# endif()

# **************************************************************************** #
# Include some modules (compiler flags, coverage, iwyu) and JCDP modules
# **************************************************************************** #
include(compiler_flags)
include(include_what_you_use)
include(configure_compiler_warnings)

# Include JCDP modules
# include(JCDP_openmp_backend)

# **************************************************************************** #
# JCDP include directories
# **************************************************************************** #
set(JCDP_include_dirs "${PROJECT_SOURCE_DIR}/include")

# Collect IWYU flags
set(JCDP_IWYU_FLAGS
  "--mapping_file=${JCDP_IWYU_DIR}/jcdp.imp"
  "--quoted_includes_first" "--cxx17ns")

# add_subdirectory(include)

# **************************************************************************** #
# Compile sources and collect libraries needed for the tests
# **************************************************************************** #
set(JCDP_libs)
add_subdirectory(src)

# **************************************************************************** #
# Compile tests & examples
# **************************************************************************** #
# enable_testing()
# add_subdirectory(tests)
# add_subdirectory(examples)

# # Setup code coverage for ctest
# ctest_code_coverage_target(
#   TOOL_ARGS
#     "--exclude-throw-branches"
#     "--exclude-unreachable-branches"
#     "--exclude-function-lines"
#   EXCLUDE
#     "${PROJECT_BINARY_DIR}/_deps/*"
#     "${PROJECT_SOURCE_DIR}/tests/*")

# **************************************************************************** #
# Build documentation
# **************************************************************************** #
# add_subdirectory(docs)

# **************************************************************************** #
# Summary
# **************************************************************************** #
# print_subtitle("Configuration summary" EMPTY_BEFORE)
# include(JCDP_config)
# print_divide(EMPTY_AFTER)
