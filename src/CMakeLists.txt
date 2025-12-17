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
   macro(set_wasm_options target_name)
      target_link_options(${target_name} PRIVATE
         "SHELL:-s FORCE_FILESYSTEM=1"
         "SHELL:-s MODULARIZE=1"
         "SHELL:-s EXPORT_NAME=createJCDPModule"
         "SHELL:-s ALLOW_MEMORY_GROWTH=0"
         "SHELL:-s PTHREAD_POOL_SIZE='navigator.hardwareConcurrency'"
         "SHELL:-s EXPORTED_FUNCTIONS='[\"_malloc\", \"_free\"]'"
         "SHELL:-s EXPORTED_RUNTIME_METHODS='[\"ENV\",\"ccall\",\"cwrap\",\"getValue\",\"setValue\",\"UTF8ArrayToString\", \"HEAP8\", \"HEAP32\", \"HEAPF64\"]'"
         "SHELL:-s INITIAL_MEMORY=2147483648"
         "SHELL:-s EXPORT_ES6=1"
         "SHELL:-s USE_ES6_IMPORT_META=1")
   endmacro()

   # Test/Dev Target (Node compatible)
   add_executable(${jcdp_lib_name}_wasm ${jcdp_lib_files})
   set_target_properties(${jcdp_lib_name}_wasm PROPERTIES
      OUTPUT_NAME ${jcdp_lib_name})
   set_wasm_options(${jcdp_lib_name}_wasm)
   target_link_options(${jcdp_lib_name}_wasm PRIVATE
      "SHELL:--emit-tsd $<TARGET_FILE_DIR:${jcdp_lib_name}_wasm>/${jcdp_lib_name}.d.ts")
   list(APPEND jcdp_libs ${jcdp_lib_name}_wasm)

   # Production Target (Web/Worker only)
   add_executable(${jcdp_lib_name}_wasm_prod ${jcdp_lib_files})
   set_target_properties(${jcdp_lib_name}_wasm_prod PROPERTIES
      OUTPUT_NAME "${jcdp_lib_name}_prod")
   set_wasm_options(${jcdp_lib_name}_wasm_prod)
   target_link_options(${jcdp_lib_name}_wasm_prod PRIVATE
      "SHELL:-s ENVIRONMENT='web,worker'")
   list(APPEND jcdp_libs ${jcdp_lib_name}_wasm_prod)

   # Custom target to create the NPM package
   set(NPM_PACKAGE_DIR "${CMAKE_BINARY_DIR}/package")
   configure_file(
      "${CMAKE_SOURCE_DIR}/additionals/package/package.json.in"
      "${NPM_PACKAGE_DIR}/package.json"
      @ONLY
   )

   add_custom_target(create_npm_package ALL
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
      COMMAND ${CMAKE_COMMAND} -E copy
         "$<TARGET_FILE_DIR:${jcdp_lib_name}_wasm>/${jcdp_lib_name}.d.ts"
         "${NPM_PACKAGE_DIR}/lib/"
      COMMENT "Copying WASM artifacts to NPM package"
   )

   find_program(NPM npm)
   if(NPM)
      add_custom_target(build_npm_package ALL
         DEPENDS create_npm_package
         WORKING_DIRECTORY ${NPM_PACKAGE_DIR}
         COMMAND ${NPM} install
         COMMAND ${NPM} run format
         COMMAND ${NPM} run build
         COMMENT "Building NPM package"
      )

      add_custom_target(publish_npm_package
         DEPENDS build_npm_package ${jcdp_lib_name}_wasm_prod
         WORKING_DIRECTORY ${NPM_PACKAGE_DIR}
         COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE_DIR:${jcdp_lib_name}_wasm_prod>/${jcdp_lib_name}_prod.js"
            "${NPM_PACKAGE_DIR}/lib/${jcdp_lib_name}.js"
         COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE_DIR:${jcdp_lib_name}_wasm_prod>/${jcdp_lib_name}_prod.wasm"
            "${NPM_PACKAGE_DIR}/lib/${jcdp_lib_name}.wasm"
         COMMAND sed -i "s/${jcdp_lib_name}_prod.wasm/${jcdp_lib_name}.wasm/g" "${NPM_PACKAGE_DIR}/lib/${jcdp_lib_name}.js"
         COMMAND sed -i "s/${jcdp_lib_name}_prod.js/${jcdp_lib_name}.js/g" "${NPM_PACKAGE_DIR}/lib/${jcdp_lib_name}.js"
         COMMAND npm run format
         COMMAND ${NPM} publish
         COMMENT "Publishing NPM package"
      )

      add_test(NAME test_npm_package_sync
         COMMAND ${NPM} run test:sync
         WORKING_DIRECTORY ${NPM_PACKAGE_DIR}
      )

      add_test(NAME test_npm_package_worker
         COMMAND ${NPM} run test:worker
         WORKING_DIRECTORY ${NPM_PACKAGE_DIR}
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
   set(dts_file "$<TARGET_FILE_DIR:${jcdp_lib_name}_wasm>/${jcdp_lib_name}.d.ts")
   install(FILES ${wasm_file} DESTINATION lib)
   install(FILES ${dts_file} DESTINATION lib)
endif()
