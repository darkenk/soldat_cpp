// automatically converted

#include "Gfx.hpp"

#include "BinPack.hpp"
#include "Input.hpp"
#include "common/Logging.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Cvar.hpp"
#include "shared/misc/FontAtlas.hpp"
#include "shared/misc/SignalUtils.hpp"
#include <SDL3/SDL.h>
#include <cmath>
// clang-format off
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#elif defined(__clang__) // __GNUC__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif // __clang__
#include <freetype/freetype.h>
#include <ft2build.h>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__) // __clang__
#pragma GCC diagnostic pop
#endif // __GNUC__
// clang-format on
#include "shared/misc/GlobalSystems.hpp"
#include "shared/misc/MemoryUtils.hpp"
#include <Tracy.hpp>
#include <algorithm>
#include <cstddef>
#include <glad/glad.h>
#include <iostream>
#include <iterator>
#include <map>
#include <memory_resource>
#include <sstream>
#include <vector>

using string = std::string;

GLint OPENGL_MAX_LABEL_LENGTH;
constexpr std::array<std::int32_t, 4> OPENGL_TEXTURE_FORMAT = {{GL_ALPHA, GL_RG, GL_RGB, GL_RGBA}};

static bool gOpenGLES = false;
static auto IsOpenGLES() -> bool { return gOpenGLES; }


/******************************************************************************/
/*                              Helper functions                              */
/******************************************************************************/
template <std::size_t N>
auto join1(const std::array<std::string, N> &list, const std::string &separator) -> string
{
  std::ostringstream os;
  std::copy(list.begin(), list.end(),
            std::ostream_iterator<const std::string &>(os, separator.c_str()));
  return os.str();
}

auto npot(std::uint32_t x) -> std::int32_t
{
  x -= 1;
  x = x | ((unsigned long)x >> 1);
  x = x | ((unsigned long)x >> 2);
  x = x | ((unsigned long)x >> 4);
  x = x | ((unsigned long)x >> 8);
  x = x | ((unsigned long)x >> 16);
  return std::max(2u, x + 1);
}

auto rectwidth(const tgfxrect &rect) -> float { return std::abs(rect.right - rect.left); }

auto rectheight(const tgfxrect &rect) -> float { return std::abs(rect.bottom - rect.top); }

/******************************************************************************/
/*                                    Gfx                                     */
/******************************************************************************/

const std::int32_t batch_min = 2048;
const std::int32_t glyph_pool_size = 64;

struct tbatchbuffer
{
  pgfxvertex data;
  std::int32_t size;
  std::int32_t capacity;
};

struct tbatch
{
  tgfxvertexbuffer *vertexbuffer;
  std::vector<tbatchbuffer> buffers;
  std::vector<tgfxdrawcommand> commands;
  std::int32_t commandssize;
};

using pglyph = struct tglyph *;
using ppglyph = pglyph *;
struct tglyph
{
  std::int32_t glyphindex;
  std::int32_t page;
  float advance;
  tgfxrect bounds;
  tgfxrect texcoords;
};

using pglyphtable = struct tglyphtable *;
struct tglyphtable
{
  std::int32_t fontsize;
  std::int32_t stretch;
  std::uint32_t flags;
  float vspace;
  float ascent;
  float descent;
  std::int32_t size;
  std::int32_t capacity;
  ppglyph glyphs;
};

using pfontnode = struct tfontnode *;
struct tfontnode
{
  std::int32_t x, y, w;
};

using pcomputedglyph = struct tcomputedglyph *;
struct tcomputedglyph
{
  MyFloat x, y;
  pglyph glyph;
};

using pfont = struct tfont *;
struct tfont
{
  FT_Face handle;
  std::int32_t width;
  std::int32_t height;
  std::uint8_t *buffer;
  std::int32_t buffersize;
  std::vector<tgfxtexture *> pages;
  std::vector<tglyphtable> tables;
  std::vector<pglyph> pool;
  std::int32_t poolindex;
  std::vector<RectangleArea> pageDesc;
};

class tscreenshotthread // : public tthread
{
private:
  std::string fname;
  std::int32_t fwidth;
  std::int32_t fheight;
  std::uint8_t *fdata;

public:
  tscreenshotthread(string filename, std::int32_t w, std::int32_t h, std::uint8_t *data);
  void execute();

  bool freeonterminate = false;
  void start();
  void waitfor();
};

static SDL_GLContext gameglcontext;

struct
{
  std::int32_t majorversion;
  GLuint shaderprogram;
  GLint matrixloc;
  tbatch batch;
  tgfxtexture *rendertarget;
  tgfxtexture *whitetexture;
  tgfxvertexbuffer *boundbuffer = nullptr;
  GLuint ditheringtexture;
  GLint maxtexturesize;
  GLint msaasamples;
  FT_Library ftlibrary;
  pfont font;
  pglyphtable glyphtable;
  tvector2 textpixelratio;
  float textscale;
  tgfxcolor textcolor;
  tvector2 textshadowoffset;
  tgfxcolor textshadowcolor;
  tgfxverticalalign textverticalalign;
  std::int32_t *textindexstr;
  ppglyph textglyphstr;
  pcomputedglyph textcomputedstr;
  std::int32_t textstrsize;
  std::int32_t textcomputedcount;
  GLuint testVao = 0;
} gfxcontext;

static auto createshader(GLenum shadertype, const std::string shadersource) -> GLuint
{
  std::string source;
  const GLchar *sourceptr;
  GLchar *info;
  GLint status;
  GLsizei len;
  GLsizei dummy;

  source = std::string(shadersource);
  sourceptr = source.c_str();

  auto result = glCreateShader(shadertype);
  glShaderSource(result, 1, &sourceptr, nullptr);
  glCompileShader(result);
  glGetShaderiv(result, GL_COMPILE_STATUS, &status);

  if (status == GLint(GL_FALSE))
  {
    dummy = 0;
    glGetShaderiv(result, GL_INFO_LOG_LENGTH, &len);
    getmem(info, len + 1);
    glGetShaderInfoLog(result, len, &dummy, info);
    gfxlog(string("-- Shader compilation failure --") + '\12' + string(info) + '\12');
    freemem(info);

    glDeleteShader(result);
    result = 0;
  }
  return result;
}

static void setupvertexattributes(tgfxvertexbuffer *buffer)
{
  if (gfxcontext.boundbuffer != buffer)
  {
    glBindBuffer(GL_ARRAY_BUFFER, buffer->handle());

    glBindVertexArray(gfxcontext.testVao);

    static_assert(sizeof(tgfxvertex) == 20);
    if (gfxcontext.shaderprogram != 0)
    {
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(tgfxvertex), (void *)nullptr);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(tgfxvertex), (void *)(8));
      // Workaround for MESA 20.1+ breakage: Pass ByteBool(1) instead of True
      glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(tgfxvertex), (void *)(16));
    }
    else
    {
      glVertexPointer(2, GL_FLOAT, sizeof(tgfxvertex), (void *)nullptr);
      glTexCoordPointer(2, GL_FLOAT, sizeof(tgfxvertex), (void *)(8));
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(tgfxvertex), (void *)(16));
    }

    gfxcontext.boundbuffer = buffer;
  }
}

auto gfxframebuffersupported() -> bool
{
  return glGenFramebuffers != nullptr && glBlitFramebuffer != nullptr;
}

static auto GetVertexShaderSource() -> std::string_view
{
  if (IsOpenGLES())
  {
    return R"(
#version 100
uniform mat3 mvp;
attribute lowp vec4 in_position;
attribute lowp vec2 in_texcoords;
attribute lowp vec4 in_color;
varying lowp vec2 texcoords;
varying lowp vec4 color;
void main()
{
    color = vec4(in_color.rgb * in_color.a, in_color.a);
    texcoords = in_texcoords;
    gl_Position.xyw = mvp * in_position.xyw;
    gl_Position.z = 0.00;
}
        )";
  }

  return R"(
#version 120
uniform mat3 mvp;
attribute vec4 in_position;
attribute vec2 in_texcoords;
attribute vec4 in_color;
varying vec2 texcoords;
varying vec4 color;
void main()
{
    color = vec4(in_color.rgb * in_color.a, in_color.a);
    texcoords = in_texcoords;
    gl_Position.xyw = mvp * in_position.xyw;
    gl_Position.z = 0.00;
}
    )";
}

static auto GetFragmentShaderSource() -> std::string_view
{
  if (IsOpenGLES())
  {
    return R"(
#version 100
#define DITHERING 0
varying lowp vec2 texcoords;
varying lowp vec4 color;
uniform sampler2D sampler;
uniform sampler2D dither;

void main()
{
    gl_FragColor = texture2D(sampler, texcoords) * color;
    #if DITHERING
    gl_FragColor.rgb += vec3(
        texture2D(dither, gl_FragCoord.xy / 8.0).a / 32.0 - 1.0/128.0);
    #endif
}
        )";
  }

  return R"(
#version 120
#define DITHERING 0
varying vec2 texcoords;
varying vec4 color;
uniform sampler2D sampler;
uniform sampler2D dither;

void main()
{
    gl_FragColor = texture2D(sampler, texcoords) * color;
    #if DITHERING
    gl_FragColor.rgb += vec3(
        texture2D(dither, gl_FragCoord.xy / 8.0).a / 32.0 - 1.0/128.0);
    #endif
}
        )";
}

