#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"


declare -r KODI_ADDONS_BUILD_DIR=$(realpath ${THIS_SCRIPT_PATH}/../kodi/kodi-build/addons)
if [[ ! -d "${KODI_ADDONS_BUILD_DIR}" ]]; then
  echo "ERROR: Could not find kodi build directory \"${KODI_ADDONS_BUILD_DIR}\"."
  exit 1
fi

declare -r KODI_LIB_DIR=/usr/local/lib/kodi/addons/visualization.goom
if [[ ! -d "${KODI_LIB_DIR}" ]]; then
  echo "ERROR: Could not find kodi lib directory \"${KODI_LIB_DIR}\"."
  exit 1
fi

declare -r KODI_DATA_DIR=/usr/local/share/kodi/addons/visualization.goom
if [[ ! -d "${KODI_DATA_DIR}" ]]; then
  echo "ERROR: Could not find kodi resources directory \"${KODI_DATA_DIR}\"."
  exit 1
fi

sudo cp -pv ${KODI_ADDONS_BUILD_DIR}/lib/kodi/addons/visualization.goom/visualization.goom.so.* ${KODI_LIB_DIR}

echo
sudo cp -pv ${KODI_ADDONS_BUILD_DIR}/share/kodi/addons/visualization.goom/addon.xml ${KODI_DATA_DIR}

echo
echo "rsyncing \"${KODI_ADDONS_BUILD_DIR}/share/kodi/addons/visualization.goom/resources/\" to \"${KODI_DATA_DIR}/resources/\"..."
sudo rsync --out-format="%n" --itemize-changes -a ${KODI_ADDONS_BUILD_DIR}/share/kodi/addons/visualization.goom/resources/ ${KODI_DATA_DIR}/resources/
