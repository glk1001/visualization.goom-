#!/bin/bash

set -e
set -u

declare -r IMAGE_NAME="kodi-nightly:goom-dev"
declare -r CONTAINER_NAME="kodi_goom_dev"
declare -r CONTAINER_HOME_DIR="${HOME}/docker/kodi-nightly"
declare -r MUSIC_SHARE="/mnt/Music"

declare -r OLD_CORE_PATTERN=$(cat /proc/sys/kernel/core_pattern)
echo
echo "Setting kodi 'core' pattern:"
echo '/tmp/core.%e.%p' | sudo /usr/bin/tee /proc/sys/kernel/core_pattern

echo
x11docker --runasroot="service lircd start" \
		  --name ${CONTAINER_NAME} \
          --hostuser=greg \
		  --network \
		  --pulseaudio \
		  --gpu \
          --share=${MUSIC_SHARE} \
          --home=${CONTAINER_HOME_DIR} \
          -- \
          --init \
		  --privileged \
          --ulimit core=-1 \
          --mount type=bind,source=/tmp/,target=/tmp/ \
          --rm \
          -- \
	      ${IMAGE_NAME}

echo "x11docker return code = $?"

echo "${OLD_CORE_PATTERN}" | sudo /usr/bin/tee /proc/sys/kernel/core_pattern > /dev/null

