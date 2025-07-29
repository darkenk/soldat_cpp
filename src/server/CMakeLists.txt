set(SRC
    Server.cpp
    ServerHelper.cpp
    BanSystem.cpp
    ServerCommands.cpp
    ServerLoop.cpp
)

set(SHARED_SRC
    ../shared/AI.cpp
    ../shared/AnimationSystem.cpp
    ../shared/Command.cpp
    ../shared/Cvar.cpp
    ../shared/Game.cpp
    ../shared/Demo.cpp
    ../shared/SharedConfig.cpp
    ../shared/mechanics/Sprites.cpp
    ../shared/mechanics/Bullets.cpp
    ../shared/mechanics/Things.cpp
    ../shared/mechanics/Control.cpp
    ../shared/mechanics/SpriteSystem.cpp
    ../shared/mechanics/BulletSystem.cpp
    ../shared/mechanics/ThingSystem.cpp
    ../shared/network/Net.cpp
    ../shared/network/NetworkServer.cpp
    ../shared/network/NetworkUtils.cpp
    ../shared/network/NetworkServerSprite.cpp
    ../shared/network/NetworkServerMessages.cpp
    ../shared/network/NetworkServerGame.cpp
    ../shared/network/NetworkServerThing.cpp
    ../shared/network/NetworkServerBullet.cpp
    ../shared/network/NetworkServerConnection.cpp
    ../shared/network/NetworkServerFunctions.cpp
    ../shared/network/NetworkServerHeartbeat.cpp
    ../shared/misc/GlobalSystems.cpp
)

add_library(SoldatServerLib OBJECT ${SRC} ${SHARED_SRC})
target_compile_definitions(SoldatServerLib PRIVATE SERVER=1)
target_include_directories(SoldatServerLib PRIVATE
    ./config
    ../
)
target_include_directories(SoldatServerLib SYSTEM PRIVATE
    ${PHYSFS_INCLUDE_DIR}
    ${GLAD_INC}
    ${STB_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${TRACY_INCLUDE_DIR}
    ${DOCTEST_INCLUDE_DIR}
)
target_link_libraries(SoldatServerLib
    SDL3::SDL3
    spdlog::spdlog
    Freetype::Freetype
    GameNetworkingSockets::GameNetworkingSockets_s
    ApprovalTests::ApprovalTests
)

target_precompile_headers(SoldatServerLib PRIVATE
    ${CommonPCH}
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/Server.hpp>"
)


add_executable(SoldatServer main.cpp)
target_compile_definitions(SoldatServer PRIVATE SERVER=1)
target_link_libraries(SoldatServer
    CommonLib
    SoldatServerLib
    SDL3::SDL3
    spdlog::spdlog
    Freetype::Freetype
    ${PHYSFS_LIBRARY}
    GameNetworkingSockets::GameNetworkingSockets_s
)
target_include_directories(SoldatServer PRIVATE
    ./config
    ../
    ${PHYSFS_INCLUDE_DIR}
    ${GLAD_INC}
    ${STB_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${TRACY_INCLUDE_DIR}
)