static auto initshaderprogram(bool dithering) -> bool
{
  const std::string_view vert_source = GetVertexShaderSource();
  const std::string_view frag_source = GetFragmentShaderSource();

  const std::array<std::uint8_t, static_cast<std::size_t>(8 * 8)> dither = {
    {0,  32, 8,  40, 2,  34, 10, 42, 48, 16, 56, 24, 50, 18, 58, 26, 12, 44, 4,  36, 14, 46,
     6,  38, 60, 28, 52, 20, 62, 30, 54, 22, 3,  35, 11, 43, 1,  33, 9,  41, 51, 19, 59, 27,
     49, 17, 57, 25, 15, 47, 7,  39, 13, 45, 5,  37, 63, 31, 55, 23, 61, 29, 53, 21}};
  GLuint fragshader;
  GLuint vertshader;
  std::string fragsrc;
  std::string vertsrc;
  GLint status;
  GLchar *info;
  GLsizei len;
  GLsizei *dummy;
  bool requiredfunctions;

  bool initshaderprogram_result = false;

  requiredfunctions =
    assigned(glCreateShader) && assigned(glShaderSource) && assigned(glCompileShader) &&
    assigned(glGetShaderiv) && assigned(glGetShaderInfoLog) && assigned(glDeleteShader) &&
    assigned(glAttachShader) && assigned(glDetachShader) && assigned(glLinkProgram) &&
    assigned(glGetProgramiv) && assigned(glGetProgramInfoLog) && assigned(glUseProgram) &&
    assigned(glGetUniformLocation) && assigned(glUniform1i) && assigned(glUniformMatrix3fv) &&
    assigned(glEnableVertexAttribArray) && assigned(glVertexAttribPointer) &&
    assigned(glBindAttribLocation) && assigned(glActiveTexture);

  if (!requiredfunctions)
  {
    return initshaderprogram_result;
  }

  NotImplemented("rendering", "Missing stringreplace");
#if 0
    if (!dithering)
        fragsrc = stringreplace(fragsrc, "#define DITHERING 1", "#define DITHERING 0");
#endif

  vertshader = createshader(GL_VERTEX_SHADER, vert_source.data());
  if (vertshader == 0u)
  {
    LogWarnG("Vertex shader compilation failed");
    Abort();
    return false;
  }
  fragshader = createshader(GL_FRAGMENT_SHADER, frag_source.data());
  if (fragshader == 0u)
  {
    LogWarnG("Fragment shader compilation failed");
    Abort();
    return false;
  }

  gfxcontext.shaderprogram = glCreateProgram();

  glBindAttribLocation(gfxcontext.shaderprogram, 0, "in_position");
  glBindAttribLocation(gfxcontext.shaderprogram, 1, "in_texcoords");
  glBindAttribLocation(gfxcontext.shaderprogram, 2, "in_color");

  glAttachShader(gfxcontext.shaderprogram, vertshader);
  glAttachShader(gfxcontext.shaderprogram, fragshader);

  glLinkProgram(gfxcontext.shaderprogram);
  glGetProgramiv(gfxcontext.shaderprogram, GL_LINK_STATUS, &status);

  if (status == GLint(GL_FALSE))
  {
    dummy = nullptr;
    glGetProgramiv(gfxcontext.shaderprogram, GL_INFO_LOG_LENGTH, &len);
    getmem(info, len + 1);
    glGetProgramInfoLog(gfxcontext.shaderprogram, len, dummy, info);
#ifdef DEVELOPMENT
    output << string("-- Program linking failure --") + '\12' + string(info) + '\12' << NL;
#endif
    freemem(info);
  }

  glDetachShader(gfxcontext.shaderprogram, vertshader);
  glDetachShader(gfxcontext.shaderprogram, fragshader);
  glDeleteShader(vertshader);
  glDeleteShader(fragshader);

  if (status == GLint(GL_FALSE))
  {
    glDeleteProgram(gfxcontext.shaderprogram);
    gfxcontext.shaderprogram = 0;
    return initshaderprogram_result;
  }

  initshaderprogram_result = true;

  glUseProgram(gfxcontext.shaderprogram);
  glGenVertexArrays(1, &gfxcontext.testVao);
  glBindVertexArray(gfxcontext.testVao);
  glUniform1i(glGetUniformLocation(gfxcontext.shaderprogram, ("dither")), 1);
  gfxcontext.matrixloc = glGetUniformLocation(gfxcontext.shaderprogram, ("mvp"));

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glDisable(GL_DEPTH_TEST);

  // dithering texture

  if (dithering)
  {
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &gfxcontext.ditheringtexture);
    glBindTexture(GL_TEXTURE_2D, gfxcontext.ditheringtexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 8, 8, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &dither[1]);
    glActiveTexture(GL_TEXTURE0);
  }
  return initshaderprogram_result;
}

static void OpenGLDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                 const GLchar *message, const void *userParam)
{
  LogDebugG("[GL] {}", message);
}

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

auto gfxinitcontext(SDL_Window *wnd, bool dithering, bool fixedpipeline) -> bool
{
  tgfxcolor color;
  std::string version;
  bool requiredfunctions;

  bool gfxinitcontext_result = true;

  struct OpenGLVersion
  {
    SDL_GLProfile profile;
    std::uint32_t major;
    std::uint32_t minor;
  };
  constexpr std::array versions{OpenGLVersion{SDL_GL_CONTEXT_PROFILE_CORE, 4, 3},
                                OpenGLVersion{SDL_GL_CONTEXT_PROFILE_ES, 3, 0}};

  for (const auto &v : versions)
  {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, v.profile);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, v.major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, v.minor);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);

    if (CVar::r_msaa > 0)
    {
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
      SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, CVar::r_msaa);
    }

    gameglcontext = SDL_GL_CreateContext(wnd);
    if (gameglcontext == nullptr)
    {
      LogInfoG("Cannot create opengl context: {}", SDL_GetError());
      continue;
    }
    gOpenGLES = v.profile == SDL_GL_CONTEXT_PROFILE_ES;
    break;
  }
  if (gameglcontext == nullptr)
  {
    LogCriticalG("Failed to create gl context");
    Abort();
    return false;
  }
  glad_set_post_callback(OpenGLGladDebug);
  if (gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) == 0)
  {
    LogCriticalG("Failed to initialize GLAD");
    Abort();
    return false;
  }
#if 0
    initopengl();
    readimplementationproperties();
    readextensions();
#endif
  SDL_GL_MakeCurrent(gamewindow, gameglcontext);

  if (not IsOpenGLES())
  {
    glDebugMessageCallback(OpenGLDebug, nullptr);
  }
  version = std::string(reinterpret_cast<const char *>(glGetString(GL_VERSION)));
  glGetIntegerv(GL_MAX_LABEL_LENGTH, &OPENGL_MAX_LABEL_LENGTH);
  gfxlog(string("OpenGL version: ") + version);

  NotImplemented("rendering", "EXTs missing in glad");
#if 0
    if (!assigned(glGenFramebuffers))
    {
        if (assigned(glGenFramebuffersExt))
        {
            &glGenFramebuffers = &glGenFramebuffersExt;
            &glDeleteFramebuffers = &glDeleteFramebuffersExt;
            &glBindFramebuffer = &glBindFramebufferxt;
            &glFramebufferTexture2D = &glFramebufferTexture2Dext;
            &glgeneratemipmap = &glgeneratemipmapext;
        }
    }
#endif

  requiredfunctions =
    assigned(glBindBuffer) && // vertex/index buffers
    assigned(glGenBuffers) && assigned(glBufferData) && assigned(glDeleteBuffers) &&
    assigned(glBufferSubData) && assigned(glGenTextures) && // textures
    assigned(glBindTexture) && assigned(glDeleteTextures) && assigned(glTexParameteri) &&
    assigned(glTexImage2D) && assigned(glTexSubImage2D) && assigned(glEnable) && // general
    assigned(glDisable) && assigned(glBlendFunc) && assigned(glPixelStorei) &&
    assigned(glViewport) && assigned(glClearColor) && assigned(glClear) && assigned(glDrawArrays) &&
    assigned(glDrawElements) && assigned(glReadPixels) && assigned(glTexEnvf) &&
    assigned(glGetIntegerv);

  if (!requiredfunctions)
  {
    gfxdestroycontext();
    return false;
  }

  if (!fixedpipeline)
  {
    if (!initshaderprogram(dithering))
    {
      gfxlog("Falling back to OpenGL fixed pipeline.");
      fixedpipeline = true;
    }
  }

  if (fixedpipeline)
  {
    requiredfunctions = assigned(glEnableClientState) && assigned(glVertexPointer) &&
                        assigned(glTexCoordPointer) && assigned(glColorPointer) &&
                        assigned(glLoadMatrixf);

    if (!requiredfunctions)
    {
      gfxdestroycontext();
      return false;
    }

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    if (dithering)
    {
      glEnable(GL_DITHER);
    }
    else
    {
      glDisable(GL_DITHER);
    }
  }

  // create a default white texture

  color.rgba = 0xffffffff;
  gfxcontext.whitetexture = gfxcreatetexture(1, 1, 4, (std::uint8_t *)&color, "white texture");

  // setup some state

  glEnable(GL_BLEND);
  if (not IsOpenGLES())
  {
    glEnable(GL_MULTISAMPLE);
  }
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gfxcontext.maxtexturesize);
  glGetIntegerv(GL_SAMPLES, &gfxcontext.msaasamples);

  // text rendering

  FT_Init_FreeType(&gfxcontext.ftlibrary);

  gfxcontext.textpixelratio = vector2(1, 1);
  gfxcontext.textscale = 1;
  gfxcontext.textcolor = rgba(0);
  gfxcontext.textshadowcolor = rgba(0);
  gfxcontext.textverticalalign = gfx_top;
  return gfxinitcontext_result;
}

void gfxdestroycontext()
{
  std::int32_t i;

  tbatch &batch = gfxcontext.batch;

  if (batch.vertexbuffer != nullptr)
  {
    gfxdeletebuffer(batch.vertexbuffer);
  }

  for (i = 0; i < batch.buffers.size(); i++)
  {
    freemem(batch.buffers[i].data);
  }

  batch.buffers.clear();
  batch.buffers.shrink_to_fit();
  batch.commands.clear();
  batch.commands.shrink_to_fit();

  if (gfxcontext.whitetexture != nullptr)
  {
    gfxdeletetexture(gfxcontext.whitetexture);
  }

  if (gfxcontext.ditheringtexture != 0)
  {
    glDeleteTextures(1, &gfxcontext.ditheringtexture);
  }

  if (gfxcontext.textstrsize > 0)
  {
    freemem(gfxcontext.textindexstr);
    freemem(gfxcontext.textglyphstr);
    freemem(gfxcontext.textcomputedstr);
  }

  if (gfxcontext.ftlibrary != nullptr)
  {
    FT_Done_FreeType(gfxcontext.ftlibrary);
  }

  fillchar(&gfxcontext, sizeof(gfxcontext), 0);
}

void gfxtarget(tgfxtexture *rendertarget)
{
  if (assigned(glBindFramebuffer))
  {
    if ((rendertarget != nullptr) && (rendertarget->ffbohandle != 0))
    {
      glBindFramebuffer(GL_FRAMEBUFFER, rendertarget->ffbohandle);
    }
    else
    {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
  }
}

void gfxblit(tgfxtexture *src, tgfxtexture *dst, trect srcrect, trect dstrect,
             tgfxtexturefilter filter)
{
  GLenum filtergl;

  filtergl = GL_NEAREST;

  if (filter == gfx_linear)
  {
    filtergl = GL_LINEAR;
  }

  glBindFramebuffer(GL_READ_FRAMEBUFFER, src->ffbohandle);

  if (dst != nullptr)
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->ffbohandle);
  }
  else
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }

  glBlitFramebuffer(srcrect.left, srcrect.bottom, srcrect.right, srcrect.top, dstrect.left,
                    dstrect.bottom, dstrect.right, dstrect.top, GL_COLOR_BUFFER_BIT, filtergl);
}

void gfxviewport(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h)
{
  glViewport(x, y, w, h);
}

void gfxtransform(const tgfxmat3 t)
// m: array[0..15] of Single = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
{
  std::array<float, 16> m;
  if (gfxcontext.shaderprogram != 0)
  {
    glUniformMatrix3fv(gfxcontext.matrixloc, 1, 0u, t.data());
  }
  else
  {
    std::memset(m.data(), 0, sizeof(m));

    m[0] = t[0];
    m[4] = t[3];
    m[8] = 0;
    m[12] = t[6];
    m[1] = t[1];
    m[5] = t[4];
    m[9] = 0;
    m[13] = t[7];
    m[2] = t[2];
    m[6] = t[5];
    m[10] = 1;
    m[14] = 0;
    m[3] = 0;
    m[7] = 0;
    m[11] = 0;
    m[15] = 1;

    glLoadMatrixf(m.data());
  }
}

