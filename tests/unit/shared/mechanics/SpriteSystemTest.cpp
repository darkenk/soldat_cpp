#include "shared/mechanics/SpriteSystem.hpp"
#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

using testing::Each;
using testing::Eq;
using testing::Field;

class SpriteSystemTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    SpriteSystem::Init();
  }

  void TearDown() override
  {
    SpriteSystem::Deinit();
  }
};

TEST_F(SpriteSystemTest, GetActiveSprites)
{
  auto &sprites = SpriteSystem::Get().GetActiveSprites();

  EXPECT_THAT(sprites, Each(Field("Active", &tsprite::active, Eq(true))));
}

TEST_F(SpriteSystemTest, IdMatchesRequestedSprite)
{
  auto &system = SpriteSystem::Get();
  auto &sprite = system.GetSprite(3);
  EXPECT_EQ(sprite.num, 3);
}

TEST_F(SpriteSystemTest, GetActiveSpritesDoubleCheck)
{
  auto &system = SpriteSystem::Get();
  auto &sprite = system.CreateSprite();
  sprite.active = true;

  auto &sprites = SpriteSystem::Get().GetActiveSprites();

  EXPECT_THAT(sprites, Each(Field("Active", &tsprite::active, Eq(true))));
  EXPECT_THAT(sprites, Each(Field("Num", &tsprite::num, Eq(1))));
}
