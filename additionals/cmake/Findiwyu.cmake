# Include print helper
include(print_helper)

# Search for include-what-you-use
find_program(IWYU_EXECUTABLE
  NAMES include-what-you-use iwyu
  DOC "include-what-you-use (iwyu) executable")
mark_as_advanced(IWYU_EXECUTABLE)

# Extract version from command "include-what-you-use --version"
if(IWYU_EXECUTABLE)
  execute_process(
    COMMAND ${IWYU_EXECUTABLE} --version
    OUTPUT_VARIABLE _iwyu_version
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(_iwyu_version MATCHES "^include-what-you-use .*")
    string(REGEX REPLACE
      "include-what-you-use ([.0-9]+).*" "\\1"
      _iwyu_version "${_iwyu_version}")

    if(NOT(iwyu_FIND_QUIETLY))
      _print_status(
        "IWYU: Found include-what-you-use version ${_iwyu_version}")
    endif()
  else()
    if(NOT(iwyu_FIND_QUIETLY))
      _print_status("IWYU: Found include-what-you-use (unknown version)")
    endif()
  endif()
  unset(_iwyu_version)

  set(iwyu_FOUND TRUE)
else()
  if(iwyu_FIND_REQUIRED)
    _print_error("IWYU: Couldn't find include-what-you-use")
  elseif(NOT(iwyu_FIND_QUIETLY))
    _print_warning("IWYU: Couldn't find include-what-you-use")
  endif()
  set(iwyu_FOUND FALSE)
endif()
