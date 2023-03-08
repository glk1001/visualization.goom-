#!/bin/bash

set -u
set -o pipefail

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-get-paths.sh"
source "${THIS_SCRIPT_PATH}/goom-get-vars.sh"

declare -r RUN_CLANG_FORMAT="${THIS_SCRIPT_PATH}/run-clang-format.py"
if [[ "${COMPILER_VERSION}" == "" ]]; then
  declare -r CLANG_FORMAT_EXE="clang-format"
else
  declare -r CLANG_FORMAT_EXE="clang-format-${COMPILER_VERSION}"
fi
declare -r CLANG_FORMAT_LOG="/tmp/goom-clang-format.log"

if [[ ! -d "${BUILD_DIRNAME}" ]]; then
  >&2 echo "ERROR: Could not find build dir: \"${BUILD_DIRNAME}\"."
  exit 1
fi

if [[ "${1:-}" != "" ]]; then
  declare -r CUSTOM_SRCE=$1
python3 "${RUN_CLANG_FORMAT}" --clang-format-executable "${CLANG_FORMAT_EXE}" \
        -i -r -j 6 -e "*build-*" -e "*vivid*" -e "*bezier" -e "*tcb*" "${CUSTOM_SRCE}"
  exit $?
fi

SECONDS=0

python3 "${RUN_CLANG_FORMAT}" --clang-format-executable "${CLANG_FORMAT_EXE}" \
        -i -r -j 6 -e "*build-*" -e "*vivid*" -e "*bezier" -e "*tcb*" "${GOOM_MAIN_ROOT_DIR}"
if [[ $? != 0 ]]; then
  >&2 echo "ERROR: There were clang-format errors."
  >&2 echo "Time of run: $(( SECONDS/60 )) min, $(( SECONDS%60 )) sec."
  exit 1
fi

FILE_CHANGES=$(if [[ $(git diff --name-only) ]]; then echo yes; else echo no; fi)
if [[ "${FILE_CHANGES}" == "yes" ]]; then
  >&2 echo "ERROR: There were clang-format changes."
  >&2 echo
  git diff --name-only
  >&2 echo
  >&2 echo "Time of run: $(( SECONDS/60 )) min, $(( SECONDS%60 )) sec."
  exit 1
fi

>&2 echo "SUCCESS: There were no clang-format changes."

>&2 echo "Time of run: $(( SECONDS/60 )) min, $(( SECONDS%60 )) sec."
