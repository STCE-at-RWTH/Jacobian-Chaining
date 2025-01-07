cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Tries to find a module
macro(_try_find_package module)
  find_package(${module} QUIET)

  if(NOT DEFINED ${module}_DIR)
    set(FOUND_MODULE_${module} TRUE)
  elseif(${module}_DIR)
    set(FOUND_MODULE_${module} TRUE)
  else()
    set(FOUND_MODULE_${module} FALSE)
  endif()
endmacro()
