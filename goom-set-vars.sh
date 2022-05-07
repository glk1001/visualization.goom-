declare EXTRA_ARGS=""

while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    -b|--build)
      declare -r BUILD_DIRNAME=${2}
      shift # past argument
      shift # past value
      ;;
    -c|--compiler)
      COMPILER=${2}
      shift # past argument
      shift # past value
      ;;
    --build-type)
      BUILD_TYPE=${2}
      shift # past argument
      shift # past value
      ;;
    *)
      EXTRA_ARGS="${EXTRA_ARGS}${key} "
      shift # past argument
      ;;
    *)
  esac
done

set -- ${EXTRA_ARGS}
unset EXTRA_ARGS

if [[ "${COMPILER:-}" == "" ]]; then
  echo "Compiler must be specified."
  echo
  exit 1
fi
if [[ "${BUILD_TYPE:-}" == "" ]]; then
  echo "Build type must be specified."
  echo
  exit 1
fi

if [[ "${COMPILER}" == "gcc-11" ]]; then
  declare -r C_COMPILER=gcc-11
  declare -r CPP_COMPILER=g++-11
elif [[ "${COMPILER}" == "gcc-12" ]]; then
  declare -r C_COMPILER=gcc-12
  declare -r CPP_COMPILER=g++-12
elif [[ "${COMPILER}" == "clang-13" ]]; then
  declare -r C_COMPILER=clang-13
  declare -r CPP_COMPILER=clang-13
elif [[ "${COMPILER}" == "clang-14" ]]; then
  declare -r C_COMPILER=clang-14
  declare -r CPP_COMPILER=clang-14
elif [[ "${COMPILER}" == "clang-15" ]]; then
  declare -r C_COMPILER=clang-15
  declare -r CPP_COMPILER=clang-15
else
  echo "Unknown compiler \"${COMPILER}\"."
  echo
  exit 1
fi

if [[ "${BUILD_TYPE}" == "Debug" ]]; then
  declare -r C_BUILD_TYPE=Debug
elif [[ "${BUILD_TYPE}" == "Release" ]]; then
  declare -r C_BUILD_TYPE=Release
elif [[ "${BUILD_TYPE}" == "RelWithDebInfo" ]]; then
  declare -r C_BUILD_TYPE=RelWithDebInfo
else
  echo "Unknown build type \"${BUILD_TYPE}\"."
  echo
  exit 1
fi

if [[ "${BUILD_DIRNAME:-}" == "" ]]; then
  declare -r BUILD_DIRNAME=build-${C_COMPILER}-${C_BUILD_TYPE}
  echo "Using BUILD_DIRNAME \"${BUILD_DIRNAME}\"."
  echo
fi
if [[ ${BUILD_DIRNAME} != build* ]]; then
  echo "ERROR: Build dirname must start with \"build\". Not this: BUILD_DIRNAME = \"${BUILD_DIRNAME}\""
  exit 1
fi

declare -r BUILD_DIR=${THIS_SCRIPT_PATH}/${BUILD_DIRNAME}

