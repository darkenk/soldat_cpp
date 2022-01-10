#include "SpriteSystem.hpp"
#include <common/Logging.hpp>

template <class TSprite>
TSpriteSystem<TSprite>::TSpriteSystem() : ActiveSprites(Sprites)
{
}

template <class TSprite>
TSprite &TSpriteSystem<TSprite>::GetSprite(int32_t i)
{
    return Sprites[i];
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
