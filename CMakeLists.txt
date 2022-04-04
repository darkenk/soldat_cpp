set(CMAKE_CXX_STANDARD 20)

project(SoldatCPP)
cmake_minimum_required(VERSION 3.18)

set(SOLDAT_PLATFORM linux_x64)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
  set(SOLDAT_PLATFORM linux_aarch64)
  link_directories(/opt/vc/lib)
endif() 
#set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(THIRD_PARTY_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/out/${SOLDAT_PLATFORM}/release/third_party)
message(STATUS "Using third party: ${THIRD_PARTY_OUT_DIR}")

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/libsodium-1.0.18/contrib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/GameNetworkingSockets-1.3.0/cmake
    ${CMAKE_MODULE_PATH}
)
set(CMAKE_PREFIX_PATH
    ${THIRD_PARTY_OUT_DIR}/gamenetworkingsockets/out/
    ${THIRD_PARTY_OUT_DIR}/libsodium/out/
    ${THIRD_PARTY_OUT_DIR}/sdl/out/
    ${THIRD_PARTY_OUT_DIR}/freetype/out/
    ${THIRD_PARTY_OUT_DIR}/openal/out/
    ${THIRD_PARTY_OUT_DIR}/spdlog/out/
    ${THIRD_PARTY_OUT_DIR}/gtest/out/
    ${THIRD_PARTY_OUT_DIR}/libressl/out/
    ${CMAKE_PREFIX_PATH}
)

set(PHYSFS_LIB ${THIRD_PARTY_OUT_DIR}/physfs/out/lib/libphysfs.a)
set(PHYSFS_INC ${THIRD_PARTY_OUT_DIR}/physfs/out/include)
set(GLAD_INC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/glad/debug/include)
# @todo make glad as static lib
set(GLAD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/glad/debug/src/glad.c)
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/stb)
# soloud
set(SOLOUD_INCLUDE_DIR ${THIRD_PARTY_OUT_DIR}/soloud/out/include)
set(SOLOUD_LIBRARY ${THIRD_PARTY_OUT_DIR}/soloud/out/lib/libsoloud_static.a)
#tracy
set(TRACY_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/tracy-0.7.8/TracyClient.cpp)
set(TRACY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/tracy-0.7.8)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#add_definitions(-DTRACY_ENABLE)

find_package(SDL2 REQUIRED)
find_package(LibreSSL REQUIRED)
find_package(Freetype REQUIRED)
find_package(GameNetworkingSockets REQUIRED)
find_package(spdlog REQUIRED)

set(CommonPCH
    "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/src/common/Logging.hpp>"
    "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/src/common/misc/PortUtils.hpp>"
    "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/src/common/misc/PortUtilsSoldat.hpp>"
)

#shared library must be compiled with client or server defines
#add_subdirectory(src/shared)
add_subdirectory(src/platform)
add_subdirectory(src/common)
add_subdirectory(src/client)
add_subdirectory(src/server)
add_subdirectory(src/game)
add_subdirectory(src/samples)

find_package(GTest REQUIRED)
enable_testing()

add_subdirectory(tests)
