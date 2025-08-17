cmake_minimum_required(VERSION 4.0)
project(SoldatCPP)
set(CMAKE_CXX_STANDARD 23)

set(third_party_dependencies_config "${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}_release")
set(third_party_dependencies_path "${CMAKE_BINARY_DIR}/../${third_party_dependencies_config}")
get_filename_component(third_party_dependencies_path ${third_party_dependencies_path} ABSOLUTE)
message(STATUS "Using third party: ${third_party_dependencies_path}")
if (NOT EXISTS ${third_party_dependencies_path}/install/third_party.cmake)
  message(STATUS "Building third party dependencies")
  
  execute_process(
    COMMAND ${CMAKE_COMMAND} --preset ${third_party_dependencies_config}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    RESULT_VARIABLE BUILD_THIRD_PARTY_RESULT
    COMMAND_ECHO STDOUT
  )
  if (NOT BUILD_THIRD_PARTY_RESULT EQUAL 0)
    file(REMOVE ${third_party_dependencies_path}/install/third_party.cmake)
    message(FATAL_ERROR "Failed to build third party dependencies")
  endif()
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build --preset ${third_party_dependencies_config}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    RESULT_VARIABLE BUILD_THIRD_PARTY_RESULT
    COMMAND_ECHO STDOUT
  )
  if (NOT BUILD_THIRD_PARTY_RESULT EQUAL 0)
    file(REMOVE ${third_party_dependencies_path}/install/third_party.cmake)
    message(FATAL_ERROR "Failed to build third party dependencies")
  endif()
endif()

include(${third_party_dependencies_path}/install/third_party.cmake)

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
endif()


add_compile_definitions(SOLDAT_UTBOT=0)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
  set(SOLDAT_PLATFORM linux_aarch64)
  link_directories(/opt/vc/lib)
endif() 
#set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(CMakePrintHelpers)
include(ExternalProject)
include(FetchContent)


ExternalProject_Add(doctest 
  PREFIX ${CMAKE_BINARY_DIR}/third_party/doctest
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/doctest
)
include_directories(SYSTEM ${DOCTEST_INCLUDE_DIR})
include_directories(SYSTEM ${third_party_dependencies_path}/install/dbg-macro/include)

ExternalProject_Get_Property(doctest source_dir)
set(DOCTEST_INCLUDE_DIR ${source_dir} CACHE INTERNAL "Path to include folder for doctest")
include(${source_dir}/scripts/cmake/doctest.cmake)

find_package(SDL3 REQUIRED)
find_package(SDL3_shadercross REQUIRED)
find_package(SoLoud REQUIRED)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/libsodium-1.0.18/contrib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/GameNetworkingSockets-1.3.0/cmake
    ${CMAKE_MODULE_PATH}
)

cmake_print_variables(CMAKE_FIND_ROOT_PATH)

set(LIBRESSL_ROOT_DIR ${third_party_dependencies_path}/install/libressl/)


find_package(PhysFS REQUIRED)

set(GLAD_INC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/glad/debug/include)
# @todo make glad as static lib
set(GLAD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/glad/debug/src/glad.c)
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/stb)
#tracy
set(TRACY_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/tracy-0.7.8/TracyClient.cpp)
set(TRACY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/tracy-0.7.8)

set(IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/imgui)
set(NUKLEAR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/src/nuklear)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Add Library
add_library(LibreSSL::Crypto UNKNOWN IMPORTED)

# Set Properties
set_target_properties(
        LibreSSL::Crypto
        PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${third_party_dependencies_path}/install/libressl/include"
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${third_party_dependencies_path}/install/libressl/lib/libcrypto.a"
        INTERFACE_LINK_LIBRARIES LibreSSL::Crypto
)

add_library(ApprovalTests::ApprovalTests UNKNOWN IMPORTED)
set_target_properties(
        ApprovalTests::ApprovalTests
        PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${third_party_dependencies_path}/install/ApprovalTests/include"
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        IMPORTED_LOCATION "${third_party_dependencies_path}/install/ApprovalTests/lib/libApprovalTests.a"
        INTERFACE_LINK_LIBRARIES ApprovalTests::ApprovalTests
)

set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "ON")
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
#find_package(LibreSSL 3.4.3 REQUIRED EXACT)
find_package(Freetype REQUIRED)
find_package(GameNetworkingSockets REQUIRED)
find_package(spdlog REQUIRED)
find_package(Boost.DI REQUIRED)

option(ENABLE_TRACY "Enable tracy profiler" OFF)
option(ENABLE_OPENGL "Enables opengl backend (deprecated)" OFF)
option(ENABLE_IWYU "Enable incluw-what-you-use" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_MOLD "Enable MOLD Linker" OFF)
option(ENABLE_COVERAGE "Enable Code Coverage" OFF)
option(ENABLE_CLANG_TIDY "Enable Clang Tidy" OFF)
option(ENABLE_PCH "Enable PCH" OFF)

