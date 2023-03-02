# Visualization.goom Binary Add-on for Kodi

This is a [Kodi](http://kodi.tv) visualization add-on.

[![License: GPL-2.0-or-later](https://img.shields.io/badge/License-GPL%20v2+-blue.svg)](LICENSE.md)

## Build Instructions

When building the add-on you have to use the correct branch depending on which version of Kodi you're
building against. If you want to build the add-on to be compatible with the latest kodi `'Nexus'` commit,
then you need to checkout the `'Nexus'` add-on branch. Make sure you follow the README from the branch
in question.

Note: For goom++ development, Docker toolchains are used to make sure the right compiler and tool versions
are available. (Currently 'clang-17' and 'g++-12'.)

### Linux Builds

The following instructions assume, as an example, the `'Nexus'` branch, and that your current local
directory is `'xbmc'`

1. `git clone --branch Nexus https://github.com/xbmc/xbmc.git` kodi
1. `git clone --branch Nexus https://github.com/xbmc/visualization.goom-pp.git`
1. `cd visualization.goom-pp`
1. `cd docker-toolchains`
1. `bash build-all.sh`
1. `cd ..`
1. `bash scripts/goom-configure-with-docker-toolchain.sh -c clang-17 --build-type RelWithDebInfo --suffix nexus`
1. `bash scripts/goom-build-with-docker-toolchain.sh -c clang-17 --build-type RelWithDebInfo --suffix nexus`
1. `bash scripts/goom-docker-build-run.sh --user-addon`
1. `cd ..`

The build add-on files will be placed in `'./kodi/kodi-build/addons/lib/kodi/addons/visualization.goom-pp'`
and `'./kodi/kodi-build/addons/share/kodi/addons/visualization.goom-pp'`. So if you build Kodi from source
and run it directly, then the add-on will be available as a system add-on.

Alternatively, the `scripts/goom-docker-...` bash scripts create a Docker image from the latest `'Nexus'` Kodi
build and the latest goom++ build files. This allows you to run a current `'Nexus'` Kodi build with a freshly
built goom++ add-on in Docker using the `goom-docker-run.sh` script. See below.

You can also build from within the kodi source tree using the following: Assuming your current local directory is `'xbmc'`:

1. `cd kodi`
1. `sudo make -j$(getconf _NPROCESSORS_ONLN) -C tools/depends/target/binary-addons PREFIX=/usr/local ADDONS="visualization.goom-pp"`

This will put the output from the build in:

1. `/usr/local/lib/kodi/addons/visualization.goom-pp`
1. `/usr/local/share/kodi/addons/visualization.goom-pp`

### Running Kodi in a Docker Container

You can run the latest Kodi `'Nexus'` build in a Docker container with a fresh goom++ build. To run
the container with X11 you need to install `'x11docker'`. This is fairly straightforward: See
`https://github.com/mviereck/x11docker#installation`

Once you've installed `'x11docker'` you can install the required goom++ files, build the Kodi Docker images,
then run Kodi Docker. Here are the steps:

First set up a symlink, '/mnt/Music', pointing to your music directory.
Then run the following scripts:

1. `bash scripts/goom-docker-install-to.sh`
2. `bash scripts/goom-docker-build.sh`
3. `bash scripts/goom-docker-run.sh`

or simply use:

1. `bash scripts/goom-docker-build-run.sh`

Kodi should now start in full-screen mode. To enable the Goom visualisation do the following:

1. For the first run of Kodi, it should pop up a 'Disabled add-ons' dialog box. Say 'yes' to enable the 'Goom'
add-on.
1. Set the Goom options to whatever you want.
1. Go to the Kodi system settings for 'Player' and change the 'Music/Visualisation' setting to 'Goom'.
1. Go back to the Kodi main menu and choose 'Music/Files'. You can add the 'Music' folder using 'Add music'.<br>
   (The Docker container has mounted a 'Music' folder to the host directory '/mnt/Music'.)
1. Now you can play a music track and the Goom visualisation should run with it.

Note: The Kodi Docker container uses `'${HOME}/docker/kodi-nexus'` as it's home directory. Kodi will install
the files it requires under `'${HOME}/docker/kodi-nexus/.kodi'`.
