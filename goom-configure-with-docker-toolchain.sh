#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-get-vars.sh"


docker run --rm                                               \
           -e TZ=${HOST_TIME_ZONE}                            \
           -e CCACHE_DIR=${DOCKER_CCACHE_DIR}                 \
           -v ${HOST_CCACHE_DIR}:${DOCKER_CCACHE_DIR}         \
           -v ${HOST_KODI_ROOT_DIR}:${DOCKER_KODI_ROOT_DIR}   \
           -v ${THIS_SCRIPT_PATH}:${DOCKER_GOOM_DIR}          \
           -t ${DOCKER_IMAGE}                                 \
           bash -c "cd ${DOCKER_GOOM_DIR} &&                  \
                    ${DOCKER_GOOM_DIR}/goom-configure.sh      \
                      --docker                                \
                      --compiler ${COMPILER}                  \
                      --build-type ${BUILD_TYPE}              \
                      --kodi-root-dir ${DOCKER_KODI_ROOT_DIR}"
