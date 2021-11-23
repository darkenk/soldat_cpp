set(SRC_DIR ../../src)

# DK_TODO: create archive works very wierd. I need to copy files to cmake binary dir to have proper paths
# I assume that this can be done better. For now I'm leaving this as it is. Maybe I'll find some better way in the future
message(STATUS "Create test archive for physfs from files:")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/physfs/test_archive/test.ini DESTINATION ${CMAKE_BINARY_DIR}/physfs_test_archive)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/physfs/test_archive/test2.ini DESTINATION ${CMAKE_BINARY_DIR}/physfs_test_archive)

file(ARCHIVE_CREATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_archive.zip
    PATHS physfs_test_archive
    FORMAT zip
    VERBOSE
)

add_executable(PhysFSTest
    physfs/PhysFSTest.cpp ${SRC_DIR}/common/PhysFSExt.cpp
    ${SRC_DIR}/common/misc/PortUtils.cpp
)

gtest_discover_tests(PhysFSTest)

message(STATUS "Copy sound files")
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sound/fall.wav DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

add_executable(SoundTest
    sound/SoundTest.cpp
)

target_link_libraries(SoundTest ${SOLOUD_LIBRARY})
target_include_directories(SoundTest PRIVATE ${SOLOUD_INCLUDE_DIR})

gtest_discover_tests(SoundTest)

add_executable(TFileStreamTest
    fs/TFileStreamTest.cpp ${SRC_DIR}/common/misc/TFileStream.cpp)

gtest_discover_tests(TFileStreamTest)
