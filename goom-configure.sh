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
echo "Using compilers - C: ${C_COMPILER}, C++: ${CPP_COMPILER}."
echo

#cmake -D CMAKE_C_COMPILER=${C_COMPILER}
#      -D CMAKE_CXX_COMPILER=${CPP_COMPILER}
cmake -DENABLE_TESTING=1 \
      -DUSE_MAGIC_ENUM=1 \
      -DOVERRIDE_PATHS=1 \
      -D CMAKE_INSTALL_PREFIX=${THIS_SCRIPT_PATH}/../xbmc/kodi-build/addons \
      ..

echo
echo "Finished cmake \"${BUILD_TYPE}\" configure in build dir \"${BUILD_DIRNAME}\"."
echo

popd >/dev/null
