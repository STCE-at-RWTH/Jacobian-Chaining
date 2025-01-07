# **************************************************************************** #
# This file is part of the JCDP build system. It provides the main CMake
# entry point, which generates all necessary sources and builds the JCDP
# libraries and executables.
# **************************************************************************** #

cmake_minimum_required(VERSION 3.25.0)
cmake_policy(VERSION 3.25.0)

add_executable(jcdp "jcdp.cpp")
target_include_directories(jcdp PRIVATE ${JCDP_include_dirs})
# check_with_iwyu(${app_name} IWYU_FLAGS ${ADM_IWYU_FLAGS})
