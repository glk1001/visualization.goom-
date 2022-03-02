#!/bin/bash

set -e
set -u

declare -r IMAGE_NAME="kodi-nightly:goom-dev"
declare -r CONTAINER_NAME="kodi_goom_dev"
declare -r CONTAINER_HOME_DIR="${HOME}/docker/kodi-nightly"
declare -r MUSIC_SHARE="/mnt/Music"


x11docker --runasroot="service lircd start" --network --pulseaudio --gpu \
          --share=${MUSIC_SHARE} --home=${CONTAINER_HOME_DIR} --name ${CONTAINER_NAME} \
          -- --privileged -- ${IMAGE_NAME}

echo "x11docker return code = $?"
