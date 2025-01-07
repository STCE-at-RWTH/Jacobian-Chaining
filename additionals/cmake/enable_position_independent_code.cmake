cmake_minimum_required(VERSION 3.10)
include_guard(GLOBAL)

# Custom flag for position independent code (enabled by default)
option(USE_FPIC "Use position independent code." ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ${USE_FPIC})
