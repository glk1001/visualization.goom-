#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-docker-paths.sh"


if [[ "${1:-}" == "--dry-run" ]]; then
  declare -r DRY_RUN="--dry-run"
else
  declare -r DRY_RUN=
fi

echo
echo "rsyncing \"${KODI_BUILD_LIB_DIR}/visualization.goom.so.*\" to \"${KODI_DOCKER_FILES_DIR}\"..."
rsync ${DRY_RUN} -avh ${KODI_BUILD_LIB_DIR}/visualization.goom.so.* ${KODI_DOCKER_FILES_DIR}

echo
echo "rsyncing \"${KODI_BUILD_SHARE_DIR}/addon.xml\" to \"${KODI_DOCKER_FILES_DIR}\"..."
rsync ${DRY_RUN} -avh ${KODI_BUILD_SHARE_DIR}/addon.xml ${KODI_DOCKER_FILES_DIR}

echo
echo "rsyncing \"${KODI_BUILD_RESOURCES_DIR}/\" to \"${KODI_DOCKER_RESOURCES_DIR}/\"..."
rsync ${DRY_RUN} --out-format="%n" --itemize-changes -a ${KODI_BUILD_RESOURCES_DIR}/ ${KODI_DOCKER_RESOURCES_DIR}/
