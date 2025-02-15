cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Custom flag for position independent code (enabled by default)
option(USE_FPIC "Use position independent code." ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ${USE_FPIC})

macro(print_fpic_status)
  _print_status("Position independent code (-fPIC): '${USE_FPIC}'")
endmacro()
