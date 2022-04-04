#include "common/port_utils/NotImplemented.hpp"
#include <gtest/gtest.h>

using PortUtilities::StringLiteral;
using PortUtilities::NotImplemented::GetArea;
using PortUtilities::NotImplemented::IsDisabled;

// Some compile time tests
static_assert(sizeof(StringLiteral("audio").value) == 6);
static_assert(GetArea() == "GENERIC");
static_assert(GetArea("AUDIO") == "AUDIO");
static_assert(GetArea("AUDIO", "some description") == "AUDIO");
static_assert(IsDisabled("AUDIO_TEST") == true);
static_assert(IsDisabled("GENERIC") == false);
static_assert(IsDisabled("SOME_DUMMY_NAME") == false);

using NIHandler = decltype(&PortUtilities::NotImplemented::DefaultShowNotImplementedMessage);

static thread_local std::function<void(const std::string_view, const std::string_view,
                                       const source_location &)>
    gNIHandler = nullptr;

void TestShowNotImplementedMessage(const std::string_view area, const std::string_view msg,
                                   const source_location &location)
{
    gNIHandler(area, msg, location);
}

static void SimpleNotImplementedFunction()
{
    NotImplemented("test", "sample_msg");
}

TEST(NotImplementedTests, CallOnlyOnce)
{
    auto calls = 0;
    gNIHandler = [&calls](const std::string_view, const std::string_view, const source_location &) {
        calls++;
    };
    SimpleNotImplementedFunction();
    SimpleNotImplementedFunction();
    EXPECT_EQ(1, calls);
}

TEST(NotImplementedTests, VerifySourceLocation)
{
    source_location location;
    gNIHandler = [&location](const std::string_view, const std::string_view,
                             const source_location &loc) { location = loc; };
#line 42
    NotImplemented();
    EXPECT_EQ(42, location.line());
}

TEST(NotImplementedTests, DisabledAreasAreNotCalled)
{
    auto calls = 0;
    gNIHandler = [&calls](const std::string_view, const std::string_view, const source_location &) {
        calls++;
    };
    NotImplemented("AUDIO_TEST");
    EXPECT_EQ(0, calls);
}
