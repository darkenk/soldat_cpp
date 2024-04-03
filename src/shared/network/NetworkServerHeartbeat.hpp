#pragma once

class tservernetwork;
template <class TSprite> class TSpriteSystem;
template <Config::Module M> class Game;

template<class TSprite, Config::Module M>
void serverheartbeat(tservernetwork& server, TSpriteSystem<TSprite>& spriteSystem, Game<M>& game);
