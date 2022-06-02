#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-set-vars.sh"


declare -r BUILD_TIME_LIB_FILE=${THIS_SCRIPT_PATH}/depends/goom-libs/src/goom/src/utils/build_time.cpp
declare -r BUILD_TIME_MAIN_FILE=${THIS_SCRIPT_PATH}/src/build_time.cpp

if [[ ! -f "${BUILD_TIME_LIB_FILE}" ]]; then
  echo "ERROR: Could not find special file \"${BUILD_TIME_LIB_FILE}\"."
  exit 1
fi
if [[ ! -f "${BUILD_TIME_MAIN_FILE}" ]]; then
  echo "ERROR: Could not find special file \"${BUILD_TIME_MAIN_FILE}\"."
  exit 1
fi

if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "ERROR: Could not find build directory \"${BUILD_DIR}\"."
  exit 1
fi

export CC=${C_COMPILER}
export CXX=${CPP_COMPILER}
echo "Using compilers - C: ${CC}, C++: ${CXX}."
echo

touch --no-create "${BUILD_TIME_LIB_FILE}" "${BUILD_TIME_MAIN_FILE}"

cmake --build "${BUILD_DIR}"

cmake --install "${BUILD_DIR}"

echo
echo "Finished cmake build and install in build dir \"${BUILD_DIRNAME}\"."
echo
