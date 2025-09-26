#include "SdlApp.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <cstdint>
#include <glad/glad.h>
#include <map>
#include <spdlog/fmt/bundled/core.h>
#include <string_view>

#include "shared/misc/SignalUtils.hpp"
#include "common/Logging.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/port_utils/NotImplemented.hpp"

namespace
{
	void OpenGLGladDebug(const char* InName, void* /*funcptr*/, int /*len_args*/, ...) // NOLINT(cert-dcl50-cpp)
	{
		auto TranslateError = [](std::uint32_t InErrorCode)
		{
			static const std::map<std::uint32_t, std::string_view> Translate{
				{ 0x0500,	  "GL_INVALID_ENUM" },
				{ 0x0501,	  "GL_INVALID_VALUE" },
				{ 0x0502, "GL_INVALID_OPERATION" },
				{ 0x0505,	  "GL_OUT_OF_MEMORY" }
			};
			return Translate.at(InErrorCode);
		};

		auto ErrorCode = glad_glGetError();

		if (ErrorCode != GL_NO_ERROR)
		{
			LogErrorG("[GL] ERROR {} in {}", TranslateError(ErrorCode), InName);
		}
		SoldatAssert(ErrorCode == GL_NO_ERROR);
	}

	auto CreateOpenGLContext(SDL_Window* InWindow) -> SDL_GLContext
	{
		SDL_GLContext Context = nullptr;
		struct FOpenGlVersion
		{
			SDL_GLProfile Profile;
			std::uint32_t Major;
			std::uint32_t Minor;
		};
		constexpr std::array kVersions{
			FOpenGlVersion{ .Profile = SDL_GL_CONTEXT_PROFILE_CORE, .Major = 4, .Minor = 3 },
			FOpenGlVersion{ .Profile = SDL_GL_CONTEXT_PROFILE_CORE, .Major = 3, .Minor = 0 },
			FOpenGlVersion{ .Profile = SDL_GL_CONTEXT_PROFILE_CORE, .Major = 2, .Minor = 0 },
			FOpenGlVersion{	.Profile = SDL_GL_CONTEXT_PROFILE_ES, .Major = 3, .Minor = 0 },
			FOpenGlVersion{	.Profile = SDL_GL_CONTEXT_PROFILE_ES, .Major = 2, .Minor = 0 },
		};

		for (const auto& v : kVersions)
		{
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, v.Profile);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, v.Major);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, v.Minor);

			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

			Context = SDL_GL_CreateContext(InWindow);
			if (Context != nullptr)
			{
				// gOpenGLES = v.profile == SDL_GL_CONTEXT_PROFILE_ES;
				break;
			}
		}
		return Context;
	}
} // namespace

FSdlApp::FSdlApp(const std::string_view InAppTitle, const int32_t InWidth, const int32_t InHeight, bool InOpengl)
{
	AbortIf(!SDL_Init(SDL_INIT_VIDEO /*| SDL_INIT_GAMEPAD*/), "Cannot init SDL. Error {}", SDL_GetError());

	if (!InOpengl)
	{
		Device = SDL_CreateGPUDevice(
			SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, true, nullptr);
		AbortIf(Device == nullptr, "Failed to create gpu device");
	}

	int NumDisplays = 0;
	SDL_DisplayID* Displays = SDL_GetDisplays(&NumDisplays);
	AbortIf(NumDisplays == 0, "Failed to get displays");
	for (int i = 0; i < NumDisplays; i++)
	{
		SDL_Rect DisplayBounds;
		SDL_GetDisplayBounds(Displays[i], &DisplayBounds);
		LogInfoG("Display {} bounds: {}x{}, pos: {}:{}",
			i,
			DisplayBounds.w,
			DisplayBounds.h,
			DisplayBounds.x,
			DisplayBounds.y);
	}
	SDL_free(Displays);

	SDL_PropertiesID const Props = SDL_CreateProperties();
	SDL_SetStringProperty(Props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, std::string(InAppTitle).c_str());
	// SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
	SDL_SetNumberProperty(Props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, InWidth);
	SDL_SetNumberProperty(Props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, InHeight);
	SDL_SetBooleanProperty(Props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);
	SDL_SetNumberProperty(Props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 10);
	SDL_SetNumberProperty(Props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 10);
	SDL_SetBooleanProperty(Props, SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN, true);
	SDL_SetBooleanProperty(Props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, !InOpengl);
	SDL_SetBooleanProperty(Props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, InOpengl);

	Window = SDL_CreateWindowWithProperties(Props);
	AbortIf(Window == nullptr, "Failed to create sdl window");
	SDL_DestroyProperties(Props);

	if (!InOpengl)
	{
		AbortIf(!SDL_ClaimWindowForGPUDevice(Device, Window),
			"Failed to claim window for gpu device. Error {}",
			SDL_GetError());
	}
}

