# Collect local headers
set(_local_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/properties.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/properties.inl)

# Setup header-only IWYU target
header_only_iwyu_targets("iwyu_include_jcdp_util"
  HEADERS ${_local_headers}
  COMPILER_FLAGS ${JCDP_IWYU_FLAGS}
  INCLUDE_DIRS ${JCDP_include_dirs}
  IWYU_FLAGS ${ADBLAS_IWYU_FLAGS})

# Setup header-only Cpplint targets
header_only_cpplint_targets("cpplint_include_jcdp_util"
  HEADERS ${_local_headers})

# **************************************************************************** #
# Cleanup
# **************************************************************************** #
unset(_local_headers)
