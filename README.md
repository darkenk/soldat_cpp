# SOLDAT_CPP
This is conversion of [Soldat](https://github.com/Soldat/soldat) from Pascal to C++.
Conversion was made with help of [ptoc](https://github.com/knizhnik/ptoc). Both client and server are converted.
The big missing part is scriptcore and pascalscript in server. Places with missing code (mostly due to converter issue)
are marked by `NotImplemented()` function.

Current focus is on client part, so server can be inoperable. To test client you need [assets](https://github.com/Soldat/base)
and Pascal version of server.

# Current external dependencies
* freetype-2.10.4
* GameNetworkingSockets-1.3.0
* glad
* googletest-1.10.0
* libsodium-1.0.18
* physfs-3.0.2
* protobuf-3.19.1
* SDL2-2.0.14
* spdlog-1.9.2
* stb
* SoLoud
* OpenGL
* SHA1

# Build
Currently only linux x86 is supported.
## Requirements
* Clang-12
* cmake-3.18
* python-3

## Steps
```bash
cd build
./setup.py
mkdir out/linux_x86
cd out/linux_x86
cmake ../../../ -DCMAKE_BUILD_TYPE:STRING=Debug
cmake --build --target SoldatClient
```
