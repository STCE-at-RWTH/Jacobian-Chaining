cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Enable IN_LIST if() operator
cmake_policy(SET CMP0057 NEW)

include(util/check_flags)
include(util/print)

# Enable sanitation option
option(SANITIZE_ADDRESS "Enable address sanitization check." OFF)
option(SANITIZE_THREADS "Enable thread sanitization check." OFF)
option(SANITIZE_LEAK "Enable leak sanitization check." OFF)
option(SANITIZE_UNDEFINED_BEHAVIOUR "Enable UB sanitization check." OFF)

get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)

macro(print_sanitation_status)
  _print_status("Address sanitizer: '${SANITIZE_ADDRESS}'")
  _print_status("Thread sanitizer: '${SANITIZE_THREADS}'")
  _print_status("Leak sanitizer: '${SANITIZE_LEAK}'")
  _print_status("Undefined Behaviour sanitizer: '${SANITIZE_UNDEFINED_BEHAVIOUR}'")
endmacro()

# **************************************************************************** #

# Searches for the address sanitation flags
function(_search_for_asan_flags language)
  set(_reg_link_options ${CMAKE_REQUIRED_LINK_OPTIONS})
  set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address")

  if(WIN32)
    _add_flag(${language} "/fsanitize=address" SANITIZE_ADDRESS)
  else()
    _add_flag(${language} "-fsanitize=address" SANITIZE_ADDRESS)
    if(NOT(_current_flags))
      return()
    endif()
    _add_flag(${language} "-fno-omit-frame-pointer" NO_OMIT_FRAME_POINTER)
  endif()

  set(CMAKE_REQUIRED_LINK_OPTIONS ${_reg_link_options})
  set(_current_flags ${_current_flags} PARENT_SCOPE)
endfunction()

# Searches for the thread sanitation flags
function(_search_for_tsan_flags language)
  set(_reg_link_options ${CMAKE_REQUIRED_LINK_OPTIONS})
  set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=thread")

  if(WIN32)
    _print_warning("Thread sanitzer not supported on Windows.")
  else()
    _add_flag(${language} "-fsanitize=thread" SANITIZE_THREAD)
    if(NOT(_current_flags))
      return()
    endif()
    _add_flag(${language} "-fno-omit-frame-pointer" NO_OMIT_FRAME_POINTER)
  endif()

  set(CMAKE_REQUIRED_LINK_OPTIONS ${_reg_link_options})
  set(_current_flags ${_current_flags} PARENT_SCOPE)
endfunction()

# Searches for the leak sanitation flags
function(_search_for_lsan_flags language)
  set(_reg_link_options ${CMAKE_REQUIRED_LINK_OPTIONS})
  set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=leak")

  if(WIN32)
    _add_flag(${language} "/fsanitize=leak" SANITIZE_LEAK)
  else()
    _add_flag(${language} "-fsanitize=leak" SANITIZE_LEAK)
    if(NOT(_current_flags))
      return()
    endif()
    _add_flag(${language} "-fno-omit-frame-pointer" NO_OMIT_FRAME_POINTER)
  endif()

  set(CMAKE_REQUIRED_LINK_OPTIONS ${_reg_link_options})
  set(_current_flags ${_current_flags} PARENT_SCOPE)
endfunction()

# Searches for the undefined behavour sanitation flags
function(_search_for_ubsan_flags language)
  set(_reg_link_options ${CMAKE_REQUIRED_LINK_OPTIONS})
  set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=undefined")

  if(WIN32)
    _print_warning("Undefined behaviour sanitzer not supported on Windows.")
  else()
    _add_flag(${language} "-fsanitize=undefined" SANITIZE_UB)
    if(NOT(_current_flags))
      return()
    endif()
    _add_flag(${language} "-fno-omit-frame-pointer" NO_OMIT_FRAME_POINTER)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      if(CMAKE_CXX_COMPILER_VERSION LESS 8)
        _add_flag(${language} "-fuse-ld=gold" SANITIZE_UB)
      endif()
    endif()
  endif()

  set(CMAKE_REQUIRED_LINK_OPTIONS ${_reg_link_options})
  set(_current_flags ${_current_flags} PARENT_SCOPE)
endfunction()

# **************************************************************************** #

# If SANITIZE_ADDRESS is set, check for address sanitation flags.
if(SANITIZE_ADDRESS)
  foreach(_language "C" "CXX" "Fortran")
    if("${_language}" IN_LIST _enabled_languages)
      # Search for architecture flag for the current language
      set(_current_flags "")
      _search_for_asan_flags(${_language})

      if(_current_flags)
        _add_linker_flags(${_language} "${_current_flags}")
        _print_list(
          "${_language} address sanitizer flags:" ${_current_flags})
      else()
        _print_status("No ${_language} address sanitizer flags found.")
      endif()
    endif()
  endforeach()
endif()

# If SANITIZE_THREADS is set, check for thread sanitation flags.
if(SANITIZE_THREADS)
  if(SANITIZE_ADDRESS OR SANITIZE_UNDEFINED_BEHAVIOUR OR SANITIZE_LEAK)
    _print_error(
      "You can't combine the thread sanitizer with other sanitizers.")
  endif()

  foreach(_language "C" "CXX" "Fortran")
    if("${_language}" IN_LIST _enabled_languages)
      # Search for architecture flag for the current language
      set(_current_flags "")
      _search_for_tsan_flags(${_language})

      if(_current_flags)
        _add_linker_flags(${_language} "${_current_flags}")
        _print_list(
          "${_language} thread sanitizer flags:" ${_current_flags})
      else()
        _print_status("No ${_language} thread sanitizer flags found.")
      endif()
    endif()
  endforeach()
endif()

# If SANITIZE_LEAK is set, check for leak sanitation flags.
if(SANITIZE_LEAK)
  foreach(_language "C" "CXX" "Fortran")
    if("${_language}" IN_LIST _enabled_languages)
      # Search for architecture flag for the current language
      set(_current_flags "")
      _search_for_lsan_flags(${_language})

      if(_current_flags)
        _add_linker_flags(${_language} "${_current_flags}")
        _print_list(
          "${_language} leak sanitizer flags:" ${_current_flags})
      else()
        _print_status("No ${_language} leak sanitizer flags found.")
      endif()
    endif()
  endforeach()
endif()

# If SANITIZE_UNDEFINED_BEHAVIOUR is set, check for undefined behaviour sanitation flags.
if(SANITIZE_UNDEFINED_BEHAVIOUR)
  foreach(_language "C" "CXX" "Fortran")
    if("${_language}" IN_LIST _enabled_languages)
      # Search for architecture flag for the current language
      set(_current_flags "")
      _search_for_ubsan_flags(${_language})

      if(_current_flags)
        _add_linker_flags(${_language} "${_current_flags}")
        _print_list(
          "${_language} undefined behaviour sanitizer flags:"
          ${_current_flags})
      else()
        _print_status(
          "No ${_language} undefined behaviour sanitizer flags found.")
      endif()
    endif()
  endforeach()
endif()

# **************************************************************************** #

# Cleanup
unset(_enabled_languages)
unset(_current_flags)
unset(_valid_flags)
unset(_invalid_flags)
