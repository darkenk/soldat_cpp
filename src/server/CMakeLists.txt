set(SRC
    main.cpp
    Server.cpp
    ServerHelper.cpp
    BanSystem.cpp
    ServerCommands.cpp
    ServerLoop.cpp
)

set(SHARED_SRC
    ../shared/AI.cpp
    ../shared/Anims.cpp
    ../shared/Command.cpp
    ../shared/Cvar.cpp
    ../shared/Game.cpp
    ../shared/Logging.cpp
    #../shared/PolyMap.cpp
    ../shared/Demo.cpp
    ../shared/Console.cpp
    ../shared/Calc.cpp
    ../shared/LogFile.cpp
    ../shared/MapFile.cpp
    ../shared/Parts.cpp
    ../shared/PolyMap.cpp
    ../shared/Util.cpp
    ../shared/Waypoints.cpp
    ../shared/Vector.cpp
    ../shared/Weapons.cpp
    ../shared/mechanics/Sprites.cpp
    #../shared/mechanics/Sparks.cpp
    ../shared/mechanics/Bullets.cpp
    ../shared/mechanics/Things.cpp
    ../shared/mechanics/Control.cpp
    ../shared/network/Net.cpp
    ../shared/network/NetworkServer.cpp
    ../shared/network/NetworkUtils.cpp
    ../shared/misc/PortUtils.cpp
    ../shared/network/NetworkServerSprite.cpp
    ../shared/network/NetworkServerMessages.cpp
    ../shared/network/NetworkServerGame.cpp
    ../shared/network/NetworkServerThing.cpp
    ../shared/network/NetworkServerBullet.cpp
    ../shared/network/NetworkServerConnection.cpp
    ../shared/network/NetworkServerFunctions.cpp
    ../shared/network/NetworkServerHeartbeat.cpp
    ../shared/SharedConfig.cpp
    ../shared/PhysFSExt.cpp
    ../shared/misc/TIniFile.cpp
    ../shared/misc/RandomGenerator.cpp
    ../shared/misc/TFileStream.cpp
)


add_executable(SoldatServer ${SRC} ${SHARED_SRC})
target_compile_definitions(SoldatServer PUBLIC SERVER=1)
target_link_libraries(SoldatServer SDL2::SDL2main SDL2::SDL2-static spdlog::spdlog Freetype::Freetype ${PHYSFS_LIB} GameNetworkingSockets::GameNetworkingSockets_s)
target_include_directories(SoldatServer PRIVATE ./config ../ ${PHYSFS_INC} ${GLAD_INC} ${STB_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
