#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

if [[ "${1:-}" == "--dry-run" ]]; then
  declare -r DRY_RUN="--dry-run"
else
  declare -r DRY_RUN=
fi

declare -r KODI_BUILD_DIR=${THIS_SCRIPT_PATH}/../xbmc/kodi-build/addons/visualization.goom
if [[ ! -d "${KODI_BUILD_DIR}" ]]; then
  echo "ERROR: Could not find kodi build directory \"${KODI_BUILD_DIR}\"."
  exit 1
fi
declare -r KODI_BUILD_RESOURCES_DIR=${KODI_BUILD_DIR}/resources
if [[ ! -d "${KODI_BUILD_RESOURCES_DIR}" ]]; then
  echo "ERROR: Could not find kodi build resources directory \"${KODI_BUILD_RESOURCES_DIR}\"."
  exit 1
fi

declare -r KODI_DOCKER_FILES_DIR=${THIS_SCRIPT_PATH}/dockerize-kodi-goom/files
if [[ ! -d "${KODI_DOCKER_FILES_DIR}" ]]; then
  echo "ERROR: Could not find kodi docker files directory \"${KODI_DOCKER_FILES_DIR}\"."
  exit 1
fi
declare -r KODI_DOCKER_RESOURCES_DIR=${KODI_DOCKER_FILES_DIR}/resources
if [[ ! -d "${KODI_DOCKER_RESOURCES_DIR}" ]]; then
  echo "ERROR: Could not find kodi docker resources directory \"${KODI_DOCKER_RESOURCES_DIR}\"."
  exit 1
fi

rsync ${DRY_RUN} -avh ${KODI_BUILD_DIR}/visualization.goom.so.* ${KODI_DOCKER_FILES_DIR}

echo
rsync ${DRY_RUN} -avh ${KODI_BUILD_DIR}/addon.xml ${KODI_DOCKER_FILES_DIR}

echo
echo "rsyncing \"${KODI_BUILD_RESOURCES_DIR}/\" to \"${KODI_DOCKER_RESOURCES_DIR}/\"..."
rsync ${DRY_RUN} --out-format="%n" --itemize-changes -a ${KODI_BUILD_RESOURCES_DIR}/ ${KODI_DOCKER_RESOURCES_DIR}/
