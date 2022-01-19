swift_add_test_library(check-utils SOURCES check_utils.c)
target_link_libraries(check-utils PUBLIC swiftnav::swiftnav)