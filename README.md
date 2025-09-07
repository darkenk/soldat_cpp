# SOLDAT_CPP
This is conversion of [Soldat](https://github.com/Soldat/soldat) from Pascal to C++.
Conversion was made with help of [ptoc](https://github.com/knizhnik/ptoc). Both client and server are converted.
The big missing part is scriptcore and pascalscript in server. Places with missing code (mostly due to converter issue)
are marked by `NotImplemented()` function.

This is rather a toy project, to test various ideas.

# Current external dependencies (**Outdated**)
* freetype-2.10.4
* GameNetworkingSockets-1.3.0
* glad
* googletest-1.10.0
* libsodium-1.0.18
* physfs-3.0.2
* protobuf-3.19.1
* SDL2-2.0.20
* spdlog-1.9.2
* stb
* SoLoud
* OpenGL
* Vulkan
* SHA1
* Tracy-0.7.8

# Build
Currently only linux x64 is actively developed.

## Targets

Press `Shift + F7`:
* SoldatGame - builds game binary
* SoldatGame_full - builds game + assets
* third_party_dependencies - builds dependencies in third_party directory


## Debugging
### Mouse locking while debugging on linux
When game triggers breakpoint under debugger, mouse is locked to game and thus you cannot use it.
To overcome it in gdb execute:
```bash
source ${PROJECT}/build/gdb/release_mouse.py
```

## Dev containers
* execute those commands in main repo directory (it creates directories outside the repo)
```bash
mkdir -p ../soldat_container/ccache
mkdir -p ../soldat_container/soldat_build
```

* rebuild devcontainer
  - `ctrl + shift + p` and press `Dev Containers: Rebuild and Reopen in Container`
