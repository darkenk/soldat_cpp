#pragma once

#include <functional>
#include <vector>

class SdlApp;

class DebugWindow
{
public:
  using ImGuiDrawFunction = std::function<void()>;

  DebugWindow(SdlApp &app);
  ~DebugWindow();

  void Draw(ImGuiDrawFunction func);
  void DrawEverything();

private:
  std::vector<ImGuiDrawFunction> PendingDrawCalls;
};
