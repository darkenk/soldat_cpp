#include "shared/misc/BitStream.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::ElementsAre;

TEST(BitStreamTest, WriteUint32)
{
    BitStream bs;
    std::uint32_t v = 4;
    bs.Write(v);
    EXPECT_THAT(bs.Data(), ElementsAre(0x0, 0x0, 0x0, 0x4));
}

TEST(BitStreamTest, WriteBoolean)
{
    BitStream bs;
    bool v1 = true;
    std::uint32_t v2 = 4;
    bs.Write(v1);
    bs.Write(v2);
    EXPECT_THAT(bs.Data(), ElementsAre(0x80, 0x0, 0x0, 0x02, 0x0));
}

TEST(BitStreamTest, WriteString)
{
    BitStream bs;
    std::string v1 = "ABC";
    bs.Write(v1);
    EXPECT_THAT(bs.Data(), ElementsAre('A', 'B', 'C', '\0'));
}

TEST(BitStreamTest, ReadUint32)
{
    std::array<std::uint8_t, 4> testData = {0x1, 0x2, 0x3, 0x4};
    BitStream bs(testData.data(), testData.size());
    std::uint32_t v = 0;
    bs.Read(v);
    EXPECT_EQ(0x01020304, v);
}

TEST(BitStreamTest, ReadBoolean)
{
    std::array<std::uint8_t, 3> testData = {0x1, 0x81, 0x0};
    BitStream bs(testData.data(), testData.size());
    std::uint8_t v1 = 0;
    bool v2 = false;
    std::uint8_t v3 = 0;
    bs.Read(v1);
    bs.Read(v2);
    bs.Read(v3);
    EXPECT_EQ(0x1, v1);
    EXPECT_EQ(true, v2);
    EXPECT_EQ(0x2, v3);
}

TEST(BitStreamTest, ReadString)
{
    std::array<std::uint8_t, 4> testData = {'A', 'B', 'C', '\0'};
    BitStream bs(testData.data(), testData.size());
    std::string v1 = {};
    bs.Read(v1);
    EXPECT_STREQ("ABC", v1.c_str());
}
