add_library(check-utils check_utils.c)

# need to undef __STRICT_ANSI__ or we lose M_PI and the like with c99
target_compile_options(check-utils PRIVATE "-std=c99")
target_compile_options(check-utils PRIVATE "-U__STRICT_ANSI__")

# TODO: determine best practice here -
# not actually linking anything, just need include path from swiftnav
target_link_libraries(check-utils swiftnav)
