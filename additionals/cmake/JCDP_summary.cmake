# Print build type again
print_build_type()

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
