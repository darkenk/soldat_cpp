#pragma once

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_events.h>

#include <string>
#include <memory>

class FSdlApp;
struct FGlobalStateControlGame
{
	void RegisterEventHandlers(std::shared_ptr<FSdlApp>& InApp);
	bool votekickreasontype = false;

private:
	auto chatkeydown(std::uint8_t keymods, SDL_Keycode keycode) -> bool;
	auto filterchattext(const std::string& str1) -> std::string;
	auto keydown(SDL_KeyboardEvent& keyevent) -> bool;
	auto keyup(SDL_KeyboardEvent& keyevent) -> bool;
	auto menukeydown(std::uint8_t keymods, SDL_Scancode keycode) -> bool;
	void clearchattext();
};

extern FGlobalStateControlGame gGlobalStateControlGame;
