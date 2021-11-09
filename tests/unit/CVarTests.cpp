#include "shared/Cvar.hpp"
#include <gtest/gtest.h>

TEST(CVarTest, ParseAndSetValueCorrectValue)
{
    CVarInt test{"t1", "", CVarFlags::NONE, 0};
    auto ret = test.ParseAndSetValue("12");
    EXPECT_EQ(12, test);
    EXPECT_EQ(true, ret);
}

TEST(CVarTest, ParseAndDontSetWrongValue)
{
    CVarInt test{"t2", "", CVarFlags::NONE, 11};
    auto ret = test.ParseAndSetValue("aa");
    EXPECT_EQ(11, test);
    EXPECT_EQ(false, ret);
}

TEST(CVarTest, ParseAndSetBoolValue)
{
    {
        CVarBool test{"b1", "", CVarFlags::NONE, false};
        auto ret = test.ParseAndSetValue("true");
        EXPECT_EQ(true, test);
        EXPECT_EQ(true, ret);
    }
    {
        CVarBool test{"b2", "", CVarFlags::NONE, false};
        auto ret = test.ParseAndSetValue("True");
        EXPECT_EQ(true, test);
        EXPECT_EQ(true, ret);
    }
    {
        CVarBool test{"b3", "", CVarFlags::NONE, true};
        auto ret = test.ParseAndSetValue("False");
        EXPECT_EQ(false, test);
        EXPECT_EQ(true, ret);
    }
    {
        CVarBool test{"b4", "", CVarFlags::NONE, true};
        auto ret = test.ParseAndSetValue("false");
        EXPECT_EQ(false, test);
        EXPECT_EQ(true, ret);
    }
}

TEST(CVarTest, ParseAndDontSetBoolValue)
{
    {
        CVarBool test{"b5", "", CVarFlags::NONE, false};
        auto ret = test.ParseAndSetValue("fas");
        EXPECT_EQ(false, test);
        EXPECT_EQ(false, ret);
    }
    {
        CVarBool test{"b6", "", CVarFlags::NONE, true};
        auto ret = test.ParseAndSetValue("FalseFalse");
        EXPECT_EQ(true, test);
        EXPECT_EQ(false, ret);
    }
}

TEST(CVarTest, ParseAndSetStringCorrectValue)
{
    CVarString test{"s1", "", CVarFlags::NONE, ""};
    auto ret = test.ParseAndSetValue("random string");
    EXPECT_STREQ("random string", ((std::string)test).c_str());
    EXPECT_EQ(true, ret);
}

TEST(CVarTest, ParseAndSetFloatCorretValue)
{
    {
        CVarFloat test{"f1", "", CVarFlags::NONE, 11.0f};
        auto ret = test.ParseAndSetValue("12.0");
        EXPECT_EQ(12.0f, test);
        EXPECT_EQ(true, ret);
    }
    {
        CVarFloat test{"f2", "", CVarFlags::NONE, 11.0f};
        auto ret = test.ParseAndSetValue("12.1f");
        EXPECT_EQ(12.1f, test);
        EXPECT_EQ(true, ret);
    }
    {
        CVarFloat test{"f3", "", CVarFlags::NONE, 11.0f};
        auto ret = test.ParseAndSetValue("12");
        EXPECT_EQ(12.0f, test);
        EXPECT_EQ(true, ret);
    }
}

TEST(CVarTest, ParseAndDontSetFloatWrongValue)
{
    CVarFloat test{"f4", "", CVarFlags::NONE, 12.0f};
    auto ret = test.ParseAndSetValue("aa");
    EXPECT_EQ(12.0f, test);
    EXPECT_EQ(false, ret);
}

TEST(CVarTest, ValueAsString)
{
    CVarInt test{"t3", "", CVarFlags::NONE, 12};
    auto ret = test.ValueAsString();
    EXPECT_STREQ("12", ret.c_str());
}

TEST(CVarTest, FindCreatedCVar)
{
    CVarInt test{"test", "some test description", CVarFlags::NONE, 12};

    auto &ref = CVarInt::Find("test");
    EXPECT_EQ(true, ref.IsValid());
    EXPECT_EQ(12, ref);
}

TEST(CVarTest, DoNotCreateCVarTwice)
{
    CVarInt test{"test1", "some test description", CVarFlags::NONE, 12};
    EXPECT_ANY_THROW({ CVarInt test1("test1", "some test description", CVarFlags::NONE, 12); });
}

TEST(CVarTest, AssingOneCVarToAnother)
{
    CVarInt t1{"t4", "", CVarFlags::NONE, 12};
    CVarInt t2{"t5", "", CVarFlags::NONE, 13};
    t1 = t2;
    EXPECT_EQ(13, t1);
}

TEST(CVarTest, IterateOverAllCVars)
{
    struct Dummy
    {
        Dummy(std::int32_t v) : value{v}
        {
        }
        std::int32_t value;
        bool operator==(const Dummy &other) const
        {
            return value == other.value;
        }
    };
    using CVarDummy = CVarBase<Dummy>;

    // random order
    CVarDummy cv1{"dummy1", "", CVarFlags::NONE, 1};
    CVarDummy cv2{"dummy2", "", CVarFlags::NONE, 2};

    EXPECT_EQ(1, std::count(CVarDummy::GetAllCVars().begin(), CVarDummy::GetAllCVars().end(), 1));

    std::int32_t n = 1;
    for (const auto &cv : CVarDummy::GetAllCVars())
    {
        EXPECT_EQ(n++, cv);
    }
}
