#!/bin/bash

set -u

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-get-vars.sh"

declare -r RUN_CLANG_TIDY="run-clang-tidy-15"
declare -r GOOM_MAIN_DIR="${THIS_SCRIPT_PATH}/src"
declare -r GOOM_LIB_DIR="${THIS_SCRIPT_PATH}/depends/goom-libs/src"
declare -r CLANG_TIDY_LOG="/tmp/goom-clang-tidy.log"

if [[ ! -d "${GOOM_MAIN_DIR}" ]]; then
  >&2 echo "ERROR: Could not find goom main source dir: \"${GOOM_MAIN_DIR}\"."
  exit 1
fi
if [[ ! -d "${GOOM_LIB_DIR}" ]]; then
  >&2 echo "ERROR: Could not find goom lib source dir: \"${GOOM_LIB_DIR}\"."
  exit 1
fi
if [[ ! -d "${BUILD_DIRNAME}" ]]; then
  >&2 echo "ERROR: Could not find build dir: \"${BUILD_DIRNAME}\"."
  exit 1
fi

SECONDS=0

"${RUN_CLANG_TIDY}" -j 6 -header-filter='^((?!catch2).)*$' -extra-arg=-Wno-unknown-warning-option \
                    -p "${BUILD_DIRNAME}" "${GOOM_MAIN_DIR}" "${GOOM_LIB_DIR}" |& tee "${CLANG_TIDY_LOG}"
if [[ $? != 0 ]]; then
  >&2 echo "ERROR: There were clang-tidy errors."
  >&2 echo "Time of run: $(( SECONDS/60 )) min, $(( SECONDS%60 )) sec."
  exit 1
fi

declare -r NUM_GOOM_MAIN_SRCE_FILES=$(fdfind -I '.*\.cpp' "${GOOM_MAIN_DIR}" | wc -l)
declare -r NUM_GOOM_LIB_SRCE_FILES=$(fdfind -I -E 'vivid/examples' -E 'vivid/tests' '.*\.cpp' "${GOOM_LIB_DIR}" | wc -l)
declare -r NUM_SRCE_FILES=$(( NUM_GOOM_MAIN_SRCE_FILES + NUM_GOOM_LIB_SRCE_FILES ))
declare -r NUM_CLANG_TIDY_FILES=$(eval "sed -n -e 's/^clang-tidy.* -p=${BUILD_DIRNAME} //p' "${CLANG_TIDY_LOG}"" | wc -l)

if [[ ${NUM_SRCE_FILES} != ${NUM_CLANG_TIDY_FILES} ]]; then
  >&2 echo "WARNING: Number of source files, ${NUM_SRCE_FILES} != ${NUM_CLANG_TIDY_FILES}, number of clang-tidy files."
else
  >&2 echo "SUCCESS: There were no clang-tidy errors."
fi

>&2 echo "Time of run: $(( SECONDS/60 )) min, $(( SECONDS%60 )) sec."
