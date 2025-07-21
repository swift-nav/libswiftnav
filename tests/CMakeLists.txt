if(libswiftnav_BUILD_TESTS)
  swift_add_test(test-swiftnav-pedantic
    UNIT_TEST
    SRCS check_pedantic.cc
    LINK swiftnav::swiftnav
  )
  swift_set_compile_options(test-swiftnav-pedantic ADD -pedantic)


swift_add_test(test-swiftnav-common
  UNIT_TEST
  SRCS
    test_almanac.cc
    test_bits.cc
    test_coord_system.cc
    test_correct_iono_tropo.cc
    test_decode_glo.cc
    test_edc.cc
    test_ephemeris.cc
    test_geoid_model.cc
    test_glo_map.cc
    test_gnss_time.cc
    test_gnss_time_cpp.cc
    test_ionosphere.cc
    test_linear_algebra.cc
    test_log.cc
    test_nav_meas.cc
    test_pvt.cc
    test_set.cc
    test_shm.cc
    test_sid_set.cc
    test_signal.cc
    test_subsystem_status_report.cc
    test_troposphere.cc
  LINK
    swiftnav::swiftnav
    gtest
    gtest_main
)

endif(libswiftnav_BUILD_TESTS)
