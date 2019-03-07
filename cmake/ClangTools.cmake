# CMake script searches for clang-tidy and clang-format and sets the following
# variables:
#
# CLANG_FORMAT_PATH  : Fully-qualified path to the clang-format executable
#
# Additionally defines the following targets:
#
# clang-format-all   : Run clang-format over all files.
# clang-format-diff  : Run clang-format over all files differing from master.
# clang-tidy-all     : Run clang-tidy over all files.
# clang-tidy-diff    : Run clang-tidy over all files differing from master.

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
        clang-format-6.0)
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

# Check for Clang tidy
set(CLANG_TIDY_PATH "NOTSET" CACHE STRING "Absolute path to the clang-tidy executable")
if("${CLANG_TIDY_PATH}" STREQUAL "NOTSET")
    find_program(CLANG_TIDY NAMES
        clang-tidy-6.0)
    if("${CLANG_TIDY}" STREQUAL "CLANG_TIDY-NOTFOUND")
        message(WARNING "Could not find 'clang-tidy' please set CLANG_TIDY_PATH:STRING")
    else()
        set(CLANG_TIDY_PATH ${CLANG_TIDY})
        message(STATUS "Found: ${CLANG_TIDY_PATH}")
    endif()
else()
    if(NOT EXISTS ${CLANG_TIDY_PATH})
        message(WARNING "Could not find 'clang-tidy': ${CLANG_TIDY_PATH}")
    else()
        message(STATUS "Found: ${CLANG_TIDY_PATH}")
    endif()
endif()

################################################################################
# Conditionally add targets.
################################################################################

if (EXISTS ${CLANG_FORMAT_PATH})
  # Format all files .c files (and their headers) in project
  if (NOT TARGET clang-format-all)
    add_custom_target(clang-format-all COMMAND ${CLANG_FORMAT_PATH} -i ../src/*.c ../include/swiftnav/*.h ../tests/*.c ../tests/common/*.c ../tests/common/*.h)
  endif()
endif()

if (EXISTS ${CLANG_TIDY_PATH})
  if (NOT TARGET clang-tidy-all)
    # Tidy all .c files (and their headers) in project
    # Second stage of pipeline makes an absolute path for each file. Note that
    # git ls-files and diff-tree behave differently in prepending the file path.
    add_custom_target(clang-tidy-all
      COMMAND git ls-files -- '../src/*.[ch]'
      | sed 's/^...//' | sed 's\#\^\#${CMAKE_SOURCE_DIR}/\#'
      | xargs -P 2 -I file "${CLANG_TIDY_PATH}"
      -export-fixes="${CMAKE_SOURCE_DIR}/fixes.yaml" -quiet file --
      "-I${CMAKE_SOURCE_DIR}/include/" "-I${CMAKE_SOURCE_DIR}/build/"
      )
  endif()
endif()
