#!/usr/bin/python3
import os
import subprocess

BASE_DIR = os.path.dirname(os.path.abspath(__file__)) + '/'
THIRD_PARTY_SRC_DIR = BASE_DIR + '../third_party/src/'
CONFIG_DIR = 'debug/'
OUT_DIR = BASE_DIR + 'out/'
PLATFORM_DIR = 'linux_x86/'
THIRD_PARTY_OUT_DIR = OUT_DIR + PLATFORM_DIR + CONFIG_DIR + 'third_party/'

JOBS = str(os.cpu_count())

print(BASE_DIR)

SODIUM_DIR = THIRD_PARTY_OUT_DIR + 'libsodium'
PROTOBUF_DIR = THIRD_PARTY_OUT_DIR + 'protobuf'

def SetupProtobuf():
    PTB_SRC = THIRD_PARTY_SRC_DIR + 'protobuf-3.19.1'
    PTB_DIR = PROTOBUF_DIR
    os.makedirs(PTB_DIR, exist_ok = True)

    print("Build protobuf in " + PTB_DIR, flush=True)

    subprocess.check_call([PTB_SRC + '/configure', '--prefix=' + PTB_DIR + '/out'], cwd=PTB_DIR)
    subprocess.check_call(['make', '-j' + JOBS], cwd=PTB_DIR)
    subprocess.check_call(['make', 'install'], cwd=PTB_DIR)

def SetupLibsodium():
    SOD_SRC = THIRD_PARTY_SRC_DIR + 'libsodium-1.0.18'
    SOD_DIR = SODIUM_DIR
    os.makedirs(SOD_DIR, exist_ok = True)

    subprocess.check_call([SOD_SRC + '/configure', '--prefix=' + SOD_DIR + '/out'], cwd=SOD_DIR)
    subprocess.check_call(['make', '-j' + JOBS], cwd=SOD_DIR)
    subprocess.check_call(['make', 'check'], cwd=SOD_DIR)
    subprocess.check_call(['make', 'install'], cwd=SOD_DIR)

def SetupGameNetworkingSockets():
    GNS_SRC = THIRD_PARTY_SRC_DIR + 'GameNetworkingSockets-1.3.0'
    GNS_DIR = THIRD_PARTY_OUT_DIR + 'gamenetworkingsockets'
    os.makedirs(GNS_DIR, exist_ok = True)

    subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX=' + GNS_DIR + '/out',
                        '-DCMAKE_BUILD_TYPE=Debug', '-DUSE_CRYPTO=libsodium',
                           '-DUSE_CRYPTO25519=libsodium', '-DCMAKE_PREFIX_PATH=' + SODIUM_DIR +'/out/;' + PROTOBUF_DIR + '/out/',
                        GNS_SRC], cwd=GNS_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=GNS_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=GNS_DIR)

def SetupSDL():
    SDL_SRC = THIRD_PARTY_SRC_DIR + 'SDL2-2.0.14'
    SDL_DIR = THIRD_PARTY_OUT_DIR + 'sdl'
    os.makedirs(SDL_DIR, exist_ok = True)

    print("Build sdl in " + SDL_DIR, flush=True)

    subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX=' + SDL_DIR + '/out', '-DSDL_SHARED=False', '-DCMAKE_BUILD_TYPE=Debug', SDL_SRC], cwd=SDL_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=SDL_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=SDL_DIR)

def SetupSoLoud():
    SDL_DIR = THIRD_PARTY_OUT_DIR + 'sdl'
    SoLD_SRC = THIRD_PARTY_SRC_DIR + 'soloud-RELEASE_20200207/build_cmake'
    SoLD_DIR = THIRD_PARTY_OUT_DIR + 'soloud'
    os.makedirs(SoLD_DIR, exist_ok = True)

    print("Build SoLoud in " + SoLD_DIR, flush=True)

    subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX=' + SoLD_DIR + '/out', '-DCMAKE_BUILD_TYPE=Debug',
                            '-DCMAKE_PREFIX_PATH=' + SDL_DIR + '/out', SoLD_SRC], cwd=SoLD_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=SoLD_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=SoLD_DIR)

def SetupSpdlog():
    SPD_SRC = THIRD_PARTY_SRC_DIR + 'spdlog-1.9.2'
    SPD_DIR = THIRD_PARTY_OUT_DIR + 'spdlog'
    os.makedirs(SPD_DIR, exist_ok = True)

    print("Build spdlog in " + SPD_DIR, flush=True)

    subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX=' + SPD_DIR + '/out', '-DCMAKE_BUILD_TYPE=Debug', SPD_SRC], cwd=SPD_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=SPD_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=SPD_DIR)

def SetupPhysFS():
    PFS_SRC = THIRD_PARTY_SRC_DIR + 'physfs-3.0.2'
    PFS_DIR = THIRD_PARTY_OUT_DIR + 'physfs'
    os.makedirs(PFS_DIR, exist_ok = True)

    subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX=' + PFS_DIR + '/out', '-DPHYSFS_BUILD_SHARED=False', '-DPHYSFS_BUILD_TEST=False', '-DCMAKE_BUILD_TYPE=Debug', PFS_SRC], cwd=PFS_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=PFS_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=PFS_DIR)

def SetupGTest():
    GTEST_SRC = THIRD_PARTY_SRC_DIR + 'googletest-release-1.10.0'
    GTEST_DIR = THIRD_PARTY_OUT_DIR + 'gtest'
    os.makedirs(GTEST_DIR, exist_ok = True)

    subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX=' + GTEST_DIR + '/out', '-DBUILD_GMOCK=ON', '-DCMAKE_BUILD_TYPE=Debug', GTEST_SRC], cwd=GTEST_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=GTEST_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=GTEST_DIR)

def SetupFreetype():
    FT_SRC = THIRD_PARTY_SRC_DIR + 'freetype-2.10.4'
    FT_DIR = THIRD_PARTY_OUT_DIR + 'freetype'
    os.makedirs(FT_DIR, exist_ok = True)

    subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX=' + FT_DIR + '/out', '-DCMAKE_BUILD_TYPE=Debug',
        "-DDISABLE_PNG=ON", "-DDISABLE_HARFBUZ=ON", "-DDISABLE_ZLIB=ON", FT_SRC], cwd=FT_DIR)
    subprocess.check_call(['cmake', '--build', '.', '--parallel', JOBS], cwd=FT_DIR)
    subprocess.check_call(['cmake', '--install', '.'], cwd=FT_DIR)

def SetupTracy():
    TRC_SRC = THIRD_PARTY_SRC_DIR + 'tracy-0.7.8'
    TRC_DIR = THIRD_PARTY_OUT_DIR + 'tracy'
    os.makedirs(TRC_DIR, exist_ok = True)

    subprocess.check_call(['make', '-j8', '--file=' + TRC_SRC + '/profiler/build/unix/Makefile', 'release'], cwd=TRC_SRC + '/profiler/build/unix/');

#SetupTracy()

SetupLibsodium()
SetupProtobuf()
SetupGameNetworkingSockets()
SetupSDL()
SetupSoLoud()
SetupSpdlog()
SetupPhysFS()
SetupGTest()
SetupFreetype()
