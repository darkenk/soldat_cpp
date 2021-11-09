#include "shared/misc/FontAtlas.hpp"
#include "gtest/gtest.h"

TEST(FontAtlasTest, PutSingleRect)
{
    Rectangle r1(0, 0, 3, 3);
    RectangleArea area(8, 8);

    EXPECT_TRUE(area.Add(r1));
    EXPECT_EQ(0, r1.x);
    EXPECT_EQ(0, r1.y);
}

TEST(FontAtlasTest, PutTwoRects)
{
    Rectangle r1(0, 0, 2, 2);
    Rectangle r2(0, 0, 3, 3);
    RectangleArea area(8, 8);

    EXPECT_TRUE(area.Add(r1));
    EXPECT_TRUE(area.Add(r2));

    EXPECT_EQ(0, r1.x);
    EXPECT_EQ(2, r2.x);
    EXPECT_EQ(0, r2.y);
}

TEST(FontAtlasTest, StackRects)
{
    Rectangle r1(0, 0, 2, 2);
    Rectangle r2(0, 0, 7, 3);
    RectangleArea area(8, 8);

    EXPECT_TRUE(area.Add(r1));
    EXPECT_TRUE(area.Add(r2));

    EXPECT_EQ(0, r1.x);
    EXPECT_EQ(0, r2.x);
    EXPECT_EQ(2, r2.y);
}

TEST(FontAtlasTest, PutThreeItemsWhichFillsWholeWidth)
{
    Rectangle r1(0, 0, 1, 2);
    Rectangle r2(0, 0, 1, 1);
    Rectangle r3(0, 0, 2, 2);
    RectangleArea area(4, 4);

    EXPECT_TRUE(area.Add(r1));
    EXPECT_TRUE(area.Add(r2));
    EXPECT_TRUE(area.Add(r3));

    EXPECT_EQ(0, r1.x);
    EXPECT_EQ(0, r1.y);
    EXPECT_EQ(1, r2.x);
    EXPECT_EQ(0, r2.y);
    EXPECT_EQ(2, r3.x);
    EXPECT_EQ(0, r3.y);
}

TEST(FontAtlasTest, WhenMergingNextToPreviousUsePreviousStartPos)
{
    Rectangle r1(0, 0, 2, 3);
    Rectangle r2(0, 0, 1, 1);
    Rectangle r3(0, 0, 2, 2);
    Rectangle r4(0, 0, 3, 1);
    RectangleArea area(5, 4);

    EXPECT_TRUE(area.Add(r1));
    EXPECT_TRUE(area.Add(r2));
    EXPECT_TRUE(area.Add(r3));
    EXPECT_TRUE(area.Add(r4));

    EXPECT_EQ(2, r4.x);
    EXPECT_EQ(2, r4.y);
}
