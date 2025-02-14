# Collect local headers
set(_local_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/branch_and_bound.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/dynamic_programming.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/optimizer.hpp)

# Setup header-only IWYU target
header_only_iwyu_targets("iwyu_include_jcdp_optimizer"
  HEADERS ${_local_headers}
  COMPILER_FLAGS ${JCDP_IWYU_FLAGS}
  INCLUDE_DIRS ${JCDP_include_dirs}
  IWYU_FLAGS ${ADBLAS_IWYU_FLAGS})

# Setup header-only Cpplint targets
header_only_cpplint_targets("cpplint_include_jcdp_optimizer"
  HEADERS ${_local_headers})

# **************************************************************************** #
# Cleanup
# **************************************************************************** #
unset(_local_headers)
