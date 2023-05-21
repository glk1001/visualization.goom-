#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/goom-get-paths.sh"
source "${THIS_SCRIPT_PATH}/goom-get-vars.sh"


declare KODI_ROOT_DIR=""
declare NO_SAN=0

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    --kodi-root-dir)
      declare KODI_ROOT_DIR=${2}
      shift # past argument
      shift # past value
      ;;
    --no-san)
      declare NO_SAN=1
      shift # past argument
      ;;
    *)
      echo "Unknown option \"${key}\"."
      echo
      exit 1
      ;;
    *)    # unknown option
  esac
done


# Make really sure this is a build directory before removing it!
if [[ $(basename ${BUILD_DIR}) != build* ]]; then
  echo "ERROR: Build basename must start with \"build\". Not this: \"$(basename ${BUILD_DIR})\""
  exit 1
fi

rm -rf "${BUILD_DIR}"
mkdir "${BUILD_DIR}"


pushd "${BUILD_DIR}" >/dev/null

echo

export CC=${C_COMPILER}
export CXX=${CPP_COMPILER}
echo "Using compilers: C: ${C_COMPILER}, C++: ${CPP_COMPILER}."

echo "Using BUILD_DIR: \"${BUILD_DIR}\"."
echo

if [[ "${KODI_ROOT_DIR}" == "" ]]; then
  declare -r KODI_ROOT_DIR_OPTION=""
else
  if [[ ! -d "${KODI_ROOT_DIR}" ]]; then
    echo "ERROR: Could not find kodi root dir: \"${KODI_ROOT_DIR}\"."
    exit 1
  fi
  declare -r KODI_ROOT_DIR_OPTION="-DKODI_ROOT_DIR=${KODI_ROOT_DIR}"
fi

cmake -DIS_THIS_MAIN_CMAKE_SCRIPT=1    \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DENABLE_TESTING=1               \
      -DUSE_MAGIC_ENUM=1               \
      -DOVERRIDE_PATHS=1               \
      -DNO_SAN=${NO_SAN}               \
      ${KODI_ROOT_DIR_OPTION}          \
      ..

echo
echo "Finished cmake \"${BUILD_TYPE}\" configure in build dir \"${BUILD_DIR}\"."
echo

popd >/dev/null
