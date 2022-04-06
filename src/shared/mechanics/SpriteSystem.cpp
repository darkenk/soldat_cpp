#include "SpriteSystem.hpp"
#include <common/Logging.hpp>

template <class TSprite>
TSpriteSystem<TSprite>::TSpriteSystem() : ActiveSprites(Sprites)
{
  Sprites.reserve(max_sprites);
  for (auto i = 0; i < max_sprites; i++)
  {
    Sprites.emplace_back(i + 1);
  }
}

template <class TSprite>
TSprite &TSpriteSystem<TSprite>::CreateSprite(const SpriteId reuseSpriteId)
{
  if (reuseSpriteId != SpriteId::Invalid())
  {
    return GetSprite(reuseSpriteId);
  }

  for (auto i = 0; i < max_sprites; i++)
  {
    if (!Sprites[i].active)
    {
      return Sprites[i];
    }
  }
  NotImplemented("Sprites");
  std::abort();
}

template <class TSprite>
TSprite &TSpriteSystem<TSprite>::GetSprite(const SpriteId id)
{
  return Sprites[id.GetId() - 1];
}

template <class TSprite>
typename TSpriteSystem<TSprite>::TActiveSprites &TSpriteSystem<TSprite>::GetActiveSprites()
{
  return ActiveSprites;
}

template <class TSprite>
void TSpriteSystem<TSprite>::CreateSpritePart(const tvector2 &start, tvector2 &vel,
                                              const float mass, const int32_t num)
{
  spriteparts.createpart(start, vel, mass, num);
}

template <class TSprite>
void TSpriteSystem<TSprite>::ResetSpriteParts()
{
  spriteparts.destroy();
  spriteparts.timestep = 1;
  spriteparts.gravity = grav;
  spriteparts.edamping = 0.99;
}

template <class TSprite>
void TSpriteSystem<TSprite>::UpdateSpriteParts()
{
  for (auto &sprite : GetActiveSprites())
  {
    if (sprite.isnotspectator())
    {
      spriteparts.doeulertimestepfor(sprite.num); // integrate sprite particles
    }
  }
}

template class TSpriteSystem<>;

#include <doctest/doctest.h>
TEST_CASE("Test for CreateSprite")
{
  SpriteSystem::Init();
  auto &sprite = SpriteSystem::Get().CreateSprite();
  CHECK(sprite.num == 1);
  SpriteSystem::Deinit();
}
