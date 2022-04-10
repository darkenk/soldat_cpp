#pragma once

#include <SDL_events.h>
#include <functional>
#include <map>

typedef struct SDL_Window SDL_Window;
typedef void *SDL_GLContext;

class SdlApp
{
public:
  using HandlerType = std::function<void(SDL_Event &)>;

  SdlApp(const std::string_view appTitle, const std::int32_t width = 1280,
         const std::int32_t height = 720);
  ~SdlApp();

  bool RegisterEventHandler(SDL_EventType evt, HandlerType handler);
  // probably only for imgui
  void RegisterEventInterception(HandlerType handler);
  void ProcessEvents();
  void Present();

  SDL_Window *GetWindow() { return Window; }
  SDL_GLContext GetContext() { return Context; }

private:
  SDL_Window *Window;
  SDL_GLContext Context;
  std::map<SDL_EventType, HandlerType> EventHandlers;
  std::vector<HandlerType> EventInterceptors;
};
