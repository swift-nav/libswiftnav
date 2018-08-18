################################################################################
# Find dependencies.
################################################################################
find_package(Check)
if(NOT CHECK_FOUND)
    message(STATUS "'Check' unit test library not found. Skipping unit tests.")
    return()
endif(NOT CHECK_FOUND)

################################################################################
# Modify Compiler Flags.
################################################################################
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS}   -Wno-missing-prototypes")
set(CMAKE_CXXFLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-prototypes")

################################################################################
# Get Subdirectory Targets.
################################################################################
# This gets us the check-utils target.
add_subdirectory(common)

################################################################################
# Enumerate source files.
################################################################################

# Some of the tests directly include the source code.
include_directories("${PROJECT_SOURCE_DIR}/libswiftnav")

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

################################################################################
# Enumerate targets and tests.
################################################################################
add_executable(test-swiftnav-common ${HDRS} ${SRCS})
target_link_libraries(test-swiftnav-common swiftnav check-utils 
                      ${CHECK_LIBRARIES} m pthread)

# Add a target for running the tests. 
add_custom_target(run-test-swiftnav-common COMMAND test-swiftnav-common)
# And run them upon completion.
add_custom_command(TARGET test-swiftnav-common POST_BUILD COMMAND test-swiftnav-common)

