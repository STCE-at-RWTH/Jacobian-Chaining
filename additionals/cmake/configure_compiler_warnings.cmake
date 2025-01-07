cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Enable IN_LIST if() operator
cmake_policy(SET CMP0057 NEW)

include(util/check_flags)
include(util/print)

option(WARNINGS_AS_ERRORS
  "Build with warnings as errors." OFF)
option(FATAL_ERRORS
  "Use fatal errors (stop printing errors after the first one)." OFF)
option(DISABLE_WARNINGS "Disable all warnings." OFF)

get_property(_enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)

# **************************************************************************** #

# Check warning and error flags
foreach(_language "C" "CXX" "Fortran")
  if(${_language} IN_LIST _enabled_languages)
    set(_current_flags "")
    if(WIN32)
      if(DISABLE_WARNINGS)
        _add_flag(${_language} "/w" W)
      else()
        _add_flag(${_language} "/W4" W4)

        if(WARNINGS_AS_ERRORS)
          _add_flag(${_language} "/WX" WX)
        endif()

        if(CMAKE_${_language}_COMPILER_ID STREQUAL "Intel")
          # disable: remarks
          _add_flag(${_language}
            "/Qdiag-disable=remark" DIAG_DISABLE_REMARK)

          # disable: pointless comparison of unsigned integer with zero
          _add_flag(${_language}
            "/Qdiag-disable:186" DIAG_DISABLE_186)

          # disable: decorated name length exceeded, name was truncated
          _add_flag(${_language}
            "/Qdiag-disable:2586" DIAG_DISABLE_2586)

          # disable: This procedure is recursive by default
          _add_flag(${_language}
            "/Qdiag-disable:7762" DIAG_DISABLE_7762)

          # disable: disabling user-directed function packaging (COMDATs)
          _add_flag(${_language}
            "/Qdiag-disable:11031" DIAG_DISABLE_11031)

          # disable: The Intel(R) C++ Compiler Classic (ICC) is deprecated
          _add_flag(${_language}
            "/Qdiag-disable:10441" DIAG_DISABLE_10441)

          if(CHECK_${_language}_DIAG_DISABLE_10441)
            _add_linker_flags(${_language} "/Qdiag-disable:10441")
          endif()

          # disable: The Intel(R) Fortran Compiler Classic (ifort) is deprecated
          _add_flag(${_language}
            "/Qdiag-disable:10448" DIAG_DISABLE_10448)

          if(CHECK_${_language}_DIAG_DISABLE_10448)
            _add_linker_flags(${_language} "/Qdiag-disable:10448")
          endif()
        endif()
      endif()
    else()
      if(DISABLE_WARNINGS)
        _add_flag(${_language} "-w" W)
      else()
        _add_flag(${_language} "-Wall" WALL)
        _add_flag(${_language} "-Wextra" WEXTRA)
        _add_flag(${_language} "-pedantic" PEDANTIC)
        _add_flag(${_language} "-Wcast-align" WCAST_ALIGN)
        _add_flag(${_language}
          "-Wdisabled-optimization" WDISABLED_OPTIMIZATION)
        _add_flag(${_language}
          "-Wmissing-include-dirs" WMISSING_INCLUDE_DIRS)

        # C/C++ only warnings
        if(NOT ${_language} STREQUAL "Fortran")
          _add_flag(${_language} "-Wcast-qual" WCAST_QUAL)
          _add_flag(${_language} "-Wswitch-default" WSWITCH_DEFAULT)
          _add_flag(${_language} "-Wredundant-decls" WREDUNDANT_DECLS)
          _add_flag(${_language} "-Wlogical-op" WLOGICAL_OP)
        endif()

        # C only warnings
        if(${_language} STREQUAL "C")
          _add_flag(${_language} "-Werror-implicit-function-declaration"
            WERROR_IMPLICIT_FUNCTION_DECLARATION)
        endif()

        # C++ only warnings
        if(${_language} STREQUAL "CXX")
          _add_flag(${_language} "-Wnoexcept" WNOEXCEPT)
          _add_flag(${_language} "-Wold-style-cast" WOLD_STYLE_CAST)
          _add_flag(${_language} "-Wc++11-extensions" WCXX11_EXTENSIONS)
          _add_flag(${_language} "-Woverloaded-virtual" WOVERLOADED_VIRTUAL)
          _add_flag(${_language}
            "-Wmissing-include-dirs" WMISSING_INCLUDE_DIRS)
          _add_flag(${_language}
            "-Wstrict-null-sentinel" WSTRICT_NULL_SENTINEL)
        endif()

        if(WARNINGS_AS_ERRORS)
          _add_flag(${_language} "-Werror" WERROR)
        endif()

        if(FATAL_ERRORS)
          _add_flag(${_language} "-Wfatal-errors" WFATAL_ERRORS)
        endif()

        if(CMAKE_${_language}_COMPILER_ID STREQUAL "Intel")
          # disable: remarks
          _add_flag(${_language} "-diag-disable=remark" DIAG_DISABLE_REMARK)
          # disable: pointless comparison of unsigned integer with zero
          _add_flag(${_language} "-diag-disable:186" DIAG_DISABLE_186)
          # disable: decorated name length exceeded, name was truncated
          _add_flag(${_language} "-diag-disable:2586" DIAG_DISABLE_2586)
          # disable: This procedure is recursive by default
          _add_flag(${_language} "-diag-disable:7762" DIAG_DISABLE_7762)
          # disable: disabling user-directed function packaging (COMDATs)
          _add_flag(${_language} "-diag-disable:11031" DIAG_DISABLE_11031)

          # disable: The Intel(R) C++ Compiler Classic (ICC) is deprecated
          _add_flag(${_language} "-diag-disable:10441" DIAG_DISABLE_10441)
          if(CHECK_${_language}_DIAG_DISABLE_10441)
            _add_linker_flags(${_language} "-diag-disable:10441")
          endif()

          # disable: The Intel(R) Fortran Compiler Classic (ifort) is deprecated
          _add_flag(${_language} "-diag-disable:10448" DIAG_DISABLE_10448)
          if(CHECK_${_language}_DIAG_DISABLE_10448)
            _add_linker_flags(${_language} "-diag-disable:10448")
          endif()
        endif()

        if(CMAKE_${_language}_COMPILER_ID STREQUAL "IntelLLVM")
          # disable: use of '-g' without any optimization-level option will
          #          turn off most compiler optimizations
          _add_flag(${_language}
            "-Wno-debug-disables-optimization" WNO_DEBUG_DISABLES_OPTIMIZATION)
        endif()
      endif()
    endif()

    if(_current_flags)
      _print_list("${_language} warning flags:" ${_current_flags})
    else()
      _print_status("No ${_language} warning flags found.")
    endif()
  endif()
endforeach()

# **************************************************************************** #

# Cleanup
unset(_current_flags)
unset(_enabled_languages)
