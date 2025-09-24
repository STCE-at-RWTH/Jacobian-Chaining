print_status("System name: ${CMAKE_SYSTEM_NAME}")

print_build_type()
print_iwyu_status()
print_cpplint_status()

if(NOT WIN32)
  print_stdlib_status()
endif()

print_sanitation_status()
print_fpic_status()
print_intrinsics_status()
print_openmp_status()

# Doxygen
if(JCDP_BUILD_DOXYGEN)
  print_status("JCDP build doxygen: ON")
else()
  print_status("JCDP build doxygen: OFF")
endif()
