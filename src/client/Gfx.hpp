#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "common/Vector.hpp"

const std::int32_t gfx_monochrome = 1;

enum tgfxtexturewrap
{
  gfx_clamp,
  gfx_repeat,
  last_tgfxtexturewrap
};
enum tgfxverticalalign
{
  gfx_top,
  gfx_baseline,
  gfx_bottom,
  last_tgfxverticalalign
};

enum tgfxtexturefilter
{
  gfx_linear,
  gfx_nearest,
  gfx_mipmap_linear,
  gfx_mipmap_nearest,
  last_tgfxtexturefilter
};

typedef std::array<float, 9> tgfxmat3;
typedef tgfxmat3 *pgfxmat3;

typedef struct tgfxcolor *pgfxcolor;
struct tgfxcolor
{
  union {
    struct
    {
      std::uint8_t r, g, b, a;
    } color;
    std::uint32_t rgba;
  };
};

typedef struct tgfxvertex *pgfxvertex;
struct tgfxvertex
{
  float x, y;
  float u, v;
  tgfxcolor color;
};

typedef struct tgfxrect *pgfxrect;
struct tgfxrect
{
  MyFloat left = 0.0f, right = 0.0f;
  MyFloat top = 0.0f, bottom = 0.0f;
  float width()
  {
    return std::abs(right - left);
  }
  float height()
  {
    return std::abs(top - bottom);
  }
};

struct trect
{
  std::int32_t left = 0, top = 0, right = 0, bottom = 0;
  trect()
  {
  }
  trect(std::int32_t left, std::int32_t top, std::int32_t right, std::int32_t bottom)
    : left(left), top(top), right(right), bottom(bottom)
  {
  }

  std::int32_t width()
  {
    return std::abs(right - left);
  }
  std::int32_t height()
  {
    return std::abs(top - bottom);
  }
};

typedef void *tgfxfont;

class tgfximage
{
private:
  std::uint8_t *fdata = nullptr;
  std::int32_t fwidth = 0;
  std::int32_t fheight = 0;
  std::int32_t fcomponents = 0;
  std::int32_t fnumframes = 0;
  bool floadedfromfile = false;

public:
  tgfximage(const std::string &filename, tgfxcolor colorkey);
  tgfximage(std::int32_t width, std::int32_t height, std::int32_t comp = 4);
  ~tgfximage();
  std::uint8_t *getimagedata(std::int32_t frame = 0);
  std::uint64_t getframedelay(std::int32_t frame = 0);
  void update(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint8_t *data,
              std::int32_t frame = 0);
  void premultiply();
  void resize(std::int32_t w, std::int32_t h);
  std::int32_t width()
  {
    return fwidth;
  };
  std::int32_t height()
  {
    return fheight;
  };
  std::int32_t components()
  {
    return fcomponents;
  }
  std::int32_t numframes()
  {
    return fnumframes;
  }
};

class tgfxtexture
{
public:
  std::uint32_t fhandle;
  std::uint32_t ffbohandle;
  std::int32_t fwidth;
  std::int32_t fheight;
  std::int32_t fcomponents;
  std::int32_t fsamples;
  SDL_GPUTexture* mTexture;
  SDL_GPUFilter mMinFilter = SDL_GPU_FILTER_LINEAR;
  SDL_GPUFilter mMagFilter = SDL_GPU_FILTER_LINEAR;
  struct
  {
    std::int32_t x, y;
    tgfxcolor color;
  } fpixel;
  void update(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h, std::uint8_t *data);
  void setwrap(tgfxtexturewrap s, tgfxtexturewrap t);
  void setfilter(tgfxtexturefilter min, tgfxtexturefilter mag);

public:
  tgfxtexture(std::int32_t width, std::int32_t height, std::int32_t comp, bool rt, bool msaa,
              std::uint8_t *data, const std::string_view &debug_name);
  ~tgfxtexture();
  std::int32_t width()
  {
    return fwidth;
  }
  std::int32_t height()
  {
    return fheight;
  }
  std::int32_t components()
  {
    return fcomponents;
  }
  std::int32_t samples()
  {
    return fsamples;
  }

  std::int32_t handle()
  {
    return fhandle;
  }

  SDL_GPUTexture* getTexture()
  {
    return mTexture;
  }
};

class tgfxvertexbuffer
{
private:
  std::uint32_t fhandle;
  std::int32_t fcapacity;
  SDL_GPUBuffer * mBuffer;

public:
  tgfxvertexbuffer(std::int32_t cap, bool _static, pgfxvertex data);
  ~tgfxvertexbuffer();
  void update(std::int32_t offset, std::int32_t count, pgfxvertex data) const;
  std::int32_t capacity()
  {
    return fcapacity;
  };

  std::int32_t handle()
  {
    return fhandle;
  }

  SDL_GPUBuffer * getBuffer()
  {
    return mBuffer;
  }
};

