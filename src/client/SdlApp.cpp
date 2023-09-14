#include "SdlApp.hpp"
#include "shared/misc/SignalUtils.hpp"
#include <glad/glad.h>

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

static SDL_GLContext CreateOpenGLContext(SDL_Window *window)
{
  SDL_GLContext context = nullptr;
  struct OpenGLVersion
  {
    SDL_GLprofile profile;
    std::uint32_t major;
    std::uint32_t minor;
  };
  constexpr std::array versions{
    OpenGLVersion{SDL_GL_CONTEXT_PROFILE_CORE, 4, 3},
    OpenGLVersion{SDL_GL_CONTEXT_PROFILE_CORE, 3, 0},
    OpenGLVersion{SDL_GL_CONTEXT_PROFILE_CORE, 2, 0},
    OpenGLVersion{SDL_GL_CONTEXT_PROFILE_ES, 3, 0},
    OpenGLVersion{SDL_GL_CONTEXT_PROFILE_ES, 2, 0},
  };

  for (const auto &v : versions)
  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, v.profile);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, v.major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, v.minor);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    context = SDL_GL_CreateContext(window);
    if (context)
    {
      // gOpenGLES = v.profile == SDL_GL_CONTEXT_PROFILE_ES;
      break;
    }
  }
  return context;
}

SdlApp::SdlApp(const std::string_view appTitle, const int32_t width, const int32_t height)
{
  AbortIf(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0,
          "Cannot init SDL. Error {}", SDL_GetError());
  const SDL_WindowFlags window_flags =
    (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  Window = SDL_CreateWindow(appTitle.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
                            height, window_flags);
  AbortIf(Window == nullptr, "Failed to create sdl window");

  Context = CreateOpenGLContext(Window);
  AbortIf(Context == nullptr, "Failed to create gl context");

  glad_set_post_callback(OpenGLGladDebug);
#if __EMSCRIPTEN__ && 0
  AbortIf(not gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress), "Failed to initialize GLAD");
#else
  AbortIf(not gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress), "Failed to initialize GLAD");
#endif

  SDL_GL_MakeCurrent(Window, Context);
  SDL_GL_SetSwapInterval(1);
}

SdlApp::~SdlApp()
{
  SDL_GL_DeleteContext(Context);
  SDL_DestroyWindow(Window);
  SDL_Quit();
}

bool SdlApp::RegisterEventHandler(SDL_EventType evt, HandlerType handler)
{
  if (EventHandlers.contains(evt))
  {
    LogWarnG("Cannot register another handler for event 0x{0:x}", evt);
    return false;
  }
  EventHandlers[evt] = handler;
  return true;
}

void SdlApp::RegisterEventInterception(HandlerType handler)
{
  EventInterceptors.emplace_back(handler);
}

void SdlApp::ProcessEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    for (auto &intercept : EventInterceptors)
    {
      intercept(event);
    }
    auto handler = EventHandlers.find(static_cast<SDL_EventType>(event.type));
    if (handler != EventHandlers.end())
    {
      handler->second(event);
    }
  }
}

void SdlApp::Present() { SDL_GL_SwapWindow(Window); }

#include <doctest/doctest.h>

TEST_CASE("Create opengl window with sdl")
{
  SdlApp app("Test app");
  CHECK(app.GetContext() != nullptr);
}

TEST_CASE("Window has desired sie")
{
  SdlApp app("Test app", 256, 128);
  app.Present();
  std::int32_t w = 0;
  std::int32_t h = 0;
  SDL_GetWindowSize(app.GetWindow(), &w, &h);
  CHECK(w == 256);
  CHECK(h == 128);
}

TEST_CASE("ProcessEvents triggers handler")
{
  SdlApp app("Test app");
  bool triggered = false;
  auto handler = [&triggered](SDL_Event &evt) { triggered = true; };
  auto myEvent = (SDL_EventType)SDL_RegisterEvents(1);
  auto b = app.RegisterEventHandler(myEvent, handler);
  CHECK(b == true);
  SDL_Event evt;
  evt.type = myEvent;
  SDL_PushEvent(&evt);
  app.ProcessEvents();
  CHECK(triggered == true);
}

TEST_CASE("Handler can be registered only once")
{
  SdlApp app("Test app");
  bool triggered = false;
  auto handler = [&triggered](SDL_Event &evt) { triggered = true; };
  auto myEvent = (SDL_EventType)SDL_RegisterEvents(1);
  {
    auto b = app.RegisterEventHandler(myEvent, handler);
    CHECK(b == true);
  }
  {
    auto b = app.RegisterEventHandler(myEvent, handler);
    CHECK(b == false);
  }
}

TEST_CASE("Event interception is called for every event")
{
  SdlApp app("Test app");
  bool triggered = false;
  auto handler = [&triggered](SDL_Event &evt) { triggered = true; };
  auto myEvent = (SDL_EventType)SDL_RegisterEvents(1);
  app.RegisterEventInterception(handler);
  SDL_Event evt;
  evt.type = myEvent;
  SDL_PushEvent(&evt);
  app.ProcessEvents();
  CHECK(triggered == true);
}
