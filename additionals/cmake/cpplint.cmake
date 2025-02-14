# **************************************************************************** #
# This file is part of the NAG's common cmake configuration. It adds helper
# functions to easily enable cpplint checks globally, for single
# targets and for header files.
#
# Copyright (c) 2023 NAG
# **************************************************************************** #

cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Enable IN_LIST if() operator
cmake_policy(SET CMP0057 NEW)

include(CMakeParseArguments)
include(util/print)

# Enable cpplint option
option(JCDP_CPPLINT "Enable Cpplint analysis." OFF)

# Check cpplint options
if(JCDP_CPPLINT)
  find_package(Cpplint REQUIRED)

  # Main header-only Cpplint target
  add_custom_target(header_only_cpplint ALL)
endif()

# **************************************************************************** #
# Setup classic Cpplint targets
# **************************************************************************** #
function(check_everything_with_cpplint)
  set(multiValueArgs CPPLINT_FLAGS)
  cmake_parse_arguments(_ARG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Enable Cpplint globally
  if(JCDP_CPPLINT AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    _print_status("Cpplint: Enabling Cpplint for all targets")

    get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    foreach(_language "C" "CXX")
      if(${_language} IN_LIST _enabled_languages)
        set(CMAKE_${_language}_CPPLINT
          ${CPPLINT_EXECUTABLE} ${_ARG_CPPLINT_FLAGS} PARENT_SCOPE)
      endif()
    endforeach()
  endif()
endfunction()

# **************************************************************************** #
# Setup Cpplint for a single target
# **************************************************************************** #
function(check_with_cpplint tgt_name)
  set(multiValueArgs CPPLINT_FLAGS)
  cmake_parse_arguments(_ARG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Check target
  if(NOT TARGET ${tgt_name})
    _print_error("Cpplint: Target '${tgt_name}' doesn't exist")
  endif()

  # Enable Cpplint for the given target
  if(JCDP_CPPLINT AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    foreach(_language "C" "CXX")
      if(${_language} IN_LIST _enabled_languages)
        set_property(TARGET ${tgt_name} PROPERTY
          ${_language}_CPPLINT ${CPPLINT_EXECUTABLE} ${_ARG_CPPLINT_FLAGS})
      endif()
    endforeach()
  endif()
endfunction()

# **************************************************************************** #
# Setup header-only Cpplint targets
# **************************************************************************** #
function(header_only_cpplint_targets header_descriptor)
  set(multiValueArgs HEADERS CPPLINT_FLAGS)
  cmake_parse_arguments(_ARG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Enable Cpplint for given headers
  if(JCDP_CPPLINT AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    _print_status(
      "Cpplint: Creating header-only cpplint targets (${header_descriptor})")

    foreach(header ${_ARG_HEADERS})
      get_filename_component(tgt_name ${header} NAME_WE)
      set(tgt_name "${header_descriptor}_${tgt_name}_cpplint")

      # Create custom Cpplint target for each header file
      add_custom_target(${tgt_name}
        COMMAND ${CPPLINT_EXECUTABLE} ${_ARG_CPPLINT_FLAGS} ${header} || true
        DEPENDS ${header}
        COMMENT "Running Cpplint on header file '${header}'")

      add_dependencies(header_only_cpplint ${tgt_name})
    endforeach()
  endif()
endfunction()
