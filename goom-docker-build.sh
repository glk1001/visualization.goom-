#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-docker-paths.sh"


bash "${DOCKERIZE_KODI_DIR}/build-all.sh" ${GOOM_DOCKER_PATHS_CMD_LINE} $@
