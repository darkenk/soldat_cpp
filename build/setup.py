#!/usr/bin/python3
import os
import platform
import subprocess
from enum import Enum
from os import path

BASE_DIR = path.dirname(path.abspath(__file__))
THIRD_PARTY_SRC_DIR = path.join(path.join(path.join(BASE_DIR, '..'), 'third_party'), 'src') + '/'
OUT_DIR = path.join(BASE_DIR, 'out')
JOBS = str(os.cpu_count())

class Platform(Enum):
    LINUX_x64 = 'linux_x64'
    LINUX_aarch64 = 'linux_aarch64'
    WEBASSEMBLY = 'webassembly'

class Config(Enum):
    Release = 'release'
    Debug = 'debug'

# current configuration
PLATFORM = Platform.LINUX_x64
CONFIG = Config.Release

if platform.machine() == 'aarch64':
    PLATFORM = Platform.LINUX_aarch64

# Helpers
def GetThirdPartyOutDir(platform, config):
    return path.join(path.join(path.join(OUT_DIR, platform.value), config.value), 'third_party') + '/'

def GetSodiumOutDir(platform, config):
    return path.join(GetThirdPartyOutDir(platform, config), 'libsodium')

def GetProtobufOutDir(platform, config):
    return path.join(GetThirdPartyOutDir(platform, config), 'protobuf')

def GetLibreSSLOutDir(platform, config):
    return path.join(GetThirdPartyOutDir(platform, config), 'libressl')

def GetCmakeArgBuildType(config):
    if config == Config.Release:
        return '-DCMAKE_BUILD_TYPE=Release'
    else:
        return '-DCMAKE_BUILD_TYPE=Debug'

def GetAdditionalConfigs(platform: Platform):
    if platform == Platform.WEBASSEMBLY:
        return ["-DCMAKE_TOOLCHAIN_FILE=/usr/lib/emscripten/cmake/Modules/Platform/Emscripten.cmake",
                "-DCMAKE_CXX_FLAGS=-pthread", "-DCMAKE_C_FLAGS=-pthread"]
    return []



print(BASE_DIR)

# 3rdparty library compilation instructions

def SetupProtobuf(platform, config):
    PTB_SRC = THIRD_PARTY_SRC_DIR + 'protobuf-3.19.1'
    PTB_DIR = GetProtobufOutDir(platform, config)
    os.makedirs(PTB_DIR, exist_ok = True)
    print("Build protobuf in " + PTB_DIR, flush=True)

    cmake_config = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + PTB_DIR + '/out',
      GetCmakeArgBuildType(config), '-DBUILD_SHARED_LIBS=OFF',
      '-Dprotobuf_BUILD_TESTS=OFF', '-DCMAKE_POSITION_INDEPENDENT_CODE=ON']
    cmake_config = cmake_config + GetAdditionalConfigs(platform)
    cmake_config.append(PTB_SRC + '/cmake')

    subprocess.check_call(cmake_config, cwd=PTB_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=PTB_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=PTB_DIR)

def SetupLibsodium(platform, config):
    SOD_SRC = THIRD_PARTY_SRC_DIR + 'libsodium-1.0.18'
    SOD_DIR = GetSodiumOutDir(platform, config)
    os.makedirs(SOD_DIR, exist_ok = True)

    subprocess.check_call([SOD_SRC + '/configure', '--prefix=' + SOD_DIR + '/out'], cwd=SOD_DIR)
    subprocess.check_call(['make', '-j' + JOBS], cwd=SOD_DIR)
    subprocess.check_call(['make', 'install'], cwd=SOD_DIR)

