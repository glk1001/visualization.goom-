#!/bin/bash

set -u

declare -r THIS_SCRIPT_PATH="$(cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P)"

declare SPELL_DIR=$(realpath "${THIS_SCRIPT_PATH}/..")
declare SPELL_FILE="${SPELL_DIR}/goom-spell.dic"
declare EXCLUDES="--exclude-dir='build-*' --exclude-dir='.git' --exclude-dir='.idea' --exclude='${SPELL_FILE}'"

if [[ ! -f "${SPELL_FILE}" ]]; then
  echo "ERROR: Could not find spell file \"${SPELL_FILE}\"."
  exit 1
fi

while IFS= read -r line; do
    grep --exclude-dir='build-*' --exclude-dir='.git' --exclude-dir='.idea' --exclude="${SPELL_FILE}" \
         -q -i -r "$line" ${SPELL_DIR}
    if [[ $? != 0 ]]; then
      echo "Could not find word: \"$line\"."
    fi
done < "${SPELL_FILE}"
