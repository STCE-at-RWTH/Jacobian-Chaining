# Collect local headers
set(_local_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/generator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/graphml.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/jacobian_chain.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/jacobian.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/operation.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/sequence.hpp)

# Setup header-only IWYU target
header_only_iwyu_targets("iwyu_include_jcdp"
  HEADERS ${_local_headers}
  COMPILER_FLAGS ${JCDP_IWYU_FLAGS}
  INCLUDE_DIRS ${JCDP_include_dirs}
  IWYU_FLAGS ${ADBLAS_IWYU_FLAGS})

# Setup header-only Cpplint targets
header_only_cpplint_targets("cpplint_include_jcdp"
  HEADERS ${_local_headers})

# **************************************************************************** #
# Cleanup and subdirectories
# **************************************************************************** #
unset(_local_headers)

add_subdirectory(optimizer)
add_subdirectory(scheduler)
add_subdirectory(util)
