cmake_minimum_required(VERSION 3.13)
include_guard(GLOBAL)

include(compiler_flags)

# Custom OpenMP runtime library and include directory
option(JCDP_BUILD_OPENMP_RUNTIME "Fetch and build OpenMP from source." OFF)
set(JCDP_OPENMP_RUNTIME "" CACHE PATH "Custom OpenMP runtime library.")
set(JCDP_OPENMP_INCLUDE_DIR "" CACHE PATH "Custom OpenMP include directory.")

if(JCDP_USE_OPENMP)
  if(JCDP_BUILD_OPENMP_RUNTIME)
    set(OPENMP_DOWNLOAD_URL "https://github.com/llvm/llvm-project/releases/download/llvmorg")
    set(OPENMP_VERSION_TAG "21.1.2" CACHE STRING "The OpenMP version which is fetched from github.")

    # Necessary packages
    include(FetchContent)
    find_package(Git QUIET)

    # Declare llvm cmake module
    FetchContent_Declare(
      llvm_cmake
      URL ${OPENMP_DOWNLOAD_URL}-${OPENMP_VERSION_TAG}/cmake-${OPENMP_VERSION_TAG}.src.tar.xz
      URL_HASH SHA256=9ccbaf5ed6bb9e0bcedd827a433fb8f73878b64556bbc1da1e17d88ec0bde0cc
      DOWNLOAD_EXTRACT_TIMESTAMP ON
      SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/llvm-cmake-${OPENMP_VERSION_TAG})

    # Fetch llvm_cmake
    FetchContent_GetProperties(llvm_cmake)
    if(NOT llvm_cmake_POPULATED)
      cmake_policy(PUSH)
      if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
        cmake_policy(SET CMP0169 OLD)
      endif()
      FetchContent_Populate(llvm_cmake)
      cmake_policy(POP)
    endif()

    set(CMAKE_MODULE_PATH ${llvm_cmake_SOURCE_DIR}/Modules ${CMAKE_MODULE_PATH})

    # OpenMP build settings
    set(OPENMP_STANDALONE_BUILD ON CACHE BOOL "" FORCE)
    mark_as_advanced(OPENMP_STANDALONE_BUILD)
    set(OPENMP_ENABLE_OMPT_TOOLS OFF CACHE BOOL "" FORCE)
    mark_as_advanced(OPENMP_ENABLE_OMPT_TOOLS)
    set(OPENMP_ENABLE_LIBOMPTARGET OFF CACHE BOOL "" FORCE)
    mark_as_advanced(OPENMP_ENABLE_LIBOMPTARGET)
    set(LIBOMP_HAVE_OMPT_SUPPORT OFF CACHE BOOL "" FORCE)
    mark_as_advanced(LIBOMP_HAVE_OMPT_SUPPORT)
    set(LIBOMP_OMPD_SUPPORT OFF CACHE BOOL "" FORCE)
    mark_as_advanced(LIBOMP_OMPD_SUPPORT)
    set(LIBOMP_USE_DEBUGGER OFF CACHE BOOL "" FORCE)
    mark_as_advanced(LIBOMP_USE_DEBUGGER)
    set(LIBOMP_FORTRAN_MODULES OFF CACHE BOOL "" FORCE)
    mark_as_advanced(LIBOMP_FORTRAN_MODULES)
    set(LIBOMP_ENABLE_SHARED OFF CACHE BOOL "" FORCE)
    mark_as_advanced(LIBOMP_ENABLE_SHARED)

    # Declare openmp
    FetchContent_Declare(
      openmp
      URL ${OPENMP_DOWNLOAD_URL}-${OPENMP_VERSION_TAG}/openmp-${OPENMP_VERSION_TAG}.src.tar.xz
      URL_HASH SHA256=f60455a1e2e127df18f5f1302f0555eab9aecd37f657904a87b2d601178d4135
      DOWNLOAD_EXTRACT_TIMESTAMP ON
      SOURCE_DIR ${FETCHCONTENT_BASE_DIR}/openmp-${OPENMP_VERSION_TAG})

    # Fetch openmp
    FetchContent_GetProperties(openmp)
    if(NOT openmp_POPULATED)
      cmake_policy(PUSH)
      if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
        cmake_policy(SET CMP0169 OLD)
      endif()
      FetchContent_Populate(openmp)
      cmake_policy(POP)

      print_subtitle("Configuring OpenMP" EMPTY_BEFORE)
      add_subdirectory(${openmp_SOURCE_DIR} ${openmp_BINARY_DIR} EXCLUDE_FROM_ALL)
      print_divide(EMPTY_AFTER)
    endif()

    # Need explicit -pthread flag to enable threading support in Emscripten
    if(EMSCRIPTEN)
      target_compile_options(omp PRIVATE -pthread)
      target_link_options(omp PRIVATE -pthread)
    endif()
  else()
    # Check if the include directory exists if it is defined
    if(JCDP_OPENMP_INCLUDE_DIR)
      if(NOT EXISTS ${JCDP_OPENMP_INCLUDE_DIR})
        print_error(
          "Custom OpenMP include directory '${JCDP_OPENMP_INCLUDE_DIR}' not found")
      endif()

      set(CMAKE_INCLUDE_PATH ${JCDP_OPENMP_INCLUDE_DIR} ${CMAKE_INCLUDE_PATH})
    endif()

    # Check if the OpenMP runtime library exists if it is defined
    if(JCDP_OPENMP_RUNTIME)
      if(NOT EXISTS ${JCDP_OPENMP_RUNTIME})
        print_error(
          "Custom OpenMP runtime library '${JCDP_OPENMP_RUNTIME}' not found")
      endif()

      get_filename_component(_omp_lib_dir ${JCDP_OPENMP_RUNTIME} DIRECTORY)
      get_filename_component(_omp_lib ${JCDP_OPENMP_RUNTIME} NAME_WE)
      string(REGEX REPLACE "^lib" "" _omp_lib ${_omp_lib})

      # AppleClang doesn't come with an OpenMP header so use the custom one
      if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(CMAKE_LIBRARY_PATH ${_omp_lib_dir} ${CMAKE_LIBRARY_PATH})
      endif()
    endif()

    # Use llvm libomp with msvc for OpenMP tasks
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
      set(OpenMP_RUNTIME_MSVC llvm)
    endif()
  endif()

  # Find OpenMP to get the compile flags
  if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(OpenMP_CXX_FLAGS "-fopenmp" CACHE STRING "OpenMP compile flags." FORCE)
  else()
    find_package(OpenMP COMPONENTS CXX REQUIRED)
  endif()

  # Override OpenMP_CXX_LIBRARIES, OpenMP_CXX_INCLUDE_DIR and OpenMP_CXX_LIBRARY_DIR
  if(JCDP_BUILD_OPENMP_RUNTIME)
    set(OpenMP_CXX_INCLUDE_DIR ${openmp_BINARY_DIR}/runtime/src)
    set(OpenMP_CXX_LIBRARIES omp)
    set(OpenMP_CXX_LIBRARY_DIR ${openmp_BINARY_DIR}/runtime/src)
  elseif(JCDP_OPENMP_RUNTIME)
    set(OpenMP_CXX_LIBRARIES ${_omp_lib})
    set(OpenMP_CXX_LIBRARY_DIR ${_omp_lib_dir})
  endif()

  # Separate multiple flags
  string(REPLACE " " ";" OpenMP_CXX_FLAGS "${OpenMP_CXX_FLAGS}")
