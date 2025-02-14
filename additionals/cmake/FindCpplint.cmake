cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Include print helper
include(print_helper)

# Search for Cpplint
find_program(CPPLINT_EXECUTABLE
  NAMES cpplint
  DOC "Cppclint executable")
mark_as_advanced(CPPLINT_EXECUTABLE)

# Extract version from command "cpplint --version"
if(CPPLINT_EXECUTABLE)
  execute_process(
    COMMAND ${CPPLINT_EXECUTABLE} --version
    OUTPUT_VARIABLE _cpplint_version
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(_cpplint_version MATCHES "^Cpplint .*")
    string(REGEX REPLACE
      "(.|\n)*cpplint ([.0-9]+)(.|\n)*" "\\2"
      _cpplint_version "${_cpplint_version}")

    if(NOT(Cpplint_FIND_QUIETLY))
      _print_status(
        "Cpplint: Found Cpplint version ${_cpplint_version}")
    endif()
  else()
    if(NOT(Cpplint_FIND_QUIETLY))
      _print_status("Cpplint: Found Cpplint (unknown version)")
    endif()
  endif()
  unset(_cpplint_version)

  set(cpplint_FOUND TRUE)
else()
  if(Cpplint_FIND_REQUIRED)
    _print_error("Cpplint: Couldn't find Cpplint")
  elseif(NOT(Cpplint_FIND_QUIETLY))
    _print_warning("Cpplint: Couldn't find Cpplint")
  endif()
  set(Cpplint_FOUND FALSE)
endif()
