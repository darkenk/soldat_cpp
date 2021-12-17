#pragma once
#include "common/misc/Singleton.hpp"
#include "Sprites.hpp"

template<class TSprite = tsprite>
class TSpriteSystem : public Singleton<TSpriteSystem<TSprite>>
{
  public:
    TSprite& GetSprite(std::int32_t i);

    PascalArray<TSprite, 1, max_sprites>& GetSprites() { return Sprites;}
  private:
    PascalArray<TSprite, 1, max_sprites> Sprites;
};

using SpriteSystem = TSpriteSystem<>;
