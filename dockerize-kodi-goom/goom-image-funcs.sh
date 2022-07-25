function get_kodi_image_name()
{
    local -r kodi_image_os_type="$1"
    local -r kodi_image_os_tag="$2"
    local -r kodi_version="$3"

    echo "${kodi_image_os_type}-${kodi_image_os_tag}/kodi-${kodi_version}"
}

function get_kodi_goom_image_name()
{
    local -r kodi_image_name="$1"

    echo "${kodi_image_name}:goom-dev"
}
