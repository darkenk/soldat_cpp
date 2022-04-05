#include "common/Calc.hpp"
#include "gtest/gtest.h"

TEST(CompilationTest, CalcTest)
{
  tvector2 start(-1.0f, 0.0f);
  tvector2 end(1.0f, 0.0f);
  tvector2 circle(0.0f, 0.0f);
  auto radius = 0.5f;
  auto ret = islineintersectingcircle(start, end, circle, radius);
  EXPECT_EQ(ret.numintersections, 2);
  EXPECT_EQ(ret.points[0].x, -0.5f);
  EXPECT_EQ(ret.points[0].y, 0.0f);
  EXPECT_EQ(ret.points[1].x, 0.5f);
  EXPECT_EQ(ret.points[1].y, 0.0f);
}
