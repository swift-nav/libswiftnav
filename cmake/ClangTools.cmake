# CMake script searches for clang-tidy and clang-format and sets the following
# variables:
#
# CLANG_FORMAT_PATH  : Fully-qualified path to the clang-format executable
#
# Additionally defines the following targets:
#
# clang-format-all   : Run clang-format over all files.
# clang-format-diff  : Run clang-format over all files differing from master.

# Do not use clang tooling when cross compiling.
if(CMAKE_CROSSCOMPILING)
    return()
endif(CMAKE_CROSSCOMPILING)

################################################################################
# Search for tools.
################################################################################

# Check for Clang format
set(CLANG_FORMAT_PATH "NOTSET" CACHE STRING "Absolute path to the clang-format executable")
if("${CLANG_FORMAT_PATH}" STREQUAL "NOTSET")
    find_program(CLANG_FORMAT NAMES
        clang-format40 clang-format-4.0
        clang-format39 clang-format-3.9
        clang-format38 clang-format-3.8
        clang-format37 clang-format-3.7
        clang-format36 clang-format-3.6
        clang-format35 clang-format-3.5
        clang-format34 clang-format-3.4
        clang-format)
    if("${CLANG_FORMAT}" STREQUAL "CLANG_FORMAT-NOTFOUND")
        message(WARNING "Could not find 'clang-format' please set CLANG_FORMAT_PATH:STRING")
    else()
        set(CLANG_FORMAT_PATH ${CLANG_FORMAT})
        message(STATUS "Found: ${CLANG_FORMAT_PATH}")
    endif()
else()
    if(NOT EXISTS ${CLANG_FORMAT_PATH})
        message(WARNING "Could not find 'clang-format': ${CLANG_FORMAT_PATH}")
    else()
        message(STATUS "Found: ${CLANG_FORMAT_PATH}")
    endif()
endif()

################################################################################
# Conditionally add targets.
################################################################################

if (EXISTS ${CLANG_FORMAT_PATH})
    # Format all files .c files (and their headers) in project
    add_custom_target(clang-format-all COMMAND clang-format -i ../src/*.c ../include/swiftnav/*.h ../tests/*.c ../tests/common/*.c ../tests/common/*.h)

    # Format all staged lines
    add_custom_target(clang-format-head COMMAND git-clang-format)

    # In-place format *.cc files that differ from master, and are not listed as
    # being DELETED.
    add_custom_target(clang-format-diff COMMAND git-clang-format master)
endif()
