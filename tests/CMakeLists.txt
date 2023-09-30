if(libswiftnav_BUILD_TEST_LIBS)
  add_subdirectory(common)
endif()

if(libswiftnav_BUILD_TESTS)
  find_package(Threads)

  set(SRCS
      check_almanac.c
      check_bits.c
      check_coord_system.c
      check_decode_glo.c
      check_edc.c
      check_ephemeris.c
      check_geoid_model.cc
      check_glo_map.c
      check_gnss_time.c
      check_gnss_time_cpp.cc
      check_ionosphere.c
      check_linear_algebra.c
      check_log.c
      check_main.c
      check_nav_meas.c
      check_set.c
      check_shm.c
      check_sid_set.c
      check_signal.c
      check_subsystem_status_report.c
      check_pvt.c
      check_troposphere.c)

  swift_add_test(test-swiftnav-common
    UNIT_TEST
    POST_BUILD
    SRCS ${SRCS}
    LINK swiftnav::check-utils check Threads::Threads
    )
  target_include_directories(test-swiftnav-common PRIVATE ${PROJECT_SOURCE_DIR})

  swift_set_compile_options(
    test-swiftnav-common 
    REMOVE
      -Wconversion
      -Wstack-protector
      -Wfloat-equal
      -Wsign-compare
    ADD
      -Wno-format-extra-args
  )

  swift_add_test(test-swiftnav-pedantic
    UNIT_TEST
    SRCS check_pedantic.cc
    LINK swiftnav::swiftnav
  )
  swift_set_compile_options(test-swiftnav-pedantic ADD -pedantic)

endif(libswiftnav_BUILD_TESTS)
