#include "DebugWindow.hpp"
// clang-format off
#include <glad/glad.h>
// clang-format on
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl.h>
#include <imgui.h>

DebugWindow::DebugWindow(SDL_Window *window, SDL_GLContext context)
{
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForOpenGL(window, context);
  ImGui_ImplOpenGL3_Init("#version 100");
  ImGui::StyleColorsLight();
}

void DebugWindow::BeginFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void DebugWindow::EndFrame()
{
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

DebugWindow::~DebugWindow()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

// tests
#include "client/SdlApp.hpp"
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
  app.RegisterEventInterception([](SDL_Event &evt) { ImGui_ImplSDL2_ProcessEvent(&evt); });
  DebugWindow dw(app.GetWindow(), app.GetContext());
  auto i = 1024;
  while (i--)
  {
    glClear(GL_COLOR_BUFFER_BIT);
    app.ProcessEvents();
    dw.BeginFrame();
    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::End();
    dw.EndFrame();
    SDL_GL_SwapWindow(app.GetWindow());
  }
}
