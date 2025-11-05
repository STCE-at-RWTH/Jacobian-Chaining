# **************************************************************************** #
# This file is part of the NAG's common cmake configuration. It fetches the
# newest stable release of nlohmann/json from
# https://github.com/nlohmann/json
#
# Copyright (c) 2022 NAG
# **************************************************************************** #

cmake_minimum_required(VERSION 3.15)

# Quick return if nlohmann_json is already imported
if(nlohmann_json_FOUND)
  return()
endif()

include(util/print)
include(util/find_package)

set(nlohmann_json_REPOSITORY "https://github.com/nlohmann/json")
set(nlohmann_json_TAG "v3.12.0" CACHE STRING
  "The nlohmann_json tag which is fetched if no local version is found.")
mark_as_advanced(nlohmann_json_TAG)
option(nlohmann_json_FORCE_FETCH "Whether to force fetch nlohmann_json." OFF)
mark_as_advanced(nlohmann_json_FORCE_FETCH)

# Fetch nlohmann_json if there is no local installation
if(NOT TARGET nlohmann_json::nlohmann_json)
  if(nlohmann_json_FORCE_FETCH)
    _print_status(
      "nlohmann_json: Fetching version '${nlohmann_json_TAG}' "
      "from '${nlohmann_json_REPOSITORY}' (forced via nlohmann_json_FORCE_FETCH).")
  else()
    _print_status(
      "nlohmann_json: No local installation found. Fetching version "
      "'${nlohmann_json_TAG}' from '${nlohmann_json_REPOSITORY}'.")
  endif()

  # Necessary packages
  include(FetchContent)
  find_package(Git QUIET)

  # Declare nlohmann_json
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY ${nlohmann_json_REPOSITORY}
    GIT_TAG ${nlohmann_json_TAG}
    GIT_SHALLOW ON
    SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/nlohmann_json-${nlohmann_json_TAG})

  # Fetch nlohmann_json
  FetchContent_GetProperties(nlohmann_json)
  if(NOT nlohmann_json_POPULATED)
    cmake_policy(PUSH)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
      cmake_policy(SET CMP0169 OLD)
    endif()
    FetchContent_Populate(nlohmann_json)
    cmake_policy(POP)
  endif()

  # Print sha
  execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
    WORKING_DIRECTORY ${nlohmann_json_SOURCE_DIR}
    OUTPUT_VARIABLE nlohmann_json_sha)

  string(REPLACE "\n" "" nlohmann_json_sha ${nlohmann_json_sha})
  _print_status(
    "nlohmann_json: Successfully fetched '${nlohmann_json_TAG}' HEAD "
    "at SHA ${nlohmann_json_sha}")

  message(STATUS ${nlohmann_json_SOURCE_DIR})

  # Check if fetch was succesful
  if(EXISTS "${nlohmann_json_SOURCE_DIR}")
    set(NLOHMANN_JSON_INCLUDE_DIR "${nlohmann_json_SOURCE_DIR}/include")

    # Create imported target nlohmann_json::nlohmann_json
    add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)

    set_target_properties(nlohmann_json::nlohmann_json PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${NLOHMANN_JSON_INCLUDE_DIR}"
      INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${NLOHMANN_JSON_INCLUDE_DIR}")
    set(nlohmann_json_FOUND 1)
  else()
    set(nlohmann_json_FOUND 0)
    _print_error("nlohmann_json: Fetched version invalid.")
  endif()
else()
  _print_status("nlohmann_json: Using local nlohmann_json installation.")
endif()
