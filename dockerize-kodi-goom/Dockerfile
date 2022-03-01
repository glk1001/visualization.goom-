FROM kodi-nightly:lirc

# Copy the built goom files to kodi system directories.
ARG GOOM_VERSION=20.0.1
ARG KODI_GOOM_LIB_DIR=/usr/lib/x86_64-linux-gnu/kodi/addons/visualization.goom
ARG KODI_GOOM_ADDON_DIR=/usr/share/kodi/addons/visualization.goom
RUN rm -rf "${KODI_GOOM_ADDON_DIR}/*"
COPY "files/visualization.goom.so.${GOOM_VERSION}" ${KODI_GOOM_LIB_DIR}/visualization.goom.so.${GOOM_VERSION}
ADD  "files/addon.xml"                             ${KODI_GOOM_ADDON_DIR}/
ADD  "files/resources"                             ${KODI_GOOM_ADDON_DIR}/resources/

COPY "files/irexec.lircrc" /etc/lirc/irexec.lircrc

COPY entrypoint.sh /usr/local/bin
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]

# TO BUILD KODI DOCKER:
# docker build -t kodi-nightly:base     -f Dockerfile-base .
# docker build -t kodi-nightly:spotify  -f Dockerfile-spotify-dependencies .
# docker build -t kodi-nightly:lirc     -f Dockerfile-lirc .
# docker build -t kodi-nightly:goom-dev -f Dockerfile .

# TO RUN KODI DOCKER GUI:
# x11docker --runasroot="service lircd start" --network --pulseaudio --gpu --share /mnt/Music
#           --home=${HOME}/docker/kodi-nightly --name kodi-goom-dev -- --privileged -- kodi-nightly:goom-dev
