#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <functional>
#include <map>
#include <string_view>
#include <cstdint>
#include <vector>

using SDL_Window = struct SDL_Window;
using SDL_GLContext = struct SDL_GLContextState*;
using SDL_GPUDevice = struct SDL_GPUDevice; // NOLINT(readability-identifier-naming)

class FSdlApp
{
public:
	using THandlerType = std::function<void(SDL_Event&)>;

	FSdlApp(const FSdlApp&) = delete;
	FSdlApp(FSdlApp&&) = delete;
	FSdlApp& operator=(const FSdlApp&) = delete;
	FSdlApp& operator=(FSdlApp&&) = delete;
	explicit FSdlApp(
		std::string_view InAppTitle, std::int32_t InWidth = 1280, std::int32_t InHeight = 720, bool InOpengl = false);
	~FSdlApp();

	bool RegisterEventHandler(SDL_EventType InEvt, THandlerType InHandler);
	// probably only for imgui
	void RegisterEventInterception(const THandlerType& InHandler);
	void ProcessEvents();
	void Present();

	SDL_Window* GetWindow() { return Window; }
	SDL_GPUDevice* GetDevice() { return Device; }

private:
	SDL_Window* Window;
	SDL_GPUDevice* Device;
	std::map<SDL_EventType, THandlerType> EventHandlers;
	std::vector<THandlerType> EventInterceptors;
};
