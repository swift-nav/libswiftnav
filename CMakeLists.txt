cmake_minimum_required(VERSION 3.0)
project(swiftnav-common)

################################################################################
# Include any extra CMake modules.
################################################################################
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../cmake)

################################################################################
# Set any mandatory compiler flags for this library. 
################################################################################
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

# TODO remove CXX flags if this library is pure C.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-default")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-enum")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wuninitialized")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wredundant-decls")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-security")

################################################################################
# Enumerate source files.
################################################################################
include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}/../../include/libswiftnav)

set(HDRS
    libswiftnav/almanac.h
    libswiftnav/bits.h
    libswiftnav/common.h
    libswiftnav/constants.h
    libswiftnav/coord_system.h
    libswiftnav/correct_iono_tropo.h
    libswiftnav/edc.h
    libswiftnav/ephemeris.h
    libswiftnav/glo_map.h
    libswiftnav/glonass_phase_biases.h
    libswiftnav/gnss_time.h
    libswiftnav/ionosphere.h
    libswiftnav/linear_algebra.h
    libswiftnav/logging.h
    libswiftnav/memcpy_s.h
    libswiftnav/nav_meas.h
    libswiftnav/set.h
    libswiftnav/shm.h
    libswiftnav/sid_set.h
    libswiftnav/signal.h
    libswiftnav/single_epoch_solver.h
    libswiftnav/troposphere.h)

set(SRCS
    libswiftnav/almanac.c
    libswiftnav/bits.c
    libswiftnav/coord_system.c
    libswiftnav/correct_iono_tropo.c
    libswiftnav/edc.c
    libswiftnav/ephemeris.c
    libswiftnav/glo_map.c
    libswiftnav/glonass_phase_biases.c
    libswiftnav/gnss_time.c
    libswiftnav/ionosphere.c
    libswiftnav/linear_algebra.c
    libswiftnav/logging_common.c
    libswiftnav/memcpy_s.c
    libswiftnav/nav_meas.c
    libswiftnav/set.c
    libswiftnav/shm.c
    libswiftnav/sid_set.c
    libswiftnav/signal.c
    libswiftnav/single_epoch_solver.c    
    libswiftnav/troposphere.c)

################################################################################
# Enumerate targets.
################################################################################
add_library(swiftnav-common ${HDRS} ${SRCS})
target_include_directories(swiftnav-common PUBLIC ${CMAKE_CURRENT_LIST_DIR})
# TODO Remove this once config.h is sorted out.
target_include_directories(swiftnav-common PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../include/libswiftnav)

################################################################################
# Include tests.
################################################################################
if(NOT CMAKE_CROSSCOMPILING AND NOT PIKSI_MULTI_UNIT_TEST)
    add_subdirectory(tests)
endif(NOT CMAKE_CROSSCOMPILING AND NOT PIKSI_MULTI_UNIT_TEST)

