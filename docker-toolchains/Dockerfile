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
      gdb                \
      clang-15           \
      clang-tidy-15      \
      gcc-12             \
      g++-12          && \
    apt-get clean -y

RUN apt-get clean -y

ARG UID=1000
RUN useradd -m -u ${UID} -s /bin/bash builder
USER builder