void gfxclear(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
{
  glClearColor((float)(r) / 255, (float)(g) / 255, (float)(b) / 255, (float)(a) / 255);
  glClear(GL_COLOR_BUFFER_BIT);
}

void gfxclear(tgfxcolor c)
{
  gfxclear(c.color.r, c.color.g, c.color.b, c.color.a);
}

void gfxdraw(tgfxvertexbuffer *buffer, std::int32_t offset, std::int32_t count)
{
  setupvertexattributes(buffer);
  glDrawArrays(GL_TRIANGLES, offset, count);
}
/*$push*/
/*$warn 4055 off*/

void gfxdraw(tgfxvertexbuffer *buffer, tgfxindexbuffer *indexbuffer, std::int32_t offset,
             std::int32_t count)
{
  setupvertexattributes(buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer->handle());
  glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (void *)(sizeof(std::uint16_t) * offset));
}

void gfxdraw(tgfxvertexbuffer *buffer, pgfxdrawcommand cmds, std::int32_t cmdcount)
{
  std::int32_t i;

  setupvertexattributes(buffer);

  for (i = 1; i <= cmdcount; i++)
  {
    gfxbindtexture(cmds->texture);
    glDrawArrays(GL_TRIANGLES, cmds->offset, cmds->count);
    cmds += 1;
  }
}

void gfxdraw(tgfxvertexbuffer *buffer, tgfxindexbuffer *indexbuffer, pgfxdrawcommand cmds,
             std::int32_t cmdcount)
{
  std::int32_t i;

  setupvertexattributes(buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer->handle());

  for (i = 1; i <= cmdcount; i++)
  {
    gfxbindtexture(cmds->texture);
    glDrawElements(GL_TRIANGLES, cmds->count, GL_UNSIGNED_SHORT,
                   (void *)(sizeof(std::uint16_t) * cmds->offset));
    cmds += 1;
  }
}

/*$pop*/

void gfxpresent(bool finish)
{
  ZoneScopedN("GfxPresent");
  if (finish)
  {
    glFinish();
  }

  SDL_GL_SwapWindow(gamewindow);
}
void gfxsetmipmapbias(float bias)
{
  NotImplemented("rendering", "This is probably some garbage after fixed pipeline");
#if 0
     glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, bias);
#endif
}

tscreenshotthread::tscreenshotthread(string filename, std::int32_t w, std::int32_t h,
                                     std::uint8_t *data)
{
  fname = filename;
  fwidth = w;
  fheight = h;
  fdata = data;
}

void tscreenshotthread::execute()
{
  std::uint8_t *src;
  std::uint8_t *dst;
  std::int32_t stride;
  std::int32_t y;

  stride = 4 * fwidth;

  src = fdata;
  dst = fdata;

  src += static_cast<ptrdiff_t>(stride * fheight);
  dst += static_cast<ptrdiff_t>(stride * (fheight - 1));

  for (y = 1; y <= fheight; y++)
  {
    std::memcpy(dst, src, stride);
    src += stride;
    dst -= stride;
  }

  stbi_write_png(fname.c_str(), fwidth, fheight, 4, fdata, stride);
  freemem(fdata);
}

void gfxsavescreen(const std::string &filename, std::int32_t x, std::int32_t y, std::int32_t w,
                   std::int32_t h, bool async)
{
  std::uint8_t *data;
  std::uint8_t *src;

  getmem(data, static_cast<std::size_t>(2 * w * h * 4));

  src = data;
  src += static_cast<ptrdiff_t>(w * h * 4);
  glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, src);

  tscreenshotthread *screenthread = new tscreenshotthread(filename, w, h, data);
  if (!async)
  {
    screenthread->execute();
    return; 
  }
  SoldatAssert(async == false);
#if 0
  screenthread->freeonterminate = true;
  screenthread->start();
  if (!async)
  {
    screenthread->waitfor();
  }
#endif // 0
}

auto argb(std::uint32_t argb) -> tgfxcolor { return rgba(argb, (std::uint32_t)argb >> 24); }
// FIXME: Please find better way to do it.
#ifdef FPC
void min(float a, float b)
{

  void min_result;
  if (a < b)
    result = a;
  else
    result = b;
  return min_result;
}
void max(float a, float b)
{

  void max_result;
  if (a > b)
    result = a;
  else
    result = b;
  return max_result;
}
#endif
auto rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) -> tgfxcolor
{
  tgfxcolor result;
  result.color.r = r;
  result.color.g = g;
  result.color.b = b;
  result.color.a = a;
  return result;
}
auto rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b) -> tgfxcolor
{
  tgfxcolor result;
  result.color.r = r;
  result.color.g = g;
  result.color.b = b;
  result.color.a = 255;
  return result;
}
auto rgba(std::uint32_t rgba) -> tgfxcolor
{
  tgfxcolor result;
  result.color.a = 255;
  result.color.r = (std::uint32_t)(rgba & 0xff0000) >> 16;
  result.color.g = (std::uint32_t)(rgba & 0xff00) >> 8;
  result.color.b = (rgba & 0xff);
  return result;
}

auto rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, double a) -> tgfxcolor
{

  tgfxcolor result;
  result.color.r = r;
  result.color.g = g;
  result.color.b = b;
  result.color.a = (std::uint8_t)(trunc(a));
  return result;
}

auto rgba(std::uint32_t rgb, float a) -> tgfxcolor
{

  tgfxcolor result;
  result.color.r = (std::uint32_t)(rgb & 0xff0000) >> 16;
  result.color.g = (std::uint32_t)(rgb & 0xff00) >> 8;
  result.color.b = (rgb & 0xff);
  result.color.a = (std::uint8_t)(trunc(a));
  return result;
}

auto gfxvertex(float x, float y, float u, float v, const tgfxcolor &c) -> tgfxvertex
{
  tgfxvertex result;
  result.x = x;
  result.y = y;
  result.u = u;
  result.v = v;
  result.color = c;
  return result;
}

void gfxbindtexture(tgfxtexture *texture)
{
  if (texture != nullptr)
  {
    glBindTexture(GL_TEXTURE_2D, texture->handle());
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, gfxcontext.whitetexture->handle());
  }
}

auto gfxcreatetexture(std::int32_t w, std::int32_t h, std::int32_t c, std::uint8_t *data,
                      const std::string_view &debug_name) -> tgfxtexture *
{
  return new tgfxtexture(w, h, c, false, false, data, debug_name);
}

auto gfxcreaterendertarget(std::int32_t w, std::int32_t h, std::int32_t c,
                           bool msaa) -> tgfxtexture *
{
  tgfxtexture *gfxcreaterendertarget_result = nullptr;

  if (gfxframebuffersupported())
  {
    gfxcreaterendertarget_result = new tgfxtexture(w, h, c, true, msaa, nullptr, "rendertarget");
  }
  return gfxcreaterendertarget_result;
}

void gfxupdatetexture(tgfxtexture *texture, std::int32_t x, std::int32_t y, std::int32_t w,
                      std::int32_t h, std::uint8_t *data)
{
  texture->update(x, y, w, h, data);
}

void gfxtexturewrap(tgfxtexture *texture, tgfxtexturewrap s, tgfxtexturewrap t)
{
  texture->setwrap(s, t);
}

void gfxtexturefilter(tgfxtexture *texture, tgfxtexturefilter min, tgfxtexturefilter mag)
{
  texture->setfilter(min, mag);
}

void gfxgeneratemipmap(tgfxtexture *texture)
{
  if (texture->handle() == 0)
  {
    return;
  }

#if 0 // only for gl2.1
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
#endif

  glBindTexture(GL_TEXTURE_2D, texture->handle());
#if 0
    glEnable(GL_TEXTURE_2D); // ati driver fuckery
#endif

  if (assigned(glGenerateMipmap))
  {
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GLint(GL_TRUE));
    glTexSubImage2D(GL_TEXTURE_2D, 0, texture->fpixel.x, texture->fpixel.y, 1, 1,
                    OPENGL_TEXTURE_FORMAT[texture->fcomponents - 1], GL_UNSIGNED_BYTE,
                    &texture->fpixel.color);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GLint(GL_FALSE));
  }
}

void gfxdeletetexture(tgfxtexture *&texture)
{
  freeandnullptr(texture);
}

static void premultiplycolor(pgfxvertex v, std::int32_t n)
{
  std::int32_t i;
  float a;

  for (i = 0; i <= n - 1; i++)
  {
    a = (float)(v->color.color.a) / 255;
    v->color.color.r = round(v->color.color.r * a);
    v->color.color.g = round(v->color.color.g * a);
    v->color.color.b = round(v->color.color.b * a);
    v += 1;
  }
}

auto gfxcreatebuffer(std::int32_t capacity, bool _static, pgfxvertex data) -> tgfxvertexbuffer *
{
  if ((data != nullptr) && (gfxcontext.shaderprogram == 0))
  {
    premultiplycolor(data, capacity);
  }

  return new tgfxvertexbuffer(capacity, _static, data);
}

void gfxupdatebuffer(tgfxvertexbuffer *b, std::int32_t i, std::int32_t n, pgfxvertex data)
{
  if (gfxcontext.shaderprogram == 0)
  {
    premultiplycolor(data, n);
  }

  b->update(i, n, data);
}

void gfxdeletebuffer(tgfxvertexbuffer *b)
{
  freeandnullptr(b);
}

auto gfxcreateindexbuffer(std::int32_t capacity, bool _static,
                          std::uint16_t *data) -> tgfxindexbuffer *
{
  return new tgfxindexbuffer(capacity, _static, data);
}

void gfxupdateindexbuffer(tgfxindexbuffer &b, std::int32_t i, std::int32_t n, std::uint16_t *data)
{
  b.update(i, n, data);
}

void gfxdeleteindexbuffer(tgfxindexbuffer *b)
{
  freeandnullptr(b);
}

void gfxbegin()
{
  std::int32_t i;

  tbatch &batch = gfxcontext.batch;

  if (length(batch.buffers) == 0)
  {
    batch.buffers.clear();
    batch.buffers.emplace_back();
    batch.buffers[0].size = 0;
    batch.buffers[0].capacity = 6 * batch_min;
    getmem(batch.buffers[0].data, batch.buffers[0].capacity * sizeof(tgfxvertex));
  }
  else if (length(batch.buffers) > 1)
  {
    for (i = 0; i < batch.buffers.size(); i++)
    {
      freemem(batch.buffers[i].data);
    }

    batch.buffers.clear();
    batch.buffers.emplace_back();
    batch.buffers[0] = batch.buffers[high(batch.buffers)];
  }

  batch.buffers[0].size = 0;
  batch.commandssize = 0;
}

void gfxend()
{
  std::int32_t n;
  std::int32_t total;

  tbatch &batch = gfxcontext.batch;
  total = 0;
  n = batch.commandssize;

  if (n > 0)
  {
    total = batch.commands[n - 1].offset + batch.commands[n - 1].count;
  }

  if (total == 0)
  {
    return;
  }

  if (batch.vertexbuffer == nullptr)
  {
    n = 1;

    while (n < total)
    {
      n = 2 * n;
    }

    batch.vertexbuffer = gfxcreatebuffer(max(n, 6 * batch_min));
  }
  else if (batch.vertexbuffer->capacity() < total)
  {
    n = batch.vertexbuffer->capacity();

    while (n < total)
    {
      n = 2 * n;
    }

    gfxdeletebuffer(batch.vertexbuffer);
    batch.vertexbuffer = gfxcreatebuffer(n);
  }

  n = 0;

  for (const auto &buffer : batch.buffers)
  {
    gfxupdatebuffer(batch.vertexbuffer, n, buffer.size, buffer.data);
    n += buffer.size;
  }

  gfxdraw(batch.vertexbuffer, batch.commands.data(), batch.commandssize);
}

