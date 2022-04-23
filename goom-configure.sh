#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-set-vars.sh"

rm -rf "${BUILD_DIR}"
mkdir "${BUILD_DIR}"

pushd "${BUILD_DIR}"

export CC=${C_COMPILER}
export CXX=${CPP_COMPILER}
echo "Using compilers - C: ${CC}, C++: ${CXX}."
echo

cmake -DADDONS_TO_BUILD=visualization.goom \
      -DADDON_SRC_PREFIX=${THIS_SCRIPT_PATH}/.. \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCMAKE_INSTALL_PREFIX=${THIS_SCRIPT_PATH}/../xbmc/kodi-build/addons \
      -DPACKAGE_ZIP=1 \
      -DBUILD_ARGS_ext="-DIS_KODI_MATRIX=0;-DENABLE_TESTING=1;-DUSE_MAGIC_ENUM=1" \
      ${THIS_SCRIPT_PATH}/../xbmc/cmake/addons

echo
echo "Finished cmake \"${BUILD_TYPE}\" configure in build dir \"${BUILD_DIRNAME}\"."
echo

popd >/dev/null
