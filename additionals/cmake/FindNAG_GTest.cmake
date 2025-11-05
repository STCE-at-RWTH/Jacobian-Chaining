# **************************************************************************** #
# This file is part of the NAG's common cmake configuration. It first searches
# for GoogleTest in the local file system and if it couldn't find it, the
# newest stable release is fetched from
# https://github.com/google/googletest.git.
#
# Copyright (c) 2022 NAG
# **************************************************************************** #

cmake_minimum_required(VERSION 3.15)

# Quick return if GBench is already imported
if(GTest_FOUND)
  return()
endif()

# Include print helper
include(NAG_print_helper)
include(util/NAG_check_flags)

# Default GTest version
if(WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  set(_nag_default_gtest_tag "release-1.10.0")
else()
  set(_nag_default_gtest_tag "v1.15.2")
endif()

set(NAG_GTest_REPOSITORY "https://github.com/google/googletest.git")
set(NAG_GTest_TAG ${_nag_default_gtest_tag} CACHE STRING
  "The gtest tag which is fetched if no local version is found.")
mark_as_advanced(NAG_GTest_TAG)
option(NAG_GTest_FORCE_FETCH "Whether to force fetch and build GTest." OFF)
mark_as_advanced(NAG_GTest_FORCE_FETCH)

if(NOT NAG_GTest_FORCE_FETCH)
  # Try to locate local GTest installation
  find_package(GTest QUIET)
endif()

# Fetch GTest if there is no local installation
if(NOT GTest_FOUND)
  if(NAG_GTest_FORCE_FETCH)
    _nag_print_status(
      "GTest: Fetching version '${NAG_GTest_TAG}' "
      "from '${NAG_GTest_REPOSITORY}' (forced via NAG_GTest_FORCE_FETCH).")
  else()
    _nag_print_status(
      "GTest: No local installation found. Fetching version '${NAG_GTest_TAG}' "
      "from '${NAG_GTest_REPOSITORY}'.")
  endif()

  # Necessary packages
  include(FetchContent)
  find_package(Git QUIET)

  # Disable gmock and gtest install
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  mark_as_advanced(BUILD_GMOCK)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  mark_as_advanced(INSTALL_GTEST)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Declare GoogleTest
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY ${NAG_GTest_REPOSITORY}
    GIT_TAG ${NAG_GTest_TAG}
    GIT_SHALLOW ON
    SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/googletest-${NAG_GTest_TAG})

  # Fetch GoogleTest
  FetchContent_GetProperties(googletest)
  if(NOT googletest_POPULATED)
    cmake_policy(PUSH)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
      cmake_policy(SET CMP0169 OLD)
    endif()
    FetchContent_Populate(googletest)
    cmake_policy(POP)

    nag_print_subtitle("Configuring Google Test" EMPTY_BEFORE)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
    nag_print_divide(EMPTY_AFTER)
  endif()

  # Print sha
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${googletest_SOURCE_DIR}
    OUTPUT_VARIABLE googletest_sha)

  string(REPLACE "\n" "" googletest_sha ${googletest_sha})
  _nag_print_status(
    "GTest: Successfully fetched '${NAG_GTest_TAG}' HEAD "
    "at SHA ${googletest_sha}")

  # Disable some warnings since GTest always compiles with -Werror or /WX
  if(TARGET gtest)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
      if(WIN32)
        # disable: decorated name length exceeded, name was truncated
        _nag_add_flag(CXX "/Qdiag-disable:2586" DIAG_DISABLE_2586 gtest)
        # disable: unrecognized gcc optimization level
        _nag_add_flag(CXX "/Qdiag-disable:3175" DIAG_DISABLE_3175 gtest)
        # disable: The Intel(R) C++ Compiler Classic (ICC) is deprecated
        _nag_add_flag(CXX "/Qdiag-disable:10441" DIAG_DISABLE_10441 gtest gtest_main)
        if(NAG_CHECK_CXX_DIAG_DISABLE_10441)
          target_link_options(gtest
            PRIVATE "$<$<COMPILE_LANGUAGE:C>:/Qdiag-disable:10441>")
          target_link_options(gtest_main
            PRIVATE "$<$<COMPILE_LANGUAGE:C>:/Qdiag-disable:10441>")
        endif()
      else()
        # disable: decorated name length exceeded, name was truncated
        _nag_add_flag(CXX "-diag-disable:2586" DIAG_DISABLE_2586 gtest)
        # disable: unrecognized gcc optimization level
        _nag_add_flag(CXX "-diag-disable:3175" DIAG_DISABLE_3175 gtest)
        # disable: The Intel(R) C++ Compiler Classic (ICC) is deprecated
        _nag_add_flag(CXX "-diag-disable:10441" DIAG_DISABLE_10441 gtest gtest_main)
        if(NAG_CHECK_CXX_DIAG_DISABLE_10441)
          target_link_options(gtest
            PRIVATE "$<$<COMPILE_LANGUAGE:C>:-diag-disable:10441>")
          target_link_options(gtest_main
            PRIVATE "$<$<COMPILE_LANGUAGE:C>:-diag-disable:10441>")
        endif()
      endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "IntelLLVM")
      _nag_add_flag(CXX "-Wno-unused-command-line-argument"
        WNO_UNUSED_COMMAND_LINE_ARGUMENT gtest gtest_main)

      get_target_property(_opt_old gtest COMPILE_FLAGS)
      string(REPLACE "/utf-8" "" _opt "${_opt_old}")
      string(REPLACE "-utf-8" "" _opt "${_opt_old}")
      set_target_properties(gtest PROPERTIES COMPILE_FLAGS "${_opt}")

      get_target_property(_opt_old gtest_main COMPILE_FLAGS)
      string(REPLACE "/utf-8" "" _opt "${_opt_old}")
      string(REPLACE "-utf-8" "" _opt "${_opt_old}")
      set_target_properties(gtest_main PROPERTIES COMPILE_FLAGS "${_opt}")

      unset(_opt_old)
      unset(_opt)
    endif()
  endif()

  # Check if fetch was succesful
  set(GTEST_INCLUDE_DIRS "${googletest_SOURCE_DIR}/googletest/include")
  if(EXISTS "${googletest_SOURCE_DIR}" AND EXISTS "${GTEST_INCLUDE_DIRS}")
    # Pre 3.20 Targets
    if(NOT TARGET GTest::GTest)
      add_library(GTest::GTest INTERFACE IMPORTED)
      target_link_libraries(GTest::GTest INTERFACE gtest)
    endif()
    if(NOT TARGET GTest::Main)
      add_library(GTest::Main INTERFACE IMPORTED)
      target_link_libraries(GTest::Main INTERFACE gtest_main)
    endif()

    # Post 3.20 Targets
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.20")
      if(NOT TARGET GTest::gtest)
        add_library(GTest::gtest INTERFACE IMPORTED)
        target_link_libraries(GTest::gtest INTERFACE GTest::GTest)
      endif()
      if(NOT TARGET GTest::gtest_main)
        add_library(GTest::gtest_main INTERFACE IMPORTED)
        target_link_libraries(GTest::gtest_main INTERFACE GTest::Main)
      endif()
    endif()

    set(GTEST_LIBRARIES GTest::GTest)
    set(GTEST_MAIN_LIBRARIES GTest::Main)
    set(GTEST_BOTH_LIBRARIES GTest::GTest GTest::Main)
    set(GTEST_FOUND 1)
  else()
    set(GTEST_FOUND 0)
    _nag_print_error("GTest: Fetched version invalid.")
  endif()
else()
  _nag_print_status(
    "GTest: Using local GTest installation: '${GTEST_INCLUDE_DIRS}'.")
endif()

# Cleanup
unset(_nag_default_gtest_tag)
