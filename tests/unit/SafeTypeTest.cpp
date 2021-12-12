#include "common/misc/SafeType.hpp"
#include <gtest/gtest.h>

TEST(SafeTypeTest, ConvertToBasicType)
{
    MyUInt32<1, 12> t{9};

    EXPECT_EQ(9, t);
}

TEST(SafeTypeTest, AddToBasicType)
{
    MyUInt32<1, 12> t{9};
    t = t + 1;
    EXPECT_EQ(10, t);
}

TEST(SafeTypeTest, IncrementDecrementBasicType)
{
    MyUInt32<1, 12> t{9};
    EXPECT_EQ(9, t++);
    EXPECT_EQ(10, t);
    t = 9;
    EXPECT_EQ(10, ++t);
    EXPECT_EQ(10, t);

    t = 2;
    EXPECT_EQ(2, t--);
    EXPECT_EQ(1, t);
    t = 2;
    EXPECT_EQ(1, --t);
    EXPECT_EQ(1, t);
}

TEST(SafeTypeTest, AssignmentOperators)
{
    MyUInt32<1, 12> t{9};
    t += 2u;
    EXPECT_EQ(11, t);

    t -= 3;
    EXPECT_EQ(8, t);

    t /= 2;
    EXPECT_EQ(4, t);

    t *= 3;
    EXPECT_EQ(12, t);
}
