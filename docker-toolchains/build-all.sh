#!/bin/bash

set -u
set -e

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

declare -r OS_TYPE=debian
declare -r OS_TAG=bookworm-slim
declare -r CLANG_VER="15"

declare -r BASE_IMAGE="${OS_TYPE}/cpp-env:base-${OS_TAG}"
declare -r TOOLS_IMAGE="${OS_TYPE}/cpp-env:dev-tools-${OS_TAG}"
declare -r CLANG_IMAGE="${OS_TYPE}/cpp-env:clang-${OS_TAG}"
declare -r FINAL_IMAGE="${OS_TYPE}/cpp-env:1.1"

if [[ "${1:-}" == "--no-cache" ]]; then
  echo "Building \"${FINAL_IMAGE}\" with '--no-cache'..."
  declare -r NO_CACHE="--no-cache"
else
  echo "Building \"${FINAL_IMAGE}\"..."
  declare -r NO_CACHE=
fi

cd "${THIS_SCRIPT_PATH}"

declare -r BUILD_ARGS="--build-arg OS_TYPE=${OS_TYPE} --build-arg OS_TAG=${OS_TAG}"

docker build ${NO_CACHE} ${BUILD_ARGS} -t ${BASE_IMAGE}  -f Dockerfile-cpp-env-base      .
docker build ${NO_CACHE} ${BUILD_ARGS} -t ${TOOLS_IMAGE} -f Dockerfile-cpp-env-dev-tools .
docker build ${NO_CACHE} ${BUILD_ARGS} \
              --build-arg CLANG_VER=${CLANG_VER} \
              --build-arg UID=$(id -u) -t ${CLANG_IMAGE} -f Dockerfile-cpp-env-clang     .

docker tag ${CLANG_IMAGE} ${FINAL_IMAGE}

cd -
