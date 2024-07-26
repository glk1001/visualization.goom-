# Visualization.goom-pp: Binary Add-on for Kodi

This is a [Kodi](http://kodi.tv) visualization add-on.

[![License: GPL-2.0-or-later](https://img.shields.io/badge/License-GPL%20v2+-blue.svg)](LICENSE.md)

## Linux Build Instructions

### Initial Setup

1. mkdir -p ${HOME}/Prj/github/xbmc
1. cd ${HOME}/Prj/github/xbmc
1. git clone --recurse-submodules https://github.com/glk1001/visualization.goom-pp.git
1. git clone --single-branch --branch Omega https://github.com/xbmc/xbmc.git kodi

### Building (assuming clang-18 installed)

#### CLion
The project has CLion .idea files configured for cmake and clang-18 or clang-19.
Once you've completed the above initial setup, CLion should be able to load 
and build the cmake project.

#### Command Line
1. `cd ${HOME}/Prj/github/xbmc/visualization.goom-pp`
1. `bash scripts/goom-configure.sh -c clang-18 --build-type RelWithDebInfo --suffix omega`
1. `bash scripts/goom-build.sh -c clang-18 --build-type RelWithDebInfo --suffix omega`

The build files will be installed into `'${HOME}/Prj/github/xbmc/visualization.goom-pp/build-clang-18-RelWithDebInfo-omega/kodi-vis-goom-addon'`.

#### Docker Toolchains
You can also build with Docker toolchains. For example, to make a 
clang-19 Docker toolchain image do:
1. `cd ${HOME}/Prj/github/xbmc/visualization.goom-pp/docker-toolchains`
1. `bash build-docker-image.sh --docker-os-type ubuntu --docker-os-tag 24.04 --clang --clang-ver 19` 

## Adding Goom++ to Kodi
After a successful build do the following (assuming you built with CLion and clang-18):
1. `cd ${HOME}/Prj/github/xbmc/visualization.goom-pp`
1. `bash scripts/goom-flatpak-install-to.sh --clion -c clang-18 --build-type RelWithDebInfo --suffix omega` 
1. `flatpak --filesystem=home run --devel tv.kodi.Kodi`
