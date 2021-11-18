add_executable(SoldatGame main.cpp)
target_include_directories(SoldatGame PRIVATE
    ${ROOT_DIR}/src
)
target_link_libraries(SoldatGame
    SoldatClientLib
    SoldatServerLib
    SDL2::SDL2main
    SDL2::SDL2-static
    spdlog::spdlog
    Freetype::Freetype
    ${PHYSFS_LIB}
    GameNetworkingSockets::GameNetworkingSockets_s
    ${SOLOUD_LIBRARY}
)
