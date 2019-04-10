cmake_minimum_required(VERSION 3.0)

include("cmake/VersionFromGit.cmake")
version_from_git()
message(STATUS "Building libswiftnav version ${VERSION}")

project(libswiftnav LANGUAGES C CXX VERSION ${VERSION})

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(FindCheck)
include(GNUInstallDirs)

set(MAX_CHANNELS "63" CACHE STRING "Maximum number of concurrent GNSS channels to support.")
configure_file(src/max_channels.h.in max_channels.h)

set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(include_install_dir "${CMAKE_INSTALL_INCLUDEDIR}")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets.cmake")
set(namespace "${PROJECT_NAME}::")

set(SRCS
    src/almanac.c
    src/bits.c
    src/coord_system.c
    src/correct_iono_tropo.c
    src/edc.c
    src/ephemeris.c
    src/fifo_byte.c
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

add_library(swiftnav ${SRCS})
add_library(${namespace}swiftnav ALIAS swiftnav)

target_include_directories(swiftnav
    PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

set_property(TARGET swiftnav PROPERTY C_STANDARD 99)

if (MSVC)
else ()
    target_link_libraries(swiftnav m)
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
    # require at least gcc 5.0
    if (CMAKE_C_COMPILER_VERSION VERSION_GREATER 5.0)
        target_compile_options(swiftnav PRIVATE "-Wfloat-conversion")
    endif()
endif()

# unit tests
if(NOT CMAKE_CROSSCOMPILING AND NOT PIKSI_MULTI_UNIT_TEST)
    add_subdirectory(tests)
endif(NOT CMAKE_CROSSCOMPILING AND NOT PIKSI_MULTI_UNIT_TEST)

include(CMakePackageConfigHelpers)

write_basic_package_version_file("${version_config}" COMPATIBILITY SameMajorVersion)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
  )

install(
    TARGETS swiftnav
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${include_install_dir}"
  )

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/libswiftnav"
    DESTINATION "${include_install_dir}"
  )

install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
  )

install(
    EXPORT "${TARGETS_EXPORT_NAME}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
  )
