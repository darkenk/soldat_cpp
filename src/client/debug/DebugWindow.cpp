#include "DebugWindow.hpp"
// clang-format off
#include <glad/glad.h>
// clang-format on
#include "client/SdlApp.hpp"
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <imgui.h>

DebugWindow::DebugWindow(SdlApp &app)
{
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForOpenGL(app.GetWindow(), app.GetContext());
  ImGui_ImplOpenGL3_Init("#version 100");
  ImGui::StyleColorsLight();
  app.RegisterEventInterception([](SDL_Event &evt) { ImGui_ImplSDL2_ProcessEvent(&evt); });
}

DebugWindow::~DebugWindow()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void DebugWindow::Draw(ImGuiDrawFunction func) { PendingDrawCalls.push_back(func); }

void DebugWindow::DrawEverything()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  for (auto &drawImGui : PendingDrawCalls)
  {
    drawImGui();
  }
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  PendingDrawCalls.clear();
}

// tests
#include <doctest/doctest.h>

class DebugWindowFixture
{
public:
  DebugWindowFixture() {}
  ~DebugWindowFixture() {}

protected:
  DebugWindowFixture(const DebugWindowFixture &) = delete;
};

TEST_CASE_FIXTURE(DebugWindowFixture, "Check whether debug window is displayed")
{
  SdlApp app("Test Window");
  DebugWindow dw(app);
  auto i = 1024;
  while (i--)
  {
    glClear(GL_COLOR_BUFFER_BIT);
    app.ProcessEvents();
    dw.Draw([]() {
      ImGui::Begin("Hello, world!");
      ImGui::Text("This is some useful text.");
      ImGui::End();
    });
    dw.DrawEverything();
    app.Present();
  }
}
