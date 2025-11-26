# **************************************************************************** #
# This file is part of the JCDP build system. It builds the two main
# executable (jcdp and jcdp_batch).
# **************************************************************************** #

cmake_minimum_required(VERSION 3.25.0)
cmake_policy(VERSION 3.25.0)

find_package(nlohmann_json REQUIRED)
set(jcdp_lib_files "jcdp_run_from_json.cpp")
set(jcdp_lib_name "jcdp")
set(jcdp_libs "")

# Create shared library
if(JCDP_SHARED_LIBS)
   add_library(${jcdp_lib_name}_shared SHARED ${jcdp_lib_files})
   set_target_properties(${jcdp_lib_name}_shared PROPERTIES
     OUTPUT_NAME ${jcdp_lib_name})
   list(APPEND jcdp_libs ${jcdp_lib_name}_shared)
endif()

# Create static library
if(JCDP_STATIC_LIBS)
   add_library(${jcdp_lib_name}_static STATIC ${jcdp_lib_files})
   set_target_properties(${jcdp_lib_name}_static PROPERTIES
     OUTPUT_NAME ${jcdp_lib_name})
   list(APPEND jcdp_libs ${jcdp_lib_name}_static)
endif()

if(EMSCRIPTEN)
   add_executable(${jcdp_lib_name}_wasm ${jcdp_lib_files})
   set_target_properties(${jcdp_lib_name}_wasm PROPERTIES
     OUTPUT_NAME ${jcdp_lib_name})

   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s FORCE_FILESYSTEM=1")
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s MODULARIZE=1")
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s EXPORT_NAME=createJCDPModule")
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=0")
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s PTHREAD_POOL_SIZE='navigator.hardwareConcurrency'")
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s EXPORTED_FUNCTIONS='[\"_jcdp_run_from_config\", \"_jcdp_run_from_json\"]")
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s EXPORTED_RUNTIME_METHODS='[\"ENV\",\"ccall\",\"cwrap\"]'")
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s INITIAL_MEMORY=2147483648")

   list(APPEND jcdp_libs ${jcdp_lib_name}_wasm)
endif()

foreach(lib IN LISTS jcdp_libs)
   target_include_directories(${lib} PRIVATE ${JCDP_include_dirs})
   target_link_libraries(${lib} PRIVATE nlohmann_json::nlohmann_json)
   check_with_iwyu(${lib} IWYU_FLAGS ${JCDP_IWYU_FLAGS})
   check_with_cpplint(${lib} IWYU_FLAGS ${JCDP_IWYU_FLAGS})

   # OpenMP
   jcdp_compile_with_openmp(PRIVATE ${lib})
   jcdp_link_openmp_runtime(PRIVATE ${lib})

   if(WIN32)
      add_cxx_flag("/EHsc" EHSC ${lib})
   endif()
endforeach()

install(TARGETS ${jcdp_libs} DESTINATION lib)
if(EMSCRIPTEN)
   set(wasm_file "$<TARGET_FILE_DIR:${jcdp_lib_name}_wasm>/${jcdp_lib_name}.wasm")
   install(FILES ${wasm_file} DESTINATION lib)
endif()
