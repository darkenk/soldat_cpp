#include "shared/misc/TFileStream.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

constexpr std::string_view TestFile = "test.ini";
constexpr std::string_view NonExistingFile = "non_existing_file.ini";

class TFileStreamTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        auto of = std::ofstream(TestFile.data());
        of << R"(; Some comments
[TestSection1]
TestEntry=1.0
)";
    }

    void TearDown() override
    {
        std::filesystem::remove(TestFile);
    }
};

TEST_F(TFileStreamTest, ReturnNullIfThereIsNoFile)
{
    auto fileStream = ReadAsFileStream(NonExistingFile.data());
    EXPECT_EQ(nullptr, fileStream);
}

TEST_F(TFileStreamTest, CreateStreamReadsFileFromBeginnig)
{
    auto fileStream = ReadAsFileStream(TestFile);
    ASSERT_NE(nullptr, fileStream);
    std::string line;
    auto success = fileStream->ReadLine(line);
    EXPECT_EQ(true, success);
    EXPECT_STREQ("; Some comments", line.c_str());
}

TEST_F(TFileStreamTest, ReadLineReturnsFalseIfReachEof)
{
    auto fileStream = ReadAsFileStream(TestFile);
    ASSERT_NE(nullptr, fileStream);
    std::string line;
    fileStream->ReadLine(line);
    fileStream->ReadLine(line);
    fileStream->ReadLine(line);
    auto success = fileStream->ReadLine(line);
    EXPECT_EQ(false, success);
}

TEST_F(TFileStreamTest, ResetAllowsReadLineFromTheBeginning)
{
    auto fileStream = ReadAsFileStream(TestFile);
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