if (ENALBE_TRACY)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
  add_definitions(-DTRACY_ENABLE)
endif()

set(iwyu "${third_party_dependencies_path}/install/iwyu/bin/include-what-you-use")
if (ENABLE_IWYU AND EXISTS ${iwyu})
  message(STATUS "Use iwyu ${iwyu}")
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${iwyu}")
else()
  set(CMAKE_CXX_INCLUDE_WAHT_YOU_USE "")
endif()

if (ENABLE_ASAN)
  # Add compile options for AddressSanitizer (ASan)
  add_compile_options(
    "$<$<CXX_COMPILER_ID:Clang>:-fsanitize=address>"
    "$<$<CXX_COMPILER_ID:Clang>:-fno-omit-frame-pointer>"
    "$<$<CXX_COMPILER_ID:GNU>:-fsanitize=address>"
    "$<$<CXX_COMPILER_ID:GNU>:-fno-omit-frame-pointer>"
  )

  # Add link options for AddressSanitizer (ASan)
  add_link_options(
    "$<$<CXX_COMPILER_ID:Clang>:-fsanitize=address>"
    "$<$<CXX_COMPILER_ID:GNU>:-fsanitize=address>"
  )
endif()

if (ENABLE_MOLD)
  # Add link options for MOLD Linker
  add_link_options(
    "$<$<CXX_COMPILER_ID:Clang>:-fuse-ld=mold>"
    "$<$<CXX_COMPILER_ID:GNU>:-fuse-ld=mold>"
  )
endif()

if (ENABLE_COVERAGE)
  add_compile_options(
    "$<$<CXX_COMPILER_ID:Clang>:-coverage>"
    "$<$<CXX_COMPILER_ID:GNU>:-coverage>"
    "$<$<CXX_COMPILER_ID:GNU>:-fprofile-abs-path>"
  )
  add_link_options(
    "$<$<CXX_COMPILER_ID:Clang>:-coverage>"
    "$<$<CXX_COMPILER_ID:GNU>:-coverage>"
  )
endif()

if (ENABLE_CLANG_TIDY)
  set(CMAKE_CXX_CLANG_TIDY clang-tidy
    #--config-file=${CMAKE_CURRENT_SOURCE_DIR}/.clang-tidy
  )
endif()

# Add compile options for uninitialized variable warnings
add_compile_options(
  "$<$<CXX_COMPILER_ID:Clang>:-Wall>"
  "$<$<CXX_COMPILER_ID:Clang>:-Wextra>"
  "$<$<CXX_COMPILER_ID:Clang>:-Wpedantic>"
  "$<$<CXX_COMPILER_ID:Clang>:-Werror>"
  "$<$<CXX_COMPILER_ID:Clang>:-Wno-unused-parameter>"
  "$<$<CXX_COMPILER_ID:Clang>:-Wno-unused-function>"
  "$<$<CXX_COMPILER_ID:Clang>:-Wno-sign-compare>"
  "$<$<CXX_COMPILER_ID:Clang>:-Wno-nested-anon-types>"
  "$<$<CXX_COMPILER_ID:Clang>:-Wno-error=int-to-pointer-cast>"
  "$<$<CXX_COMPILER_ID:Clang>:-Wno-error=deprecated-declarations>"

  "$<$<CXX_COMPILER_ID:GNU>:-Wall>"
  "$<$<CXX_COMPILER_ID:GNU>:-Wextra>"
  "$<$<CXX_COMPILER_ID:GNU>:-Wpedantic>"
  "$<$<CXX_COMPILER_ID:GNU>:-Werror>"
  "$<$<CXX_COMPILER_ID:GNU>:-Wno-unused-parameter>"
  "$<$<CXX_COMPILER_ID:GNU>:-Wno-unused-function>"
  "$<$<CXX_COMPILER_ID:GNU>:-Wno-sign-compare>"
  "$<$<CXX_COMPILER_ID:GNU>:-Wno-error=int-to-pointer-cast>"
  "$<$<CXX_COMPILER_ID:GNU>:-Wno-error=deprecated-declarations>"
)

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
#add_subdirectory(src/samples)
add_subdirectory(assets)


add_custom_target(third_party_dependencies
  COMMAND ${CMAKE_COMMAND} --preset ${third_party_dependencies_config}
  COMMAND ${CMAKE_COMMAND} --build --preset ${third_party_dependencies_config}
  COMMENT "Building external dependencies"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party
  USES_TERMINAL
)


#find_package(GTest REQUIRED)
#enable_testing()

#add_subdirectory(tests)
