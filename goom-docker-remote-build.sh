#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-docker-path.sh"

if [[ "${1:-}" != "--no-cache" ]]; then
  declare -r NO_CACHE=
else
  declare -r NO_CACHE="--no-cache"
  shift
fi

declare REMOTE_HOST=$1

if ! ping -q -c 2 ${REMOTE_HOST} ; then
  echo "Could not connect to host \"${REMOTE_HOST}\"."
  exit 1
fi

ssh ${REMOTE_HOST} "bash "${REMOTE_KODI_HOME_DIR}/build-all.sh" ${NO_CACHE} $@"
