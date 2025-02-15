# Collect local headers
set(_local_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/properties.hpp)
  #${CMAKE_CURRENT_SOURCE_DIR}/properties.inl)

# Setup header-only IWYU target
header_only_iwyu_targets("jcdp_util"
  HEADERS ${_local_headers}
  COMPILER_FLAGS ${JCDP_IWYU_COMPILER_FLAGS}
  INCLUDE_DIRS ${JCDP_include_dirs}
  IWYU_FLAGS ${JCDP_IWYU_FLAGS})

# Setup header-only Cpplint targets
header_only_cpplint_targets("jcdp_util"
  HEADERS ${_local_headers})

# **************************************************************************** #
# Cleanup
# **************************************************************************** #
unset(_local_headers)
