cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

set(CMAKE_LINE_LENGHT "80" CACHE STRING "CMake output line lenght.")
mark_as_advanced(CMAKE_LINE_LENGHT)
set(_config_string "JCDP config - ")

# **************************************************************************** #

# Print a status message
function(_print_status)
  message(STATUS ${_config_string} ${ARGN})
endfunction()

# Print a warning
function(_print_warning)
  message(WARNING ${_config_string} ${ARGN})
endfunction()

# Print an error message
function(_print_error)
  message(FATAL_ERROR ${_config_string} ${ARGN})
endfunction()

# **************************************************************************** #

# Reapeat a character a certain amount of times
function(_repeat_character character amount output_string)
  set(_str "")
  foreach(_i RANGE 1 ${amount})
    string(APPEND _str ${character})
  endforeach()
  set(${output_string} ${_str} PARENT_SCOPE)
endfunction()

# **************************************************************************** #

# Print items of a list with line breaks if there are too many
function(_print_list description items)
  set(items ${items} ${ARGN})

  string(LENGTH ${_config_string} _config_length)
  string(LENGTH ${description} _desc_length)
  math(EXPR _allowed_length "${CMAKE_LINE_LENGHT} - ${_config_length} - 3")

  # If the description is too long print it alone and put items on next line
  if(${_desc_length} LESS 23)
    set(_current_string "${description}")
  else()
    _print_status(${description})
    set(_desc_length 2)
    _repeat_character(" " ${_desc_length} _current_string)
  endif()

  foreach(_str ${items})
    string(LENGTH ${_current_string} _length)
    string(LENGTH ${_str} _str_length)
    math(EXPR _new_length "${_length} + ${_str_length} + 1")

    if(${_allowed_length} LESS ${_new_length})
      _print_status(${_current_string})
      unset(_current_string)
      _repeat_character(" " ${_desc_length} _current_string)
    endif()

    string(CONCAT _current_string ${_current_string} " ${_str}")
  endforeach()

  _print_status(${_current_string})
endfunction()
