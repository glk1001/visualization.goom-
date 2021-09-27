#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

declare -r BUILD_DIR=${THIS_SCRIPT_PATH}/build

rm -rf "${BUILD_DIR}"

mkdir "${BUILD_DIR}" && cd "${BUILD_DIR}"

cmake -DADDONS_TO_BUILD=visualization.goom \
      -DADDON_SRC_PREFIX=/home/greg/Prj/github/xbmc \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DCMAKE_INSTALL_PREFIX=/home/greg/Prj/github/xbmc/xbmc/kodi-build/addons \
      -DPACKAGE_ZIP=1 \
      ${THIS_SCRIPT_PATH}/../xbmc/cmake/addons

cd "${BUILD_DIR}"
