#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

declare -r BUILD_DIR=${THIS_SCRIPT_PATH}/build-lib-goom

rm -rf "${BUILD_DIR}"

mkdir "${BUILD_DIR}" && cd "${BUILD_DIR}"

cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo "${THIS_SCRIPT_PATH}/lib-goom"

cd "${BUILD_DIR}"
