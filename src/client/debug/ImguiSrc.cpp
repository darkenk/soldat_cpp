 #define IMGUI_IMPLEMENTATION
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#define IMGUI_IMPL_OPENGL_ES3
// clang-format off
#include <glad/glad.h>
#undef GL_POLYGON_MODE
#include "misc/single_file/imgui_single_file.h"
#include "backends/imgui_impl_opengl3.cpp"
#include "backends/imgui_impl_sdl.cpp"
// clang-format on
