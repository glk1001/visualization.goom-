declare -r KODI_DOCKER_DIR=${THIS_SCRIPT_PATH}/dockerize-kodi-goom
if [[ ! -d "${KODI_DOCKER_DIR}" ]]; then
  echo "ERROR: Could not find kodi docker directory \"${KODI_DOCKER_DIR}\"."
  exit 1
fi

declare -r REMOTE_KODI_HOME_DIR="${HOME}/Prj/Docker/kodi-nightly"
