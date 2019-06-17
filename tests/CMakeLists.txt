if(libswiftnav_BUILD_TEST_LIBS)
  add_subdirectory(common)
endif()

if(libswiftnav_BUILD_TESTS)
  set(HDRS
      check_suites.h)

  set(SRCS
      check_almanac.c
      check_bits.c
      check_coord_system.c
      check_edc.c
      check_ephemeris.c
      check_glo_map.c
      check_gnss_time.c
      check_ionosphere.c
      check_linear_algebra.c
      check_main.c
      check_nav_meas.c
      check_set.c
      check_shm.c
      check_sid_set.c
      check_signal.c
      check_pvt.c
      check_troposphere.c)

  add_executable(test-swiftnav-common ${HDRS} ${SRCS})
  target_link_libraries(test-swiftnav-common PRIVATE check-utils
                        ${CHECK_LIBRARIES} pthread)
  target_code_coverage(test-swiftnav-common AUTO ALL)

  set_property(TARGET test-swiftnav-common PROPERTY C_STANDARD 99)

  # Add a target for running the tests.
  add_custom_target(run-test-swiftnav-common COMMAND test-swiftnav-common)
  # And run them upon completion.
  add_custom_command(TARGET test-swiftnav-common POST_BUILD COMMAND test-swiftnav-common)
endif(libswiftnav_BUILD_TESTS)
