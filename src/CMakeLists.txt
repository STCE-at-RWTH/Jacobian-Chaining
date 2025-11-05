# **************************************************************************** #
# This file is part of the JCDP build system. It builds the two main
# executable (jcdp and jcdp_batch).
# **************************************************************************** #

cmake_minimum_required(VERSION 3.25.0)
cmake_policy(VERSION 3.25.0)

add_executable(jcdp "jcdp_run_from_config.cpp")
target_include_directories(jcdp PRIVATE ${JCDP_include_dirs})
check_with_iwyu(jcdp IWYU_FLAGS ${JCDP_IWYU_FLAGS})
check_with_cpplint(jcdp IWYU_FLAGS ${JCDP_IWYU_FLAGS})

# OpenMP
jcdp_compile_with_openmp(PRIVATE jcdp)
jcdp_link_openmp_runtime(PRIVATE jcdp)

if(EMSCRIPTEN)
  target_link_options(jcdp PRIVATE "SHELL:-s FORCE_FILESYSTEM=1")
  target_link_options(jcdp PRIVATE "SHELL:-s MODULARIZE=1")
  target_link_options(jcdp PRIVATE "SHELL:-s EXPORT_NAME=createJCDPModule")
  target_link_options(jcdp PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=0")
  target_link_options(jcdp PRIVATE "SHELL:-s PTHREAD_POOL_SIZE='navigator.hardwareConcurrency'")
  target_link_options(jcdp PRIVATE "SHELL:-s EXPORTED_FUNCTIONS='[\"_jcdp_run_from_config\"]")
  target_link_options(jcdp PRIVATE "SHELL:-s EXPORTED_RUNTIME_METHODS='[\"ENV\",\"ccall\",\"cwrap\"]'")
  target_link_options(jcdp PRIVATE "SHELL:-s INITIAL_MEMORY=2147483648")
  target_link_options(jcdp PRIVATE "SHELL:--preload-file ${PROJECT_SOURCE_DIR}/web/config.txt@/config.txt")
endif()

if(WIN32)
  add_cxx_flag("/EHsc" EHSC jcdp)
endif()

install(TARGETS jcdp DESTINATION .)
