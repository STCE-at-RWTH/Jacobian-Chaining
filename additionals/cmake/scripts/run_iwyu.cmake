cmake_minimum_required(VERSION 3.10)

# **************************************************************************** #
# Check inputs (defined variables)
# **************************************************************************** #
foreach(_arg EXECUTABLE FILE)
  if(NOT IWYU_${_arg})
    message("run_iwyu: IWYU_${_arg} not defined!")
  endif()
endforeach()

# **************************************************************************** #
# Run IWYU and parse output
# **************************************************************************** #
execute_process(
  COMMAND ${IWYU_EXECUTABLE} ${IWYU_ARGS} ${IWYU_FILE}
  RESULT_VARIABLE _retcode
  ERROR_VARIABLE _error
  OUTPUT_QUIET)

if(${_retcode})
  message(
    "Error running "
    "${IWYU_EXECUTABLE} ${IWYU_ARGS} ${IWYU_FILE}:\n"
    "${_error}\n")
endif()

if("${_error}" MATCHES "should (add|remove) these lines:")
  message("Warning: include-what-you-use reported diagnostics:\n${_error}\n")
endif()
