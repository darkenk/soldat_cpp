# SOLDAT_CPP
This is conversion of [Soldat](https://github.com/Soldat/soldat) from Pascal to C++.
Conversion was made with help of [ptoc](https://github.com/knizhnik/ptoc). Both client and server are converted.
The big missing part is scriptcore and pascalscript in server. Places with missing code (mostly due to converter issue)
are marked by `NotImplemented()` function.

To test client and server you need [assets](https://github.com/Soldat/base).

# Current external dependencies
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
* SHA1
* Tracy-0.7.8

# Build
Currently only linux x64 and linux aarch64 are supported.
## Requirements
* Clang-12
* cmake-3.18
* python-3
* automake
* autotools-dev

## Steps
```bash
cd build
./setup.py
mkdir out/linux_x86
cd out/linux_x86
cmake ../../../ -DCMAKE_BUILD_TYPE:STRING=Debug
cmake --build --target SoldatClient SoldatServer
```

## Debugging
### Mouse locking while debugging on linux
When game triggers breakpoint under debugger, mouse is locked to game and thus you cannot use it.
To overcome it in gdb execute:
```bash
source ${PROJECT}/build/gdb/release_mouse.py
```