template <typename Allocator>
void gfxdrawquad(tgfxtexture *texture, const std::vector<tgfxvertex, Allocator> &vertices)
{

  tbatch &b = gfxcontext.batch;

  // update commands list

  tgfxtexture *curtex = nullptr;
  std::int32_t n = b.commandssize;

  if (n > 0)
  {
    curtex = b.commands[n - 1].texture;
  }

  if (texture == nullptr)
  {
    texture = gfxcontext.whitetexture;
  }

  if (curtex != texture)
  {
    if (length(b.commands) < (n + 1))
    {
      b.commands.resize(max(2 * length(b.commands), 32uL));
    }

    b.commands[n].texture = texture;
    b.commands[n].offset = 0;
    b.commands[n].count = 0;

    if (n > 0)
    {
      b.commands[n].offset = b.commands[n - 1].offset + b.commands[n - 1].count;
    }

    b.commandssize += 1;
    n = b.commandssize;
  }

  b.commands[n - 1].count += 6;

  // update buffer

  n = length(b.buffers);
  tbatchbuffer *buf = &b.buffers[n - 1];

  if ((buf->size + 6) > buf->capacity)
  {
    n += 1;
    b.buffers.resize(n);
    buf = &b.buffers[n - 1];
    buf->size = 0;
    buf->capacity = 2 * b.buffers[n - 2].capacity;
    getmem(buf->data, buf->capacity * sizeof(tgfxvertex));
  }

  pgfxvertex v = buf->data;
  v += buf->size;

  *v = vertices[0];
  v += 1;
  *v = vertices[1];
  v += 1;
  *v = vertices[2];
  v += 1;
  *v = vertices[2];
  v += 1;
  *v = vertices[3];
  v += 1;
  *v = vertices[0];

  buf->size += 6;
}

void gfxdrawquad(tgfxtexture *texture, const tgfxvertex &a, const tgfxvertex &b,
                 const tgfxvertex &c, const tgfxvertex &d)
{
  std::vector<tgfxvertex> v{4};

  v[0] = a;
  v[1] = b;
  v[2] = c;
  v[3] = d;

  gfxdrawquad(texture, v);
}

void gfxspritevertices(pgfxsprite s, float x, float y, float w, float h, float sx, float sy,
                       float cx, float cy, float r, tgfxcolor color, pgfxvertex v)
{
  tgfxmat3 m;
  tvector2 p;

  m = gfxmat3transform(x, y, sx * s->scale, sy * s->scale, cx, cy, r);

  p = gfxmat3mul(m, 0, 0);
  v->x = p.x;
  v->y = p.y;
  v->u = s->texcoords.left;
  v->v = s->texcoords.top;
  v->color = color;

  v += 1;

  p = gfxmat3mul(m, w, 0);
  v->x = p.x;
  v->y = p.y;
  v->u = s->texcoords.right;
  v->v = s->texcoords.top;
  v->color = color;

  v += 1;

  p = gfxmat3mul(m, w, h);
  v->x = p.x;
  v->y = p.y;
  v->u = s->texcoords.right;
  v->v = s->texcoords.bottom;
  v->color = color;

  v += 1;

  p = gfxmat3mul(m, 0, h);
  v->x = p.x;
  v->y = p.y;
  v->u = s->texcoords.left;
  v->v = s->texcoords.bottom;
  v->color = color;
}

void gfxdrawsprite(pgfxsprite s, float x, float y)
{
  gfxdrawsprite(s, x, y, 1, 1, 0, 0, 0, rgba(0xffffff));
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float scale)
{
  gfxdrawsprite(s, x, y, scale, scale, 0, 0, 0, rgba(0xffffff));
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy)
{
  gfxdrawsprite(s, x, y, sx, sy, 0, 0, 0, rgba(0xffffff));
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float rx, float ry, float r)
{
  gfxdrawsprite(s, x, y, 1, 1, rx, ry, r, rgba(0xffffff));
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, float rx, float ry, float r)
{
  gfxdrawsprite(s, x, y, sx, sy, rx, ry, r, rgba(0xffffff));
}

void gfxdrawsprite(pgfxsprite s, float x, float y, const tgfxrect &rc)
{
  gfxdrawsprite(s, x, y, 1, 1, 0, 0, 0, rgba(0xffffff), rc);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float scale, const tgfxrect &rc)
{
  gfxdrawsprite(s, x, y, scale, scale, 0, 0, 0, rgba(0xffffff), rc);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, const tgfxrect &rc)
{
  gfxdrawsprite(s, x, y, sx, sy, 0, 0, 0, rgba(0xffffff), rc);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float rx, float ry, float r, const tgfxrect &rc)
{
  gfxdrawsprite(s, x, y, 1, 1, rx, ry, r, rgba(0xffffff), rc);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, float rx, float ry, float r,
                   const tgfxrect &rc)
{
  gfxdrawsprite(s, x, y, sx, sy, rx, ry, r, rgba(0xffffff), rc);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, const tgfxcolor &color)
{
  gfxdrawsprite(s, x, y, 1, 1, 0, 0, 0, color);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float scale, const tgfxcolor &color)
{
  gfxdrawsprite(s, x, y, scale, scale, 0, 0, 0, color);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, const tgfxcolor &color)
{
  gfxdrawsprite(s, x, y, sx, sy, 0, 0, 0, color);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float rx, float ry, float r,
                   const tgfxcolor &color)
{
  gfxdrawsprite(s, x, y, 1, 1, rx, ry, r, color);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, const tgfxcolor &color, const tgfxrect &rc)
{
  gfxdrawsprite(s, x, y, 1, 1, 0, 0, 0, color, rc);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float scale, const tgfxcolor &color,
                   const tgfxrect &rc)
{
  gfxdrawsprite(s, x, y, scale, scale, 0, 0, 0, color, rc);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, const tgfxcolor &color,
                   const tgfxrect &rc)
{
  gfxdrawsprite(s, x, y, sx, sy, 0, 0, 0, color, rc);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float rx, float ry, float r,
                   const tgfxcolor &color, const tgfxrect &rc)
{
  gfxdrawsprite(s, x, y, 1, 1, rx, ry, r, color, rc);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, float rx, float ry, float r,
                   const tgfxcolor &color)
{
  std::vector<tgfxvertex> v{4};

  gfxspritevertices(s, x, y, s->width, s->height, sx, sy, rx, ry, -r, color, v.data());
  gfxdrawquad(s->texture, v);
}

void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, float rx, float ry, float r,
                   const tgfxcolor &color, const tgfxrect &rc)
{
  float w;
  float h;
  std::vector<tgfxvertex> v{4};
  tgfxrect rect;
  pgfxrect tc;

  w = min(rc.right - rc.left, (float)s->width);
  h = min(rc.bottom - rc.top, (float)s->height);
  tc = &s->texcoords;

  gfxspritevertices(s, x, y, w, h, sx, sy, rx, ry, -r, color, v.data());

  rect.left = (float)((s->x + rc.left)) / s->texture->width();
  rect.top = (float)((s->y + rc.top)) / s->texture->height();
  rect.right = (float)((s->x + rc.right)) / s->texture->width();
  rect.bottom = (float)((s->y + rc.bottom)) / s->texture->height();

  rect.left = max(min(rect.left, tc->right), tc->left);
  rect.top = max(min(rect.top, tc->bottom), tc->top);
  rect.right = max(min(rect.right, tc->right), tc->left);
  rect.bottom = max(min(rect.bottom, tc->bottom), tc->top);

  v[0].u = rect.left;
  v[0].v = rect.top;
  v[1].u = rect.right;
  v[1].v = rect.top;
  v[2].u = rect.right;
  v[2].v = rect.bottom;
  v[3].u = rect.left;
  v[3].v = rect.bottom;

  gfxdrawquad(s->texture, v);
}

/******************************************************************************/
/*                              Font (internal)                               */
/******************************************************************************/

void addfontpage(pfont f)
{
  fillchar(f->buffer, static_cast<std::size_t>(4 * f->width * f->height), 0);

  setlength(f->pages, length(f->pages) + 1);
  f->pages[high(f->pages)] = gfxcreatetexture(f->width, f->height, 4, f->buffer, "Font page");
  gfxtexturefilter(f->pages[high(f->pages)], gfx_nearest, gfx_nearest);

  f->pageDesc.emplace_back(f->width, f->height);
}

void requestfontsize(pfont f, std::int32_t fontsize, std::int32_t stretch)
{
  float fontsizef;
  FT_Size_RequestRec sizereq;

  fontsizef = ((float)(fontsize) / 1000) * ((float)(96) / 72);

  sizereq.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
  sizereq.width = round(64 * fontsizef * ((float)(stretch) / 1000));
  sizereq.height = round(64 * fontsizef);
  sizereq.horiResolution = 0;
  sizereq.vertResolution = 0;

  FT_Request_Size(f->handle, &sizereq);
}

auto addglyphtable(pfont f, std::int32_t fontsize, std::int32_t stretch,
                   std::uint32_t flags) -> std::int32_t
{
  std::int32_t i;

  std::int32_t result;
  i = length(f->tables);
  f->tables.resize(i + 1);

  f->tables[i].fontsize = fontsize;
  f->tables[i].stretch = stretch;
  f->tables[i].flags = flags;
  f->tables[i].vspace = (float)(f->handle->size->metrics.height) / 64;
  f->tables[i].ascent = fabs(f->handle->size->metrics.ascender) / 64;
  f->tables[i].descent = fabs(f->handle->size->metrics.descender) / 64;
  f->tables[i].size = 0;
  f->tables[i].capacity = glyph_pool_size;

  getmem(f->tables[i].glyphs, sizeof(pglyph) * glyph_pool_size);

  result = i;
  return result;
}

auto findglyphtable(pfont f, std::int32_t fontsize, std::int32_t stretch,
                    std::uint32_t flags) -> std::int32_t
{
  std::int32_t result;
  requestfontsize(f, fontsize, stretch);
  result = 0;
  for (result = 0; result < f->tables.size(); result++)
  {
    if ((f->tables[result].fontsize == fontsize) && (f->tables[result].stretch == stretch) &&
        (f->tables[result].flags == flags))
    {
      return result;
    }
  }

  result = addglyphtable(f, fontsize, stretch, flags);
  return result;
}

auto allocglyph(pfont f) -> pglyph
{
  pglyph result;
  if (f->poolindex == glyph_pool_size)
  {
    f->poolindex = 0;
    f->pool.push_back({});
    f->pool[high(f->pool)] = f->pool[0];
    getmem(f->pool[0], glyph_pool_size * sizeof(tglyph));
  }

  result = f->pool[0];
  result += f->poolindex;
  f->poolindex += 1;
  return result;
}

void DumpGlyph(const FT_Bitmap &glyph)
{
  LogDebugG("dump glyph:");
  for (auto y = 0u; y < glyph.rows; y++)
  {
    std::string line;
    for (auto x = 0u; x < glyph.width; x++)
    {
      line += *(glyph.buffer + static_cast<size_t>(y * glyph.pitch) + x);
    }
    LogDebugG("    {}", line);
  }
}

