#pragma once

class NetworkServer;
template <class TSprite> class TSpriteSystem;
template <Config::Module M> class Game;

template<class TSprite, Config::Module M>
void serverheartbeat(NetworkServer& server, TSpriteSystem<TSprite>& spriteSystem, Game<M>& game);
