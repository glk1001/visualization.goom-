#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "$(realpath ${THIS_SCRIPT_PATH}/..)/goom-docker-paths.sh"


declare IS_USER_ADDON="no"
declare NO_CACHE=""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    --no-cache)
      NO_CACHE="--no-cache"
      shift # past argument
      ;;
    --user-addon)
      IS_USER_ADDON="yes"
      shift # past argument
      ;;
    *)
      echo "ERROR: Unknown command line argument: $key"
      exit 1
      ;;
    *)
  esac
done


if [[ "${IS_USER_ADDON}" == "yes" ]]; then
  # Copying files to "/tmp" in Docker means the files will be lost when container starts.
  # We want this for user addon files because they are seperately copied to the kodi home addons directory.
  declare -r KODI_GOOM_LIB_DIR=/tmp
  declare -r KODI_GOOM_ADDON_DIR=/tmp
else
  declare -r KODI_GOOM_LIB_DIR=/usr/lib/x86_64-linux-gnu/kodi/addons/visualization.goom
  declare -r KODI_GOOM_ADDON_DIR=/usr/share/kodi/addons/visualization.goom
fi

if [[ "${KODI_VERSION}" == "matrix" ]]; then
  declare -r KODI_PPA="ppa:team-xbmc/ppa"
elif [[ "${KODI_VERSION}" == "nightly" ]]; then
  declare -r KODI_PPA="ppa:team-xbmc/xbmc-nightly"
else
  echo "ERROR: Unknown Kodi version: \"${KODI_VERSION}\"."
  exit 1
fi

if [[ "${NO_CACHE}" != "" ]]; then
  echo "Building \"${KODI_GOOM_IMAGE}\" with '--no-cache'..."
else
  echo "Building \"${KODI_GOOM_IMAGE}\"..."
fi


cd "${THIS_SCRIPT_PATH}"

declare -r BUILD_BASE_ARGS="--build-arg OS_TYPE=${KODI_IMAGE_OS_TYPE} --build-arg OS_TAG=${KODI_IMAGE_OS_TAG}\
                            --build-arg KODI_PPA=${KODI_PPA}"
declare -r BUILD_SPOTIFY_ARGS="--build-arg KODI_BASE_IMAGE=${KODI_BASE_IMAGE}"
declare -r BUILD_LIRC_ARGS="--build-arg KODI_SPOTIFY_IMAGE=${KODI_SPOTIFY_IMAGE}"
declare -r BUILD_GOOM_ARGS="--build-arg KODI_LIRC_IMAGE=${KODI_LIRC_IMAGE} --build-arg GOOM_VERSION=${GOOM_VERSION}\
                            --build-arg KODI_GOOM_LIB_DIR=${KODI_GOOM_LIB_DIR} --build-arg KODI_GOOM_ADDON_DIR=${KODI_GOOM_ADDON_DIR}"

docker build ${NO_CACHE} -t ${KODI_BASE_IMAGE}    ${BUILD_BASE_ARGS}    -f Dockerfile-base                 .
docker build ${NO_CACHE} -t ${KODI_SPOTIFY_IMAGE} ${BUILD_SPOTIFY_ARGS} -f Dockerfile-spotify-dependencies .
docker build ${NO_CACHE} -t ${KODI_LIRC_IMAGE}    ${BUILD_LIRC_ARGS}    -f Dockerfile-lirc                 .
docker build ${NO_CACHE} -t ${KODI_GOOM_IMAGE}    ${BUILD_GOOM_ARGS}    -f Dockerfile                      .

if [[ "${IS_USER_ADDON}" == "yes" ]]; then
  # Copy all the required goom addon files to kodi home directory used by the kodi goom container.
  declare -r CONTAINER_GOOM_ADDON_DIR="${KODI_CONTAINER_HOME_DIR}/.kodi/addons/visualization.goom"
  mkdir -p "${CONTAINER_GOOM_ADDON_DIR}"
  echo
  echo "rsyncing \"${KODI_DOCKER_FILES_DIR}/visualization.goom.so.${GOOM_VERSION}\" to \"${CONTAINER_GOOM_ADDON_DIR}\""
  rsync -avh "${KODI_DOCKER_FILES_DIR}/visualization.goom.so.${GOOM_VERSION}" "${CONTAINER_GOOM_ADDON_DIR}"
  echo
  echo "rsyncing \"${KODI_DOCKER_FILES_DIR}/addon.xml\" to \"${CONTAINER_GOOM_ADDON_DIR}\""
  rsync -avh "${KODI_DOCKER_FILES_DIR}/addon.xml" "${CONTAINER_GOOM_ADDON_DIR}/addon.xml"
  echo
  echo "rsyncing \"${KODI_DOCKER_FILES_DIR}/resources/\" to \"${CONTAINER_GOOM_ADDON_DIR}resources/\""
  rsync --out-format="%n" --itemize-changes -a "${KODI_DOCKER_FILES_DIR}/resources/" "${CONTAINER_GOOM_ADDON_DIR}/resources/"
fi

cd - >/dev/null
