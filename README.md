# visualization.goom addon for Kodi

This is a [Kodi](http://kodi.tv) visualization addon.

[![License: GPL-2.0-or-later](https://img.shields.io/badge/License-GPL%20v2+-blue.svg)](LICENSE.md)
[![Build Status](https://dev.azure.com/teamkodi/binary-addons/_apis/build/status/xbmc.visualization.goom?branchName=Nexus)](https://dev.azure.com/teamkodi/binary-addons/_build/latest?definitionId=38&branchName=Nexus)
[![Build Status](https://jenkins.kodi.tv/view/Addons/job/xbmc/job/visualization.goom/job/Nexus/badge/icon)](https://jenkins.kodi.tv/blue/organizations/jenkins/xbmc%2Fvisualization.goom/branches/)

## Build instructions

When building the addon you have to use the correct branch depending on which version of Kodi you're building against.
If you want to build the addon to be compatible with the latest kodi `'master'` commit, you need to checkout the branch with the current Kodi codename. For example, if you, the latest Kodi codename is `'Nexus'`, then your current addon branch must be `'Nexus'`.
Also make sure you follow this README from the branch in question.

### Linux

The following instructions assume, as an example, the `'Nexus'` branch. You need to have already built Kodi in the `'kodi-build'` directory suggested by the README.

1. `git clone --branch master https://github.com/xbmc/xbmc.git`
1. `git clone --branch Nexus https://github.com/xbmc/visualization.goom.git`
1. `cd visualization.goom`
1. `./goom-configure.sh -b <build_dir> -c <compiler: 'gcc' or 'clang'>`
1. `./goom-build.sh -b <build_dir> -c <compiler: 'gcc' or 'clang'>`
1. `./goom-docker-install-to.sh && ./goom-docker-build.sh`
1. `./goom-docker-run.sh`

The build addon files will be placed in `'../xbmc/kodi-build/addons/visualization.goom'`.
So if you build Kodi from source and run it directly, the addon will be available as a system addon.

Alternatively, the `goom-docker-...` scripts take the Kodi goom build files and install them in a Docker image.
This allows you to run Kodi with the freshly built goom add-on using the `./goom-docker-run.sh` script.

Currently, the Docker scripts are set up for the Kodi nightly builds.
