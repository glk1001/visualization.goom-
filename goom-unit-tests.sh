#!/bin/bash

set -u

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-get-vars.sh"


if [[ ! -d "${BUILD_DIR}" ]]; then
  echo "ERROR: Could not find build directory \"${BUILD_DIR}\"."
  exit 1
fi

echo "Using TEST_DIR \"${BUILD_DIR}\"."
echo

${BUILD_DIR}/goom-libs-artifacts/bin/${BUILD_TYPE}/goom_lib_tests -d yes
if [[ $? != 0 ]]; then
  exit 1
fi
