cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Enable IN_LIST if() operator
cmake_policy(SET CMP0057 NEW)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckFortranCompilerFlag)

include(util/print)

set(_supported_languages "C" "CXX" "Fortran")

# **************************************************************************** #

# Check the given compiler flags and sort them into valid and invalid flags
function(_check_flags language flags test_var)
  # Check language
  if(NOT("${language}" IN_LIST _supported_languages))
    _print_error(
      "Tried to check compiler flags for unsupported language '${language}'.")
  endif()

  get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
  if(NOT("${language}" IN_LIST _languages))
    _print_warning(
      "Tried to check compiler flag but the language '${language}' "
      "is not enabled.")
    return()
  endif()

  # Check for optional arguments (FORCE)
  set(_force_check OFF)
  foreach(_arg IN LISTS ARGN)
    string(TOUPPER "${_arg}" _arg_upper)
    if(_arg_upper STREQUAL "FORCE")
      set(_force_check ON)
    else()
      _print_error("Invalid argument in '_check_flags': ${_arg}")
    endif()
  endforeach()

  set(_test_variable "CHECK_${language}_${test_var}")

  # Check arguments
  string(REPLACE " " ";" _flag_list "${flags}")
  foreach(flag IN LISTS _flag_list)
    # If the check is forced, remove the cached result from the previous check
    if(_force_check AND DEFINED "${_test_variable}")
      unset(${_test_variable})
      unset(${_test_variable} CACHE)
    endif()

    # With CMake 3.18.0 this could be done via:
    # string(TOLOWER "${language}" _language_lower)
    # cmake_language(CALL check_${_language_lower}_compiler_flag
    #   ${flag} ${_test_variable}))
    if("${language}" STREQUAL "C")
      check_c_compiler_flag(${flag} ${_test_variable})
    elseif("${language}" STREQUAL "CXX")
      check_cxx_compiler_flag(${flag} ${_test_variable})
    else()
      check_fortran_compiler_flag(${flag} ${_test_variable})
    endif()

    if("${${_test_variable}}")
      list(APPEND _tmp_valid_flags "${flag}")
    else()
      list(APPEND _tmp_invalid_flags "${flag}")
    endif()
  endforeach()

  set(_valid_flags ${_valid_flags} "${_tmp_valid_flags}" PARENT_SCOPE)
  set(_invalid_flags ${_invalid_flags} "${_tmp_invalid_flags}" PARENT_SCOPE)
endfunction()

# **************************************************************************** #

# Add the given compiler flag if it is valid
function(_add_flag language flag test_var)
  list(LENGTH flag _flag_amount)
  if(${_flag_amount} GREATER 1)
    _print_warning("Used _add_flag with more than one flag.")
  endif()

  # Check for optional arguments (targets)
  set(_targets "")
  foreach(_arg IN LISTS ARGN)
    if(TARGET ${_arg})
      list(APPEND _targets ${_arg})
    else()
      _print_warning(
        "Tried to add flag '${flag}' to non-existent target '${_arg}'.")
    endif()
  endforeach()

  unset(_valid_flags)
  _check_flags(${language} "${flag}" ${test_var})

  if(_valid_flags)
    if(_targets)
      foreach(_target IN LISTS _targets)
        target_compile_options(
          ${_target} PRIVATE "$<$<COMPILE_LANGUAGE:${language}>:${flag}>")
      endforeach()
    else()
      add_compile_options("$<$<COMPILE_LANGUAGE:${language}>:${flag}>")
    endif()
    set(_current_flags ${_current_flags} ${flag} PARENT_SCOPE)
  endif()
endfunction()

# **************************************************************************** #

# Add the given compiler definition if it is valid
function(_add_definition language def)
  list(LENGTH def _def_amount)
  if(${_def_amount} GREATER 1)
    _print_warning(
      "Used _add_definition with more than one definition.")
  endif()

  # Check for optional arguments (targets)
  set(_targets "")
  foreach(_arg IN LISTS ARGN)
    if(TARGET ${_arg})
      list(APPEND _targets ${_arg})
    else()
      _print_warning(
        "Tried to add definition '${def}' to non-existent target '${_arg}'.")
    endif()
  endforeach()

  if(_targets)
    foreach(_target IN LISTS _targets)
      target_compile_definitions(
        ${_target} PRIVATE "$<$<COMPILE_LANGUAGE:${language}>:${def}>")
    endforeach()
  else()
    if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
      if(WIN32)
        add_compile_options("$<$<COMPILE_LANGUAGE:${language}>:/D${def}>")
      else()
        add_compile_options("$<$<COMPILE_LANGUAGE:${language}>:-D${def}>")
      endif()
    else()
      add_compile_definitions("$<$<COMPILE_LANGUAGE:${language}>:${def}>")
    endif()
  endif()
endfunction()

# **************************************************************************** #

# Add linker option
function(_add_linker_flags language flags)
  foreach(_flag IN LISTS flags)
    add_link_options("$<$<COMPILE_LANGUAGE:${language}>:${_flag}>")
  endforeach()
endfunction()