class tgfxindexbuffer
{
private:
  std::uint32_t fhandle;
  std::int32_t fcapacity;
  SDL_GPUBuffer * mBuffer;

public:
  void update(std::int32_t offset, std::int32_t count, std::uint16_t *data) const;
  tgfxindexbuffer(std::int32_t cap, bool _static, std::uint16_t *data);
  ~tgfxindexbuffer();
  std::int32_t capacity()
  {
    return fcapacity;
  }

  std::uint32_t handle()
  {
    return fhandle;
  }
  
  SDL_GPUBuffer * getBuffer()
  {
    return mBuffer;
  }
};

typedef struct tgfxsprite *pgfxsprite;
struct tgfxsprite
{
  std::int32_t x, y;
  std::int32_t width, height;
  float scale;
  std::int32_t delay;
  tgfxrect texcoords;
  tgfxtexture *texture;
  pgfxsprite next;
};

typedef pgfxsprite *tgfxspritearray;

class tgfxspritesheet
{
private:
  std::vector<tgfxtexture *> ftextures;
  std::vector<tgfxsprite> fsprites;
  std::vector<tgfxsprite *> fadditionalsprites;
  void *floaddata;
  std::int32_t getspritecount();
  std::int32_t gettexturecount();
  bool isloading();
  void loadnextimage();
  void packrects();
  void updatenextsprite();
  void updatetexture();
  void cleanup();

public:
  tgfxspritesheet(std::int32_t count);
  ~tgfxspritesheet();
  void addimage(const std::string &path, tgfxcolor colorkey, float targetscale);
  void addimage(const std::string &path, tgfxcolor colorkey, tvector2 targetsize);
  void addimage(tgfximage *image);
  void load();
  void startloading();
  void continueloading();
  void finishloading();
  std::int32_t spritecount()
  {
    return getspritecount();
  };
  std::int32_t texturecount()
  {
    return gettexturecount();
  };
  // property Sprites[Index: Integer]: PGfxSprite read GetSprite; default;
  // property Texture[Index: Integer]: TGfxTexture read GetTexture;
  pgfxsprite getsprite(std::int32_t index);
  tgfxtexture *gettexture(std::int32_t index);
  bool loading()
  {
    return isloading();
  }
};

typedef struct tgfxdrawcommand *pgfxdrawcommand;
struct tgfxdrawcommand
{
  tgfxtexture *texture = nullptr;
  std::int32_t offset;
  std::int32_t count;
};

typedef struct SDL_Window SDL_Window;

// genera l
bool gfxframebuffersupported();
bool gfxinitcontext(SDL_Window *wnd, bool dithering, bool fixedpipeline);
void gfxdestroycontext();
void gfxpresent(bool finish);
void gfxtarget(tgfxtexture *rendertarget);
void gfxblit(tgfxtexture *src, tgfxtexture *dst, trect srcrect, trect dstrect,
             tgfxtexturefilter filter);
void gfxclear(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
void gfxclear(tgfxcolor c);
void gfxdraw(tgfxvertexbuffer *buffer, std::int32_t offset, std::int32_t count);
void gfxdraw(tgfxvertexbuffer *buffer, tgfxindexbuffer *indexbuffer, std::int32_t offset,
             std::int32_t count);
void gfxdraw(tgfxvertexbuffer *buffer, pgfxdrawcommand cmds, std::int32_t cmdcount);
void gfxdraw(tgfxvertexbuffer *buffer, tgfxindexbuffer *indexbuffer, pgfxdrawcommand cmds,
             std::int32_t cmdcount);
void gfxviewport(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
void gfxtransform(const tgfxmat3 t);
void gfxspritevertices(pgfxsprite s, float x, float y, float w, float h, float sx, float sy,
                       float cx, float cy, float r, tgfxcolor color, pgfxvertex v);

std::unique_ptr<std::uint8_t[]> gfxsavescreen(std::int32_t x, std::int32_t y, std::int32_t w, std::int32_t h);
void gfxsavescreen(const std::string &filename, std::int32_t x, std::int32_t y, std::int32_t w,
                   std::int32_t h, bool async = true);
void gfxsetmipmapbias(float bias);

// pseudo constructors
tgfxcolor argb(std::uint32_t argb);
tgfxcolor rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a);
tgfxcolor rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b);
tgfxcolor rgba(std::uint32_t rgba);
// tgfxcolor rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, double a);
tgfxcolor rgba(std::uint32_t rgb, float a);
tgfxvertex gfxvertex(float x, float y, float u, float v, const tgfxcolor &c);

// texture
tgfxtexture *gfxcreatetexture(std::int32_t w, std::int32_t h, std::int32_t c /*= 4*/,
                              std::uint8_t *data /*= 0*/, const std::string_view &debug_name);
tgfxtexture *gfxcreaterendertarget(std::int32_t w, std::int32_t h, std::int32_t c = 4,
                                   bool msaa = false);
