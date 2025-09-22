add_custom_target(case_studies.pde ALL)
## Sets compile option for this directory and all subdirectories.
nag_add_cxx_definition(DCO_STD_COMPATIBILITY)

set(SUBDIRS a1s a1s_checkpointing cfd primal socfd t1s t2s_a1s t2s_a1s_checkpointing t2s_t1s)
if(DCO_BUILD_CASE_STUDIES)
  foreach (cs ${SUBDIRS})
    DCO_ADD_CASE_STUDY("${CMAKE_CURRENT_SOURCE_DIR}/scenario_1.in" ${cs}/main.cpp)
  endforeach()
endif()
## installation
# used in configure below
JOIN("${SUBDIRS}" " " SUBDIRS_STRING)

set(DCO_EXE_PARAMETERS ../scenario_1.in)
foreach(subsubdir ${SUBDIRS})
  # install ref_results file
  FIND_REF_RESULT_FILE("${subsubdir}/")
  install(
    FILES ${REF_RES_FILE}
    DESTINATION case_studies/pde/${subsubdir}
    RENAME ref_results
    )
  if (NOT(FILE_FOUND))
    MESSAGE(FATAL_ERROR "No ref_results file found for pde/${subsubdir}")
  endif()
endforeach()

# copy example source and header files
install_with_copyright(DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR} DESTINATION case_studies TARGET case_studies.pde)
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  DESTINATION case_studies
  FILES_MATCHING 
  PATTERN "*.in"
  PATTERN "approx_results"
  PATTERN "README"
  )

configure_file(
  ${PROJECT_SOURCE_DIR}/additionals/template_files/CMakeLists.txt.case_studies.pde.in
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt
  @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeLists.txt
  DESTINATION case_studies/pde)

