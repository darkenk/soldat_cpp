#include "DebugWindow.hpp"
// clang-format off
#include <glad/glad.h>
// clang-format on
#include "client/SdlApp.hpp"

#include <backends/imgui_impl_sdlgpu3.h>
#include <backends/imgui_impl_sdl3.h>
#include <imgui.h>

DebugWindow::DebugWindow(SdlApp &app): mApp{app}
{
  ImGui::CreateContext();
  ImGui::StyleColorsLight();

  ImGui_ImplSDL3_InitForSDLGPU(app.GetWindow());
  ImGui_ImplSDLGPU3_InitInfo init_info = {};
  init_info.Device = app.GetDevice();
  init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(app.GetDevice(), app.GetWindow());
  init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
  ImGui_ImplSDLGPU3_Init(&init_info);
  app.RegisterEventInterception([](SDL_Event &evt) { ImGui_ImplSDL3_ProcessEvent(&evt); });
}

DebugWindow::~DebugWindow()
{
  SDL_WaitForGPUIdle(mApp.GetDevice());
  ImGui_ImplSDL3_Shutdown();
  ImGui_ImplSDLGPU3_Shutdown();
  ImGui::DestroyContext();
}

void DebugWindow::Draw(ImGuiDrawFunction func) { PendingDrawCalls.push_back(func); }

void DebugWindow::DrawEverything(SDL_GPUCommandBuffer* _command_buffer,  SDL_GPUTexture* _texture)
{
  ImGui_ImplSDLGPU3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();
  for (auto &drawImGui : PendingDrawCalls)
  {
    drawImGui();
  }
  ImGui::Render();
  ImDrawData* draw_data = ImGui::GetDrawData();
  // This is mandatory: call Imgui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
  Imgui_ImplSDLGPU3_PrepareDrawData(draw_data, _command_buffer);
  
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  // Setup and start a render pass
  SDL_GPUColorTargetInfo target_info = {};
  target_info.texture = _texture;
  target_info.clear_color = SDL_FColor { clear_color.x, clear_color.y, clear_color.z, clear_color.w };
  target_info.load_op = SDL_GPU_LOADOP_CLEAR;
  target_info.store_op = SDL_GPU_STOREOP_STORE;
  target_info.mip_level = 0;
  target_info.layer_or_depth_plane = 0;
  target_info.cycle = false;
  SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(_command_buffer, &target_info, 1, nullptr);

  // Render ImGui
  ImGui_ImplSDLGPU3_RenderDrawData(draw_data, _command_buffer, render_pass);

  SDL_EndGPURenderPass(render_pass);
  PendingDrawCalls.clear();
}


// tests
#include <doctest/doctest.h>

namespace
{

class DebugWindowFixture
{
public:
  DebugWindowFixture() = default;
  ~DebugWindowFixture() = default;
  DebugWindowFixture(const DebugWindowFixture &) = delete;
};

TEST_CASE_FIXTURE(DebugWindowFixture, "Check whether debug window is displayed")
{
  SdlApp app("Test Window");
  DebugWindow dw(app);
  auto i = 1;
  while ((i--) != 0)
  {
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(app.GetDevice()); // Acquire a GPU command buffer

    SDL_GPUTexture* swapchain_texture;
    SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, app.GetWindow(), &swapchain_texture, nullptr, nullptr); // Acquire a swapchain texture
    app.ProcessEvents();
    dw.Draw([]() {
      ImGui::Begin("Hello, world!");
      ImGui::Text("This is some useful text.");
      ImGui::End();
    });
    dw.DrawEverything(command_buffer, swapchain_texture);
    SDL_SubmitGPUCommandBuffer(command_buffer);
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
