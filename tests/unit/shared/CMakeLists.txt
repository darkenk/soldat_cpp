include(GoogleTest)

find_package(Threads REQUIRED)

set(INC_DIRS
    ${GTEST_INCLUDE_DIR}
    ${ROOT_DIR}/src/shared
    ${ROOT_DIR}/src/
    config
)

set(LIB_DIRS
    ${GTEST_MAIN_LIBRARY}
    ${GTEST_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GMOCK_MAIN_LIBRARY}
    ${GMOCK_LIBRARY}
    spdlog::spdlog
    ${PHYSFS_LIB}
    SDL2::SDL2main
    SDL2::SDL2-static
    GameNetworkingSockets::GameNetworkingSockets_s
)

add_executable(AnimsTest AnimsTest.cpp ${ROOT_DIR}/src/shared/AnimationSystem.cpp)
target_include_directories(AnimsTest PRIVATE ${INC_DIRS})
target_link_libraries(AnimsTest ${LIB_DIRS})
gtest_discover_tests(AnimsTest)

add_executable(SharedTest
    ${ROOT_DIR}/src/common/misc/PortUtils.cpp
    mechanics/SpriteSystemTest.cpp ${ROOT_DIR}/src/shared/mechanics/SpriteSystem.cpp)
target_include_directories(SharedTest PRIVATE ${INC_DIRS})
target_link_libraries(SharedTest ${LIB_DIRS})
gtest_discover_tests(SharedTest)
