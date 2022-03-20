include(GoogleTest)

find_package(Threads REQUIRED)


add_subdirectory(unit/shared)
add_subdirectory(unit/shared/misc)

include_directories(${GTEST_INCLUDE_DIR})
include_directories(../src/client)
include_directories(../src/shared)
include_directories(../src/)
include_directories(config)
include_directories(${TRACY_INCLUDE_DIR})
include_directories(${PHYSFS_INC})
remove_definitions(-DTRACY_ENABLE=1)


link_libraries(GTest::gmock_main
    GTest::gtest
    ${CMAKE_THREAD_LIBS_INIT}
    spdlog::spdlog
    ${PHYSFS_LIB}
    SDL2::SDL2main
    SDL2::SDL2-static
    GameNetworkingSockets::GameNetworkingSockets_s
)

add_executable(UnitTests
    unit/BinPackTest.cpp ../src/client/BinPack.cpp
    unit/VectorTest.cpp ../src/common/Vector.cpp
    unit/CalcTest.cpp ../src/common/Calc.cpp
    unit/UtilTests.cpp ../src/common/Util.cpp ../src/common/misc/SHA1.cpp ../src/common/misc/SHA1Helper.cpp
    unit/CVarTests.cpp ../src/shared/Cvar.cpp
    unit/TIniFileTest.cpp ../src/common/misc/TIniFile.cpp
    ../src/common/misc/PortUtils.cpp
    unit/FontAtlasTest.cpp ../src/shared/misc/FontAtlas.cpp
    unit/BitStreamTest.cpp
    unit/SafeTypeTest.cpp
    unit/TMemoryStreamTest.cpp ../src/common/misc/TMemoryStream.cpp
    unit/SharedConfigTest.cpp ../src/shared/SharedConfig.cpp ../src/common/Weapons.cpp
    unit/PolyMapTest.cpp ../src/common/PolyMap.cpp ../src/common/MapFile.cpp ../src/common/PhysFSExt.cpp
    unit/common/WeaponSystemTest.cpp ../src/common/WeaponSystem.cpp
)
gtest_discover_tests(UnitTests)

add_executable(UtilsTest
    unit/FlagSetTest.cpp
)
gtest_discover_tests(UtilsTest)

add_subdirectory(integration)
