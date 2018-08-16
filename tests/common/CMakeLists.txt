include_directories("${CMAKE_CURRENT_LIST_DIR}")
add_library(check-utils check_utils.h check_utils.c)
target_include_directories(check-utils PUBLIC "${CMAKE_CURRENT_LIST_DIR}")


