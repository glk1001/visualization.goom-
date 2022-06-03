#!/bin/bash

set -u

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-set-vars.sh"


if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "ERROR: Could not find build directory \"${BUILD_DIR}\"."
  exit 1
fi

declare -r DOCKER_IMAGE=clion/ubuntu/cpp-env:1.1-jammy
declare -r DOCKER_GOOM_DIR="/tmp/visualization.goom"
declare -r TEST_DIR="${DOCKER_GOOM_DIR}/${BUILD_DIRNAME}"

docker run --rm                                            \
           -v ${THIS_SCRIPT_PATH}:${DOCKER_GOOM_DIR}       \
           -t ${DOCKER_IMAGE}                              \
           bash -c "ctest --verbose --test-dir ${TEST_DIR}"
if [[ $? == 0 ]]; then
  exit 0
fi


declare -r RERUN_OPTS="--rerun-failed --output-on-failure"

docker run --rm                                                          \
           -v ${THIS_SCRIPT_PATH}:${DOCKER_GOOM_DIR}                     \
           -t ${DOCKER_IMAGE}                                            \
           bash -c "ctest ${RERUN_OPTS} --verbose --test-dir ${TEST_DIR}"
