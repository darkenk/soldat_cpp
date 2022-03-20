add_executable(SoldatGame main.cpp)
target_include_directories(SoldatGame PRIVATE
    ${ROOT_DIR}/src
    ${TRACY_INCLUDE_DIR}
)
target_link_libraries(SoldatGame
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

set_target_properties(SoldatGame PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/game)
