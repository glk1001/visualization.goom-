#!/bin/bash

set -u


declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"


declare -r GLSL_DIR="$(realpath ${THIS_SCRIPT_PATH}/../visualization.goom-pp/resources/data/shaders/GL)"
declare -r GLSL_INCLUDE_DIR="${GLSL_DIR}"
declare -r GLSL_VALIDATOR="glslangValidator"
declare -r GLSL_INCLUDE_EXPANDER="expand_includes"

if ! command -v "${GLSL_VALIDATOR}" &> /dev/null ; then
  echo "ERROR: Glsl validator program \"${GLSL_VALIDATOR}\" not found."
  exit 1
fi
if ! command -v "${GLSL_INCLUDE_EXPANDER}" &> /dev/null ; then
  echo "ERROR: Glsl include expander program \"${GLSL_INCLUDE_EXPANDER}\" not found."
  exit 1
fi


declare -r IN_FILE="$1"
declare -r IN_FILENAME=$(basename "${IN_FILE}")

declare -r IN_FILE_EXT="${IN_FILENAME##*.}"
if [[ "${IN_FILE_EXT}" == "fs" ]]; then
  declare -r SHADER_STAGE="frag"
elif [[ "${IN_FILE_EXT}" == "vs" ]]; then
  declare -r SHADER_STAGE="vert"
elif [[ "${IN_FILE_EXT}" == "cs" ]]; then
  declare -r SHADER_STAGE="comp"
else
  echo "ERROR: Unknown shader stage: \"${IN_FILE_EXT}\"."
  exit 1
fi


if [[ "${IN_FILE}" == "${IN_FILENAME}" ]]; then
  declare -r FULL_IN_FILE="${GLSL_DIR}/${IN_FILE}"
else
  declare -r FULL_IN_FILE="$(realpath ${IN_FILE})"
fi
if [[ ! -f "${FULL_IN_FILE}" ]]; then
  echo "ERROR: Could not find input file: \"${FULL_IN_FILE}\"."
  exit 1
fi

declare -r FILE_TO_VALIDATE="/tmp/${IN_FILENAME}"


rm -f "${FILE_TO_VALIDATE}"

${GLSL_INCLUDE_EXPANDER} -I "${GLSL_INCLUDE_DIR}" -i "${FULL_IN_FILE}" -o "${FILE_TO_VALIDATE}"
if [[ $? != 0 ]]; then
  exit 1
fi

sed -i 's/$.*;/0;/g' ${FILE_TO_VALIDATE}
sed -i 's/$.*/0/g' ${FILE_TO_VALIDATE}

echo

${GLSL_VALIDATOR} -S ${SHADER_STAGE} ${FILE_TO_VALIDATE}
if [[ $? == 0 ]]; then
  echo "No errors in '${SHADER_STAGE}' shader file \"${FULL_IN_FILE}\"."
else
  echo "There were errors in '${SHADER_STAGE}' shader file \"${FULL_IN_FILE}\"."
  exit 1
fi
