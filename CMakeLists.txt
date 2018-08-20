cmake_minimum_required(VERSION 3.0)
project(libswiftnav)

# Include any extra CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ClangTools)
include(FindCheck)
# TODO: figure out why we are missing a defintion of log_ in the .a
set(BUILD_SHARED_LIBS ON CACHE BOOL "If set, build shared libraries where possible.")

# Set any mandatory compiler flags for this library.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch-default")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch-enum")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wuninitialized")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat=2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wimplicit-function-declaration")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wredundant-decls")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat-security")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U__STRICT_ANSI__")

# Write configuration to a templated header file.
set(MAX_CHANNELS "63" CACHE STRING "Maximum number of concurrent GNSS channels to support.")
configure_file("include/swiftnav/config.h.in"
  "${CMAKE_SOURCE_DIR}/include/swiftnav/config.h")

include_directories(${CMAKE_SOURCE_DIR}/include)

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
target_include_directories(swiftnav PUBLIC ${CMAKE_SOURCE_DIR}/include)
install(FILES ${HDRS} DESTINATION include)

# unit tests
if(NOT CMAKE_CROSSCOMPILING AND NOT PIKSI_MULTI_UNIT_TEST)
    add_subdirectory(tests)
endif(NOT CMAKE_CROSSCOMPILING AND NOT PIKSI_MULTI_UNIT_TEST)
