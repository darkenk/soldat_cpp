#include "shared/mechanics/Sprites.hpp"
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

using testing::Each;
using testing::Eq;
using testing::Field;

TEST(SpriteTest, BringIntoTestHarness)
{
    auto s = Sprite();
}
