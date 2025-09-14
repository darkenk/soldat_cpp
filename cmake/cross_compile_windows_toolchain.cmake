# cross_compile_windwos_toolchain.cmakes

# Target system
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(WINSDK_ARCH "x64")

# Use clang/clang++ as compilers
set(CMAKE_C_COMPILER "clang-cl" CACHE FILEPATH "" FORCE)
set(CMAKE_CXX_COMPILER "clang-cl" CACHE FILEPATH "" FORCE)
set(CMAKE_LINKER "lld-link" CACHE FILEPATH "" FORCE)
set(CMAKE_AR "llvm-lib" CACHE FILEPATH "" FORCE)
set(CMAKE_ASM_MASM_COMPILER "llvm-ml" CACHE FILEPATH "" FORCE)

# Target triple for MSVC ABI
set(CLANG_TARGET x86_64-pc-windows-msvc)

# Base path where xwin extracted files
set(XWIN_ROOT "/winsysroot")

function(get_highest_version the_dir the_ver)
  file(GLOB entries LIST_DIRECTORIES true RELATIVE "${the_dir}" "${the_dir}/[0-9.]*")
  foreach(entry ${entries})
    if(IS_DIRECTORY "${the_dir}/${entry}")
      set(${the_ver} "${entry}" PARENT_SCOPE)
    endif()
  endforeach()
endfunction()

if (NOT MSVC_VER)
  get_highest_version("${XWIN_ROOT}/VC/Tools/MSVC" MSVC_VER)
endif()

if (NOT WINSDK_VER)
  get_highest_version("${XWIN_ROOT}/Windows Kits/10/include" WINSDK_VER)
endif()

if (NOT MSVC_VER OR NOT WINSDK_VER)
  message(SEND_ERROR
          "Must specify CMake variable MSVC_VER and WINSDK_VER")
endif()

set(ATLMFC_LIB     "${XWIN_ROOT}/VC/Tools/MSVC/${MSVC_VER}/atlmfc/lib")
set(MSVC_INCLUDE   "${XWIN_ROOT}/VC/Tools/MSVC/${MSVC_VER}/include")
set(MSVC_LIB       "${XWIN_ROOT}/VC/Tools/MSVC/${MSVC_VER}/lib")
set(WINSDK_INCLUDE "${XWIN_ROOT}/Windows Kits/10/include/${WINSDK_VER}")
set(WINSDK_LIB     "${XWIN_ROOT}/Windows Kits/10/lib/${WINSDK_VER}")

set(COMPILE_FLAGS
     --target=${CLANG_TARGET}
     -D_CRT_SECURE_NO_WARNINGS
     -fms-compatibility-version=19.29
     -vctoolsversion ${MSVC_VER}
     -winsdkversion ${WINSDK_VER}
     -winsysroot ${XWIN_ROOT}
)
string(REPLACE ";" " " COMPILE_FLAGS "${COMPILE_FLAGS}")

set(CMAKE_CXX_FLAGS_INIT "${COMPILE_FLAGS}")
set(CMAKE_C_FLAGS_INIT "${COMPILE_FLAGS}")
set(CMAKE_RC_FLAGS_INIT "${COMPILE_FLAGS} /C 1252")
set(CMAKE_VS_PLATFORM_TOOLSET ClangCL)

# Library search paths (xwin)
set(LINK_FLAGS
    /manifest:no
    -libpath:"${ATLMFC_LIB}/${WINSDK_ARCH}"
    -libpath:"${MSVC_LIB}/${WINSDK_ARCH}"
    -libpath:"${WINSDK_LIB}/ucrt/${WINSDK_ARCH}"
    -libpath:"${WINSDK_LIB}/um/${WINSDK_ARCH}"
)

string(REPLACE ";" " " LINK_FLAGS "${LINK_FLAGS}")
string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${LINK_FLAGS}")
string(APPEND CMAKE_MODULE_LINKER_FLAGS_INIT " ${LINK_FLAGS}")
string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " ${LINK_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT}")

set(CMAKE_C_STANDARD_LIBRARIES "" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD_LIBRARIES "" CACHE STRING "" FORCE)