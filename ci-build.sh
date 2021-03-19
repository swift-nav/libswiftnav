#!/bin/bash

# Copyright (C) 2018-2021 Swift Navigation Inc.
# Contact: Swift Navigation <dev@swiftnav.com>

set -ex
set -o errexit
set -o pipefail

#************************************************************************
# UTILITY FUNCTIONS
#************************************************************************
check_format_errors() {
  if [[ $(git --no-pager diff --name-only HEAD) ]]; then
    echo "######################################################"
    echo "####### clang-format warning found! Exiting... #######"
    echo "######################################################"
    echo ""
    echo "This should be formatted locally and pushed again..."
    git --no-pager diff
    exit 1
  fi
}

check_tidy_errors() {
  if [ -e ../fixes.yaml ]; then
    echo "####################################################"
    echo "####### clang-tidy warning found! Exiting... #######"
    echo "####################################################"
    echo ""
    echo " ^^ Please see and correct the clang-tidy warnings found above ^^"
    exit 1
  fi
}

function build() {
  # Create and enter build directory.
  mkdir -p build && cd build
  cmake ../
  make -j4 VERBOSE=1
  if [ "$TEST_SUITE" == "lint" ]; then
    make clang-format-all && check_format_errors
    make clang-tidy-all && check_tidy_errors
  fi
  cd ../
}

build
