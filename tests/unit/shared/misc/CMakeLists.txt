include(GoogleTest)

find_package(Threads REQUIRED)

set(INC_DIRS
    ${GTEST_INCLUDE_DIR}
    ${ROOT_DIR}/src/shared
    ${ROOT_DIR}/src/
)

set(LIB_DIRS
    GTest::gmock_main
    GTest::gtest
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog
    ${PHYSFS_LIBRARY}
    SDL2::SDL2main
    SDL2::SDL2-static
    GameNetworkingSockets::GameNetworkingSockets_s
)

