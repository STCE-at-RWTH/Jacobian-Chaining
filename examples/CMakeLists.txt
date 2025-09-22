# **************************************************************************** #
# This file is part of the JCDP build system. It builds the two main
# executable.
# **************************************************************************** #

cmake_minimum_required(VERSION 3.25.0)
cmake_policy(VERSION 3.25.0)

add_executable(example "main.cpp")
target_include_directories(example PRIVATE ${JCDP_include_dirs})
check_with_iwyu(example IWYU_FLAGS ${JCDP_IWYU_FLAGS})
check_with_cpplint(example IWYU_FLAGS ${JCDP_IWYU_FLAGS})

# OpenMP
jcdp_compile_with_openmp(PRIVATE example)
jcdp_link_openmp_runtime(PRIVATE example)

if(WIN32)
  add_cxx_flag("/EHsc" EHSC example)
endif()
