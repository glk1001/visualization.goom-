#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-get-paths.sh"
source "${THIS_SCRIPT_PATH}/goom-get-vars.sh"

if [[ ! -d "${BUILD_DIRNAME}" ]]; then
  >&2 echo "ERROR: Could not find build dir: \"${BUILD_DIRNAME}\"."
  exit 1
fi

declare -r CLANG_TIDY_LOG="/tmp/goom-clang-tidy.log"

fdfind -I ".*\.cpp" "src" > "/tmp/goom-src1.out"
fdfind -I -E "vivid/examples" -E "vivid/tests" ".*\.cpp" "depends/goom-libs/src"  > "/tmp/goom-src2.out"
cat "/tmp/goom-src1.out" "/tmp/goom-src2.out" | sort > "/tmp/goom-src-sorted.out"

sed -n -e "s/^clang-tidy.* -p=build-clion-clang-15-RelWithDebInfo-nexus //p" "${CLANG_TIDY_LOG}" | sort > "/tmp/goom-clang-tidy-sorted.out"
sed -i "s#${GOOM_MAIN_ROOT_DIR}/##g" "/tmp/goom-clang-tidy-sorted.out"

bcompare "/tmp/goom-clang-tidy-sorted.out" "/tmp/goom-src-sorted.out"
