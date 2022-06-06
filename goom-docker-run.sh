#!/bin/bash

set -e
set -u

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-docker-paths.sh"


declare -r OLD_LINUX_CORE_PATTERN=$(cat /proc/sys/kernel/core_pattern)
echo
echo "Setting kodi 'core' pattern:"
echo '/tmp/core.%e.%p' | sudo /usr/bin/tee /proc/sys/kernel/core_pattern

echo
x11docker --runasroot="service lircd start" \
		  --name ${KODI_CONTAINER_NAME} \
          --hostuser=${USER} \
		  --network \
		  --pulseaudio \
		  --gpu \
          --share=${MUSIC_SHARE} \
          --home=${KODI_CONTAINER_HOME_DIR} \
          -- \
          --init \
		  --privileged \
          --ulimit core=-1 \
          --mount type=bind,source=/tmp/,target=/tmp/ \
          --rm \
          -- \
	      ${KODI_IMAGE_NAME}

echo "x11docker return code = $?"

echo "${OLD_LINUX_CORE_PATTERN}" | sudo /usr/bin/tee /proc/sys/kernel/core_pattern > /dev/null

