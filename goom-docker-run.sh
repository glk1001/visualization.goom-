#!/bin/bash

set -e
set -u

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-docker-paths.sh"


declare -r OLD_CORE_PATTERN=$(cat /proc/sys/kernel/core_pattern)
declare -r CORE_PATTERN="/tmp/core.%e.%p"
echo
echo "Setting linux 'core' pattern: \"${CORE_PATTERN}\""
ulimit -c unlimited
sudo /usr/sbin/sysctl -q -w kernel.core_pattern="${CORE_PATTERN}"

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

sudo /usr/sbin/sysctl -q -w kernel.core_pattern="${OLD_CORE_PATTERN}"
