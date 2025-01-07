cmake_minimum_required(VERSION 3.15)

# Quick return if GBench is already imported
if(GBench_FOUND)
  return()
endif()

# Include print helper
include(print_helper)

# Quick return if GBench::GBench is already imported, e.g. if a subproject
# fetches GBench as well.
if(TARGET GBench::GBench)
  return()
endif()

set(GBench_REPOSITORY "https://github.com/google/benchmark.git")
set(GBench_TAG "v1.9.0" CACHE STRING
  "The gbench tag which is fetched if no local version is found.")
mark_as_advanced(GBench_TAG)
option(GBench_FORCE_FETCH "Whether to force fetch and build GTest." OFF)
mark_as_advanced(GBench_FORCE_FETCH)

if(NOT GBench_FORCE_FETCH)
  # TODO: Local search
endif()

# Fetch GBench if there is no local installation
if(NOT GBench_FOUND)
  if(GTest_FORCE_FETCH)
    _print_status(
      "GBench: Fetching version '${GBench_TAG}' from "
      "'${GBench_REPOSITORY}' (forced via GBench_FORCE_FETCH).")
  else()
    _print_status(
      "GBench: No local installation found. Fetching version "
      "'${GBench_TAG}' from '${GBench_REPOSITORY}'.")
  endif()

  # Necessary packages
  include(FetchContent)
  find_package(Git QUIET)

  # Set configuration and mark everything as advanced
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_ENABLE_TESTING)
  set(BENCHMARK_ENABLE_EXCEPTIONS ON CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_ENABLE_EXCEPTIONS)
  set(BENCHMARK_ENABLE_LTO OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_ENABLE_LTO)
  set(BENCHMARK_USE_LIBCXX OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_USE_LIBCXX)
  set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_ENABLE_WERROR)
  set(BENCHMARK_FORCE_WERROR OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_FORCE_WERROR)
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_ENABLE_INSTALL)
  set(BENCHMARK_ENABLE_DOXYGEN OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_ENABLE_DOXYGEN)
  set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_INSTALL_DOCS)
  set(BENCHMARK_DOWNLOAD_DEPENDENCIES OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_DOWNLOAD_DEPENDENCIES)
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_ENABLE_GTEST_TESTS)
  set(BENCHMARK_USE_BUNDLED_GTEST ON CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_USE_BUNDLED_GTEST)
  set(BENCHMARK_ENABLE_LIBPFM OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_ENABLE_LIBPFM)
  set(BENCHMARK_BUILD_32_BITS OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_BUILD_32_BITS)
  set(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BENCHMARK_ENABLE_ASSEMBLY_TESTS)

  # Declare GoogleBenchmark
  FetchContent_Declare(
    gbenchmark
    GIT_REPOSITORY ${GBench_REPOSITORY}
    GIT_TAG ${GBench_TAG}
    GIT_SHALLOW ON
    SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/gbenchmark-${GBench_TAG})

  # Fetch GoogleBenchmark
  FetchContent_GetProperties(gbenchmark)
  if(NOT gbenchmark_POPULATED)
    cmake_policy(PUSH)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
      cmake_policy(SET CMP0169 OLD)
    endif()
    FetchContent_Populate(gbenchmark)
    cmake_policy(POP)

    print_subtitle("Configuring Google Benchmark" EMPTY_BEFORE)
    add_subdirectory(${gbenchmark_SOURCE_DIR} ${gbenchmark_BINARY_DIR})
    print_divide(EMPTY_AFTER)
  endif()

  # Print sha
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${gbenchmark_SOURCE_DIR}
    OUTPUT_VARIABLE gbenchmark_sha)

  string(REPLACE "\n" "" gbenchmark_sha ${gbenchmark_sha})
  _print_status(
    "GBench: Successfully fetched "
    "'${GBench_TAG}' HEAD at SHA ${gbenchmark_sha}")

  # Check if fetch was succesful
  set(GBench_INCLUDE_DIRS "${gbenchmark_SOURCE_DIR}/include")
  if(EXISTS "${gbenchmark_SOURCE_DIR}" AND EXISTS "${GBench_INCLUDE_DIRS}")
    if(NOT TARGET GBench::GBench)
      add_library(GBench::GBench INTERFACE IMPORTED)
      target_link_libraries(GBench::GBench INTERFACE benchmark::benchmark)
      set_target_properties(GBench::GBench
        PROPERTIES INTERFACE_LINK_DIRECTORIES "${GBench_INCLUDE_DIRS}")
    endif()

    if(NOT TARGET GBench::Main)
      add_library(GBench::Main INTERFACE IMPORTED)
      target_link_libraries(GBench::Main INTERFACE benchmark::benchmark_main)
      set_target_properties(GBench::Main
        PROPERTIES INTERFACE_LINK_DIRECTORIES "${GBench_INCLUDE_DIRS}")
    endif()

    set(GBench_FOUND 1)
  else()
    set(GBench_FOUND 0)
    _print_error("GBench: Fetched version invalid.")
  endif()
else()
  _print_status(
    "GBench: Using local GBench installation: '${GBench_INCLUDE_DIRS}'.")
endif()