void loadglyphbitmap(pfont f, pglyphtable table, pglyph glyph)
{
  std::uint8_t pixel;
  std::uint8_t *srcrow;
  std::uint8_t *src;
  std::uint8_t *dst;
  std::int32_t i;
  std::int32_t j;
  std::int32_t k;
  std::int32_t w;
  std::int32_t h;
  std::int32_t wb;
  std::int32_t wr;
  std::int32_t bufsize;
  std::int32_t x = 0;
  std::int32_t y = 0;

  w = round(glyph->bounds.right - glyph->bounds.left);
  h = round(glyph->bounds.bottom - glyph->bounds.top);
  bufsize = 4 * w * h;

  if (f->buffersize < bufsize)
  {
    freemem(f->buffer);
    f->buffersize = npot(bufsize);
    getmem(f->buffer, f->buffersize);
  }

  Rectangle r = Rectangle(0, 0, w + 1, h + 1);

  if (f->pageDesc.empty() || !f->pageDesc.back().Add(r))
  {
    addfontpage(f);
    f->pageDesc.back().Add(r);
  }
  x = r.x;
  y = r.y;

  glyph->page = high(f->pages);
  glyph->texcoords.left = (float)(x) / f->width;
  glyph->texcoords.top = (float)(y) / f->height;
  glyph->texcoords.right = (float)((x + w)) / f->width;
  glyph->texcoords.bottom = (float)((y + h)) / f->height;

  srcrow = f->handle->glyph->bitmap.buffer;
  dst = f->buffer;

  if ((table->flags & gfx_monochrome) == 0)
  {
    for (j = 1; j <= h; j++)
    {
      src = srcrow;

      for (i = 1; i <= w; i++)
      {
        *dst = *src;
        dst += 1;
        *dst = *src;
        dst += 1;
        // extra bytes
        *dst = *src;
        dst += 1;
        *dst = *src;
        dst += 1;
        // extra bytes

        src += 1;
      }

      srcrow += f->handle->glyph->bitmap.pitch;
    }
  }
  else
  {
    wb = w / 8;
    wr = 8 - (w % 8);

    for (j = 1; j <= h; j++)
    {
      src = srcrow;

      // unpack full bytes
      for (i = 1; i <= wb; i++)
      {
        for (k = 7; k >= 0; k--)
        {
          pixel = 0xff & ((std::uint32_t)0xff00 >> ((((std::uint32_t)*src >> k) & 1) << 3));
          *dst = pixel;
          dst += 1;
          *dst = pixel;
          dst += 1;
          // extra bytes
          *dst = pixel;
          dst += 1;
          *dst = pixel;
          dst += 1;
          // extra bytes
        }

        src += 1;
      }

      // unpack bits from leftover std::uint8_t if any
      for (k = 7; k >= wr; k--)
      {
        pixel = 0xff & ((std::uint32_t)0xff00 >> ((((std::uint32_t)*src >> k) & 1) << 3));
        *dst = pixel;
        dst += 1;
        *dst = pixel;
        dst += 1;
        // extra bytes
        *dst = pixel;
        dst += 1;
        *dst = pixel;
        dst += 1;
        // extra bytes
      }

      srcrow += f->handle->glyph->bitmap.pitch;
    }
  }

  gfxupdatetexture(f->pages[glyph->page], x, y, w, h, f->buffer);
}

auto loadglyph(pfont f, pglyphtable table, std::int32_t glyphindex) -> pglyph
{
  std::int32_t w;
  std::int32_t h;

  pglyph result;
  auto flags = FT_LOAD_RENDER | FT_LOAD_TARGET_MONO;
  if ((table->flags & gfx_monochrome) == 0)
  {
    flags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT;
  }

  auto glyphError = FT_Load_Glyph(f->handle, glyphindex, flags);
  SoldatAssert(glyphError == 0);

  w = f->handle->glyph->bitmap.width;
  h = f->handle->glyph->bitmap.rows;

  result = allocglyph(f);
  result->glyphindex = glyphindex;
  result->page = -2;
  result->advance = (float)(f->handle->glyph->advance.x) / 64.0f;
  result->bounds.left = f->handle->glyph->bitmap_left;
  result->bounds.top = -f->handle->glyph->bitmap_top;
  result->bounds.right = result->bounds.left + w;
  result->bounds.bottom = result->bounds.top + h;

  if ((w == 0) || (h == 0) || (w > (f->width - 2)) || (h > (f->height - 2)))
  {
    result->page = -1;
    fillchar(&result->bounds, sizeof(tgfxrect), 0);
  }
  else
  {
    loadglyphbitmap(f, table, result);
  }
  return result;
}

void insertglyph(pglyphtable table, std::int32_t index, pglyph glyph)
{
  ppglyph glyphs;
  ppglyph src;
  ppglyph dst;

  if (table->capacity == table->size)
  {
    getmem(glyphs, 2 * sizeof(pglyph) * table->capacity);
    std::memcpy(glyphs, table->glyphs, sizeof(pglyph) * table->capacity);
    freemem(table->glyphs);
    table->glyphs = glyphs;
    table->capacity = 2 * table->capacity;
  }

  src = table->glyphs;
  dst = table->glyphs;
  src += index;
  dst += index + 1;
  std::memmove(dst, src, (table->size - index) * sizeof(pglyph));
  table->size += 1;
  *src = glyph;
}

auto findglyph(pfont f, pglyphtable table, std::int32_t glyphindex) -> pglyph
{
  std::int32_t lo;
  std::int32_t hi;
  std::int32_t mi;
  ppglyph g;

  pglyph result;
  lo = 0;
  hi = table->size;

  while (lo < hi)
  {
    mi = lo + ((std::uint32_t)(hi - lo) >> 1);
    g = table->glyphs;
    g += mi;

    if (glyphindex < (*g)->glyphindex)
    {
      hi = mi;
    }
    else if (glyphindex > (*g)->glyphindex)
    {
      lo = mi + 1;
    }
    else
    {
      result = *g;
      return result;
    }
  }

  result = loadglyph(f, table, glyphindex);
  insertglyph(table, lo, result);
  return result;
}

auto iswhitespace(char ch) -> bool { return (ch == '\12') || (ch == '\15'); }

void strtoglyphs(pfont f, pglyphtable table, const std::string &s)
{
  std::int32_t i;
  std::int32_t *indexstr;
  ppglyph glyphstr;

  if (gfxcontext.textstrsize < length(s))
  {
    if (gfxcontext.textstrsize > 0)
    {
      freemem(gfxcontext.textindexstr);
      freemem(gfxcontext.textglyphstr);
      freemem(gfxcontext.textcomputedstr);
    }

    i = npot(length(s));

    gfxcontext.textstrsize = i;
    gfxcontext.textcomputedcount = 0;
    getmem(gfxcontext.textindexstr, i * sizeof(std::int32_t));
    getmem(gfxcontext.textglyphstr, i * sizeof(pglyph));
    getmem(gfxcontext.textcomputedstr, i * sizeof(tcomputedglyph));
  }

  indexstr = gfxcontext.textindexstr;

  for (const auto &c : s)
  {
    *indexstr = 0;

    if (!iswhitespace(c))
    {
      *indexstr = FT_Get_Char_Index(f->handle, FT_ULong(c));
    }

    indexstr += 1;
  }

  indexstr = gfxcontext.textindexstr;
  glyphstr = gfxcontext.textglyphstr;

  for (const auto &c : s)
  {
    *glyphstr = nullptr;

    if (!iswhitespace(c))
    {
      *glyphstr = findglyph(f, table, *indexstr);
    }

    indexstr += 1;
    glyphstr += 1;
  }
}

void computeglyphs(pfont f, pglyphtable table, const std::string &s)
{
  std::int32_t i;
  std::int32_t prev;
  float x;
  float y;
  float vspace;
  FT_Vector kerning;
  std::int32_t *indexstr;
  ppglyph glyphstr;
  pcomputedglyph computed;

  strtoglyphs(f, table, s);

  gfxcontext.textcomputedcount = 0;
  indexstr = gfxcontext.textindexstr;
  glyphstr = gfxcontext.textglyphstr;
  computed = gfxcontext.textcomputedstr;

  x = 0;
  y = 0;
  vspace = table->vspace;
  prev = -1;

  for (i = 1; i <= length(s); i++)
  {
    if (s[i] == '\12')
    {
      x = 0;
      y = y + vspace;
      prev = -1;
    }
    else if ((s[i] != '\15') && (*glyphstr != nullptr))
    {
      if (prev != -1)
      {
        FT_Get_Kerning(f->handle, prev, *indexstr, 0, &kerning);
        x = x + (float)(kerning.x) / 64;
      }

      prev = *indexstr;
      computed->x = x;
      computed->y = y;
      computed->glyph = *glyphstr;
      x = x + (*glyphstr)->advance;

      computed += 1;
      gfxcontext.textcomputedcount += 1;
    }

    indexstr += 1;
    glyphstr += 1;
  }
}

void drawglyph(pfont f, pglyph g, float x, float y, const tgfxcolor &color)
{
  ZoneScopedN("DrawGlyph");
  std::array<tgfxvertex, 4> buff;
  std::pmr::monotonic_buffer_resource res(buff.data(), buff.size() * sizeof(tgfxvertex));
  std::pmr::vector<tgfxvertex> v{4, &res};
  tvector2 pxl;

  pxl = gfxcontext.textpixelratio;
  pxl.x = pxl.x * gfxcontext.textscale;
  pxl.y = pxl.y * gfxcontext.textscale;

  v[0].x = x + pxl.x * g->bounds.left;
  v[0].y = y + pxl.y * g->bounds.top;
  v[0].u = g->texcoords.left;
  v[0].v = g->texcoords.top;
  v[0].color = color;

  v[1].x = x + pxl.x * g->bounds.right;
  v[1].y = y + pxl.y * g->bounds.top;
  v[1].u = g->texcoords.right;
  v[1].v = g->texcoords.top;
  v[1].color = color;

  v[2].x = x + pxl.x * g->bounds.right;
  v[2].y = y + pxl.y * g->bounds.bottom;
  v[2].u = g->texcoords.right;
  v[2].v = g->texcoords.bottom;
  v[2].color = color;

  v[3].x = x + pxl.x * g->bounds.left;
  v[3].y = y + pxl.y * g->bounds.bottom;
  v[3].u = g->texcoords.left;
  v[3].v = g->texcoords.bottom;
  v[3].color = color;

  gfxdrawquad(f->pages[g->page], v);
}

/******************************************************************************/
/*                               Font (public)                                */
/******************************************************************************/

