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

#include "shared/misc/SignalUtils.hpp"

class SdlApp
{
public:
  SdlApp(const std::string_view appTitle)
  {
    AbortIf(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0,
            "Cannot init SDL. Error {}", SDL_GetError());
    const SDL_WindowFlags window_flags =
      (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    Window = SDL_CreateWindow(appTitle.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280,
                              720, window_flags);
    AbortIf(Window == nullptr, "Failed to create sdl window");
    struct OpenGLVersion
    {
      SDL_GLprofile profile;
      std::uint32_t major;
      std::uint32_t minor;
    };
    constexpr std::array versions{
      OpenGLVersion{SDL_GL_CONTEXT_PROFILE_CORE, 4, 3},
      OpenGLVersion{SDL_GL_CONTEXT_PROFILE_CORE, 3, 0},
      OpenGLVersion{SDL_GL_CONTEXT_PROFILE_ES, 3, 0},
    };

    for (const auto &v : versions)
    {
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, v.profile);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, v.major);
      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, v.minor);

      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

      Context = SDL_GL_CreateContext(Window);
      if (Context)
      {
        // gOpenGLES = v.profile == SDL_GL_CONTEXT_PROFILE_ES;
        break;
      }
    }
    AbortIf(Context == nullptr, "Failed to create gl context");

    glad_set_post_callback(SdlApp::OpenGLGladDebug);
    AbortIf(not gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress), "Failed to initialize GLAD");

    SDL_GL_MakeCurrent(Window, Context);
    SDL_GL_SetSwapInterval(1);
  }

  ~SdlApp()
  {
    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Window);
    SDL_Quit();
  }

  bool ProcessEvents()
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
      {
        return true;
      }
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(Window))
      {
        return true;
      }
    }
    return false;
  }

  static void OpenGLGladDebug(const char *name, void * /*funcptr*/, int /*len_args*/, ...)
  {
    auto TranslateError = [](std::uint32_t errorCode) {
      static const std::map<std::uint32_t, std::string_view> translate{
        {0x0500, "GL_INVALID_ENUM"},
        {0x0501, "GL_INVALID_VALUE"},
        {0x0502, "GL_INVALID_OPERATION"},
        {0x0505, "GL_OUT_OF_MEMORY"}};
      return translate.at(errorCode);
    };

    auto error_code = glad_glGetError();

    if (error_code != GL_NO_ERROR)
    {
      LogErrorG("[GL] ERROR {} in {}", TranslateError(error_code), name);
    }
    SoldatAssert(error_code == GL_NO_ERROR);
  }

  SDL_Window *GetWindow() { return Window; }
  SDL_GLContext GetContext() { return Context; }

private:
  SDL_Window *Window;
  SDL_GLContext Context;
};

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
  DebugWindow dw(app.GetWindow(), app.GetContext());
  bool done = false;
  while (!done)
  {
    glClear(GL_COLOR_BUFFER_BIT);
    done = app.ProcessEvents();
    dw.BeginFrame();
    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::End();
    dw.EndFrame();
    SDL_GL_SwapWindow(app.GetWindow());
  }
}