endif()

# **************************************************************************** #
# Cleanup
# **************************************************************************** #
if(JCDP_OPENMP_RUNTIME)
  unset(_omp_lib_dir)
  unset(_omp_lib)
endif()

# **************************************************************************** #
# Helper function to apply the OpenMP flags to a target
# **************************************************************************** #
function(jcdp_compile_with_openmp visibility targets)
  set(targets ${targets} ${ARGN})

  # Check visibility
  if(NOT ${visibility} STREQUAL "PRIVATE" AND
     NOT ${visibility} STREQUAL "PUBLIC" AND
     NOT ${visibility} STREQUAL "INTERFACE" AND
     NOT ${visibility} STREQUAL "NONE")
    print_error(
      "First argument must be either "
      "'PRIVATE', 'PUBLIC', 'INTERFACE' or 'NONE'.")
  endif()

  if(${visibility} STREQUAL "NONE")
    set(visibility "")
  endif()

  foreach(tgt ${targets})
    if(NOT TARGET ${tgt})
      print_error("OpenMP compile flags: Target ${tgt} doesn't exist.")
    endif()

    if(OpenMP_CXX_INCLUDE_DIR)
      target_include_directories(
        ${tgt} SYSTEM ${visibility} ${OpenMP_CXX_INCLUDE_DIR})
    endif()

    if(JCDP_USE_OPENMP)
      target_compile_options(${tgt} ${visibility} ${OpenMP_CXX_FLAGS})

      # Need explicit -pthread flag to enable threading support in Emscripten
      if(EMSCRIPTEN)
        target_compile_options(${tgt} ${visibility} -pthread)
      endif()
    else()
      add_cxx_flag("-Wno-unknown-pragmas" WNO_UNKNOWN_PRAGMAS ${tgt})
    endif()
  endforeach()
endfunction()

# **************************************************************************** #
# Helper function to link the OpenMP runtime against a target
# **************************************************************************** #
function(jcdp_link_openmp_runtime visibility targets)
  set(targets ${targets} ${ARGN})

  # Check visibility
  if(NOT ${visibility} STREQUAL "PRIVATE" AND
     NOT ${visibility} STREQUAL "PUBLIC" AND
     NOT ${visibility} STREQUAL "INTERFACE" AND
     NOT ${visibility} STREQUAL "NONE")
    print_error(
      "First argument must be either "
      "'PRIVATE', 'PUBLIC', 'INTERFACE' or 'NONE'.")
  endif()

  if(${visibility} STREQUAL "NONE")
    set(visibility "")
  endif()

  foreach(tgt ${targets})
    if(NOT TARGET ${tgt})
      print_error("OpenMP runtime linking: Target ${tgt} doesn't exist.")
    endif()

    if(JCDP_USE_OPENMP)
      target_link_libraries(${tgt} ${visibility} ${OpenMP_CXX_LIBRARIES})

      # Need explicit -pthread flag to enable threading support in Emscripten
      if(EMSCRIPTEN)
        target_link_options(${tgt} ${visibility} -pthread)
      endif()
      if(OpenMP_CXX_LIBRARY_DIR)
        target_link_directories(${tgt} PRIVATE ${OpenMP_CXX_LIBRARY_DIR})
      endif()
    endif()
  endforeach()
endfunction()

# **************************************************************************** #
# Helper function to print the OpenMP configuration
# **************************************************************************** #
function(print_openmp_status)
  if(JCDP_USE_OPENMP)
    if(JCDP_BUILD_OPENMP_RUNTIME)
      print_status("JCDP OpenMP support: ON (built from source)")
    elseif(JCDP_OPENMP_RUNTIME)
      print_status("JCDP OpenMP support: ON (custom runtime)")
    else()
      print_status("JCDP OpenMP support: ON")
    endif()
  else()
    print_status("JCDP OpenMP support: OFF")
  endif()
endfunction()