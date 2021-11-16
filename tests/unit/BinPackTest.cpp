#include "BinPack.hpp"
#include "shared/misc/SoldatConfig.hpp"
#include "gtest/gtest.h"

TEST(CompilationTest, BinPackTest)
{
    auto w = 8;
    auto h = 8;
    tbprectarray rects;
    rects.push_back(tbprect(w, h));
    rects.push_back(tbprect(w, h));

    auto ret = packrects(8, 16, rects);
    EXPECT_EQ(ret, 2);
    EXPECT_EQ(rects[0].x, 0);
    EXPECT_EQ(rects[0].y, 0);
    EXPECT_EQ(rects[1].x, 0);
    EXPECT_EQ(rects[1].y, 8);
}
