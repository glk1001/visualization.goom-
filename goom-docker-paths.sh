function get_this_sourced_script_path()
{
  local dirname=""
  for i in "${BASH_SOURCE[@]}"; do
    dirname=$(dirname $i)
    filename=$(basename $i)
    if [[ "${filename}" == "${GOOM_DOCKER_PATHS_SCRIPT_NAME}" ]]; then
      break
    fi
  done

  echo ${dirname}
}

declare -r GOOM_DOCKER_PATHS_SCRIPT_NAME="goom-docker-paths.sh"
declare -r GOOM_DOCKER_PATHS_SCRIPT_PATH=$(get_this_sourced_script_path)


declare -r DOCKERIZE_KODI_DIR=${GOOM_DOCKER_PATHS_SCRIPT_PATH}/dockerize-kodi-goom
if [[ ! -d "${DOCKERIZE_KODI_DIR}" ]]; then
  echo "ERROR: Could not find dockerize kodi directory \"${DOCKERIZE_KODI_DIR}\"."
  exit 1
fi

declare -r KODI_DOCKER_FILES_DIR=${GOOM_DOCKER_PATHS_SCRIPT_PATH}/dockerize-kodi-goom/files
if [[ ! -d "${KODI_DOCKER_FILES_DIR}" ]]; then
  echo "ERROR: Could not find kodi docker files directory \"${KODI_DOCKER_FILES_DIR}\"."
  exit 1
fi
declare -r KODI_DOCKER_RESOURCES_DIR=${KODI_DOCKER_FILES_DIR}/resources
if [[ ! -d "${KODI_DOCKER_RESOURCES_DIR}" ]]; then
  echo "ERROR: Could not find kodi docker resources directory \"${KODI_DOCKER_RESOURCES_DIR}\"."
  exit 1
fi


declare -r ADDON_XML="${KODI_DOCKER_FILES_DIR}/addon.xml"
if [[ ! -f "${ADDON_XML}" ]]; then
  echo "ERROR: Could not find add-on xml file \"${ADDON_XML}\"."
  exit 1
fi
declare -r GOOM_VERSION=$(sed -n 's#.*version="\([0-9][0-9].*[\.0-9]\)".*#\1#p' < "${ADDON_XML}")

if [[ "${GOOM_VERSION:0:2}" == "19" ]]; then
  declare -r KODI_VERSION=matrix
elif [[ "${GOOM_VERSION:0:2}" == "20" ]]; then
  declare -r KODI_VERSION=nightly
else
  echo "ERROR: Unknown Goom version: \"${GOOM_VERSION}\" (from ${ADDON_XML})."
  exit 1
fi

declare -r KODI_IMAGE_OS_TYPE=ubuntu
declare -r KODI_IMAGE_OS_TAG=impish
# Use ubuntu impish so we get python 3.9. There is a problem with python 3.10 and sqlite3
# which affects the Spotify plugin. See https://bbs.archlinux.org/viewtopic.php?id=272121

declare -r KODI_IMAGE_NAME="${KODI_IMAGE_OS_TYPE}-${KODI_IMAGE_OS_TAG}/kodi-${KODI_VERSION}"
declare -r KODI_BASE_IMAGE="${KODI_IMAGE_NAME}:base"
declare -r KODI_SPOTIFY_IMAGE="${KODI_IMAGE_NAME}:spotify"
declare -r KODI_LIRC_IMAGE="${KODI_IMAGE_NAME}:lirc"
declare -r KODI_GOOM_IMAGE="${KODI_IMAGE_NAME}:goom-dev"

declare -r KODI_CONTAINER_HOME_DIR="${HOME}/docker/kodi-${KODI_VERSION}"

declare -r KODI_CONTAINER_NAME="kodi_goom_dev"
declare -r MUSIC_SHARE="/mnt/Music"

declare -r REMOTE_KODI_GOOM_DIR="${HOME}/Prj/github/xbmc/visualization.goom"
declare -r REMOTE_KODI_BUILD_DIR="${REMOTE_KODI_GOOM_DIR}/dockerize-kodi-goom"
