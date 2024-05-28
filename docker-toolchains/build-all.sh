#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/build-get-vars.sh"


declare NO_CACHE=""
declare DOCKER_OS_TYPE="${DOCKER_BUILD_OS_TYPE}"
declare DOCKER_OS_TAG="${DOCKER_BUILD_OS_TAG}"
declare DOCKER_TAG="${DOCKER_BUILD_TAG}"

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    --no-cache)
      NO_CACHE="--no-cache"
      shift # past argument
      ;;
    --docker-os-type)
      DOCKER_OS_TYPE=${2}
      shift # past argument
      shift # past value
      ;;
    --docker-os-tag)
      DOCKER_OS_TAG=${2}
      shift # past argument
      shift # past value
      ;;
    --docker-tag)
      DOCKER_TAG=${2}
      shift # past argument
      shift # past value
      ;;
    --clang)
      COMPILER=clang
      shift # past argument
      ;;
    --clang-ver)
      CLANG_VER=${2}
      shift # past argument
      shift # past value
      ;;
    --gcc)
      COMPILER=gcc
      shift # past argument
      ;;
    --gcc-ver)
      GCC_VER=${2}
      shift # past argument
      shift # past value
      ;;
    *)
      echo "Unknown option \"${key}\"."
      echo
      exit 1
      ;;
    *)    # unknown option
  esac
done


if [[ "${COMPILER}" == "gcc" ]]; then
  declare -r DOCKER_FILE="Dockerfile-gcc"
  if [[ "${GCC_VER}" == "" ]]; then
    echo "ERROR: You need to give a gcc version."
    echo
    exit 1
  fi
  CLANG_VER="x"
  declare -r COMPILER_VER=${GCC_VER}
elif [[ "${COMPILER}" == "clang" ]]; then
  declare -r DOCKER_FILE="Dockerfile-clang"
  if [[ "${CLANG_VER}" == "" ]]; then
    echo "ERROR: You need to give a clang version."
    echo
    exit 1
  fi
  declare -r COMPILER_VER=${CLANG_VER}
else
  echo "ERROR: You need to use one of the flags \"--clang\" or \"--gcc\"."
  echo
  exit 1
fi

declare -r BUILD_IMAGE="$(get_docker_build_image ${DOCKER_OS_TYPE} ${DOCKER_OS_TAG} ${DOCKER_TAG} ${COMPILER} ${COMPILER_VER})"


echo "Building \"${BUILD_IMAGE}\" for compiler \"${COMPILER}\"..."
echo

pushd "${THIS_SCRIPT_PATH}" > /dev/null

declare -r BUILD_ARGS="--build-arg OS_TYPE=${DOCKER_OS_TYPE} \
                       --build-arg OS_TAG=${DOCKER_OS_TAG}   \
                       --build-arg CLANG_VER=${CLANG_VER}   \
                       --build-arg UID=$(id -u)"

docker build ${NO_CACHE} ${BUILD_ARGS} -t ${BUILD_IMAGE} -f "${DOCKER_FILE}" .

docker system prune -f

popd > /dev/null

