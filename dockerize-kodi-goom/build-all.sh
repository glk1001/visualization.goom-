#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

declare -r KODI_BASE_IMAGE="kodi-nightly:base"
declare -r KODI_SPOTIFY_IMAGE="kodi-nightly:spotify"
declare -r KODI_LIRC_IMAGE="kodi-nightly:lirc"
declare -r KODI_GOOM_IMAGE="kodi-nightly:goom-dev"

if [[ "${1:-}" == "--no-cache" ]]; then
  echo "Building \"${KODI_GOOM_IMAGE}\" with '--no-cache'..."
  declare -r NO_CACHE="--no-cache"
else
  echo "Building \"${KODI_GOOM_IMAGE}\"..."
  declare -r NO_CACHE=
fi

cd "${THIS_SCRIPT_PATH}"

docker build -t ${KODI_BASE_IMAGE}    -f Dockerfile-base .
docker build -t ${KODI_SPOTIFY_IMAGE} -f Dockerfile-spotify-dependencies .
docker build -t ${KODI_LIRC_IMAGE}    -f Dockerfile-lirc .
docker build -t ${KODI_GOOM_IMAGE}    -f Dockerfile .

cd -
