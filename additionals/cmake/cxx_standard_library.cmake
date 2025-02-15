cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Check if we are on Windows
if(WIN32)
  return()
endif()

# Enable IN_LIST if() operator
cmake_policy(SET CMP0057 NEW)

include(util/check_flags)
include(util/print)

# Check if C++ is enabled
get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
if(NOT("CXX" IN_LIST _enabled_languages))
  _print_error("C++ is not enabled. Standard library is not relevant.")
endif()

set(_cxx_std_libs "default")

# Check libstdc++ (gnu)
unset(_valid_flags)
_check_flags("CXX" "-stdlib=libstdc++" STDLIB_LIBSTDCXX)
if(_valid_flags)
  list(APPEND _cxx_std_libs "libstdc++")
endif()

# Check libc++ (llvm)
unset(_valid_flags)
_check_flags("CXX" "-stdlib=libc++" STDLIB_LIBCXX)
if(_valid_flags)
  list(APPEND _cxx_std_libs "libc++")
endif()

# **************************************************************************** #

# Use libc++ for clang by default
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND "libc++" IN_LIST _cxx_std_libs)
  set(_default_std_lib "libc++")
else()
  set(_default_std_lib "default")
endif()

# Create the cache variables
set(CXX_STD_LIB ${_default_std_lib}
  CACHE STRING "The C++ standard library that is used.")
set_property(CACHE CXX_STD_LIB PROPERTY STRINGS ${_cxx_std_libs})

if(CMAKE_CXX_COMPILER_ID MATCHES "(Clang|IntelLLVM|NVHPC)" OR
   CMAKE_C_COMPILER_ID MATCHES "(Clang|IntelLLVM|NVHPC)")
  if(DEFINED ENV{GCC_ROOT})
    set(_default_gcc_toolchain $ENV{GCC_ROOT})
  endif()

  set(CLANG_GCC_TOOLCHAIN "${_default_gcc_toolchain}"
    CACHE STRING "The '--gcc-toolchain=' used by Clang-based compilers.")
endif()

macro(print_stdlib_status)
  _print_status("C++ standard library: '${CXX_STD_LIB}'")
endmacro()

# **************************************************************************** #

if(NOT CXX_STD_LIB STREQUAL "default")
  _print_status("Using C++ standard library: ${CXX_STD_LIB}")

  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-stdlib=${CXX_STD_LIB}>")
  add_link_options("$<$<COMPILE_LANGUAGE:CXX>:-stdlib=${CXX_STD_LIB}>")

  if(CXX_STD_LIB STREQUAL "libc++")
    add_link_options("$<$<COMPILE_LANGUAGE:CXX>:-lc++abi>")
  endif()
endif()

# **************************************************************************** #

if(CLANG_GCC_TOOLCHAIN)
  _print_status(
    "Clang uses custom GCC toolchain: ${CLANG_GCC_TOOLCHAIN}")

  set(_toolchain "--gcc-toolchain=${CLANG_GCC_TOOLCHAIN}")

  foreach(_language "C" "CXX")
    if(CMAKE_${_language}_COMPILER_ID MATCHES "(Clang|IntelLLVM|NVHPC)")
      _add_flag(${_language} "${_toolchain}" GCC_TOOLCHAIN)
      if(CHECK_${_language}_GCC_TOOLCHAIN)
        add_link_options("$<$<COMPILE_LANGUAGE:${_language}>:${_toolchain}>")
      endif()
    endif()
  endforeach()
endif()

# **************************************************************************** #

# Cleanup
unset(_enabled_languages)
unset(_cxx_std_libs)
unset(_default_std_lib)
unset(_default_gcc_toolchain)
unset(_valid_flags)
unset(_invalid_flags)
unset(_toolchain)
