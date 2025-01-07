cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

include(util/print)

# **************************************************************************** #

# Print a status message
function(print_status)
  _print_status(${ARGN})
endfunction()

# Print a warning
function(print_warning)
  _print_warning(${ARGN})
endfunction()

# Print an error message
function(print_error)
  _print_error(${ARGN})
endfunction()

# Print items of a list with line breaks if there are too many
function(print_list)
  _print_list(${ARGN})
endfunction()

# **************************************************************************** #

# Print a divide into the config (CMAKE_LINE_LENGHT long)
function(print_divide)
  set(options EMPTY_BEFORE EMPTY_AFTER)
  cmake_parse_arguments(ARG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  math(EXPR _characters "${CMAKE_LINE_LENGHT} - 4")
  _repeat_character("*" ${_characters} _fill_in)

  set(_divide "# ")
  string(APPEND _divide "${_fill_in}")
  string(APPEND _divide " #")

  # Print optional empty line
  if(ARG_EMPTY_BEFORE)
    message("")
  endif()

  # Print divide
  message(${_divide})

  # Print optional empty line
  if(ARG_EMPTY_AFTER)
    message("")
  endif()
endfunction()

# Print a project title with a centered title
function(print_title title)
  set(options EMPTY_BEFORE EMPTY_AFTER)
  cmake_parse_arguments(ARG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  math(EXPR _characters "${CMAKE_LINE_LENGHT} - 4")
  string(LENGTH ${title} title_length)
  if(${title_length} GREATER ${_characters})
    _print_error(
      "Title too long (${title_length} characters). Reduced to ${_characters} "
      "characters or increase CMAKE_LINE_LENGHT accordingly.")
  endif()

  math(EXPR _rest "(${_characters} - ${title_length}) % 2")
  math(EXPR _space "(${_characters} - ${title_length}) / 2")
  _repeat_character(" " ${_space} _fill_in)

  set(title_string "# ")
  string(APPEND title_string "${_fill_in}")
  string(APPEND title_string "${title}")
  string(APPEND title_string "${_fill_in}")
  if(${_rest})
    string(APPEND title_string " ")
  endif()
  string(APPEND title_string " #")

  # Print optional empty line
  if(ARG_EMPTY_BEFORE)
    message("")
  endif()

  # Print title
  print_divide()
  message(${title_string})
  print_divide()

  # Print optional empty line
  if(ARG_EMPTY_AFTER)
    message("")
  endif()
endfunction()

# Print a subtitle with a centered title
function(print_subtitle subtitle)
  set(options EMPTY_BEFORE EMPTY_AFTER)
  cmake_parse_arguments(ARG
    "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  math(EXPR _characters "${CMAKE_LINE_LENGHT} - 6")
  string(LENGTH ${subtitle} title_length)
  if(${title_length} GREATER ${_characters})
    _print_error(
      "Title too long (${title_length} characters). Reduced to ${_characters} "
      "characters or increase CMAKE_LINE_LENGHT accordingly.")
  endif()

  math(EXPR _rest "(${_characters} - ${title_length}) % 2")
  math(EXPR _space "(${_characters} - ${title_length}) / 2")
  _repeat_character("*" ${_space} _fill_in)

  set(title_string "# ")
  string(APPEND title_string "${_fill_in}")
  string(APPEND title_string " ${subtitle} ")
  string(APPEND title_string "${_fill_in}")
  if(${_rest})
    string(APPEND title_string "*")
  endif()
  string(APPEND title_string " #")

  # Print optional empty line
  if(ARG_EMPTY_BEFORE)
    message("")
  endif()

  # Print subtitle
  message(${title_string})

  # Print optional empty line
  if(ARG_EMPTY_AFTER)
    message("")
  endif()
endfunction()
