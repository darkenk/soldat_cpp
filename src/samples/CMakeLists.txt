add_executable(PolyMapSample PolyMapSample.cpp)
target_include_directories(PolyMapSample PRIVATE
    ${ROOT_DIR}/src
    ${TRACY_INCLUDE_DIR}
)
target_link_libraries(PolyMapSample
    SoldatClientLib
    SoldatServerLib
    CommonLib
    PlatformLib
    SDL2::SDL2main
    SDL2::SDL2-static
    spdlog::spdlog
    Freetype::Freetype
    ${PHYSFS_LIB}
    GameNetworkingSockets::GameNetworkingSockets_s
    ${SOLOUD_LIBRARY}
)
