#include "shared/misc/TIniFile.hpp"
#include <gtest/gtest.h>
#include <string>

class TestStream : public TStream
{
  public:
    TestStream(const std::string_view &testContent) : Content{testContent.data()}
    {
    }

    ~TestStream() = default;

    bool ReadLine(std::string &out) override
    {
        if (Content.eof())
        {
            return false;
        }
        std::getline(Content, out);
        return true;
    }

    void Reset() override
    {
        Content.clear(std::istringstream::goodbit);
        Content.seekg(0);
    }

  private:
    std::istringstream Content;
};

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
    TIniFile ini(std::make_unique<TestStream>(TestIni));
    TIniFile::Entries out;
    ini.ReadSectionValues("Section1", out);
    ASSERT_EQ(2, out.size());
    EXPECT_STREQ("1", out.at("Entry1").data());
    EXPECT_STREQ("1.2", out.at("Entry2").data());
}

TEST(TIniFileTest, OmittsComment)
{
    TIniFile ini(std::make_unique<TestStream>(TestIni));
    TIniFile::Entries out;
    ini.ReadSectionValues("Section2", out);
    ASSERT_EQ(2, out.size());
    EXPECT_STREQ("test", out.at("Entry1").data());
    EXPECT_STREQ("complex string", out.at("Entry2").data());
}

TEST(TIniFileTest, ReadTwoDifferentSections)
{
    TIniFile ini(std::make_unique<TestStream>(TestIni));
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
