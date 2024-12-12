cmake_minimum_required(VERSION 3.20)
project(SoldatCPP)
set(CMAKE_CXX_STANDARD 20)

if (CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  set(SOLDAT_PLATFORM webassembly)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -sPTHREAD_POOL_SIZE=10 \
      --preload-file game/soldat.smod \
      --preload-file game/soldat_server.smod \
      -gsplit-dwarf -gdwarf-5 -O0 -sFULL_ES3 -pthread -sALLOW_MEMORY_GROWTH \
      -sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE=['$autoResumeAudioContext','$dynCall'] \
      -s TOTAL_MEMORY=128MB \
      -lopenal \
      ")
  set(THIRD_PARTY_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/out/${SOLDAT_PLATFORM}/debug/third_party)
else()
  set(SOLDAT_PLATFORM linux_x64)
  set(THIRD_PARTY_OUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/out/${SOLDAT_PLATFORM}/release/third_party)
endif()


add_compile_definitions(SOLDAT_UTBOT=0)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
  set(SOLDAT_PLATFORM linux_aarch64)
  link_directories(/opt/vc/lib)
endif() 
#set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Using third party: ${THIRD_PARTY_OUT_DIR}")

include(CMakePrintHelpers)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/libsodium-1.0.18/contrib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/GameNetworkingSockets-1.3.0/cmake
    ${CMAKE_MODULE_PATH}
)
#     ${THIRD_PARTY_OUT_DIR}/gamenetworkingsockets/out/
set(CMAKE_PREFIX_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/build/out/${SOLDAT_PLATFORM}/debug/third_party/gamenetworkingsockets/out/
    ${THIRD_PARTY_OUT_DIR}/libsodium/out/
    ${THIRD_PARTY_OUT_DIR}/sdl/out
    ${THIRD_PARTY_OUT_DIR}/freetype/out/
    ${THIRD_PARTY_OUT_DIR}/openal/out/
    ${THIRD_PARTY_OUT_DIR}/spdlog/out/
    ${THIRD_PARTY_OUT_DIR}/gtest/out/
    ${THIRD_PARTY_OUT_DIR}/libressl/out/
    ${CMAKE_PREFIX_PATH}
)

set(CMAKE_FIND_ROOT_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/build/out/${SOLDAT_PLATFORM}/debug/third_party/gamenetworkingsockets/out/
  ${THIRD_PARTY_OUT_DIR}/libsodium/out/
  ${THIRD_PARTY_OUT_DIR}/sdl/out/
  ${THIRD_PARTY_OUT_DIR}/freetype/out/
  ${THIRD_PARTY_OUT_DIR}/openal/out/
  ${THIRD_PARTY_OUT_DIR}/spdlog/out/
  ${THIRD_PARTY_OUT_DIR}/gtest/out/
  ${THIRD_PARTY_OUT_DIR}/libressl/out/
  ${CMAKE_FIND_ROOT_PATH}
)

cmake_print_variables(CMAKE_FIND_ROOT_PATH)

set(LIBRESSL_ROOT_DIR ${THIRD_PARTY_OUT_DIR}/libressl/out/)



set(PHYSFS_LIB ${CMAKE_CURRENT_SOURCE_DIR}/build/out/${SOLDAT_PLATFORM}/debug/third_party/physfs/out/lib/libphysfs.a)
set(PHYSFS_INC ${CMAKE_CURRENT_SOURCE_DIR}/build/out/${SOLDAT_PLATFORM}/debug/third_party/physfs/out/include)
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
set(DOCTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/doctest-2.4.8)
set(IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/imgui-1.87)

set(CMAKE_VERBOSE_MAKEFILE ON)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#add_definitions(-DTRACY_ENABLE)

# Add Library
add_library(LibreSSL::Crypto UNKNOWN IMPORTED)

# Set Properties
set_target_properties(
        LibreSSL::Crypto
        PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${THIRD_PARTY_OUT_DIR}/libressl/out/include"
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${THIRD_PARTY_OUT_DIR}/libressl/out/lib/libcrypto.a"
        INTERFACE_LINK_LIBRARIES LibreSSL::Crypto
)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
set(SDL2_DIR ${THIRD_PARTY_OUT_DIR}/sdl/out)
#set(LibreSSL_DIR ${THIRD_PARTY_OUT_DIR}/libressl/out)
find_package(SDL2 REQUIRED)
#find_package(LibreSSL 3.4.3 REQUIRED EXACT)
find_package(Freetype REQUIRED)
find_package(GameNetworkingSockets REQUIRED)
find_package(spdlog REQUIRED)

set(CommonPCH
    "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/src/common/Logging.hpp>"
    "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/src/common/misc/PortUtils.hpp>"
    "$<$<COMPILE_LANGUAGE:CXX>:${ROOT_DIR}/src/common/misc/PortUtilsSoldat.hpp>"
)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

#shared library must be compiled with client or server defines
#add_subdirectory(src/shared)
add_subdirectory(src/platform)
add_subdirectory(src/common)
add_subdirectory(src/client)
add_subdirectory(src/server)
add_subdirectory(src/game)
add_subdirectory(src/samples)

#find_package(GTest REQUIRED)
#enable_testing()

#add_subdirectory(tests)
