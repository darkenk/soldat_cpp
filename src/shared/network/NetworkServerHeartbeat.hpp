#pragma once

#include "common/misc/SoldatConfig.hpp" // IWYU pragma: keep

class NetworkServer;
template <class TSprite> class TSpriteSystem;
template <Config::Module M> class Game;

template<class TSprite, Config::Module M>
void serverheartbeat(NetworkServer& transport, TSpriteSystem<TSprite>& spriteSystem, Game<M>& game);
