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
  NotImplemented("sdl3");
  //ImGui_ImplSDL2_InitForOpenGL(app.GetWindow(), app.GetContext());
  ImGui_ImplOpenGL3_Init("#version 100");
  ImGui::StyleColorsLight();
  NotImplemented("sdl3");
  //app.RegisterEventInterception([](SDL_Event &evt) { ImGui_ImplSDL2_ProcessEvent(&evt); });
}

DebugWindow::~DebugWindow()
{
  ImGui_ImplOpenGL3_Shutdown();
  NotImplemented("sdl3");
  //ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void DebugWindow::Draw(ImGuiDrawFunction func) { PendingDrawCalls.push_back(func); }

void DebugWindow::DrawEverything()
{
  ImGui_ImplOpenGL3_NewFrame();
  //ImGui_ImplSDL2_NewFrame();
  NotImplemented("sdl3");
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
#include <doctest.h>

namespace
{

class DebugWindowFixture
{
public:
  DebugWindowFixture() = default;
  ~DebugWindowFixture() = default;
  DebugWindowFixture(const DebugWindowFixture &) = delete;
};

TEST_CASE_FIXTURE(DebugWindowFixture, "Check whether debug window is displayed" * doctest::skip(true))
{
  SdlApp app("Test Window");
  DebugWindow dw(app);
  auto i = 1;
  while ((i--) != 0)
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
  static auto Get() -> SampleServiceLocator & { return *s_SampleServiceLocator; }
  auto DebugWindow() -> class DebugWindow &
  {
    return *reinterpret_cast<class DebugWindow *>(window.data());
    }
    ::std::array<::std::byte, sizeof(class DebugWindow)> window;
    static SampleServiceLocator *s_SampleServiceLocator;
};

SampleServiceLocator* SampleServiceLocator::s_SampleServiceLocator = nullptr;

TEST_CASE_FIXTURE(DebugWindowFixture, "Draw without passing DebugWindow to function" * doctest::skip(true))
{
    SdlApp app("t1");
    SampleServiceLocator::s_SampleServiceLocator = new SampleServiceLocator;
    new (SampleServiceLocator::Get().window.data())DebugWindow(app);
    DebugWindow::DrawStatic<SampleServiceLocator>([](){});
    delete SampleServiceLocator::s_SampleServiceLocator;
}

}
