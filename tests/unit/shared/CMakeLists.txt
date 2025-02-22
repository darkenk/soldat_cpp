include(GoogleTest)

find_package(Threads REQUIRED)

set(INC_DIRS
    ${GTEST_INCLUDE_DIR}
    ${ROOT_DIR}/src/shared
    ${ROOT_DIR}/src/
    config
    ${TRACY_INCLUDE_DIR}
    ${PHYSFS_INCLUDE_DIR}
    ${DOCTEST_INCLUDE_DIR}
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

add_executable(SharedTest
    mechanics/SpriteSystemTest.cpp ${ROOT_DIR}/src/shared/mechanics/SpriteSystem.cpp
    #mechanics/SpriteTest.cpp ${ROOT_DIR}/src/shared/mechanics/Sprites.cpp
    )
target_include_directories(SharedTest PRIVATE ${INC_DIRS})
target_link_libraries(SharedTest CommonLib ${LIB_DIRS})
gtest_discover_tests(SharedTest)