FSdlApp::~FSdlApp()
{
	if (Device != nullptr)
	{
		SDL_ReleaseWindowFromGPUDevice(Device, Window);
	}
	SDL_DestroyWindow(Window);
	if (Device != nullptr)
	{
		SDL_DestroyGPUDevice(Device);
	}
	SDL_Quit();
}

auto FSdlApp::RegisterEventHandler(SDL_EventType InEvt, THandlerType InHandler) -> bool
{
	if (EventHandlers.contains(InEvt))
	{
		LogWarnG("Cannot register another handler for event 0x{0:x}", InEvt);
		return false;
	}
	EventHandlers[InEvt] = std::move(InHandler);
	return true;
}

void FSdlApp::RegisterEventInterception(const THandlerType& InHandler)
{
	EventInterceptors.emplace_back(InHandler);
}

void FSdlApp::ProcessEvents()
{
	SDL_Event Event;
	while (static_cast<int>(SDL_PollEvent(&Event)) != 0)
	{
		for (auto& Intercept : EventInterceptors)
		{
			Intercept(Event);
		}
		auto Handler = EventHandlers.find(static_cast<SDL_EventType>(Event.type));
		if (Handler != EventHandlers.end())
		{
			Handler->second(Event);
		}
	}
}

void FSdlApp::Present()
{
	NotImplemented("Present");
}

#include <doctest/doctest.h>
#include <array>
#include <utility>

TEST_CASE("Create SDL window")
{
	FSdlApp App("Test app");
	CHECK(App.GetWindow() != nullptr);
}

TEST_CASE("Window has desired sie")
{
	FSdlApp App("Test app", 256, 128);
	App.Present();
	std::int32_t w = 0;
	std::int32_t h = 0;
	SDL_GetWindowSize(App.GetWindow(), &w, &h);
	CHECK(w == 256);
	CHECK(h == 128);
}

TEST_CASE("ProcessEvents triggers handler")
{
	FSdlApp App("Test app");
	bool Triggered = false;
	auto Handler = [&Triggered](SDL_Event& /*evt*/)
	{
		Triggered = true;
	};
	auto MyEvent = static_cast<SDL_EventType>(SDL_RegisterEvents(1));
	auto b = App.RegisterEventHandler(MyEvent, Handler);
	CHECK(b == true);
	SDL_Event Evt;
	Evt.type = MyEvent;
	SDL_PushEvent(&Evt);
	App.ProcessEvents();
	CHECK(Triggered == true);
}

TEST_CASE("Handler can be registered only once")
{
	FSdlApp App("Test app");
	bool Triggered = false;
	auto Handler = [&Triggered](SDL_Event& /*evt*/)
	{
		Triggered = true;
	};
	auto MyEvent = static_cast<SDL_EventType>(SDL_RegisterEvents(1));
	{
		auto b = App.RegisterEventHandler(MyEvent, Handler);
		CHECK(b == true);
	}
	{
		auto b = App.RegisterEventHandler(MyEvent, Handler);
		CHECK(b == false);
	}
}

TEST_CASE("Event interception is called for every event")
{
	FSdlApp App("Test app");
	bool Triggered = false;
	auto Handler = [&Triggered](SDL_Event& /*evt*/)
	{
		Triggered = true;
	};
	auto MyEvent = static_cast<SDL_EventType>(SDL_RegisterEvents(1));
	App.RegisterEventInterception(Handler);
	SDL_Event Evt;
	Evt.type = MyEvent;
	SDL_PushEvent(&Evt);
	App.ProcessEvents();
	CHECK(Triggered == true);
}
