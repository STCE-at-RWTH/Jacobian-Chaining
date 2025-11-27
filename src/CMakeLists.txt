# **************************************************************************** #
# This file is part of the JCDP build system. It builds the two main
# executable (jcdp and jcdp_batch).
# **************************************************************************** #

cmake_minimum_required(VERSION 3.25.0)
cmake_policy(VERSION 3.25.0)

find_package(nlohmann_json REQUIRED)
set(jcdp_lib_files "json_api.cpp")
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
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s EXPORTED_FUNCTIONS='[\"_malloc\", \"_free\"]'")
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s EXPORTED_RUNTIME_METHODS='[\"ENV\",\"ccall\",\"cwrap\",\"getValue\",\"UTF8ToString\"]'")
   target_link_options(${jcdp_lib_name}_wasm PRIVATE "SHELL:-s INITIAL_MEMORY=2147483648")

   list(APPEND jcdp_libs ${jcdp_lib_name}_wasm)

   # Custom target to create the NPM package
   set(NPM_PACKAGE_DIR "${CMAKE_BINARY_DIR}/package")

   configure_file(
      "${CMAKE_SOURCE_DIR}/additionals/package/package.json.in"
      "${NPM_PACKAGE_DIR}/package.json"
      @ONLY
   )

   add_custom_target(create_npm_package
      DEPENDS ${jcdp_lib_name}_wasm
      COMMENT "Creating NPM package..."
   )

   # Copy the package template
   add_custom_command(TARGET create_npm_package POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory ${NPM_PACKAGE_DIR}
      COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/additionals/package" ${NPM_PACKAGE_DIR}
      COMMENT "Copying package template"
   )

   # Copy the WASM artifacts
   add_custom_command(TARGET create_npm_package POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory ${NPM_PACKAGE_DIR}/lib
      COMMAND ${CMAKE_COMMAND} -E copy
         "$<TARGET_FILE_DIR:${jcdp_lib_name}_wasm>/${jcdp_lib_name}.js"
         "${NPM_PACKAGE_DIR}/lib/"
      COMMAND ${CMAKE_COMMAND} -E copy
         "$<TARGET_FILE_DIR:${jcdp_lib_name}_wasm>/${jcdp_lib_name}.wasm"
         "${NPM_PACKAGE_DIR}/lib/"
      COMMENT "Copying WASM artifacts to NPM package"
   )

   find_program(NPM npm)
   if(NPM)
      add_custom_target(build_npm_package
         DEPENDS create_npm_package
         WORKING_DIRECTORY ${NPM_PACKAGE_DIR}
         COMMAND ${NPM} install
         COMMAND ${NPM} run format
         COMMAND ${NPM} run build
         COMMENT "Building NPM package"
      )

      add_custom_target(publish_npm_package
         DEPENDS build_npm_package
         WORKING_DIRECTORY ${NPM_PACKAGE_DIR}
         COMMAND ${NPM} publish
         COMMENT "Publishing NPM package"
      )
   endif()

   install(DIRECTORY ${NPM_PACKAGE_DIR} DESTINATION .)
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
