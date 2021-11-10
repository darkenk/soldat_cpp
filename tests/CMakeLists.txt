include(GoogleTest)

find_package(Threads REQUIRED)

include_directories(${GTEST_INCLUDE_DIR})
include_directories(../src/client)
include_directories(../src/shared)
include_directories(../src/)
link_libraries(${GTEST_MAIN_LIBRARY}
    ${GTEST_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GMOCK_MAIN_LIBRARY}
    ${GMOCK_LIBRARY}
    spdlog::spdlog
    ${PHYSFS_LIB}
    SDL2::SDL2main
    SDL2::SDL2-static
)

add_executable(UnitTests
    unit/BinPackTest.cpp ../src/client/BinPack.cpp
    unit/VectorTest.cpp ../src/shared/Vector.cpp
    unit/CalcTest.cpp ../src/shared/Calc.cpp
    unit/UtilTests.cpp ../src/shared/Util.cpp
    unit/CVarTests.cpp ../src/shared/Cvar.cpp
    unit/TIniFileTest.cpp ../src/shared/misc/TIniFile.cpp
    ../src/shared/misc/PortUtils.cpp
    unit/FontAtlasTest.cpp ../src/shared/misc/FontAtlas.cpp
    unit/BitStreamTest.cpp
    unit/SafeTypeTest.cpp
)
gtest_discover_tests(UnitTests)

add_executable(UtilsTest
    unit/FlagSetTest.cpp
)
gtest_discover_tests(UtilsTest)

add_subdirectory(integration)
