#include "shared/PhysFSExt.hpp"
#include <filesystem>
#include <gtest/gtest.h>

const std::string_view CopiedFileFromArchive = "test_archive_file.txt";

class PhysFSTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        auto ret = PHYSFS_init(nullptr);
        ASSERT_NE(0, ret) << "Cannot initialize physfs library. Error code is "
                          << PHYSFS_getLastErrorCode();
        ret = PHYSFS_mount("test_archive.zip", "test", 0);
        ASSERT_NE(0, ret) << "Cannot mount test archive. Error code is "
                          << PHYSFS_getLastErrorCode();
        std::filesystem::remove(CopiedFileFromArchive);
    }

    void TearDown() override
    {
        PHYSFS_deinit();
        std::filesystem::remove(CopiedFileFromArchive);
    }
};

const std::string_view TestFile = "test/physfs_test_archive/test.ini";
const std::string_view TestFile2 = "test/physfs_test_archive/test2.ini";
const std::string_view NonExistingFile = "test/physfs_test_archive/non_existing_file.ini";

TEST_F(PhysFSTest, InitPhysFSTestArchive)
{
    auto ret = PHYSFS_exists(TestFile.data());
    EXPECT_NE(0, ret);
}

TEST_F(PhysFSTest, ReturnNullIfThereIsNoFile)
{
    auto fileStream = PhysFS_ReadAsStream(NonExistingFile.data());
    EXPECT_EQ(nullptr, fileStream);
}

TEST_F(PhysFSTest, CreateStreamReadsFileFromBeginnig)
{
    auto fileStream = PhysFS_ReadAsStream(TestFile.data());
    ASSERT_NE(nullptr, fileStream);
    std::string line;
    auto success = fileStream->ReadLine(line);
    EXPECT_EQ(true, success);
    EXPECT_STREQ("; Some comments", line.c_str());
}

TEST_F(PhysFSTest, ReadLineReturnsFalseIfReachEof)
{
    auto fileStream = PhysFS_ReadAsStream(TestFile2.data());
    ASSERT_NE(nullptr, fileStream);
    std::string line;
    fileStream->ReadLine(line);
    fileStream->ReadLine(line);
    fileStream->ReadLine(line);
    auto success = fileStream->ReadLine(line);
    EXPECT_EQ(false, success);
}

TEST_F(PhysFSTest, ResetAllowsReadLineFromTheBeginning)
{
    auto fileStream = PhysFS_ReadAsStream(TestFile.data());
    ASSERT_NE(nullptr, fileStream);
    std::string line;
    auto success = fileStream->ReadLine(line);
    EXPECT_EQ(true, success);
    EXPECT_STREQ("; Some comments", line.c_str());
    fileStream->ReadLine(line);
    fileStream->ReadLine(line);
    EXPECT_STRNE("; Some comments", line.c_str());
    fileStream->Reset();
    success = fileStream->ReadLine(line);
    EXPECT_EQ(true, success);
    EXPECT_STREQ("; Some comments", line.c_str());
}

TEST_F(PhysFSTest, CopyFileFromArchive)
{
    PhysFS_CopyFileFromArchive(TestFile, CopiedFileFromArchive);
    EXPECT_EQ(true, std::filesystem::exists(CopiedFileFromArchive));
}

TEST_F(PhysFSTest, DoNotCopyFileIfFileExists)
{
    auto h = std::fopen(CopiedFileFromArchive.data(), "w");
    std::fclose(h);
    EXPECT_EQ(0, std::filesystem::file_size(CopiedFileFromArchive.data()));
    EXPECT_EQ(false, PhysFS_CopyFileFromArchive(TestFile, CopiedFileFromArchive));
    EXPECT_EQ(0, std::filesystem::file_size(CopiedFileFromArchive.data()));
}
