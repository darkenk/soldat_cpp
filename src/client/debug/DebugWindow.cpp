#include "DebugWindow.hpp"
// clang-format off
#include <glad/glad.h>
// clang-format on
#include "client/SdlApp.hpp"

#define DK_ES2 0
#if __EMSCRIPTEN__ && DK_ES2
#include <backends/imgui_impl_opengl2.h>
#else
#include <backends/imgui_impl_opengl3.h>
#endif
#include <backends/imgui_impl_sdl.h>
#include <imgui.h>

DebugWindow::DebugWindow(SdlApp &app)
{
  ImGui::CreateContext();
  ImGui_ImplSDL2_InitForOpenGL(app.GetWindow(), app.GetContext());
#if __EMSCRIPTEN__ && DK_ES2
  ImGui_ImplOpenGL2_Init();
#else
  ImGui_ImplOpenGL3_Init("#version 100");
#endif
  ImGui::StyleColorsLight();
  app.RegisterEventInterception([](SDL_Event &evt) { ImGui_ImplSDL2_ProcessEvent(&evt); });
}

DebugWindow::~DebugWindow()
{
#if __EMSCRIPTEN__ && DK_ES2
  ImGui_ImplOpenGL2_Shutdown();
#else
  ImGui_ImplOpenGL3_Shutdown();
#endif
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void DebugWindow::Draw(ImGuiDrawFunction func) { PendingDrawCalls.push_back(func); }

void DebugWindow::DrawEverything()
{
#if __EMSCRIPTEN__ && DK_ES2
  ImGui_ImplOpenGL2_NewFrame();
#else
  ImGui_ImplOpenGL3_NewFrame();
#endif
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  for (auto &drawImGui : PendingDrawCalls)
  {
    drawImGui();
  }
  ImGui::Render();
#if __EMSCRIPTEN__ && DK_ES2
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
#else
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
  PendingDrawCalls.clear();
}


// tests
#include <doctest/doctest.h>

namespace
{

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
  auto i = 1;
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

struct SampleServiceLocator
{
    static SampleServiceLocator& Get() { return *s_SampleServiceLocator; }
    inline class DebugWindow& DebugWindow() { return *reinterpret_cast<class DebugWindow*>(window.data()); }
    ::std::array<::std::byte, sizeof(class DebugWindow)> window;
    static SampleServiceLocator *s_SampleServiceLocator;
};

SampleServiceLocator* SampleServiceLocator::s_SampleServiceLocator = nullptr;

TEST_CASE_FIXTURE(DebugWindowFixture, "Draw without passing DebugWindow to function")
{
    SdlApp app("t1");
    SampleServiceLocator::s_SampleServiceLocator = new SampleServiceLocator;
    new (SampleServiceLocator::Get().window.data())DebugWindow(app);
    DebugWindow::DrawStatic<SampleServiceLocator>([](){});
    delete SampleServiceLocator::s_SampleServiceLocator;
}

}
