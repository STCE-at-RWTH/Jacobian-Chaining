cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Enable IN_LIST if() operator
cmake_policy(SET CMP0057 NEW)

include(CMakeParseArguments)
include(util/print)

# Needed to set the correct gcc toolchain used by iwyu for header-only targets
include(cxx_standard_library)

# Enable IWYU option
option(JCDP_INCLUDE_WHAT_YOU_USE "Enable include-what-you-use checks." OFF)

# Check IWYU options
if(JCDP_INCLUDE_WHAT_YOU_USE)
  find_package(iwyu REQUIRED)

  # Check if compiler is clang
  get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
  foreach(_language "C" "CXX")
    if(${_language} IN_LIST _enabled_languages)
      if(NOT(CMAKE_${_language}_COMPILER_ID STREQUAL "Clang"))
        _print_warning(
          "IWYU: ${_language} compiler is not Clang. Since IWYU uses Clang "
          "implicitly we recommend to use Clang everywhere when IWYU is "
          "enabled.")
      endif()
    endif()
  endforeach()

  # Main header-only IWYU target
  add_custom_target(header_only_iwyu ALL)
endif()

# **************************************************************************** #
# Setup classic IWYU targets
# **************************************************************************** #
function(check_everything_with_iwyu)
  set(multiValueArgs IWYU_FLAGS)
  cmake_parse_arguments(_ARG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Collect flags
  set(_flags "")
  foreach(_iwyu_flag ${_ARG_IWYU_FLAGS})
    list(APPEND _flags "-Xiwyu" "${_iwyu_flag}")
  endforeach()

  # Enable IWYU globally
  if(JCDP_INCLUDE_WHAT_YOU_USE AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    _print_status("IWYU: Enabling include-what-you-use for all targets")

    get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    foreach(_language "C" "CXX")
      if(${_language} IN_LIST _enabled_languages)
        set(CMAKE_${_language}_JCDP_INCLUDE_WHAT_YOU_USE
          ${IWYU_EXECUTABLE} ${_flags} PARENT_SCOPE)
      endif()
    endforeach()
  endif()
endfunction()

# **************************************************************************** #
# Setup IWYU for a single target
# **************************************************************************** #
function(check_with_iwyu tgt_name)
  set(multiValueArgs IWYU_FLAGS)
  cmake_parse_arguments(_ARG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Check target
  if(NOT TARGET ${tgt_name})
    _print_error("IWYU: Target '${tgt_name}' doesn't exist")
  endif()

  # Collect flags
  set(_flags "")
  foreach(_iwyu_flag ${_ARG_IWYU_FLAGS})
    list(APPEND _flags "-Xiwyu" "${_iwyu_flag}")
  endforeach()

  # Enable IWYU for the given target
  if(JCDP_INCLUDE_WHAT_YOU_USE AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
    foreach(_language "C" "CXX")
      if(${_language} IN_LIST _enabled_languages)
        set_property(TARGET ${tgt_name}
          PROPERTY ${_language}_JCDP_INCLUDE_WHAT_YOU_USE
          ${IWYU_EXECUTABLE} ${_flags})
      endif()
    endforeach()
  endif()
endfunction()

# **************************************************************************** #
# Setup header-only IWYU targets
# **************************************************************************** #
function(header_only_iwyu_targets tgt_name)
  set(multiValueArgs
    HEADERS COMPILER_FLAGS INCLUDE_DIRS
    SYSTEM_INCLUDE_DIRS LIBRARIES IWYU_FLAGS DEPENDS)
  cmake_parse_arguments(_ARG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Enable IWYU for given headers
  if(JCDP_INCLUDE_WHAT_YOU_USE AND CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    # Collect flags
    set(_flags ${_ARG_COMPILER_FLAGS})
    foreach(_incdir ${_ARG_INCLUDE_DIRS})
      list(APPEND _flags "-I${_incdir}")
    endforeach()
    foreach(_incdir ${_ARG_SYSTEM_INCLUDE_DIRS})
      list(APPEND _flags "-isystem" "${_incdir}")
    endforeach()
    foreach(_iwyu_flag ${_ARG_IWYU_FLAGS})
      list(APPEND _flags "-Xiwyu" "${_iwyu_flag}")
    endforeach()
    foreach(_lib ${_ARG_LIBRARIES})
      if(TARGET ${_lib})
        get_target_property(_sys_inc
          ${_lib} INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
        if(_sys_inc)
          foreach(_incdir ${_sys_inc})
            list(APPEND _flags "-isystem" "${_incdir}")
          endforeach()
        endif()

        get_target_property(_inc ${_lib} INTERFACE_INCLUDE_DIRECTORIES)
        if(_inc)
          foreach(_incdir ${_inc})
            if(NOT(${_incdir} IN_LIST _sys_inc))
              list(APPEND _flags "-I${_incdir}")
            endif()
          endforeach()
        endif()
      else()
        _print_warning("Header-only IWYU: Target ${_lib} does not exist.")
      endif()
    endforeach()

    # See cxx_standard_library
    if(NOT CXX_STD_LIB STREQUAL "default")
      list(APPEND _flags "-stdlib=${CXX_STD_LIB}")
    endif()
    if(CLANG_GCC_TOOLCHAIN)
      list(APPEND _flags "--gcc-toolchain=${CLANG_GCC_TOOLCHAIN}")
    endif()

    _print_status(
      "IWYU: Creating header-only include-what-you-use targets (${tgt_name})")

    # Escape semicolons
    string (REPLACE ";" "\;" _flags "${_flags}")

    add_custom_target(${tgt_name})
    foreach(header ${_ARG_HEADERS})
      get_filename_component(file_tgt_name ${header} NAME_WE)
      set(file_tgt_name "${tgt_name}_${file_tgt_name}")

      # Create custom IWYU target for each header file
      add_custom_target(${file_tgt_name}
        COMMAND ${CMAKE_COMMAND}
          -DIWYU_EXECUTABLE="${IWYU_EXECUTABLE}"
          -DIWYU_ARGS="${_flags}"
          -DIWYU_FILE="${header}"
          -P ${CMake_Config_DIR}/scripts/run_iwyu.cmake
        DEPENDS ${header}
        COMMENT "Running IWYU on header file '${header}'")

      if(_ARG_DEPENDS)
        add_dependencies(${file_tgt_name} ${_ARG_DEPENDS})
      endif()
      add_dependencies(${tgt_name} ${file_tgt_name})
    endforeach()
    add_dependencies(header_only_iwyu ${tgt_name})
  endif()
endfunction()

# **************************************************************************** #

# Cleanup
unset(_enabled_languages)
unset(_language)
