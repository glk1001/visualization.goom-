function get_this_sourced_script_path()
{
  local dirname=""
  for i in "${BASH_SOURCE[@]}"; do
    dirname=$(dirname $i)
    filename=$(basename $i)
    if [[ "${filename}" == "${GOOM_GET_PATHS_SCRIPT_NAME}" ]]; then
      break
    fi
  done

  echo ${dirname}
}

declare -r GOOM_GET_PATHS_SCRIPT_NAME="goom-get-paths.sh"
declare -r GOOM_GET_PATHS_SCRIPT_PATH=$(get_this_sourced_script_path)
declare -r GOOM_MAIN_ROOT_DIR=$(realpath ${GOOM_GET_PATHS_SCRIPT_PATH}/..)

declare -r HOST_KODI_ROOT_DIR=$(realpath ${GOOM_MAIN_ROOT_DIR}/..)
declare -r HOST_CCACHE_DIR=${CCACHE_DIR}
declare -r DOCKER_CCACHE_DIR=/tmp/ccache
declare -r DOCKER_KODI_ROOT_DIR=/tmp/xbmc
declare -r DOCKER_GOOM_MAIN_ROOT_DIR=/tmp/xbmc/visualization.goom-pp
declare -r DOCKER_GOOM_SCRIPTS_DIR=${DOCKER_GOOM_MAIN_ROOT_DIR}/scripts

declare -r KODI_BUILD_ROOT_DIR=${HOST_KODI_ROOT_DIR}/kodi/kodi-build/addons
declare -r KODI_BUILD_LIB_DIR=${KODI_BUILD_ROOT_DIR}/lib/kodi/addons/visualization.goom-pp
declare -r KODI_BUILD_SHARE_DIR=${KODI_BUILD_ROOT_DIR}/share/kodi/addons/visualization.goom-pp
declare -r KODI_BUILD_RESOURCES_DIR=${KODI_BUILD_SHARE_DIR}/resources
