# **************************************************************************** #
# This file is part of the JCDP build system. It provides the main CMake
# entry point, which generates all necessary sources and builds the JCDP
# libraries and executables.
# **************************************************************************** #

cmake_minimum_required(VERSION 3.25.0)
cmake_policy(VERSION 3.25.0)

add_executable(jcdp "jcdp.cpp")
target_include_directories(jcdp PRIVATE ${JCDP_include_dirs})
check_with_iwyu(jcdp IWYU_FLAGS ${ADM_IWYU_FLAGS})
check_with_cpplint(jcdp IWYU_FLAGS ${ADM_IWYU_FLAGS})

add_executable(jcdp_batch "jcdp_batch.cpp")
target_include_directories(jcdp_batch PRIVATE ${JCDP_include_dirs})
check_with_cpplint(jcdp_batch IWYU_FLAGS ${ADM_IWYU_FLAGS})

# OpenMP
jcdp_link_openmp_runtime(PRIVATE jcdp jcdp_batch)
jcdp_compile_with_openmp(PRIVATE jcdp jcdp_batch)

if(WIN32)
  add_cxx_flag("/EHsc" EHSC jcdp jcdp_batch)
endif()
