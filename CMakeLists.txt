cmake_minimum_required(VERSION 3.0)
project(libswiftnav)

# Include any extra CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
include(ClangTools)
include(FindCheck)
# TODO: figure out why we are missing a defintion of log_ in the .a
set(BUILD_SHARED_LIBS ON CACHE BOOL "If set, build shared libraries where possible.")

# Write configuration to a templated header file.
set(MAX_CHANNELS "63" CACHE STRING "Maximum number of concurrent GNSS channels to support.")
configure_file("include/swiftnav/config.h.in"
  "${PROJECT_SOURCE_DIR}/include/swiftnav/config.h")

set(HDRS
    include/swiftnav/almanac.h
    include/swiftnav/bits.h
    include/swiftnav/common.h
    include/swiftnav/config.h
    include/swiftnav/constants.h
    include/swiftnav/coord_system.h
    include/swiftnav/correct_iono_tropo.h
    include/swiftnav/edc.h
    include/swiftnav/ephemeris.h
    include/swiftnav/glo_map.h
    include/swiftnav/glonass_phase_biases.h
    include/swiftnav/gnss_time.h
    include/swiftnav/ionosphere.h
    include/swiftnav/linear_algebra.h
    include/swiftnav/logging.h
    include/swiftnav/memcpy_s.h
    include/swiftnav/nav_meas.h
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
    src/edc.c
    src/ephemeris.c
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
target_include_directories(swiftnav PUBLIC ${PROJECT_SOURCE_DIR}/include)
install(FILES ${HDRS} DESTINATION include)

# Set any mandatory compiler flags for this library.
target_compile_options(swiftnav PRIVATE "-Werror")
target_compile_options(swiftnav PRIVATE "-Wmissing-prototypes")
target_compile_options(swiftnav PRIVATE "-Wimplicit")
target_compile_options(swiftnav PRIVATE "-Wshadow")
target_compile_options(swiftnav PRIVATE "-Wswitch-default")
target_compile_options(swiftnav PRIVATE "-Wswitch-enum")
target_compile_options(swiftnav PRIVATE "-Wundef")
target_compile_options(swiftnav PRIVATE "-Wuninitialized")
target_compile_options(swiftnav PRIVATE "-Wpointer-arith")
target_compile_options(swiftnav PRIVATE "-Wstrict-prototypes")
target_compile_options(swiftnav PRIVATE "-Wcast-align")
target_compile_options(swiftnav PRIVATE "-Wformat=2")
target_compile_options(swiftnav PRIVATE "-Wimplicit-function-declaration")
target_compile_options(swiftnav PRIVATE "-Wredundant-decls")
target_compile_options(swiftnav PRIVATE "-Wformat-security")

# unit tests
if(NOT CMAKE_CROSSCOMPILING AND NOT PIKSI_MULTI_UNIT_TEST)
    add_subdirectory(tests)
endif(NOT CMAKE_CROSSCOMPILING AND NOT PIKSI_MULTI_UNIT_TEST)
