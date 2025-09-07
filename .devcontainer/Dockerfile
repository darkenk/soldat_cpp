FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04

COPY update-alternatives.sh update-alternatives.sh

# cmake 4.0
RUN apt-get update && apt-get -y install ca-certificates gpg wget software-properties-common

RUN test -f /usr/share/doc/kitware-archive-keyring/copyright || \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

RUN apt-get remove -y clang-18 llvm-18

# clang
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 20 all

# install packages
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends \
    cmake \
    ccache \
    libvulkan-dev \ 
    libwayland-dev \
    wayland-protocols \
    libxkbcommon-dev \
    libegl1-mesa-dev \
    libgles2-mesa-dev \
    libdrm-dev \
    libgbm-dev \
    libxcursor-dev \
    libxi-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxext-dev \
    pkg-config \
    libpipewire-0.3-dev \
    pipewire-bin \
    libvulkan1 \
    vulkan-tools \
    mesa-vulkan-drivers \
    mesa-utils \
    mold

RUN apt-get autoremove -y && apt-get clean -y

RUN ./update-alternatives.sh