void gfxbindtexture(tgfxtexture *texture);
void gfxtexturewrap(tgfxtexture *texture, tgfxtexturewrap s, tgfxtexturewrap t);
void gfxtexturefilter(tgfxtexture *texture, tgfxtexturefilter min, tgfxtexturefilter mag);
void gfxupdatetexture(tgfxtexture *texture, std::int32_t x, std::int32_t y, std::int32_t w,
                      std::int32_t h, std::uint8_t *data);
void gfxgeneratemipmap(tgfxtexture *texture);
void gfxdeletetexture(tgfxtexture *&texture);

// vertex buffer
tgfxvertexbuffer *gfxcreatebuffer(std::int32_t capacity, bool _static = false,
                                  pgfxvertex data = nullptr);
void gfxupdatebuffer(tgfxvertexbuffer *b, std::int32_t i, std::int32_t n, pgfxvertex data);
void gfxdeletebuffer(tgfxvertexbuffer *b);

// index buffer
tgfxindexbuffer *gfxcreateindexbuffer(std::int32_t capacity, bool _static = false,
                                      std::uint16_t *data = nullptr);
void gfxupdateindexbuffer(tgfxindexbuffer &b, std::int32_t i, std::int32_t n, uint16_t *data);
void gfxdeleteindexbuffer(tgfxindexbuffer *b);

// fonts
tgfxfont gfxcreatefont(const std::string &filename, std::int32_t w = 512, std::int32_t h = 512);
void gfxdeletefont(tgfxfont font);
std::int32_t gfxsetfont(tgfxfont font, float fontsize, std::uint32_t flags, float stretch = 1);
void gfxsetfonttable(tgfxfont font, std::int32_t tableindex);
void gfxtextpixelratio(const tvector2 &pixelratio);
void gfxtextscale(float s);
void gfxtextcolor(const tgfxcolor &color);
void gfxtextshadow(float dx, float dy, const tgfxcolor &color);
void gfxtextverticalalign(tgfxverticalalign align);
tgfxrect gfxtextmetrics();
tgfxrect gfxtextmetrics(const std::string &text);
void gfxdrawtext(MyFloat x, MyFloat y);
void gfxdrawtext(const std::wstring &text, float x, float y);
void gfxdrawtext(const std::string &text, float x, float y);

// batching
void gfxbegin();
void gfxend();
template <typename Allocator = std::allocator<tgfxvertex>>
void gfxdrawquad(tgfxtexture *texture, const std::vector<tgfxvertex, Allocator> &vertices);
void gfxdrawquad(tgfxtexture *texture, const tgfxvertex &a, const tgfxvertex &b,
                 const tgfxvertex &c, const tgfxvertex &d);

void gfxdrawsprite(pgfxsprite s, float x, float y);
void gfxdrawsprite(pgfxsprite s, float x, float y, float scale);
void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy);
void gfxdrawsprite(pgfxsprite s, float x, float y, float rx, float ry, float r);
void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, float rx, float ry, float r);

void gfxdrawsprite(pgfxsprite s, float x, float y, const tgfxrect &rc);
void gfxdrawsprite(pgfxsprite s, float x, float y, float scale, const tgfxrect &rc);
void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, const tgfxrect &rc);
void gfxdrawsprite(pgfxsprite s, float x, float y, float rx, float ry, float r, const tgfxrect &rc);
void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, float rx, float ry, float r,
                   const tgfxrect &rc);

void gfxdrawsprite(pgfxsprite s, float x, float y, const tgfxcolor &color);
void gfxdrawsprite(pgfxsprite s, float x, float y, float scale, const tgfxcolor &color);
void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, const tgfxcolor &color);
void gfxdrawsprite(pgfxsprite s, float x, float y, float rx, float ry, float r,
                   const tgfxcolor &color);
void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, float rx, float ry, float r,
                   const tgfxcolor &color);

void gfxdrawsprite(pgfxsprite s, float x, float y, const tgfxcolor &color, const tgfxrect &rc);
void gfxdrawsprite(pgfxsprite s, float x, float y, float scale, const tgfxcolor &color,
                   const tgfxrect &rc);
void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, const tgfxcolor &color,
                   const tgfxrect &rc);
void gfxdrawsprite(pgfxsprite s, float x, float y, float rx, float ry, float r,
                   const tgfxcolor &color, const tgfxrect &rc);
void gfxdrawsprite(pgfxsprite s, float x, float y, float sx, float sy, float rx, float ry, float r,
                   const tgfxcolor &color, const tgfxrect &rc);

// matrix
tgfxmat3 gfxmat3rot(float r);
tgfxmat3 gfxmat3ortho(float l, float r, float t, float b);
tgfxmat3 gfxmat3transform(float tx, float ty, float sx, float sy, float cx, float cy, float r);
tvector2 gfxmat3mul(const tgfxmat3 &m, float x, float y);

// other
std::int32_t npot(std::uint32_t x); // next power of two
float rectwidth(const tgfxrect &rect);
float rectheight(const tgfxrect &rect);

extern void gfxlog(const std::string &s);
