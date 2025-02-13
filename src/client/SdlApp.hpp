#pragma once

#include <SDL3/SDL_events.h>
#include <functional>
#include <map>

typedef struct SDL_Window SDL_Window;
typedef struct SDL_GLContextState *SDL_GLContext;

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

  SDL_Window *GetWindow() { return mWindow; }
  SDL_GPUDevice *GetDevice() { return mDevice; }

private:
  SDL_Window *mWindow;
  SDL_GPUDevice *mDevice;
  std::map<SDL_EventType, HandlerType> mEventHandlers;
  std::vector<HandlerType> mEventInterceptors;
};
