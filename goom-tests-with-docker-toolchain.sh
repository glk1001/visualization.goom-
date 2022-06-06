#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-set-vars.sh"


if [[ "${COMPILER}" == "gcc-12" ]]; then
  declare -r DOCKER_IMAGE=clion/ubuntu/cpp-env:1.1-kinetic
else
  declare -r DOCKER_IMAGE=clion/ubuntu/cpp-env:1.1-jammy
fi

declare -r TIME_ZONE=$(cat /etc/timezone)
declare -r DOCKER_GOOM_DIR="/tmp/visualization.goom"

docker run --rm                                      \
           -e TZ=${TIME_ZONE}                           \
           -v ${THIS_SCRIPT_PATH}:${DOCKER_GOOM_DIR} \
           -t ${DOCKER_IMAGE}                        \
           bash -c "cd ${DOCKER_GOOM_DIR} &&         \
                    ${DOCKER_GOOM_DIR}/goom-tests.sh \
                      --docker                       \
                      --compiler ${COMPILER}         \
                      --build-type ${BUILD_TYPE}"
