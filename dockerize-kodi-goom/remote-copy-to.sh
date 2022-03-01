#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

if [[ "${1}" != "--dry-run" ]]; then
  declare -r DRY_RUN=
else
  declare -r DRY_RUN="--dry-run"
  shift
fi

declare -r REMOTE_HOST=$1
declare -r REMOTE_KODI_HOME_DIR=$2

if ! ping -q -c 2 ${REMOTE_HOST} ; then
  echo "Could not connect to host \"${REMOTE_HOST}\"."
  exit 1
fi

echo
echo "Rsyncing \"${THIS_SCRIPT_PATH}/\" to \"${REMOTE_HOST}:${REMOTE_KODI_HOME_DIR}/\""
echo

rsync ${DRY_RUN} -avh --exclude=remote-copy-to.sh "${THIS_SCRIPT_PATH}/" "${REMOTE_HOST}:${REMOTE_KODI_HOME_DIR}/"
