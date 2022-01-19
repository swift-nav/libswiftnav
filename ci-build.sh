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

function build_c() {
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

function build_codecov() {

    mkdir "${HOME}/.sonar"

    # download build-wrapper
    curl -sSLo "${HOME}/.sonar/build-wrapper-linux-x86.zip" https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip
    unzip -o "${HOME}/.sonar/build-wrapper-linux-x86.zip" -d "${HOME}/.sonar/"
    export PATH=${HOME}/.sonar/build-wrapper-linux-x86:${PATH}

    # configure
    cmake --version
    cmake \
      "-DCODE_COVERAGE=ON" \
      "-DCMAKE_BUILD_TYPE=Debug" \
      -S . -B ./build

    # build with wrapper
    build-wrapper-linux-x86-64 --out-dir ./bw-output cmake --build ./build --target ccov-all-export -j8

    if [[ -z "${SONAR_SCANNER_VERSION}" ]]; then
        echo "Error: SONAR_SCANNER_VERSION must be configured" >&2
        exit 1
    fi

    export SONAR_SCANNER_HOME="${HOME}/.sonar/sonar-scanner-${SONAR_SCANNER_VERSION}-linux"

    # download sonar-scanner
    curl -sSLo "${HOME}/.sonar/sonar-scanner.zip" \
      "https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-${SONAR_SCANNER_VERSION}-linux.zip"
    unzip -o "${HOME}/.sonar/sonar-scanner.zip" -d "${HOME}/.sonar/"
    export PATH=${SONAR_SCANNER_HOME}/bin:${PATH}
    export SONAR_SCANNER_OPTS="-server"

    # Run sonar scanner
    [[ -n "${SONAR_TOKEN:-}" ]] && SONAR_TOKEN_CMD_ARG="-Dsonar.login=${SONAR_TOKEN}"
    [[ -n "${SONAR_ORGANIZATION:-}" ]] && SONAR_ORGANIZATION_CMD_ARG="-Dsonar.organization=${SONAR_ORGANIZATION}"
    [[ -n "${SONAR_PROJECT_NAME:-}" ]] && SONAR_PROJECT_NAME_CMD_ARG="-Dsonar.projectName=${SONAR_PROJECT_NAME}"

    # TODO: setup sonar.projectVersion so that it actually does something useful
    #  see https://swift-nav.atlassian.net/browse/DEVINFRA-504
    SONAR_OTHER_ARGS="\
        -Dsonar.projectVersion=1.0 \
        -Dsonar.sources=. \
        -Dsonar.cfamily.build-wrapper-output=./bw-output \
        -Dsonar.cfamily.threads=1 \
        -Dsonar.cfamily.cache.enabled=false \
        -Dsonar.sourceEncoding=UTF-8"

    # shellcheck disable=SC2086
    sonar-scanner \
        "-Dsonar.cfamily.llvm-cov.reportPath=./build/ccov/coverage.txt" \
        "-Dsonar.host.url=${SONAR_HOST_URL}" \
        "-Dsonar.projectKey=${SONAR_PROJECT_KEY}" \
        ${SONAR_OTHER_ARGS} \
        "${SONAR_PROJECT_NAME_CMD_ARG}" \
        "${SONAR_TOKEN_CMD_ARG}" \
        "${SONAR_ORGANIZATION_CMD_ARG}"
}

if [ "$TESTENV" == "c" ]; then
    build_c
elif [ "$TESTENV" == "codecov" ]; then
    build_codecov
else
    echo "Unknown TESTENV value: $TESTENV" >&2
    exit 1
fi
