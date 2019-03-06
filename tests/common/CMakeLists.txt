add_library(check-utils check_utils.c)
target_link_libraries(check-utils swiftnav)
set_property(TARGET check-utils PROPERTY C_STANDARD 99)
