include(GoogleTest)

find_package(Threads REQUIRED)

set(INC_DIRS
    ${GTEST_INCLUDE_DIR}
    ${ROOT_DIR}/src/shared
    ${ROOT_DIR}/src/
    config
    ${TRACY_INCLUDE_DIR}
    ${PHYSFS_INC}
)

set(LIB_DIRS
    GTest::gmock_main
    GTest::gtest
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog
    ${PHYSFS_LIB}
    SDL2::SDL2main
    SDL2::SDL2-static
    GameNetworkingSockets::GameNetworkingSockets_s
)

add_executable(AnimsTest
    AnimsTest.cpp ${ROOT_DIR}/src/shared/AnimationSystem.cpp
    ${ROOT_DIR}/src/shared/mechanics/SpriteSystem.cpp
    ${ROOT_DIR}/src/shared/LogFile.cpp
    ${ROOT_DIR}/src/common/Anims.cpp
    ${ROOT_DIR}/src/common/Parts.cpp
)
target_include_directories(AnimsTest PRIVATE ${INC_DIRS})
target_link_libraries(AnimsTest ${LIB_DIRS})
gtest_discover_tests(AnimsTest)

add_executable(SharedTest
    mechanics/SpriteSystemTest.cpp ${ROOT_DIR}/src/shared/mechanics/SpriteSystem.cpp
    #mechanics/SpriteTest.cpp ${ROOT_DIR}/src/shared/mechanics/Sprites.cpp
    )
target_include_directories(SharedTest PRIVATE ${INC_DIRS})
target_link_libraries(SharedTest CommonLib ${LIB_DIRS})
gtest_discover_tests(SharedTest)
