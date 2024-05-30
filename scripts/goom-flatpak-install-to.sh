#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-get-paths.sh"
source "${THIS_SCRIPT_PATH}/goom-get-vars.sh"


if [[ "${1:-}" == "--dry-run" ]]; then
  declare -r DRY_RUN="--dry-run"
else
  declare -r DRY_RUN=
fi

declare -r KODI_GOOM_ADDON_DIR="${BUILD_DIR}/kodi-vis-goom-addon"
declare -r KODI_FLATPAK_FILES_DIR="${HOME}/.var/app/tv.kodi.Kodi/data"
declare -r KODI_FLATPAK_GOOM_ADDON_DIR="${KODI_FLATPAK_FILES_DIR}/addons/visualization.goom-pp"

echo "Installing Goom add-on in \"${KODI_GOOM_ADDON_DIR}\" to flatpak kodi add-on directory \"${KODI_FLATPAK_GOOM_ADDON_DIR}\"..."
echo

if [[ ! -d "${KODI_GOOM_ADDON_DIR}" ]]; then
  echo "ERROR: Could not find goom add-on directory \"${KODI_GOOM_ADDON_DIR}\"."
  exit 1
fi
if [[ ! -d "${KODI_FLATPAK_FILES_DIR}" ]]; then
  echo "ERROR: Could not find kodi flatpak add-on directory \"${KODI_FLATPAK_FILES_DIR}\"."
  exit 1
fi

echo
echo "rsyncing \"${KODI_GOOM_ADDON_DIR}/\" to \"${KODI_FLATPAK_FILES_DIR}/\"..."
rsync ${DRY_RUN} --out-format="%n" --itemize-changes -a ${KODI_GOOM_ADDON_DIR}/ ${KODI_FLATPAK_GOOM_ADDON_DIR}/

echo
