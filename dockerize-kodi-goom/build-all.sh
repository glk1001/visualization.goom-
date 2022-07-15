#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

declare -r OS_TYPE=ubuntu
declare -r OS_TAG=impish
# Use ubuntu impish so we get python 3.9. There is a problem is with python 3.10 and sqlite3
# which affects the Spotify plugin. See https://bbs.archlinux.org/viewtopic.php?id=272121

#declare -r KODI_VERSION=nightly
#declare -r KODI_PPA="ppa:team-xbmc/xbmc-nightly"
declare -r KODI_VERSION=matrix
declare -r KODI_PPA="ppa:team-xbmc/ppa"

declare -r KODI_IMAGE_NAME="${OS_TYPE}-${OS_TAG}/kodi-${KODI_VERSION}"
declare -r KODI_BASE_IMAGE="${KODI_IMAGE_NAME}:base"
declare -r KODI_SPOTIFY_IMAGE="${KODI_IMAGE_NAME}:spotify"
declare -r KODI_LIRC_IMAGE="${KODI_IMAGE_NAME}:lirc"
declare -r KODI_GOOM_IMAGE="${KODI_IMAGE_NAME}:goom-dev"

if [[ "${1:-}" == "--no-cache" ]]; then
  echo "Building \"${KODI_GOOM_IMAGE}\" with '--no-cache'..."
  declare -r NO_CACHE="--no-cache"
else
  echo "Building \"${KODI_GOOM_IMAGE}\"..."
  declare -r NO_CACHE=
fi

cd "${THIS_SCRIPT_PATH}"


declare -r BUILD_BASE_ARGS="--build-arg OS_TYPE=${OS_TYPE} --build-arg OS_TAG=${OS_TAG} --build-arg KODI_PPA=${KODI_PPA}"
declare -r BUILD_SPOTIFY_ARGS="--build-arg KODI_BASE_IMAGE=${KODI_BASE_IMAGE}"
declare -r BUILD_LIRC_ARGS="--build-arg KODI_SPOTIFY_IMAGE=${KODI_SPOTIFY_IMAGE}"
declare -r BUILD_GOOM_ARGS="--build-arg KODI_LIRC_IMAGE=${KODI_LIRC_IMAGE}"

docker build ${NO_CACHE} -t ${KODI_BASE_IMAGE}    ${BUILD_BASE_ARGS}    -f Dockerfile-base                 .
docker build ${NO_CACHE} -t ${KODI_SPOTIFY_IMAGE} ${BUILD_SPOTIFY_ARGS} -f Dockerfile-spotify-dependencies .
docker build ${NO_CACHE} -t ${KODI_LIRC_IMAGE}    ${BUILD_LIRC_ARGS}    -f Dockerfile-lirc                 .
docker build ${NO_CACHE} -t ${KODI_GOOM_IMAGE}    ${BUILD_GOOM_ARGS}    -f Dockerfile                      .

cd -