auto gfxcreatefont(const string &filename, std::int32_t w, std::int32_t h) -> tgfxfont
{
  pfont f;
  FT_Face fonthandle;

  tgfxfont gfxcreatefont_result = nullptr;

  gfxlog(string("Loading font (") + filename + ')');

  auto loadFont = [&]() {
    if (filename == "play-regular.ttf")
    {
      extern unsigned char sDefaultFont[];
      extern unsigned int sDefaultFont_len;
      return FT_New_Memory_Face(gfxcontext.ftlibrary, sDefaultFont, sDefaultFont_len, 0, &fonthandle);
    }
    return FT_New_Face(gfxcontext.ftlibrary, filename.c_str(), 0, &fonthandle);
  };
  if (loadFont() != 0)
  {
    gfxlog(string("Failed to load font (") + filename + ')');
    return gfxcreatefont_result;
  }

  if (FT_Select_Charmap(fonthandle, ft_encoding_unicode) != 0)
  {
    gfxlog(string("Font doesn't support unicode (") + filename + ')');
    FT_Done_Face(fonthandle);
    return gfxcreatefont_result;
  }

  f = new tfont;

  f->handle = fonthandle;
  f->width = w;
  f->height = h;

  f->buffersize = 4 * w * h;
  getmem(f->buffer, f->buffersize);

  f->poolindex = 0;
  setlength(f->pool, 1);
  getmem(f->pool[0], glyph_pool_size * sizeof(tglyph));

  return f;
}

void gfxdeletefont(tgfxfont font)
{
  std::int32_t i;
  pfont f;

  f = (pfont)font;

  if (f->handle != nullptr)
  {
    FT_Done_Face(f->handle);
  }

  if (f->buffer != nullptr)
  {
    freemem(f->buffer);
  }

  for (i = 0; i < f->pages.size(); i++)
  {
    gfxdeletetexture(f->pages[i]);
  }

  for (i = 0; i < f->tables.size(); i++)
  {
    freemem(f->tables[i].glyphs);
  }

  for (i = 0; i < f->pool.size(); i++)
  {
    freemem(f->pool[i]);
  }

  delete f;
  font = nullptr;
}

auto gfxsetfont(tgfxfont font, float fontsize, std::uint32_t flags, float stretch) -> std::int32_t
{
  std::int32_t result;
  result = findglyphtable((pfont)font, trunc(fontsize * 1000), trunc(stretch * 1000), flags);

  gfxcontext.font = (pfont)font;
  gfxcontext.glyphtable = &((pfont)(font))->tables[result];
  gfxcontext.textcomputedcount = 0;
  return result;
}

void gfxsetfonttable(tgfxfont font, std::int32_t tableindex)
{
  gfxcontext.font = (pfont)font;
  gfxcontext.glyphtable = &((pfont)(font))->tables[tableindex];
  gfxcontext.textcomputedcount = 0;

  requestfontsize((pfont)font, gfxcontext.glyphtable->fontsize, gfxcontext.glyphtable->stretch);
}

void gfxtextpixelratio(const tvector2 &pixelratio)
{
  gfxcontext.textpixelratio = pixelratio;
}

void gfxtextscale(float s)
{
  gfxcontext.textscale = s;
}

void gfxtextcolor(const tgfxcolor &color)
{
  gfxcontext.textcolor = color;
}

void gfxtextshadow(float dx, float dy, const tgfxcolor &color)
{
  gfxcontext.textshadowoffset.x = dx;
  gfxcontext.textshadowoffset.y = dy;
  gfxcontext.textshadowcolor = color;
}

void gfxtextverticalalign(tgfxverticalalign align)
{
  gfxcontext.textverticalalign = align;
}

auto gfxtextmetrics() -> tgfxrect
{
  pcomputedglyph comp;
  std::int32_t i;
  tvector2 pxl;

  tgfxrect result;
  comp = gfxcontext.textcomputedstr;
  pxl = gfxcontext.textpixelratio;
  pxl.x = pxl.x * gfxcontext.textscale;
  pxl.y = pxl.y * gfxcontext.textscale;

  for (i = 1; i <= gfxcontext.textcomputedcount; i++)
  {
    result.left = min(result.left, (MyFloat)(pxl.x * (comp->x + comp->glyph->bounds.left)));
    result.right = max(result.right, (MyFloat)(pxl.x * (comp->x + comp->glyph->bounds.right)));
    result.top = min(result.top, (MyFloat)(pxl.y * (comp->y + comp->glyph->bounds.top)));
    result.bottom = max(result.bottom, (MyFloat)(pxl.y * (comp->y + comp->glyph->bounds.bottom)));
    comp += 1;
  }
  return result;
}

auto gfxtextmetrics(const std::string &text) -> tgfxrect
{
  computeglyphs(gfxcontext.font, gfxcontext.glyphtable, text);
  return gfxtextmetrics();
}

void gfxdrawtext(MyFloat x, MyFloat y)
{
  pfont f;
  pglyphtable table;
  pcomputedglyph comp;
  std::int32_t i;
  float s;
  float dx;
  float dy;
  tvector2 p;
  tvector2 pxl;
  tgfxcolor textcolor;
  tgfxcolor shadowcolor;

  f = gfxcontext.font;
  table = gfxcontext.glyphtable;
  comp = gfxcontext.textcomputedstr;
  textcolor = gfxcontext.textcolor;
  shadowcolor = gfxcontext.textshadowcolor;
  shadowcolor.color.a = trunc(shadowcolor.color.a * ((float)(textcolor.color.a) / 255));
  pxl = gfxcontext.textpixelratio;
  s = gfxcontext.textscale;
  dx = gfxcontext.textshadowoffset.x * pxl.x;
  dy = gfxcontext.textshadowoffset.y * pxl.y;

  switch (gfxcontext.textverticalalign)
  {
  case gfx_top:
    y = y + pxl.y * table->ascent * s;
    break;
  case gfx_bottom:
    y = y - pxl.y * table->descent * s;
    break;
  case gfx_baseline:
    //y = y;
    break;
  default:
    SoldatAssert(false);
  }

  x = pxl.x * floor((float)(x) / pxl.x);
  y = pxl.y * floor((float)(y) / pxl.y);

  pxl.x = pxl.x * s;
  pxl.y = pxl.y * s;

  for (i = 1; i <= gfxcontext.textcomputedcount; i++)
  {
    if (comp->glyph->page >= 0)
    {
      p.x = x + pxl.x * comp->x;
      p.y = y + pxl.y * comp->y;

      if (shadowcolor.color.a > 0)
      {
        drawglyph(f, comp->glyph, p.x + dx, p.y + dy, shadowcolor);
      }

      drawglyph(f, comp->glyph, p.x, p.y, textcolor);
    }

    comp += 1;
  }
}

void gfxdrawtext(const std::string &text, float x, float y)
{
  computeglyphs(gfxcontext.font, gfxcontext.glyphtable, text);
  gfxdrawtext(x, y);
}

void gfxdrawtext(const std::string text, float x, float y)
{
  computeglyphs(gfxcontext.font, gfxcontext.glyphtable, (text));
  gfxdrawtext(x, y);
}
/******************************************************************************/
/*                                   Matrix                                   */
/******************************************************************************/

auto gfxmat3rot(float r) -> tgfxmat3
{

  float c;
  float s;

  tgfxmat3 result;
  c = cos(r);
  s = sin(r);
  tgfxmat3 &m = result;

  m[0] = c;
  m[3] = -s;
  m[6] = 0;
  m[1] = s;
  m[4] = c;
  m[7] = 0;
  m[2] = 0;
  m[5] = 0;
  m[8] = 1;
  return result;
}

auto gfxmat3ortho(float l, float r, float t, float b) -> tgfxmat3
{
  float w;
  float h;

  tgfxmat3 result;
  w = r - l;
  h = t - b;

  result[0] = 2.f / w;
  result[3] = 0.f;
  result[6] = -(r + l) / w;
  result[1] = 0.f;
  result[4] = 2.f / h;
  result[7] = -(t + b) / h;
  result[2] = 0;
  result[5] = 0;
  result[8] = 1.f;
  return result;
}

auto gfxmat3transform(float tx, float ty, float sx, float sy, float cx, float cy,
                      float r) -> tgfxmat3
{

  float c;
  float s;

  tgfxmat3 result;
  c = cos(r);
  s = sin(r);

  tgfxmat3 &m = result; // m = T(tx,ty) * T(cx,cy) * R(r) * T(-cx,-cy) * S(sx,sy)
  m[0] = c * sx;
  m[3] = -s * sy;
  m[6] = tx + cy * s - c * cx + cx;
  m[1] = s * sx;
  m[4] = c * sy;
  m[7] = ty - cx * s - c * cy + cy;
  m[2] = 0;
  m[5] = 0;
  m[8] = 1;
  return result;
}

auto gfxmat3mul(const tgfxmat3 &m, float x, float y) -> tvector2
{
  tvector2 result;
  result.x = m[0] * x + m[3] * y + m[6];
  result.y = m[1] * x + m[4] * y + m[7];
  return result;
}

/******************************************************************************/
/*                                 TGfxImage                                  */
/******************************************************************************/

void applycolorkey(std::uint8_t *data, std::int32_t w, std::int32_t h, tgfxcolor colorkey)
{
  std::int32_t i;
  std::uint8_t *p;
  tgfxcolor c;

  p = data;

  for (i = 0; i <= (w * h) - 1; i++)
  {
    c.color.r = *p;
    p += 1;
    c.color.g = *p;
    p += 1;
    c.color.b = *p;
    p += 1;
    c.color.a = *p;

    if (c.rgba == colorkey.rgba)
    {
      *p = 0;
    }

    p += 1;
  }
}

tgfximage::tgfximage(const std::string &filename, tgfxcolor colorkey)
{
  auto& fs = GS::GetFileSystem();

  auto filebuffer = fs.ReadFile(filename);
  if (length(filebuffer) > 0)
  {
    fdata = stbi_load_from_memory(filebuffer.data(), length(filebuffer), &fwidth, &fheight,
                                  &fnumframes, STBI_rgb_alpha);
  }
  else
  {
    fdata = nullptr;
  }

  if (fdata != nullptr)
  {
    fnumframes = 1;
    fcomponents = 4;
    floadedfromfile = true;
    if ((fnumframes == 1) && (colorkey.rgba != 0))
    {
      applycolorkey(fdata, fwidth, fheight, colorkey);
    }
  }
  else
  {
    fwidth = 0;
    fheight = 0;
    fnumframes = 0;
    gfxlog(string("Failed to load image ") + filename);
  }
}

tgfximage::tgfximage(std::int32_t width, std::int32_t height, std::int32_t comp)
{
  getmem(fdata, static_cast<std::size_t>(width * height * comp));
  fillchar(fdata, static_cast<std::size_t>(width * height * comp), 0);
  fwidth = width;
  fheight = height;
  fcomponents = comp;
  fnumframes = 1;
  floadedfromfile = false;
}

tgfximage::~tgfximage()
{
  if (floadedfromfile)
  {
    stbi_image_free(fdata);
  }
  else if (fdata != nullptr)
  {
    freemem(fdata);
  }
}

auto tgfximage::getimagedata(std::int32_t frame) -> std::uint8_t *
{
  auto *result = fdata;
  result += static_cast<ptrdiff_t>((fwidth * fheight * fcomponents + 2) * frame);
  return result;
}

auto tgfximage::getframedelay(std::int32_t frame) -> std::uint64_t
{
  std::uint8_t *p;

  std::uint64_t result = 0;

  if (fnumframes > 1)
  {
    p = fdata;
    p += (fwidth * fheight * fcomponents + 2) * (frame + 1) - 2;
    result = *p;
    p += 1;
    result = result | ((std::uint64_t)(*p) << 8);
  }
  return result;
}

