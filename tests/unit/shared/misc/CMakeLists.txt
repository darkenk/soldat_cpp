include(GoogleTest)

find_package(Threads REQUIRED)

set(INC_DIRS
    ${GTEST_INCLUDE_DIR}
    ${ROOT_DIR}/src/shared
    ${ROOT_DIR}/src/
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

add_library(GlobalVariableClientTest STATIC GlobalVariableStorage.cpp)
target_include_directories(GlobalVariableClientTest PRIVATE ${INC_DIRS} ${ROOT_DIR}/src/client/config)

add_library(GlobalVariableServerTest STATIC GlobalVariableStorage.cpp)
target_include_directories(GlobalVariableServerTest PRIVATE ${INC_DIRS} ${ROOT_DIR}/src/server/config)

add_executable(GlobalVariableTest GlobalVariableTest.cpp GlobalVariableStorage.cpp ${ROOT_DIR}/src/shared/misc/GlobalVariable.cpp)
target_include_directories(GlobalVariableTest PRIVATE ${INC_DIRS} ${ROOT_DIR}/tests/config)
target_link_libraries(GlobalVariableTest ${LIB_DIRS} GlobalVariableClientTest GlobalVariableServerTest)
gtest_discover_tests(GlobalVariableTest)

