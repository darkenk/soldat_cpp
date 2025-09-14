FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-24.04

COPY update-alternatives.sh update-alternatives.sh
COPY VisualStudio.17.Release.chman vs_manifest.json

# cmake 4.0
RUN dpkg --add-architecture i386 && apt-get update && apt-get -y install ca-certificates gpg wget software-properties-common

RUN test -f /usr/share/doc/kitware-archive-keyring/copyright || \
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

RUN wget -O /etc/apt/keyrings/winehq-archive.key https://dl.winehq.org/wine-builds/winehq.key
RUN wget -NP /etc/apt/sources.list.d/ https://dl.winehq.org/wine-builds/ubuntu/dists/noble/winehq-noble.sources

RUN apt-get remove -y clang-18 llvm-18

# clang
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 21 all

RUN set -eux; \
    xwin_version="0.6.7"; \
    xwin_prefix="xwin-$xwin_version-x86_64-unknown-linux-musl"; \
    # Install xwin to cargo/bin via github release. Note you could also just use `cargo install xwin`.
    curl --fail -L https://github.com/Jake-Shadle/xwin/releases/download/$xwin_version/$xwin_prefix.tar.gz | tar -xzv -C /usr/local/bin --strip-components=1 $xwin_prefix/xwin; \
    # Splat the CRT and SDK files to /xwin/crt and /xwin/sdk respectively
    for i in {1..20}; do xwin --manifest vs_manifest.json --accept-license download && break || { echo "xwin download attempt $i failed"; }; done || exit 1; \
    xwin --manifest vs_manifest.json -L debug --accept-license splat --preserve-ms-arch-notation --use-winsysroot-style  --include-debug-libs --output /winsysroot;  \
    ln -s /winsysroot/Windows\ Kits/10/include /winsysroot/Windows\ Kits/10/Include; \
    ln -s /winsysroot/Windows\ Kits/10/lib /winsysroot/Windows\ Kits/10/Lib; \   
    # Remove unneeded files to reduce image size
    rm -rf .xwin-cache /usr/local/bin/xwin;

#RUN curl --fail -L https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2505.1/linux_dxc_2025_07_14.x86_64.tar.gz | tar -xzv -C /usr/local

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
    libfmt-dev \
    mold

RUN sudo apt-get -y install --install-recommends winehq-stable
RUN wine wineboot --init

RUN apt-get autoremove -y && apt-get clean -y

RUN ./update-alternatives.sh

