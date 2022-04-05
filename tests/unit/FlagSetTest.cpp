#include "shared/misc/FlagSet.hpp"
#include <gtest/gtest.h>

enum class TestFlags
{
  NONE = 0x0,
  ONE = 0x1,
  TWO = 0x2,
  THREE = 0x4
};

template <>
struct EnableFlagSet<TestFlags>
{
  static constexpr bool enable = true;
};

TEST(FlagSetTest, SetTwoFlags)
{
  FlagSet flagTest{TestFlags::ONE};
  flagTest = TestFlags::ONE | TestFlags::TWO;
  EXPECT_EQ(true, flagTest.Test(TestFlags::ONE));
  EXPECT_EQ(true, flagTest.Test(TestFlags::TWO));
  EXPECT_EQ(false, flagTest.Test(TestFlags::THREE));
}

TEST(FlagSetTest, SetThreeFlags)
{
  FlagSet flagTest{TestFlags::ONE};
  flagTest = TestFlags::ONE | TestFlags::TWO | TestFlags::THREE;
  EXPECT_EQ(true, flagTest.Test(TestFlags::TWO));
  EXPECT_EQ(true, flagTest.Test(TestFlags::THREE));
}

TEST(FlagSetTest, AndOperation)
{
  FlagSet flagTest{TestFlags::ONE | TestFlags::TWO};
  flagTest = flagTest & TestFlags::TWO;
  EXPECT_EQ(true, flagTest.Test(TestFlags::TWO));
  EXPECT_EQ(false, flagTest.Test(TestFlags::ONE));
}
