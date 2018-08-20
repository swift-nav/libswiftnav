#!/bin/bash

# Copyright (C) 2018 Swift Navigation Inc.
# Contact: Swift Navigation <dev@swiftnav.com>

# Run Travis setup

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
    travis_terminate 1
  fi
}

check_tidy_errors() {
  if [ -e ../fixes.yaml ]; then
    echo "####################################################"
    echo "####### clang-tidy warning found! Exiting... #######"
    echo "####################################################"
    echo ""
    echo " ^^ Please see and correct the clang-tidy warnings found above ^^"
    travis_terminate 1
  fi
}

function build() {
  # Create and enter build directory.
  mkdir -p build && cd build
  cmake ../
  make -j4 VERBOSE=1
  make clang-format-all && check_format_errors
  make clang-tidy-all && check_tidy_errors
  cd ../
}

if [ "$TESTENV" == "lint" ]; then
  ./travis-clang-format-check.sh
else
  build
fi
