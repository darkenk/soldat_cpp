#include "common/misc/TIniFile.hpp"
#include "common/misc/TMemoryStream.hpp"
#include <gtest/gtest.h>
#include <string>

std::string_view TestIni = R"(
[Section1]
Entry1=1
Entry2=1.2

[Section2]
Entry1=test
Entry2=complex string
; some comments
)";

TEST(TIniFileTest, ReadSectionValues)
{
    TIniFile ini(ReadAsMemoryStream(TestIni));
    TIniFile::Entries out;
    ini.ReadSectionValues("Section1", out);
    ASSERT_EQ(2, out.size());
    EXPECT_STREQ("1", out.at("Entry1").data());
    EXPECT_STREQ("1.2", out.at("Entry2").data());
}

TEST(TIniFileTest, OmittsComment)
{
    TIniFile ini(ReadAsMemoryStream(TestIni));
    TIniFile::Entries out;
    ini.ReadSectionValues("Section2", out);
    ASSERT_EQ(2, out.size());
    EXPECT_STREQ("test", out.at("Entry1").data());
    EXPECT_STREQ("complex string", out.at("Entry2").data());
}

TEST(TIniFileTest, ReadTwoDifferentSections)
{
    TIniFile ini(ReadAsMemoryStream(TestIni));
    {
        TIniFile::Entries out;
        ini.ReadSectionValues("Section2", out);
    }
    TIniFile::Entries out;
    ini.ReadSectionValues("Section1", out);
    ASSERT_EQ(2, out.size());
    EXPECT_STREQ("1", out.at("Entry1").data());
    EXPECT_STREQ("1.2", out.at("Entry2").data());
}

TEST(TIniFileTest, ReadSectionValuesReturnFalseInCaseOfMissingFile)
{
    TIniFile ini(nullptr);
    TIniFile::Entries out;
    EXPECT_EQ(false, ini.ReadSectionValues("Section1", out));
}

TEST(TIniFileTest, TrimWhiteSpaceFromLine)
{
    auto section = "[Section1]\r\nEntry1=1\r\nEntry2=1.2\r\n";
    TIniFile ini(ReadAsMemoryStream(section));
    TIniFile::Entries out;
    EXPECT_EQ(true, ini.ReadSectionValues("Section1", out));
    ASSERT_EQ(2, out.size());
    EXPECT_STREQ("1", out.at("Entry1").data());
    EXPECT_STREQ("1.2", out.at("Entry2").data());
}

TEST(TIniFileTest, ReadReturnsFalseInCaseOfMissingSection)
{
    TIniFile ini(ReadAsMemoryStream(TestIni));
    TIniFile::Entries out;
    EXPECT_EQ(false, ini.ReadSectionValues("NonExistedSection", out));
    EXPECT_EQ(0, out.size());
}
