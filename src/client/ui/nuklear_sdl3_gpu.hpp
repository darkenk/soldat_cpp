/*
 * Nuklear - 4.9.4 - public domain
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
//#include "Nuklear.hpp"
#include <SDL3/SDL.h>
//#define NK_SDL_GPU_IMPLEMENTATION

#ifndef NK_SDL_GPU_H_
#define NK_SDL_GPU_H_

NK_API struct nk_context*   nk_sdl_init(SDL_Window *win, SDL_GPUDevice *device);
NK_API void                 nk_sdl_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_sdl_font_stash_end(void);
NK_API int                  nk_sdl_handle_event(SDL_Event *evt);
NK_API void                 nk_sdl_prepare_render_data(enum nk_anti_aliasing AA, SDL_GPUCommandBuffer* command_buffer, unsigned int fb_width, unsigned int fb_height);
NK_API void                 nk_sdl_render(enum nk_anti_aliasing, SDL_GPUCommandBuffer* command_buffer, SDL_GPURenderPass* render_pass);
NK_API void                 nk_sdl_shutdown(void);

#if SDL_MAJOR_VERSION < 3
#error "nuklear_sdl_gpu requires at least SDL 3.0.0"
#endif

#endif /* NK_SDL_GPU_H_ */