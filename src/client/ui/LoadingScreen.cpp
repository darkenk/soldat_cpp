#include "Nuklear.hpp"



#pragma region tests
#include <doctest.h>
#include "../SdlApp.hpp"


extern unsigned char sDefaultFont[];
extern unsigned int sDefaultFont_len;

namespace
{

class LoadingScreenFixture
{
public:
  LoadingScreenFixture() = default;
  ~LoadingScreenFixture() = default;
  LoadingScreenFixture(const LoadingScreenFixture&) = delete;
protected:
};

TEST_SUITE("LoadingScreenSuite")
{

TEST_CASE_FIXTURE(LoadingScreenFixture, "Nuklear sdl3 gpu test")
{
  SdlApp app("Nuklear");
  app.RegisterEventInterception([](SDL_Event &evt) { nk_sdl_handle_event(&evt); });

  nk_context* ctx = nk_sdl_init(app.GetWindow(), app.GetDevice());
  struct nk_font_atlas *atlas;
  nk_sdl_font_stash_begin(&atlas);
  nk_sdl_font_stash_end();

  enum {EASY, HARD};
  int op = EASY;
  float value = 0.6f;

  auto i = 1;
  while (i--)
  {
    nk_input_begin(ctx);
    app.ProcessEvents();
    nk_input_end(ctx);
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(app.GetDevice()); // Acquire a GPU command buffer

    SDL_GPUTexture* swapchain_texture;
    Uint32 swapchain_texture_width;
    Uint32 swapchain_texture_height;
    SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, app.GetWindow(), &swapchain_texture, &swapchain_texture_width, &swapchain_texture_height); // Acquire a swapchain texture

    
    if (nk_begin(ctx, "Show", nk_rect(50, 50, 220, 220),
      NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE)) {
      /* fixed widget pixel width */
      nk_layout_row_static(ctx, 30, 80, 1);
      if (nk_button_label(ctx, "button")) {
          /* event handling */
      }

      /* fixed widget window ratio width */
      nk_layout_row_dynamic(ctx, 30, 2);
      if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
      if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

      /* custom widget pixel width */
      nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
      {
          nk_layout_row_push(ctx, 50);
          nk_label(ctx, "Volume:", NK_TEXT_LEFT);
          nk_layout_row_push(ctx, 110);
          nk_slider_float(ctx, 0, &value, 1.0f, 0.1f);
      }
      nk_layout_row_end(ctx);
    }
    nk_end(ctx);
    
    nk_sdl_prepare_render_data(NK_ANTI_ALIASING_ON, command_buffer, swapchain_texture_width, swapchain_texture_height);

    // Setup and start a render pass
    SDL_GPUColorTargetInfo target_info = {};
    target_info.texture = swapchain_texture;
    target_info.clear_color = SDL_FColor { 0.10f, 0.18f, 0.24f, 1.00f };
    target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    target_info.store_op = SDL_GPU_STOREOP_STORE;
    target_info.mip_level = 0;
    target_info.layer_or_depth_plane = 0;
    target_info.cycle = false;
    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);

    nk_sdl_render(NK_ANTI_ALIASING_ON, command_buffer, render_pass);

    SDL_EndGPURenderPass(render_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
  }
  nk_sdl_shutdown();
}

TEST_CASE_FIXTURE(LoadingScreenFixture, "Loading screen")
{
  SdlApp app("Nuklear");
  app.RegisterEventInterception([](SDL_Event &evt) { nk_sdl_handle_event(&evt); });

  nk_context* ctx = nk_sdl_init(app.GetWindow(), app.GetDevice());
  struct nk_font_atlas *atlas;
  nk_sdl_font_stash_begin(&atlas);
  nk_font* default_font = nk_font_atlas_add_from_memory(atlas, sDefaultFont, sDefaultFont_len, 28, 0);
  nk_sdl_font_stash_end();
  nk_style_set_font(ctx, &default_font->handle);

  auto i = 1;
  while (i--)
  {
    nk_input_begin(ctx);
    app.ProcessEvents();
    nk_input_end(ctx);
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(app.GetDevice()); // Acquire a GPU command buffer

    SDL_GPUTexture* swapchain_texture;
    Uint32 swapchain_texture_width;
    Uint32 swapchain_texture_height;
    SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, app.GetWindow(), &swapchain_texture, &swapchain_texture_width, &swapchain_texture_height); // Acquire a swapchain texture

    if (nk_begin(ctx, "Loading screen", nk_rect(0, 0, swapchain_texture_width, swapchain_texture_height), NK_WINDOW_NO_INPUT | NK_WINDOW_NO_SCROLLBAR))
    {
      nk_layout_row_dynamic(ctx, swapchain_texture_height/2, 1);
      nk_label(ctx, "Loading...", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_BOTTOM);
      nk_layout_row_dynamic(ctx, swapchain_texture_height/2, 1);
      nk_label(ctx, "Please wait", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_TOP);     
    }
    nk_end(ctx);
    
    nk_sdl_prepare_render_data(NK_ANTI_ALIASING_ON, command_buffer, swapchain_texture_width, swapchain_texture_height);

    // Setup and start a render pass
    SDL_GPUColorTargetInfo target_info = {};
    target_info.texture = swapchain_texture;
    target_info.clear_color = SDL_FColor { 0.10f, 0.18f, 0.24f, 1.00f };
    target_info.load_op = SDL_GPU_LOADOP_CLEAR;
    target_info.store_op = SDL_GPU_STOREOP_STORE;
    target_info.mip_level = 0;
    target_info.layer_or_depth_plane = 0;
    target_info.cycle = false;
    SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);

    nk_sdl_render(NK_ANTI_ALIASING_ON, command_buffer, render_pass);

    SDL_EndGPURenderPass(render_pass);

    SDL_SubmitGPUCommandBuffer(command_buffer);
  }
  nk_sdl_shutdown();
}

} // end of LoadingScreenSuite
} // end of unnamed namespace
#pragma endregion tests