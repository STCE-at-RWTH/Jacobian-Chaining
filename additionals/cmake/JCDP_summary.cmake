print_build_type()
print_iwyu_status()
print_cpplint_status()

if(NOT WIN32)
  print_stdlib_status()
endif()

print_sanitation_status()
print_fpic_status()
print_intrinsics_status()

# OpenMP
if(JCDP_USE_OPENMP)
  print_status("JCDP OpenMP support: ON")
else()
  print_status("JCDP OpenMP support: OFF")
endif()

# Doxygen
if(JCDP_BUILD_DOXYGEN)
  print_status("JCDP build doxygen: ON")
else()
  print_status("JCDP build doxygen: OFF")
endif()
