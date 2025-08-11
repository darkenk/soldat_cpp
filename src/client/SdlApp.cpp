#include "SdlApp.hpp"

#include <glad/glad.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>

#include "shared/misc/SignalUtils.hpp"
#include "common/Logging.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"

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

static auto CreateOpenGLContext(SDL_Window *window) -> SDL_GLContext
{
  SDL_GLContext context = nullptr;
  struct OpenGLVersion
  {
    SDL_GLProfile profile;
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
    if (context != nullptr)
    {
      // gOpenGLES = v.profile == SDL_GL_CONTEXT_PROFILE_ES;
      break;
    }
  }
  return context;
}

SdlApp::SdlApp(const std::string_view appTitle, const int32_t width, const int32_t height, bool opengl)
{
  AbortIf(SDL_Init(SDL_INIT_VIDEO /*| SDL_INIT_GAMEPAD*/) == false,
          "Cannot init SDL. Error {}", SDL_GetError());

  if (!opengl)
  {
    mDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, true, nullptr);
    AbortIf(mDevice == nullptr, "Failed to create gpu device");
  }
  
  int num_displays;
  SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);
  AbortIf(num_displays == 0, "Failed to get displays");
  for (int i = 0; i < num_displays; i++)
  {
    SDL_Rect display_bounds;
    SDL_GetDisplayBounds(displays[i], &display_bounds);
    LogInfoG("Display {} bounds: {}x{}, pos: {}:{}", i, display_bounds.w, display_bounds.h, display_bounds.x, display_bounds.y);
  }
  SDL_free(displays);

  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, appTitle.data());
  //SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
  SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 10);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 10);
  SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN, true);
  SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, !opengl);
  SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, opengl);

  mWindow = SDL_CreateWindowWithProperties(props);
  AbortIf(mWindow == nullptr, "Failed to create sdl window");
  SDL_DestroyProperties(props);

  if (!opengl)
  {
	  AbortIf(!SDL_ClaimWindowForGPUDevice(mDevice, mWindow), "Failed to claim window for gpu device. Error {}", SDL_GetError());
  }
}

SdlApp::~SdlApp()
{
  if (mDevice)
  {
	  SDL_ReleaseWindowFromGPUDevice(mDevice, mWindow);
  }
  SDL_DestroyWindow(mWindow);
  if (mDevice)
  {
    SDL_DestroyGPUDevice(mDevice);
  }
  SDL_Quit();
}

auto SdlApp::RegisterEventHandler(SDL_EventType evt, HandlerType handler) -> bool
{
  if (mEventHandlers.contains(evt))
  {
    LogWarnG("Cannot register another handler for event 0x{0:x}", evt);
    return false;
  }
  mEventHandlers[evt] = handler;
  return true;
}

void SdlApp::RegisterEventInterception(HandlerType handler)
{
  mEventInterceptors.emplace_back(handler);
}

void SdlApp::ProcessEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0)
  {
    for (auto &intercept : mEventInterceptors)
    {
      intercept(event);
    }
    auto handler = mEventHandlers.find(static_cast<SDL_EventType>(event.type));
    if (handler != mEventHandlers.end())
    {
      handler->second(event);
    }
  }
}

void SdlApp::Present() { NotImplemented("Present"); }

#include <doctest/doctest.h>
#include <array>
#include <utility>

TEST_CASE("Create SDL window")
{
  SdlApp app("Test app");
  CHECK(app.GetWindow() != nullptr);
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
