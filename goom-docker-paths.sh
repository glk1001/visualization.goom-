declare -r DOCKERIZE_KODI_DIR=${THIS_SCRIPT_PATH}/dockerize-kodi-goom
if [[ ! -d "${DOCKERIZE_KODI_DIR}" ]]; then
  echo "ERROR: Could not find dockerize kodi directory \"${DOCKERIZE_KODI_DIR}\"."
  exit 1
fi


declare -r KODI_BUILD_ROOT_DIR=$(realpath ${THIS_SCRIPT_PATH}/../xbmc/kodi-build/addons)

declare -r KODI_BUILD_LIB_DIR=${KODI_BUILD_ROOT_DIR}/lib/addons/visualization.goom
if [[ ! -d "${KODI_BUILD_LIB_DIR}" ]]; then
  echo "ERROR: Could not find kodi build lib directory \"${KODI_BUILD_LIB_DIR}\"."
  exit 1
fi
declare -r KODI_BUILD_SHARE_DIR=${KODI_BUILD_ROOT_DIR}/share/kodi/addons/visualization.goom
if [[ ! -d "${KODI_BUILD_SHARE_DIR}" ]]; then
  echo "ERROR: Could not find kodi build share directory \"${KODI_BUILD_SHARE_DIR}\"."
  exit 1
fi
declare -r KODI_BUILD_RESOURCES_DIR=${KODI_BUILD_SHARE_DIR}/resources
if [[ ! -d "${KODI_BUILD_RESOURCES_DIR}" ]]; then
  echo "ERROR: Could not find kodi build resources directory \"${KODI_BUILD_RESOURCES_DIR}\"."
  exit 1
fi

declare -r KODI_DOCKER_FILES_DIR=${THIS_SCRIPT_PATH}/dockerize-kodi-goom/files
if [[ ! -d "${KODI_DOCKER_FILES_DIR}" ]]; then
  echo "ERROR: Could not find kodi docker files directory \"${KODI_DOCKER_FILES_DIR}\"."
  exit 1
fi
declare -r KODI_DOCKER_RESOURCES_DIR=${KODI_DOCKER_FILES_DIR}/resources
if [[ ! -d "${KODI_DOCKER_RESOURCES_DIR}" ]]; then
  echo "ERROR: Could not find kodi docker resources directory \"${KODI_DOCKER_RESOURCES_DIR}\"."
  exit 1
fi


declare -r KODI_IMAGE_NAME="ubuntu-impish/kodi-matrix:goom-dev"
declare -r KODI_CONTAINER_HOME_DIR="${HOME}/docker/kodi-matrix"
declare -r REMOTE_KODI_HOME_DIR="${HOME}/Prj/Docker/kodi-matrix"

#declare -r KODI_IMAGE_NAME="ubuntu-impish/kodi-nightly:goom-dev"
#declare -r KODI_CONTAINER_HOME_DIR="${HOME}/docker/kodi-nightly"
#declare -r REMOTE_KODI_HOME_DIR="${HOME}/Prj/Docker/kodi-nightly"

declare -r KODI_CONTAINER_NAME="kodi_goom_dev"
declare -r MUSIC_SHARE="/mnt/Music"
