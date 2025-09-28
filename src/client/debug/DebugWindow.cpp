#include "DebugWindow.hpp"

#include <imgui_impl_sdlgpu3.h>
#include <imgui_impl_sdl3.h>
#include <imgui.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_pixels.h>

// clang-format on
#include "client/SdlApp.hpp"

FDebugWindow::FDebugWindow(FSdlApp& InApp) : App{ InApp }
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplSDL3_InitForSDLGPU(InApp.GetWindow());
	ImGui_ImplSDLGPU3_InitInfo InitInfo = {};
	InitInfo.Device = InApp.GetDevice();
	InitInfo.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(InApp.GetDevice(), InApp.GetWindow());
	InitInfo.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
	ImGui_ImplSDLGPU3_Init(&InitInfo);
	InApp.RegisterEventInterception(
		[](SDL_Event& InEvt)
		{
			if (!ImGui_ImplSDL3_ProcessEvent(&InEvt))
			{
				return false;
			}
			ImGuiIO& io = ImGui::GetIO();
			return io.WantCaptureMouse || io.WantCaptureKeyboard;
		});
}

FDebugWindow::~FDebugWindow()
{
	SDL_WaitForGPUIdle(App.GetDevice());
	ImGui_ImplSDL3_Shutdown();
	ImGui_ImplSDLGPU3_Shutdown();
	ImGui::DestroyContext();
}

void FDebugWindow::Draw(const TImGuiDrawFunction& InFunc)
{
	PendingDrawCalls.push_back(InFunc);
}

void FDebugWindow::DrawEverything(SDL_GPUCommandBuffer* InCommandBuffer, SDL_GPUTexture* InTexture)
{
	ImGui_ImplSDLGPU3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	for (auto& DrawImGui : PendingDrawCalls)
	{
		DrawImGui();
	}
	ImGui::Render();
	ImDrawData* DrawData = ImGui::GetDrawData();
	// This is mandatory: call Imgui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
	ImGui_ImplSDLGPU3_PrepareDrawData(DrawData, InCommandBuffer);

	ImVec4 const ClearColor = ImVec4(0.45F, 0.55F, 0.60F, 1.00F);
	// Setup and start a render pass
	SDL_GPUColorTargetInfo TargetInfo = {};
	TargetInfo.texture = InTexture;
	TargetInfo.clear_color = SDL_FColor{ ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w };
	TargetInfo.load_op = SDL_GPU_LOADOP_LOAD;
	TargetInfo.store_op = SDL_GPU_STOREOP_STORE;
	TargetInfo.mip_level = 0;
	TargetInfo.layer_or_depth_plane = 0;
	TargetInfo.cycle = false;
	SDL_GPURenderPass* RenderPass = SDL_BeginGPURenderPass(InCommandBuffer, &TargetInfo, 1, nullptr);

	// Render ImGui
	ImGui_ImplSDLGPU3_RenderDrawData(DrawData, InCommandBuffer, RenderPass);

	SDL_EndGPURenderPass(RenderPass);
	PendingDrawCalls.clear();
}

// tests
#include <doctest/doctest.h>
#include <array>
#include <cstddef>
#include <new>

namespace
{

	class FDebugWindowFixture
	{
	public:
		FDebugWindowFixture() = default;
		FDebugWindowFixture(FDebugWindowFixture&&) = delete;
		FDebugWindowFixture(const FDebugWindowFixture&) = delete;
		FDebugWindowFixture& operator=(const FDebugWindowFixture&) = delete;
		FDebugWindowFixture& operator=(FDebugWindowFixture&&) = delete;
		~FDebugWindowFixture() = default;
	};

	TEST_CASE_FIXTURE(FDebugWindowFixture, "Check whether debug window is displayed")
	{
		FSdlApp App("Test Window");
		FDebugWindow dw(App);
		auto i = 1;
		while ((i--) != 0)
		{
			SDL_GPUCommandBuffer* CommandBuffer =
				SDL_AcquireGPUCommandBuffer(App.GetDevice()); // Acquire a GPU command buffer

			SDL_GPUTexture* SwapchainTexture = nullptr;
			SDL_WaitAndAcquireGPUSwapchainTexture(
				CommandBuffer, App.GetWindow(), &SwapchainTexture, nullptr, nullptr); // Acquire a swapchain texture
			App.ProcessEvents();
			dw.Draw(
				[]()
				{
					ImGui::Begin("Hello, world!");
					ImGui::Text("This is some useful text.");
					ImGui::End();
				});
			dw.DrawEverything(CommandBuffer, SwapchainTexture);
			SDL_SubmitGPUCommandBuffer(CommandBuffer);
		}
	}

	struct FSampleServiceLocator
	{
		static auto Get() -> FSampleServiceLocator& { return *gSSampleServiceLocator; }
		auto DebugWindow() -> class FDebugWindow& { return *reinterpret_cast<class FDebugWindow*>(Window.data()); }
		::std::array<::std::byte, sizeof(class FDebugWindow)> Window;
		static FSampleServiceLocator* gSSampleServiceLocator;
	};

	FSampleServiceLocator* FSampleServiceLocator::gSSampleServiceLocator = nullptr;

	TEST_CASE_FIXTURE(FDebugWindowFixture, "Draw without passing DebugWindow to function" * doctest::skip(true))
	{
		FSdlApp App("t1");
		FSampleServiceLocator::gSSampleServiceLocator = new FSampleServiceLocator;
		new (FSampleServiceLocator::Get().Window.data()) FDebugWindow(App);
		FDebugWindow::DrawStatic<FSampleServiceLocator>(
			[]()
			{
			});
		delete FSampleServiceLocator::gSSampleServiceLocator;
	}

} // namespace
