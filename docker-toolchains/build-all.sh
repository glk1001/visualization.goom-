#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

source "${THIS_SCRIPT_PATH}/build-get-vars.sh"


declare NO_CACHE=""
declare DOCKER_OS_TYPE="${DOCKER_BUILD_OS_TYPE}"
declare DOCKER_OS_TAG="${DOCKER_BUILD_OS_TAG}"

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
    --clang)
      COMPILER=clang
      shift # past argument
      ;;
    --gcc)
      COMPILER=gcc
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


if [[ "${COMPILER}" == "clang" ]]; then
  declare -r DOCKER_FILE="Dockerfile-clang"
elif [[ "${COMPILER}" == "gcc" ]]; then
  declare -r DOCKER_FILE="Dockerfile-gcc"
else
  echo "ERROR: You need to use one of the flags \"--clang\" or \"--gcc\"."
  echo
  exit 1
fi

declare -r BUILD_IMAGE="$(get_docker_build_image ${DOCKER_OS_TYPE} ${DOCKER_OS_TAG})"


echo "Building \"${BUILD_IMAGE}\" for compiler \"${COMPILER}\"..."
echo

pushd "${THIS_SCRIPT_PATH}" > /dev/null

declare -r BUILD_ARGS="--build-arg OS_TYPE=${DOCKER_OS_TYPE} \
                       --build-arg OS_TAG=${DOCKER_OS_TAG}   \
                       --build-arg UID=$(id -u)"

docker build ${NO_CACHE} ${BUILD_ARGS} -t ${BUILD_IMAGE} -f "${DOCKER_FILE}" .

docker system prune -f

popd > /dev/null
