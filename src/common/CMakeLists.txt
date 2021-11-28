set(SRC
    misc/PortUtils.cpp
    misc/RandomGenerator.cpp
    misc/SHA1.cpp
    misc/SHA1Helper.cpp
    misc/TFileStream.cpp
    misc/TIniFile.cpp
    misc/TMemoryStream.cpp
    network/Net.cpp
    Anims.cpp
    Calc.cpp
    Logging.cpp
    MapFile.cpp
    Parts.cpp
    PhysFSExt.cpp
    Util.cpp
    Vector.cpp
    Waypoints.cpp
    Weapons.cpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # enable optimization for SHA1.cpp to boost startup of the game in debug mode
    set_source_files_properties(misc/SHA1.cpp PROPERTIES COMPILE_FLAGS -O3)
endif()

add_library(CommonLib ${SRC})
target_link_libraries(CommonLib
    spdlog::spdlog
    GameNetworkingSockets::GameNetworkingSockets_s
)
