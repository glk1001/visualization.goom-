if [[ "${1:-}" != "" ]]; then
  declare -r BUILD_DIRNAME=${1}
else  
  declare -r BUILD_DIRNAME=build
  echo "USING DEFAULT BUILD DIR \"${BUILD_DIRNAME}\"."
  echo
fi
if [[ ${BUILD_DIRNAME} != build* ]]; then
  echo "ERROR: Build dirname must start with \"build\". Not this BUILD_DIRNAME = \"${BUILD_DIRNAME}\""
  exit 1
fi

declare -r BUILD_DIR=${THIS_SCRIPT_PATH}/${BUILD_DIRNAME}
