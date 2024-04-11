#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-get-paths.sh"


if [[ "${1:-}" == "--dry-run" ]]; then
  declare -r DRY_RUN="--dry-run"
else
  declare -r DRY_RUN=
fi

declare -r KODI_FLATPAK_FILES_DIR="${HOME}/.var/app/tv.kodi.Kodi/data"
declare -r KODI_FLATPAK_GOOM_ADDON_DIR="${KODI_FLATPAK_FILES_DIR}/addons/visualization.goom-pp"
declare -r KODI_FLATPAK_GOOM_ADDON_RESOURCES_DIR="${KODI_FLATPAK_GOOM_ADDON_DIR}/resources"

echo "Installing Goom add-on to flatpak files directory \"${KODI_FLATPAK_FILES_DIR}\"..."
echo

if [[ ! -d "${KODI_BUILD_LIB_DIR}" ]]; then
  echo "ERROR: Could not find kodi build lib directory \"${KODI_BUILD_LIB_DIR}\"."
  exit 1
fi
if [[ ! -d "${KODI_BUILD_SHARE_DIR}" ]]; then
  echo "ERROR: Could not find kodi build share directory \"${KODI_BUILD_SHARE_DIR}\"."
  exit 1
fi
if [[ ! -d "${KODI_BUILD_RESOURCES_DIR}" ]]; then
  echo "ERROR: Could not find kodi build resources directory \"${KODI_BUILD_RESOURCES_DIR}\"."
  exit 1
fi

echo "rsyncing \"${KODI_BUILD_LIB_DIR}/visualization.goom-pp.so.*\" to \"${KODI_FLATPAK_GOOM_ADDON_DIR}\"..."
rsync ${DRY_RUN} -avh ${KODI_BUILD_LIB_DIR}/visualization.goom-pp.so.* ${KODI_FLATPAK_GOOM_ADDON_DIR}

echo
echo "rsyncing \"${KODI_BUILD_SHARE_DIR}/addon.xml\" to \"${KODI_FLATPAK_GOOM_ADDON_DIR}\"..."
rsync ${DRY_RUN} -avh ${KODI_BUILD_SHARE_DIR}/addon.xml ${KODI_FLATPAK_GOOM_ADDON_DIR}

echo
echo "rsyncing \"${KODI_BUILD_RESOURCES_DIR}/\" to \"${KODI_FLATPAK_GOOM_ADDON_RESOURCES_DIR}/\"..."
rsync ${DRY_RUN} --out-format="%n" --itemize-changes -a ${KODI_BUILD_RESOURCES_DIR}/ ${KODI_FLATPAK_GOOM_ADDON_RESOURCES_DIR}/

echo
