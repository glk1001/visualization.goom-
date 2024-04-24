ARG OS_TYPE
ARG OS_TAG

FROM ${OS_TYPE}:${OS_TAG}

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update    && \
    apt-get install -y   \
      tzdata             \
      libfreetype-dev    \
      libglu1-mesa-dev   \
      mesa-common-dev    \
      libglfw3-dev       \
      python3-dev        \
      python3-matplotlib \
      locales-all        \
      make               \
      autoconf           \
      automake           \
      cmake              \
      ninja-build        \
      dos2unix           \
      rsync              \
      tar                \
      fd-find            \
      git                \
      catch2             \
      ccache             \
      build-essential    \
      gcc-14             \
      g++-14             \
      clang-18           \
      clang-tools-18     \
      clang-tidy-18      \
      clang-format-18    \
      libboost-all-dev   \
      gdb             && \
    apt-get clean -y

# Install Clang
ARG CLANG_VER=""

COPY "llvm.sh" /tmp/llvm.sh
WORKDIR /tmp

RUN apt-get update                                       && \
    apt-get install -y                                      \
      --no-install-recommends software-properties-common    \
      lsb-release                                           \
      wget

RUN if [ -n "${CLANG_VER}" ] ; then bash llvm.sh ${CLANG_VER} && apt-get install -y clang-tidy-${CLANG_VER} clang-format-${CLANG_VER} ; fi

RUN apt-get clean -y

ARG BUILD_UID=1001
RUN useradd -m -u ${BUILD_UID} -s /bin/bash builder
USER builder
