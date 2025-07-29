set(SRC
    misc/PortUtils.cpp
    misc/RandomGenerator.cpp
    misc/SHA1.cpp
    misc/SHA1Helper.cpp
    misc/TFileStream.cpp
    misc/TIniFile.cpp
    misc/TMemoryStream.cpp
    network/Net.cpp
    port_utils/NotImplemented.cpp
    Anims.cpp
    Calc.cpp
    Console.cpp
    DefaultFont.cpp
    FileUtility.cpp
    GameStrings.cpp
    Logging.cpp
    MapFile.cpp
    Parts.cpp
    PhysFSExt.cpp
    PolyMap.cpp
    Util.cpp
    Vector.cpp
    Waypoints.cpp
    Weapons.cpp
    WeaponSystem.cpp
    LogFile.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # enable optimization for SHA1.cpp to boost startup of the game in debug mode
    set_source_files_properties(misc/SHA1.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS TRUE COMPILE_FLAGS -O3)
endif()

add_library(CommonLib ${SRC} ${TRACY_SRC})
target_link_libraries(CommonLib
    spdlog::spdlog
    GameNetworkingSockets::GameNetworkingSockets_s
    ApprovalTests::ApprovalTests
)

target_include_directories(CommonLib SYSTEM PRIVATE
    ${TRACY_INCLUDE_DIR}
    ${PHYSFS_INCLUDE_DIR}
    ${DOCTEST_INCLUDE_DIR}
)

target_precompile_headers(CommonLib PRIVATE
    ${CommonPCH}
)
