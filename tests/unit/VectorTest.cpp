#include "common/Vector.hpp"
#include "gtest/gtest.h"

TEST(CompilationTest, VectorTest)
{
  auto v1 = vector2(2.0f, 2.0f);
  auto v2 = vector2(1.0f, 3.0f);
  auto ret1 = vec2add(v1, v2);
  EXPECT_EQ(ret1.x, 3.0f);
  EXPECT_EQ(ret1.y, 5.0f);
  auto ret2 = vec2subtract(v1, v2);
  EXPECT_EQ(ret2.x, 1.0f);
  EXPECT_EQ(ret2.y, -1.0f);
}
