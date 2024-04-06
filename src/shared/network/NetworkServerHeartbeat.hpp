#pragma once

class ServerNetwork;
template <class TSprite> class TSpriteSystem;
template <Config::Module M> class Game;

template<class TSprite, Config::Module M>
void serverheartbeat(ServerNetwork& server, TSpriteSystem<TSprite>& spriteSystem, Game<M>& game);
