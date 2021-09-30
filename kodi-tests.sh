#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/kodi-set-vars.sh"

if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "ERROR: Could not find build directory \"${BUILD_DIR}\"."
  exit 1
fi  

ctest --test-dir ${BUILD_DIR}/visualization.goom-prefix/src/visualization.goom-build/goom_libs-prefix/src/goom_libs-build
