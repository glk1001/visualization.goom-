ARG KODI_LIRC_IMAGE

FROM ${KODI_LIRC_IMAGE}

# Make sure we have a 'libstdc++.so' compatible with our build tools.
# We need this, for example, when building a Docker image with Ubuntu
# Impish but compiling goom on Ubuntu Jammy.
ARG LIBSTDCPP_VERSION=6.0.30
ARG LIBSTDCPP_DIR=/usr/lib/x86_64-linux-gnu
COPY "files/libstdc++.so.${LIBSTDCPP_VERSION}" ${LIBSTDCPP_DIR}/libstdc++.so.${LIBSTDCPP_VERSION}
RUN cd ${LIBSTDCPP_DIR} && ln -sf libstdc++.so.${LIBSTDCPP_VERSION} libstdc++.so.6

# Copy the built goom files to kodi add-on directories.
ARG GOOM_VERSION
ARG KODI_GOOM_LIB_DIR
ARG KODI_GOOM_ADDON_DIR
RUN rm -rf "${KODI_GOOM_ADDON_DIR}/*"
COPY "files/visualization.goom.so.${GOOM_VERSION}" ${KODI_GOOM_LIB_DIR}/visualization.goom.so.${GOOM_VERSION}
ADD  "files/addon.xml"                             ${KODI_GOOM_ADDON_DIR}/
ADD  "files/resources"                             ${KODI_GOOM_ADDON_DIR}/resources/

COPY "files/irexec.lircrc" /etc/lirc/irexec.lircrc

COPY entrypoint.sh /usr/local/bin
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
