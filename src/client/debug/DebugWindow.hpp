#pragma once

#include <functional>
#include <vector>

class SdlApp;
struct SDL_GPUCommandBuffer;
struct SDL_GPUTexture;

class DebugWindow
{
public:
  using ImGuiDrawFunction = std::function<void()>;

  DebugWindow(SdlApp &app);
  ~DebugWindow();

  template<typename ServiceLocator>
  static void DrawStatic(ImGuiDrawFunction func) {ServiceLocator::Get().DebugWindow().Draw(func);}

  void Draw(ImGuiDrawFunction func);
  void DrawEverything(SDL_GPUCommandBuffer* _command_buffer,  SDL_GPUTexture* _texture);

private:
  std::vector<ImGuiDrawFunction> PendingDrawCalls;
  SdlApp &mApp;
};
