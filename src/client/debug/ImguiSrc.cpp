 #define IMGUI_IMPLEMENTATION
#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#define IMGUI_IMPL_OPENGL_ES3
// clang-format off
#include <glad/glad.h>
#undef GL_POLYGON_MODE
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#endif // __GNUC__
#include "misc/single_file/imgui_single_file.h"
#include "backends/imgui_impl_opengl3.cpp" // NOLINT
//#include "backends/imgui_impl_sdl3.cpp" // NOLINT
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__
// clang-format on
