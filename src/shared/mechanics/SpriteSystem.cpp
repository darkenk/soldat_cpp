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

template class TSpriteSystem<>;
