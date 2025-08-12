set(SRC
    Client.cpp
    InterfaceGraphics.cpp
    ClientGame.cpp
    GameRendering.cpp
    $<$<BOOL:${ENABLE_OPENGL}>:Gfx.cpp>
    $<$<NOT:$<BOOL:${ENABLE_OPENGL}>>:GfxSDLGpu.cpp>
    $<$<BOOL:${ENABLE_OPENGL}>:${GLAD_SRC}>
    GameMenus.cpp
    Input.cpp
    BinPack.cpp
    Sound.cpp
    MapGraphics.cpp
    UpdateFrame.cpp
    WeatherEffects.cpp
    ClientCommands.cpp
    GostekGraphics.cpp
    ControlGame.cpp
    CVarClient.cpp
    debug/DebugWindow.cpp
    debug/ImguiSrc.cpp
    ui/LoadingScreen.cpp
    ui/NuklearImpl.cpp
    ui/nuklear_sdl3_gpu.cpp
    SdlApp.cpp
)

set(SHARED_SRC
    ../shared/AnimationSystem.cpp
    ../shared/Command.cpp
    ../shared/Cvar.cpp
    ../shared/Game.cpp
    ../shared/Demo.cpp
    ../shared/mechanics/Sprites.cpp
    ../shared/mechanics/Sparks.cpp
    ../shared/mechanics/Bullets.cpp
    ../shared/mechanics/Things.cpp
    ../shared/mechanics/Control.cpp
    ../shared/mechanics/BulletSystem.cpp
    ../shared/mechanics/SpriteSystem.cpp
    ../shared/mechanics/ThingSystem.cpp
    ../shared/network/Net.cpp
    ../shared/network/NetworkClient.cpp
    ../shared/network/NetworkClientConnection.cpp
    ../shared/network/NetworkClientSprite.cpp
    ../shared/network/NetworkClientGame.cpp
    ../shared/network/NetworkUtils.cpp
    ../shared/network/NetworkClientFunctions.cpp
    ../shared/network/NetworkClientMessages.cpp
    ../shared/network/NetworkClientBullet.cpp
    ../shared/network/NetworkClientThing.cpp
    ../shared/network/NetworkClientHeartbeat.cpp
    ../shared/misc/FontAtlas.cpp
    ../shared/misc/BitStream.cpp
    ../shared/misc/GlobalSystems.cpp
)

add_library(SoldatClientLib OBJECT ${SRC} ${SHARED_SRC})
target_include_directories(SoldatClientLib
    PRIVATE
        ./config
        ../
        ${PROJECT_BINARY_DIR}
)

target_include_directories(SoldatClientLib SYSTEM PRIVATE
    ${PHYSFS_INCLUDE_DIR}
    ${GLAD_INC}
    ${STB_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${TRACY_INCLUDE_DIR}
    ${DOCTEST_INCLUDE_DIR}
    ${IMGUI_INCLUDE_DIR}
    ${NUKLEAR_INCLUDE_DIR}
)

target_link_libraries(SoldatClientLib
    SDL3::SDL3
    Freetype::Freetype
    GameNetworkingSockets::GameNetworkingSockets_s
    spdlog::spdlog
    ApprovalTests::ApprovalTests
    SoLoud::soloud
)

if (ENABLE_PCH)
    target_precompile_headers(SoldatClientLib PRIVATE
        ${CommonPCH}
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/Client.hpp>"
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/Input.hpp>"
    )
endif()

add_executable(SoldatClient main.cpp)
target_link_libraries(SoldatClient
    CommonLib
    SoldatClientLib
    SDL3::SDL3
    Freetype::Freetype
    ${PHYSFS_LIBRARY}
    GameNetworkingSockets::GameNetworkingSockets_s
    spdlog::spdlog
)
target_include_directories(SoldatClient PRIVATE
    ./config
    ../
    ${PHYSFS_INCLUDE_DIR}
    ${GLAD_INC}
    ${STB_INCLUDE_DIR}
    ${FREETYPE_INCLUDE_DIRS}
    ${TRACY_INCLUDE_DIR}
)