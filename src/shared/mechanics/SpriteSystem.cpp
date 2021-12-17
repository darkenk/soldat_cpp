#include "SpriteSystem.hpp"

template <class TSprite>
TSprite &TSpriteSystem<TSprite>::GetSprite(int32_t i)
{
    return Sprites[i];
}

template class TSpriteSystem<>;