def SetupLibressl(platform, config):
    SSL_SRC = THIRD_PARTY_SRC_DIR + 'libressl-3.4.3'
    SSL_DIR = GetThirdPartyOutDir(platform, config) + 'libressl'
    os.makedirs(SSL_DIR, exist_ok = True)

    cmake_config = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + SSL_DIR + '/out',
      GetCmakeArgBuildType(config), '-DLIBRESSL_APPS=OFF', '-DLIBRESSL_TESTS=OFF', '-DBUILD_SHARED_LIBS=OFF'
    ]
    cmake_config = cmake_config + GetAdditionalConfigs(platform)
    cmake_config.append(SSL_SRC)

    subprocess.check_call(cmake_config, cwd=SSL_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=SSL_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=SSL_DIR)

def SetupGameNetworkingSockets(platform, config):
    GNS_SRC = THIRD_PARTY_SRC_DIR + 'GameNetworkingSockets-1.3.0'
    GNS_DIR = GetThirdPartyOutDir(platform, config) + 'gamenetworkingsockets'
    os.makedirs(GNS_DIR, exist_ok = True)

    cmake_config = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + GNS_DIR + '/out',
                             GetCmakeArgBuildType(config), '-DUSE_CRYPTO=LibreSSL',
                             '-DUSE_CRYPTO25519=Reference', '-DCMAKE_PREFIX_PATH=' + GetLibreSSLOutDir(platform, config) +'/out/;' + GetProtobufOutDir(platform, config) + '/out/',
                             '-Dprotobuf_BUILD_TESTS=OFF', '-Dprotobuf_BUILD_SHARED_LIBS=OFF',
                             '-Dprotobuf_USE_STATIC_LIBS=ON',
                             '-DLIGHT_TESTS=OFF',
                             '-DGAMENETWORKINGSOCKETS_BUILD_EXAMPLES=OFF',
                             '-DGAMENETWORKINGSOCKETS_BUILD_TESTS=OFF',
                             '-DProtobuf_PROTOC_EXECUTABLE='+ GetProtobufOutDir(Platform.LINUX_x64, config) + '/out/bin/protoc',
                             '-DCMAKE_PROGRAM_PATH='+ GetProtobufOutDir(Platform.LINUX_x64, config) + '/out/',
                             '-DCMAKE_FIND_ROOT_PATH=' + GetLibreSSLOutDir(platform, config) +'/out/;' + GetProtobufOutDir(platform, config) + '/out/']
    cmake_config = cmake_config + GetAdditionalConfigs(platform)
    cmake_config.append(GNS_SRC)

    subprocess.check_call(cmake_config, cwd=GNS_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=GNS_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=GNS_DIR)

def SetupSDL(platform, config):
    SDL_SRC = THIRD_PARTY_SRC_DIR + 'SDL2-2.0.20'
    SDL_DIR = GetThirdPartyOutDir(platform, config) + 'sdl'
    os.makedirs(SDL_DIR, exist_ok = True)

    print("Build sdl in " + SDL_DIR, flush=True)

    cmake_config = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + SDL_DIR + '/out', '-DSDL_SHARED=False', GetCmakeArgBuildType(config)]
    cmake_config = cmake_config + GetAdditionalConfigs(platform)
    cmake_config.append(SDL_SRC)

    subprocess.check_call(cmake_config, cwd=SDL_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=SDL_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=SDL_DIR)

def SetupSoLoud(platform, config):
    SDL_DIR = GetThirdPartyOutDir(platform, config) + 'sdl'
    SoLD_SRC = THIRD_PARTY_SRC_DIR + 'soloud-RELEASE_20200207/build_cmake'
    SoLD_DIR = GetThirdPartyOutDir(platform, config) + 'soloud'
    os.makedirs(SoLD_DIR, exist_ok = True)

    print("Build SoLoud in " + SoLD_DIR, flush=True)
    cmake_config = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + SoLD_DIR + '/out', GetCmakeArgBuildType(config),
                                '-DCMAKE_PREFIX_PATH=' + SDL_DIR + '/out',
                                '-DSDL2_ROOT={SDL_DIR}/out',
                                f'-DCMAKE_FIND_ROOT_PATH={SDL_DIR}/out']
    cmake_config = cmake_config + GetAdditionalConfigs(platform)
    cmake_config.append(SoLD_SRC)

    subprocess.check_call(cmake_config, cwd=SoLD_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=SoLD_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=SoLD_DIR)

