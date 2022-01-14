#include "common/Util.hpp"
#include "gtest/gtest.h"

TEST(CompilationTest, UtilTest)
{
    EXPECT_EQ(2, charcount(' ', "Sample  text"));
}

TEST(UtilTest, SplitStr)
{
    {
        auto ret = splitstr("aa:bb", ':');
        EXPECT_EQ(2, ret.size());
        EXPECT_STREQ("aa", ret.at(0).c_str());
        EXPECT_STREQ("bb", ret.at(1).c_str());
    }
    {
        auto ret = splitstr("aa:bb:cc", ':');
        EXPECT_EQ(3, ret.size());
        EXPECT_STREQ("aa", ret.at(0).c_str());
        EXPECT_STREQ("cc", ret.at(2).c_str());
    }
    {
        auto ret = splitstr("aa:bb:cc", ':', 1);
        EXPECT_EQ(2, ret.size());
        EXPECT_STREQ("aa", ret.at(0).c_str());
        EXPECT_STREQ("bb:cc", ret.at(1).c_str());
    }
}

TEST(UtilTest, PosEx)
{
    EXPECT_EQ(4, posex("o", "Hello world"));
    EXPECT_EQ(4, posex("o", "Hello world", 1));
    EXPECT_EQ(7, posex("o", "Hello world", 6));
    EXPECT_EQ(-1, posex("o", "Hello world", 9));
}

TEST(UtilTest, OverrideFileExt)
{
    // crashes due to PhysFS_exists
    // EXPECT_EQ("file.png", overridefileext("file.txt", "png"));
    EXPECT_EQ("filetxt", overridefileext("filetxt", "png"));
}

TEST(UtilTest, GetSize)
{
    EXPECT_EQ("768 B", getsize(768));
    EXPECT_EQ("1 Kb", getsize(1025));
    EXPECT_EQ("2 Mb", getsize(2 * 1024 * 1024));
    EXPECT_EQ("2 Gb", getsize(2 * 1024 * 1024 * 1024LL));
}

#include "Sound.hpp"

constexpr bool Add(std::array<std::string_view, ToUint32(SfxEffect::COUNT)> &ref, SfxEffect sample,
                   std::string_view name)
{
    // return ref[ToUint32(sample)].empty();
    //     auto &s = ref[ToUint32(sample)];
    //     if (s.empty())
    //     {
    //         s = name;
    //         return false;
    //     }
    return true;
}

constexpr std::array<std::string_view, ToUint32(SfxEffect::COUNT)> GenerateSampleFileNames()
{
    std::array<std::string_view, ToUint32(SfxEffect::COUNT)> ref;
    static_assert(Add(ref, SfxEffect::ak74_fire, "ak74-fire.wav"), "f");
    return ref;
}

constexpr auto sample_files = GenerateSampleFileNames();

TEST(Sample, GetNiceSample)
{
    std::string_view v;
}
