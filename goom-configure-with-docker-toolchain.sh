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

declare -r HOST_CCACHE_DIR=${CCACHE_DIR}
declare -r KODI_ROOT_DIR=${HOME}/Prj/github/xbmc

declare -r TIME_ZONE=$(cat /etc/timezone)
declare -r DOCKER_CCACHE_DIR=/tmp/ccache
declare -r DOCKER_KODI_ROOT_DIR=/tmp/xbmc
declare -r DOCKER_GOOM_DIR="/tmp/visualization.goom"

docker run --rm                                         \
           -e TZ=${TIME_ZONE}                           \
           -e CCACHE_DIR=${DOCKER_CCACHE_DIR}           \
           -v ${HOST_CCACHE_DIR}:${DOCKER_CCACHE_DIR}   \
           -v ${KODI_ROOT_DIR}:${DOCKER_KODI_ROOT_DIR}  \
           -v ${THIS_SCRIPT_PATH}:${DOCKER_GOOM_DIR}    \
           -t ${DOCKER_IMAGE}                           \
           bash -c "cd ${DOCKER_GOOM_DIR} &&                  \
                    ${DOCKER_GOOM_DIR}/goom-configure.sh      \
                      --docker                                \
                      --compiler ${COMPILER}                  \
                      --build-type ${BUILD_TYPE}              \
                      --kodi-root-dir ${DOCKER_KODI_ROOT_DIR}"
