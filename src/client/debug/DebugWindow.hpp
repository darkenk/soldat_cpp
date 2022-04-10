#pragma once

typedef struct SDL_Window SDL_Window;
typedef void *SDL_GLContext;

class DebugWindow
{
public:
  DebugWindow(SDL_Window *window, SDL_GLContext context);
  void BeginFrame();
  void EndFrame();
  ~DebugWindow();
};