void tgfximage::update(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h,
                       std::uint8_t *data, std::int32_t frame)
{
  std::uint8_t *src;
  std::uint8_t *dst;
  std::int32_t i;
  std::int32_t srcline;
  std::int32_t dstline;

  // - number of components same on both buffers or gtfo
  // - update within bounds or gtfo

  srcline = fcomponents * w;
  dstline = fcomponents * fwidth;

  src = data;
  dst = fdata;

  dst += (dstline * fheight + 2) * frame + (dstline * y + x * fcomponents);

  for (i = 0; i <= h - 1; i++)
  {
    std::memcpy(dst, src, srcline);
    src += srcline;
    dst += dstline;
  }
}

void tgfximage::premultiply()
{
  std::uint8_t *p;
  std::int32_t i;
  std::int32_t j;
  float a;

  if (fcomponents != 4)
  {
    return;
  }

  p = fdata;

  for (i = 0; i < fnumframes; i++)
  {
    for (j = 0; j < fwidth * fheight; j++)
    {
      p += 3;
      a = (float)(*p) / 255;
      p -= 3;

      *p = round(*p * a);
      p += 1;
      *p = round(*p * a);
      p += 1;
      *p = round(*p * a);
      p += 2;
    }

    p += 2;
  }
}

void tgfximage::resize(std::int32_t w, std::int32_t h)
{
  std::int32_t i;
  std::int32_t size;
  std::uint8_t *data;
  std::uint8_t *dst;
  std::uint64_t delay;

  if (fdata == nullptr)
  {
    return;
  }

  size = w * h * fcomponents + (2 * std::int32_t(fnumframes > 1));
  getmem(data, static_cast<std::size_t>(fnumframes * size));

  dst = data;

  for (i = 0; i <= fnumframes - 1; i++)
  {
    stbir_resize_uint8(getimagedata(i), fwidth, fheight, 0, dst, w, h, 0, fcomponents);

    dst += size;

    if (fnumframes > 1)
    {
      delay = getframedelay(i);
      dst -= 2;
      *dst = delay & 0xff;
      dst += 1;
      *dst = ((std::uint32_t)delay >> 8) & 0xff;
      dst += 1;
    }
  }
  if (floadedfromfile)
  {
    stbi_image_free(fdata);
  }
  else
  {
    freemem(fdata);
  }

  fwidth = w;
  fheight = h;
  fdata = data;
  floadedfromfile = false;
}

/******************************************************************************/
/*                                TGfxTexture                                 */
/******************************************************************************/

tgfxtexture::tgfxtexture(std::int32_t width, std::int32_t height, std::int32_t comp, bool rt,
                         bool msaa, std::uint8_t *data, const std::string_view &debug_name)
{
  fwidth = width;
  fheight = height;
  fcomponents = comp;
  ffbohandle = 0;
  fsamples = 0;
  fpixel.x = 0;
  fpixel.y = 0;
  fpixel.color.rgba = 0;

  glGenTextures(1, &fhandle);
  if (msaa && (gfxcontext.msaasamples > 0))
  {
    fsamples = gfxcontext.msaasamples;

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fhandle);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, gfxcontext.msaasamples,
                            OPENGL_TEXTURE_FORMAT[comp], width, height, 1u);

    glGenFramebuffers(1, &ffbohandle);
    glBindFramebuffer(GL_FRAMEBUFFER, ffbohandle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fhandle,
                           0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, fhandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, OPENGL_TEXTURE_FORMAT[comp - 1], width, height, 0,
                 OPENGL_TEXTURE_FORMAT[comp - 1], GL_UNSIGNED_BYTE, data);

    if (rt)
    {
      glGenFramebuffers(1, &ffbohandle);
      glBindFramebuffer(GL_FRAMEBUFFER, ffbohandle);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fhandle, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
  }
  auto debugNameSize = debug_name.size();
  if (debugNameSize > OPENGL_MAX_LABEL_LENGTH)
  {
    debugNameSize = OPENGL_MAX_LABEL_LENGTH;
    LogInfoG("debug name {} is too long. Max length is {}", debug_name, OPENGL_MAX_LABEL_LENGTH);
  }
  if (not IsOpenGLES())
  {
    glObjectLabel(GL_TEXTURE, fhandle, debugNameSize, debug_name.data());
  }
  if (data != nullptr)
  {
    std::memcpy(&fpixel.color, data, comp);
  }
}

tgfxtexture::~tgfxtexture()
{
  if (ffbohandle != 0)
  {
    glDeleteFramebuffers(1, &ffbohandle);
  }

  if (fhandle != 0)
  {
    glDeleteTextures(1, &fhandle);
  }
}

void tgfxtexture::update(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h,
                         std::uint8_t *data)
{
  if (fhandle == 0)
  {
    return;
  }

  glBindTexture(GL_TEXTURE_2D, fhandle);
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, OPENGL_TEXTURE_FORMAT[fcomponents - 1],
                  GL_UNSIGNED_BYTE, data);
  auto glError = glGetError();
  SoldatAssert(glError == 0);
  glFinish();

  fpixel.x = x;
  fpixel.y = y;
  std::memcpy(&fpixel.color, data, fcomponents);
}

void gfxSetGpuDevice(SDL_GPUDevice*)
{
  
}

void tgfxtexture::setwrap(tgfxtexturewrap s, tgfxtexturewrap t)
{
  if (fhandle == 0)
  {
    return;
  }

  glBindTexture(GL_TEXTURE_2D, fhandle);

  switch (s)
  {
  case gfx_clamp:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    break;
  case gfx_repeat:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    break;
  default:
    SoldatAssert(false);
  }

  switch (t)
  {
  case gfx_clamp:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    break;
  case gfx_repeat:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    break;
  default:
    SoldatAssert(false);
  }
}

void tgfxtexture::setfilter(tgfxtexturefilter min, tgfxtexturefilter mag)
{
  if (fhandle == 0)
  {
    return;
  }

  glBindTexture(GL_TEXTURE_2D, fhandle);

  switch (min)
  {
  case gfx_linear:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    break;
  case gfx_nearest:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    break;
  case gfx_mipmap_linear:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    break;
  case gfx_mipmap_nearest:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    break;
  default:
    SoldatAssert(false);
  }

  switch (mag)
  {
  case gfx_linear:
  case gfx_mipmap_linear:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    break;
  case gfx_nearest:
  case gfx_mipmap_nearest:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    break;
  default:
    SoldatAssert(false);
  }
}

/******************************************************************************/
/*                              TGfxVertexBuffer                              */
/******************************************************************************/

tgfxvertexbuffer::tgfxvertexbuffer(std::int32_t cap, bool _static, pgfxvertex data)
{
  GLenum hint;

  fcapacity = cap;

  if (_static)
  {
    hint = GL_STATIC_DRAW;
  }
  else
  {
    hint = GL_STREAM_DRAW;
  }

  glGenBuffers(1, &fhandle);
  glBindBuffer(GL_ARRAY_BUFFER, fhandle);
  glBufferData(GL_ARRAY_BUFFER, sizeof(tgfxvertex) * cap, data, hint);
}

tgfxvertexbuffer::~tgfxvertexbuffer()
{
  if (fhandle != 0)
  {
    glDeleteBuffers(1, &fhandle);
  }
}

void tgfxvertexbuffer::update(std::int32_t offset, std::int32_t count, pgfxvertex data) const
{
  const std::int32_t size = sizeof(tgfxvertex);

  glBindBuffer(GL_ARRAY_BUFFER, fhandle);
  glBufferSubData(GL_ARRAY_BUFFER, static_cast<GLintptr>(size * offset),
                  static_cast<GLsizeiptr>(size * count), data);
}

/******************************************************************************/
/*                              TGfxIndexBuffer                               */
/******************************************************************************/

tgfxindexbuffer::tgfxindexbuffer(std::int32_t cap, bool _static, uint16_t *data)
{
  GLenum hint;

  fcapacity = cap;

  if (_static)
  {
    hint = GL_STATIC_DRAW;
  }
  else
  {
    hint = GL_STREAM_DRAW;
  }

  glGenBuffers(1, &fhandle);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fhandle);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint16_t) * cap, data, hint);
}

tgfxindexbuffer::~tgfxindexbuffer()
{
  if (fhandle != 0)
  {
    glDeleteBuffers(1, &fhandle);
  }
}

void tgfxindexbuffer::update(std::int32_t offset, std::int32_t count, uint16_t *data) const
{
  const std::int32_t size = sizeof(std::uint16_t);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fhandle);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLintptr>(size * offset),
                  static_cast<GLsizeiptr>(size * count), data);
}

/******************************************************************************/
/*                              TGfxSpritesheet                               */
/******************************************************************************/

const std::int32_t padding = 8;

using ttexturearray = std::vector<tgfxtexture *>;

using prectinfo = struct trectinfo *;
struct trectinfo
{
  std::int32_t image;
  std::int32_t frame;
  std::int32_t texture;
};

struct tsheetloaddata
{
  std::vector<tgfximage *> atlasimages;
  std::vector<tgfximage *> images;
  std::vector<tgfxcolor> imageskey;
  std::vector<std::string> imagespath;
  std::vector<tvector2> imagestargetsize;
  std::vector<float> imagestargetscale;
  std::int32_t imageindex;
  tbprectarray rects;
  std::vector<trectinfo> rectinfo;
  std::int32_t loadingstage;
  std::int32_t loadingindex;
  std::int32_t additionalframes;
};

tgfxspritesheet::tgfxspritesheet(std::int32_t count)
{
  tsheetloaddata *ld;

  setlength(fsprites, count);

  ld = new tsheetloaddata;
  floaddata = ld;

  ld->imageindex = 0;
  ld->loadingstage = 0;
  ld->loadingindex = 0;
  ld->additionalframes = 0;
  setlength(ld->images, count);
  setlength(ld->imageskey, count);
  setlength(ld->imagespath, count);
  setlength(ld->imagestargetsize, count);
  setlength(ld->imagestargetscale, count);
}

tgfxspritesheet::~tgfxspritesheet()
{
  std::int32_t i;

  cleanup();

  for (i = 0; i < ftextures.size(); i++)
  {
    if (ftextures[i] != nullptr)
    {
      freeandnullptr(ftextures[i]);
    }
  }
}

auto tgfxspritesheet::getsprite(std::int32_t index) -> pgfxsprite { return &fsprites[index]; }

auto tgfxspritesheet::gettexture(std::int32_t index) -> tgfxtexture * { return ftextures[index]; }

auto tgfxspritesheet::getspritecount() -> std::int32_t { return length(fsprites); }

auto tgfxspritesheet::gettexturecount() -> std::int32_t { return length(ftextures); }

auto tgfxspritesheet::isloading() -> bool
{
  tsheetloaddata *ld;

  bool result;
  ld = reinterpret_cast<tsheetloaddata *>(floaddata);

  if (ld == nullptr)
  {
    result = false;
  }
  else
  {
    result = ld->loadingstage > 0;
  }
  return result;
}

void tgfxspritesheet::addimage(const string &path, tgfxcolor colorkey, float targetscale)
{
  tsheetloaddata *ld;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);

  if (ld->imageindex < length(ld->images))
  {
    ld->imagespath[ld->imageindex] = path;
    ld->imageskey[ld->imageindex] = colorkey;
    ld->imagestargetsize[ld->imageindex].x = 0;
    ld->imagestargetsize[ld->imageindex].y = 0;
    ld->imagestargetscale[ld->imageindex] = max(0.f, min(1.f, targetscale));
    ld->imageindex += 1;
  }
}