def SetupSpdlog(platform, config):
    SPD_SRC = THIRD_PARTY_SRC_DIR + 'spdlog-1.9.2'
    SPD_DIR = GetThirdPartyOutDir(platform, config) + 'spdlog'
    os.makedirs(SPD_DIR, exist_ok = True)

    print("Build spdlog in " + SPD_DIR, flush=True)

    cmake_config = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + SPD_DIR + '/out', GetCmakeArgBuildType(config)]
    cmake_config = cmake_config + GetAdditionalConfigs(platform)
    cmake_config.append(SPD_SRC)

    subprocess.check_call(cmake_config, cwd=SPD_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=SPD_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=SPD_DIR)

def SetupPhysFS(platform, config):
    PFS_SRC = THIRD_PARTY_SRC_DIR + 'physfs-3.0.2'
    PFS_DIR = GetThirdPartyOutDir(platform, config) + 'physfs'
    os.makedirs(PFS_DIR, exist_ok = True)

    cmake_config = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + PFS_DIR + '/out', '-DPHYSFS_BUILD_SHARED=False', '-DPHYSFS_BUILD_TEST=False', GetCmakeArgBuildType(config)]
    cmake_config = cmake_config + GetAdditionalConfigs(platform)
    cmake_config.append(PFS_SRC)

    subprocess.check_call(cmake_config, cwd=PFS_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=PFS_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=PFS_DIR)

def SetupGTest(platform, config):
    GTEST_SRC = THIRD_PARTY_SRC_DIR + 'googletest-release-1.10.0'
    GTEST_DIR = GetThirdPartyOutDir(platform, config) + 'gtest'
    os.makedirs(GTEST_DIR, exist_ok = True)

    subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX=' + GTEST_DIR + '/out', '-DBUILD_GMOCK=ON', GetCmakeArgBuildType(config), GTEST_SRC], cwd=GTEST_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=GTEST_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=GTEST_DIR)

def SetupFreetype(platform, config):
    FT_SRC = THIRD_PARTY_SRC_DIR + 'freetype-2.10.4'
    FT_DIR = GetThirdPartyOutDir(platform, config) + 'freetype'
    os.makedirs(FT_DIR, exist_ok = True)

    cmake_config = ['cmake', '-DCMAKE_INSTALL_PREFIX=' + FT_DIR + '/out',
            GetCmakeArgBuildType(config),
            "-DDISABLE_PNG=ON",
            "-DDISABLE_HARFBUZ=ON",
            "-DDISABLE_ZLIB=ON",
            "-DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE",
            "-DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE"]
    cmake_config = cmake_config + GetAdditionalConfigs(platform)
    cmake_config.append(FT_SRC)

    subprocess.check_call(cmake_config, cwd=FT_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=FT_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=FT_DIR)

def SetupTracy(platform, config):
    TRC_SRC = THIRD_PARTY_SRC_DIR + 'tracy-0.7.8'
    TRC_DIR = GetThirdPartyOutDir(platform, config) + 'tracy'
    os.makedirs(TRC_DIR, exist_ok = True)

    subprocess.check_call(['make', '-j8', '--file=' + TRC_SRC + '/profiler/build/unix/Makefile', 'release'], cwd=TRC_SRC + '/profiler/build/unix/');

#SetupTracy(PLATFORM, CONFIG)

SetupLibressl(PLATFORM, CONFIG)
SetupProtobuf(PLATFORM, CONFIG)
SetupGameNetworkingSockets(PLATFORM, CONFIG.Debug)
#SetupSDL(PLATFORM, CONFIG)
#SetupSoLoud(PLATFORM, CONFIG)
SetupSpdlog(PLATFORM, CONFIG)
SetupPhysFS(PLATFORM, Config.Debug)
#SetupGTest(PLATFORM, CONFIG)
SetupFreetype(PLATFORM, CONFIG)
