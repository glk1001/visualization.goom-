function get_docker_build_image()
{
    local -r os_type="$1"
    local -r os_tag="$2"
    local -r docker_tag="$3"
    local -r compiler="$4"
    local -r compiler_ver="$5"

    echo "cpp-toolchain/${os_type}-${os_tag}/${compiler}-${compiler_ver}:${docker_tag}"
}

function get_docker_toolchain_tag()
{
    local -r docker_tag="$1"
    local -r compiler="$2"
    local -r compiler_ver="$3"

    echo "cpp-toolchain/${compiler}-${compiler_ver}:${docker_tag}"
}

declare -r DOCKER_BUILD_OS_TYPE=ubuntu
declare -r DOCKER_BUILD_OS_TAG=24.04
declare -r DOCKER_BUILD_TAG=1.1
