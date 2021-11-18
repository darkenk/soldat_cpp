set(CMAKE_C_COMPILER clang-12)
set(CMAKE_CXX_COMPILER clang++-12)
set(CMAKE_CXX_STANDARD 20)

project(SoldatCPP)
cmake_minimum_required(VERSION 3.18)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(THIRD_PARTY_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/out/linux_x86/debug/third_party)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/GameNetworkingSockets-1.3.0/cmake/)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH}
    ${THIRD_PARTY_OUT_DIR}/gamenetworkingsockets/out/
    ${THIRD_PARTY_OUT_DIR}/libsodium/out/
    ${THIRD_PARTY_OUT_DIR}/sdl/out/
    ${THIRD_PARTY_OUT_DIR}/freetype/out/
    ${THIRD_PARTY_OUT_DIR}/openal/out/
    ${THIRD_PARTY_OUT_DIR}/spdlog/out/
)

set(PHYSFS_LIB ${THIRD_PARTY_OUT_DIR}/physfs/out/lib/libphysfs.a)
set(PHYSFS_INC ${THIRD_PARTY_OUT_DIR}/physfs/out/include)
set(GLAD_INC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/glad/debug/include)
# @todo make glad as static lib
set(GLAD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/glad/debug/src/glad.c)
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/stb)
# gtest are missing
set(GTEST_INCLUDE_DIR ${THIRD_PARTY_OUT_DIR}/gtest/out/include)
set(GTEST_LIBRARY ${THIRD_PARTY_OUT_DIR}/gtest/out/lib/libgtestd.a)
set(GTEST_MAIN_LIBRARY ${THIRD_PARTY_OUT_DIR}/gtest/out/lib/libgtest_maind.a)
set(GMOCK_LIBRARY ${THIRD_PARTY_OUT_DIR}/gtest/out/lib/libgmockd.a)
set(GMOCK_MAIN_LIBRARY ${THIRD_PARTY_OUT_DIR}/gtest/out/lib/libgmock_maind.a)
# solout
set(SOLOUD_INCLUDE_DIR ${THIRD_PARTY_OUT_DIR}/soloud/out/include)
set(SOLOUD_LIBRARY ${THIRD_PARTY_OUT_DIR}/soloud/out/lib/libsoloud_static.a)


set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(GameNetworkingSockets REQUIRED)
find_package(spdlog REQUIRED)

#shared library must be compiled with client or server defines
#add_subdirectory(src/shared)
add_subdirectory(src/client)
add_subdirectory(src/server)
add_subdirectory(src/game)

find_package(GTest REQUIRED)
enable_testing()

add_subdirectory(tests)
