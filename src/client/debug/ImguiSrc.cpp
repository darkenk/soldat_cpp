 #define DK_ES2 0

#define IMGUI_IMPLEMENTATION
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#if __EMSCRIPTEN__ && DK_ES2
#define IMGUI_IMPL_OPENGL_ES2
#else
#define IMGUI_IMPL_OPENGL_ES3
#endif
// clang-format off
#include <glad/glad.h>
#if __EMSCRIPTEN__ && DK_ES2
#else
#undef GL_POLYGON_MODE
#endif
#include "misc/single_file/imgui_single_file.h"
#if __EMSCRIPTEN__ && DK_ES2
#include "backends/imgui_impl_opengl2.cpp"
#else
#include "backends/imgui_impl_opengl3.cpp"
#endif
#include "backends/imgui_impl_sdl.cpp"
// clang-format on
