#include "common/misc/TMemoryStream.hpp"
#include <gtest/gtest.h>

class TMemStreamTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
  }

  void TearDown() override
  {
  }
};

constexpr std::string_view TestContent = R"(; Some comments
[TestSection1]
TestEntry=1.0)";

TEST_F(TMemStreamTest, CreateStreamReadsFileFromBeginnig)
{
  auto fileStream = ReadAsMemoryStream(TestContent);
  ASSERT_NE(nullptr, fileStream);
  std::string line;
  auto success = fileStream->ReadLine(line);
  EXPECT_EQ(true, success);
  EXPECT_STREQ("; Some comments", line.c_str());
}

TEST_F(TMemStreamTest, ReadLineReturnsFalseIfReachEof)
{
  auto fileStream = ReadAsMemoryStream(TestContent);
  ASSERT_NE(nullptr, fileStream);
  std::string line;
  fileStream->ReadLine(line);
  fileStream->ReadLine(line);
  fileStream->ReadLine(line);
  auto success = fileStream->ReadLine(line);
  EXPECT_EQ(false, success);
}

TEST_F(TMemStreamTest, ResetAllowsReadLineFromTheBeginning)
{
  auto fileStream = ReadAsMemoryStream(TestContent);
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