void tgfxspritesheet::addimage(const string &path, tgfxcolor colorkey, tvector2 targetsize)
{
  tsheetloaddata *ld;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);

  if (ld->imageindex < length(ld->images))
  {
    ld->imagespath[ld->imageindex] = path;
    ld->imageskey[ld->imageindex] = colorkey;
    ld->imagestargetsize[ld->imageindex].x = targetsize.x;
    ld->imagestargetsize[ld->imageindex].y = targetsize.y;
    ld->imagestargetscale[ld->imageindex] = 0;
    ld->imageindex += 1;
  }
}

void tgfxspritesheet::addimage(tgfximage *image)
{
  tsheetloaddata *ld;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);

  if (ld->imageindex < length(ld->images))
  {
    ld->images[ld->imageindex] = image;
    ld->imagestargetsize[ld->imageindex].x = 0;
    ld->imagestargetsize[ld->imageindex].y = 0;
    ld->imagestargetscale[ld->imageindex] = 1;
    ld->imageindex += 1;
  }
}

void tgfxspritesheet::load()
{
  this->startloading();
  this->finishloading();
}

void tgfxspritesheet::startloading()
{
  tsheetloaddata *ld;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);

  if (ld->loadingstage == 0)
  {
    ld->loadingstage = 1;
  }
}

void tgfxspritesheet::continueloading()
{
  tsheetloaddata *ld;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);

  switch (ld->loadingstage)
  {
  case 1:
    this->loadnextimage();
    break;
  case 2:
    this->packrects();
    break;
  case 3:
    this->updatenextsprite();
    break;
  case 4:
    this->updatetexture();
    break;
  }
}

void tgfxspritesheet::finishloading()
{
  while (this->isloading())
  {
    continueloading();
  }
}

void gettargetdimensions(tgfximage &image, float scale, std::int32_t &w, std::int32_t &h)
{
  w = image.width();
  h = image.height();

  if (scale < 1)
  {
    h = max(1.f, round(image.height() * scale));
    w = round(image.width() * ((float)(h) / image.height()));

    if (w == 0)
    {
      w = 1;
      h = round((float)(image.height()) / image.width());
    }
  }

  if ((w > gfxcontext.maxtexturesize) || (h > gfxcontext.maxtexturesize))
  {
    if (w > h)
    {
      w = gfxcontext.maxtexturesize;
      h = max(1.f, round(image.height() * ((float)(w) / image.width())));
    }
    else
    {
      h = gfxcontext.maxtexturesize;
      w = max(1.f, round(image.width() * ((float)(h) / image.height())));
    }
  }
}

void tgfxspritesheet::loadnextimage()
{
  std::int32_t i;
  std::int32_t j;
  std::int32_t k;
  std::int32_t w = 0;
  std::int32_t h = 0;
  pgfxsprite sprite;
  tsheetloaddata *ld;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);
  i = ld->loadingindex;

  if (ld->images[i] == nullptr)
  {
    ld->images[i] = new tgfximage(ld->imagespath[i], ld->imageskey[i]);
    ld->images[i]->premultiply();

    if (ld->images[i]->getimagedata() == nullptr)
    {
      freeandnullptr(ld->images[i]);
      ld->images[i] = new tgfximage(16, 16);
      ld->imagestargetsize[i].x = 0;
      ld->imagestargetsize[i].y = 0;
      ld->imagestargetscale[i] = 1;
      fillchar(ld->images[i]->getimagedata(0), static_cast<std::size_t>(16 * 16 * 4), 255);
    }

    if (ld->imagestargetscale[i] == 0)
    {
      ld->imagestargetscale[i] = max((float)(ld->imagestargetsize[i].x) / ld->images[i]->width(),
                                     (float)(ld->imagestargetsize[i].y) / ld->images[i]->height());
    }

    gettargetdimensions(*ld->images[i], ld->imagestargetscale[i], w, h);

    if ((w != ld->images[i]->width()) || (h != ld->images[i]->height()))
    {
      ld->imagestargetscale[i] = (float)(h) / ld->images[i]->height();
      ld->images[i]->resize(w, h);
    }
    else
    {
      ld->imagestargetscale[i] = 1;
    }

    ld->additionalframes += ld->images[i]->numframes() - 1;
  }

  ld->loadingindex += 1;

  if (ld->loadingindex == length(ld->images))
  {
    if (ld->additionalframes > 0)
    {
      k = 0;

      setlength(fadditionalsprites, ld->additionalframes);
      fillchar(fadditionalsprites[0], sizeof(tgfxsprite) * ld->additionalframes, 0);

      for (i = 0; i < ld->images.size(); i++)
      {
        sprite = &fsprites[i];

        for (j = 0; j <= ld->images[i]->numframes() - 1; j++)
        {
          sprite->next = fadditionalsprites[k];
          sprite = sprite->next;
          k += 1;
        }
      }
    }

    ld->loadingstage += 1;
  }
}

void packrectsrecursive(tbprectarray &rects, ttexturearray &textures)
{
  std::int32_t i;
  std::int32_t n;
  std::int32_t w;
  std::int32_t h;
  std::int64_t a;
  std::int64_t aa;
  tbprectarray rects1;
  tbprectarray rects2;

  if (length(rects) == 1)
  {
    n = length(textures);
    setlength(textures, n + 1);
    w = npot(rects[0].w - padding);
    h = npot(rects[0].h - padding);
    textures[n] = gfxcreatetexture(w, h, 4, nullptr, "bin_pack_texture");
    ((prectinfo)(rects[0].data))->texture = n;
    return;
  }

  a = 0;

  for (i = 0; i < rects.size(); i++)
  {
    a = a + std::int64_t(rects[i].w) * std::int64_t(rects[i].h);
  }

  w = npot(round(ceil(sqrt(a + 0.0))));
  h = w;

  while ((w <= gfxcontext.maxtexturesize) && (h <= gfxcontext.maxtexturesize) &&
         (packrects(w + padding, h + padding, rects) != length(rects)))
  {
    h = h << (std::int32_t)(h < w);
    w = w << (std::int32_t)(w <= h);
  }

  if ((w <= gfxcontext.maxtexturesize) && (h <= gfxcontext.maxtexturesize))
  {
    n = length(textures);
    setlength(textures, n + 1);
    textures[n] = gfxcreatetexture(w, h, 4, nullptr, "bin_pack_texture");

    for (i = 0; i < rects.size(); i++)
    {
      ((prectinfo)(rects[i].data))->texture = n;
    }
  }
  else
  {
    i = 0;
    a = a / 2;
    aa = 0;

    while ((aa < a) && (i < length(rects) - 1))
    {
      aa = aa + std::int64_t(rects[i].w) * std::int64_t(rects[i].h);
      i += 1;
    }

    setlength(rects1, i);
    setlength(rects2, length(rects) - i);

    std::memcpy(rects1.data(), rects.data(), sizeof(tbprect) * length(rects1));
    std::memcpy(rects2.data(), &rects[i], sizeof(tbprect) * length(rects2));

    packrectsrecursive(rects1, textures);
    packrectsrecursive(rects2, textures);

    std::memcpy(rects.data(), rects1.data(), sizeof(tbprect) * length(rects1));
    std::memcpy(&rects[i], rects2.data(), sizeof(tbprect) * length(rects2));
  }
}

void tgfxspritesheet::packrects()
{
  std::int32_t i;
  std::int32_t j;
  std::int32_t k;
  std::int32_t n;
  ttexturearray textures;
  tsheetloaddata *ld;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);
  n = length(fsprites) + length(fadditionalsprites);
  k = 0;

  setlength(ld->rects, n);
  setlength(ld->rectinfo, n);

  for (i = 0; i < ld->images.size(); i++)
  {
    SoldatAssert(ld->images[i]->numframes() == 1);
    for (j = 0; j <= ld->images[i]->numframes() - 1; j++)
    {
      SoldatAssert(k < n);

      ld->rectinfo[k].image = i;
      ld->rectinfo[k].frame = j;
      ld->rectinfo[k].texture = 0;

      ld->rects[k].w = ld->images[i]->width() + padding;
      ld->rects[k].h = ld->images[i]->height() + padding;
      ld->rects[k].data = reinterpret_cast<std::uint8_t *>(&ld->rectinfo[k]);

      k += 1;
    }
  }

  packrectsrecursive(ld->rects, textures);
  setlength(ftextures, length(textures));
  setlength(ld->atlasimages, length(textures));

  for (i = 0; i < textures.size(); i++)
  {
    ftextures[i] = textures[i];
    ld->atlasimages[i] = new tgfximage(textures[i]->width(), textures[i]->height());
  }

  ld->loadingindex = 0;
  ld->loadingstage += 1;
}

void tgfxspritesheet::updatenextsprite()
{
  std::int32_t x;
  std::int32_t y;
  std::int32_t w;
  std::int32_t h;
  pgfxsprite sprite;
  tsheetloaddata *ld;
  trectinfo *info;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);
  info = reinterpret_cast<trectinfo *>(ld->rects[ld->loadingindex].data);

  x = ld->rects[ld->loadingindex].x;
  y = ld->rects[ld->loadingindex].y;
  w = ld->rects[ld->loadingindex].w - padding;
  h = ld->rects[ld->loadingindex].h - padding;

  ld->atlasimages[info->texture]->update(x, y, w, h,
                                         ld->images[info->image]->getimagedata(info->frame));

  sprite = &fsprites[info->image];

  if (info->frame > 0)
  {
    sprite = sprite->next;
    sprite += info->frame - 1;
  }

  sprite->texture = ftextures[info->texture];
  sprite->delay = ld->images[info->image]->getframedelay(info->frame);

  sprite->x = x;
  sprite->y = y;
  sprite->width = w;
  sprite->height = h;
  sprite->scale = (float)(1) / ld->imagestargetscale[info->image];

  sprite->texcoords.left = (float)(sprite->x) / sprite->texture->width();
  sprite->texcoords.top = (float)(sprite->y) / sprite->texture->height();
  sprite->texcoords.right = (float)((sprite->x + sprite->width)) / sprite->texture->width();
  sprite->texcoords.bottom = (float)((sprite->y + sprite->height)) / sprite->texture->height();

  ld->loadingindex += 1;

  if (ld->loadingindex == length(ld->rects))
  {
    ld->loadingstage += 1;
  }
}

void tgfxspritesheet::updatetexture()
{
  tsheetloaddata *ld;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);
  for (auto i = 0u; i < ld->atlasimages.size(); i++)
  {
    gfxupdatetexture(ftextures[i], 0, 0, ld->atlasimages[i]->width(), ld->atlasimages[i]->height(),
                     ld->atlasimages[i]->getimagedata());
  }
  this->cleanup();
}

void tgfxspritesheet::cleanup()
{
  tsheetloaddata *ld;

  ld = reinterpret_cast<tsheetloaddata *>(floaddata);
  floaddata = nullptr;

  if (ld != nullptr)
  {
    for (auto *atlas : ld->atlasimages)
    {
      freeandnullptr(atlas);
    }
    for (auto *image : ld->images)
    {
      freeandnullptr(image);
    }
    delete ld;
  }
}

void gfxlog(const std::string &s)
{
  LogDebugG("[GFX] {}", s);
}
