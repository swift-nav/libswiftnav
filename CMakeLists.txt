cmake_minimum_required(VERSION 3.0)
project(libswiftnav)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/" "${CMAKE_CURRENT_LIST_DIR}/cmake/common")

set(MAX_CHANNELS "63" CACHE STRING "Maximum number of concurrent GNSS channels to support.")
configure_file(src/max_channels.h.in max_channels.h)

include(GNUInstallDirs)
include(CCache)
include(SwiftCmakeOptions)
include(TestTargets)
include(LanguageStandards)
include(CompileOptions)

set(disable_tests FALSE)
if(WIN32)
  message(STATUS "MSVC detected, skipping unit tests")
  set(disable_tests TRUE)
endif()

swift_create_project_options(
    HAS_TESTS
    HAS_TEST_LIBS
    DISABLE_TEST_COMPONENTS ${disable_tests}
    TEST_PACKAGES "Check"
    )

include(ClangFormat)
swift_setup_clang_format()
include(ClangTidy)
swift_setup_clang_tidy(PATTERNS "src/*.c")
include(CodeCoverage)
add_code_coverage_all_targets()

option(LIBSWIFTNAV_ENABLE_STDERR_LOGGING "Enable logging to stderr by default" ON)

set(HDRS
    include/swiftnav/almanac.h
    include/swiftnav/array_tools.h
    include/swiftnav/bits.h
    include/swiftnav/ch_meas.h
    include/swiftnav/common.h
    include/swiftnav/constants.h
    include/swiftnav/coord_system.h
    include/swiftnav/correct_iono_tropo.h
    include/swiftnav/decode_glo.h
    include/swiftnav/edc.h
    include/swiftnav/ephemeris.h
    include/swiftnav/fifo_byte.h
    include/swiftnav/geoid_model.h
    include/swiftnav/glo_map.h
    include/swiftnav/glonass_phase_biases.h
    include/swiftnav/gnss_time.h
    include/swiftnav/ionosphere.h
    include/swiftnav/linear_algebra.h
    include/swiftnav/logging.h
    include/swiftnav/macro_overload.h
    include/swiftnav/memcpy_s.h
    include/swiftnav/nav_meas.h
    include/swiftnav/pvt_result.h
    include/swiftnav/sbas_raw_data.h
    include/swiftnav/set.h
    include/swiftnav/shm.h
    include/swiftnav/sid_set.h
    include/swiftnav/signal.h
    include/swiftnav/single_epoch_solver.h
    include/swiftnav/troposphere.h)

set(SRCS
    src/almanac.c
    src/bits.c
    src/coord_system.c
    src/correct_iono_tropo.c
    src/decode_glo.c
    src/edc.c
    src/ephemeris.c
    src/fifo_byte.c
    src/geoid_model.c
    src/glo_map.c
    src/glonass_phase_biases.c
    src/gnss_time.c
    src/ionosphere.c
    src/linear_algebra.c
    src/logging_common.c
    src/logging.c
    src/memcpy_s.c
    src/nav_meas.c
    src/set.c
    src/shm.c
    src/sid_set.c
    src/signal.c
    src/single_epoch_solver.c
    src/troposphere.c)

add_library(swiftnav ${HDRS} ${SRCS})
swift_set_language_standards(swiftnav)
swift_set_compile_options(swiftnav ADD -pedantic REMOVE -Wconversion -Wstack-protector)

target_code_coverage(swiftnav AUTO ALL)
target_include_directories(swiftnav PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(swiftnav PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(LIBSWIFTNAV_ENABLE_STDERR_LOGGING)
  target_compile_definitions(swiftnav PRIVATE "LIBSWIFTNAV_ENABLE_STDERR_LOGGING")
endif()

target_compile_options(swiftnav PRIVATE "-UNDEBUG")

target_code_coverage(swiftnav AUTO ALL)
target_include_directories(swiftnav PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(swiftnav PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

if (MSVC)
else ()
  target_link_libraries(swiftnav PRIVATE m)
endif()

install(FILES ${HDRS} DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/swiftnav)
install(TARGETS swiftnav DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR})

# unit tests
if(libswiftnav_BUILD_TESTS OR libswiftnav_BUILD_TEST_LIBS)
  add_subdirectory(tests)
endif()
