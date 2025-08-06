// automatically converted

#include "GameRendering.hpp"
#include "Client.hpp"
#include "ClientGame.hpp"
#include "GostekGraphics.hpp"
#include "Input.hpp"
#include "InterfaceGraphics.hpp"
#include "MapGraphics.hpp"
#include "common/Logging.hpp"
#include "common/Util.hpp"
#include "common/Vector.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/TFileStream.hpp"
#include "common/misc/TIniFile.hpp"
#include "shared/Constants.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/Bullets.hpp"
#include "shared/mechanics/Sparks.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/mechanics/Things.hpp"
#include "shared/misc/GlobalSystems.hpp"

#include <SDL3/SDL.h>
#include <Tracy.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <set>
#include <string>

#include "common/gfx.hpp"

tgamerenderingparams gamerenderingparams;
tgfxspritearray textures;

using string = std::string;

struct ttextureloaddata
{
  std::int32_t id;
  std::int32_t group;
  std::string path;
  std::uint32_t colorkey;
};

struct tinterpolationstate
{
  tvector2 camera;
  tvector2 mouse;
  std::array<PascalArray<tvector2, 1, max_sprites>, 24> spritepos;
  PascalArray<tvector2, 1, max_bullets> bulletpos;
  PascalArray<tvector2, 1, max_bullets> bulletvel;
  PascalArray<float, 1, max_bullets> bullethitmul;
  PascalArray<tvector2, 1, max_sparks> sparkpos;
  PascalArray<PascalArray<tvector2, 1, 4>, 1, max_things> thingpos;
};

struct tfontstyle
{
  tgfxfont font;
  std::int32_t tableindex;
  float size;
  float stretch;
  std::uint32_t flags;
};

// struct GfxDataEntry
//{
//    constexpr GfxDataEntry(const std::int32_t ID, const std::int32_t Group,
//                           const std::string_view Path, const std::uint32_t ColorKey)
//        : ID{ID}, Group{Group}, Path{Path}, ColorKey{ColorKey}
//    {
//    }
//    const std::int32_t ID;
//    const std::int32_t Group;
//    const std::string_view Path;
//    const std::uint32_t ColorKey;
//};

//  GFXData: array[1..GFXID_END] of TTextureLoadData = (
//    {$DEFINE GFXDATA}
//    {$INCLUDE gfx.inc}
//    {$UNDEF GFXDATA}
//  );

bool initialized;
std::string loadedinterfacename;
tgfxspritesheet *mainspritesheet = nullptr;
tgfxspritesheet *interfacespritesheet = nullptr;
std::array<tgfxfont, 2> fonts;
PascalArray<tfontstyle, 0, font_last> fontstyles;
tgfxtexture *actionsnaptexture = nullptr;
tgfxtexture *rendertarget = nullptr;
tgfxtexture *rendertargetaa = nullptr;
std::string screenshotpath;
bool screenshotasync;
PascalArray<float, 1, GFX::END> imagescale;
static TIniFile::Entries gostekdata;
static struct
{
  TIniFile::Entries root;
  TIniFile::Entries currentmod;
  TIniFile::Entries custominterface;
} scaledata;

static SDL_Surface *IconSurface = nullptr;

void loadmodinfo()
{
  std::unique_ptr<TIniFile> rootini;
  std::unique_ptr<TIniFile> modini;
  std::unique_ptr<TIniFile> interfaceini;

  // load required ini's

  auto& fs = GS::GetFileSystem();

  auto rootinistream = ReadAsFileStream(fs, "/mod.ini");

  if (gostekdata.empty() or scaledata.root.empty())
  {
    rootini = std::make_unique<TIniFile>(std::move(rootinistream));
  }

  auto modinistream = ReadAsFileStream(fs, moddir + "mod.ini");

  if (gostekdata.empty() or scaledata.currentmod.empty())
  {
    modini = std::make_unique<TIniFile>(std::move(modinistream));
  }

  auto interfaceinistream = ReadAsFileStream(fs,
    (string("custom-interfaces/") + gamerenderingparams.interfacename + "/mod.ini"));

  if (interfaceinistream)
  {
    interfaceini = std::make_unique<TIniFile>(std::move(interfaceinistream));
  }

  // gostek

  if (gostekdata.empty())
  {
    rootini->ReadSectionValues("GOSTEK", gostekdata);

    TIniFile::Entries modgostek;
    modini->ReadSectionValues("GOSTEK", modgostek);
    for (const auto &entry : modgostek)
    {
      const auto &key = entry.first;
      if (!key.empty())
      {
        gostekdata[key] = entry.second;
      }
    }
    loadgostekdata(gostekdata);
  }

  // scale

  if (scaledata.root.empty())
  {
    rootini->ReadSectionValues("SCALE", scaledata.root);
  }

  if (scaledata.currentmod.empty())
  {
    modini->ReadSectionValues("SCALE", scaledata.currentmod);
  }

  if (interfaceini)
  {
    interfaceini->ReadSectionValues("SCALE", scaledata.custominterface);
  }

  // cleanup
}

auto getimagescale(const std::string &imagepath) -> float
{
  std::string scale;
  std::string key;

  std::string intdir =
    string("/custom-interfaces/") + lowercase(gamerenderingparams.interfacename) + '/';

  TIniFile::Entries *data = &scaledata.root;
  std::string path = lowercase(imagepath);
  NotImplemented("rendering");
#if 0
    if (((moddir != "") && (lowercase(moddir) == path.substr(1, length(moddir)))))
    {
        data = &scaledata.currentmod;
        path = copy(path, length(moddir) + 1, length(path));
    }
    else if (copy(path, 1, length(intdir)) == intdir)
    {
        data = &scaledata.custominterface;
        path = copy(path, length(intdir) + 1, length(path));
    }
#endif

  std::replace(path.begin(), path.end(), '\\', '/');
  key = path;
  if (const auto it = data->find(key); it != data->end())
  {
    scale = it->second;
  }

  if (scale.empty())
  {
    NotImplemented("rendering");
#if 0
        key = stringreplace(extractfiledir(path), '\\', '/', set::of(rfreplaceall, eos));
        scale = data->at(key);
#endif

    if (scale.empty())
    {
      scale = data->at("DefaultScale");
    }
  }

  return strtofloatdef(scale, 1);
}

void takescreenshot(string filename, bool async)
{
  screenshotpath = filename;
  screenshotasync = async;
}

auto pngoverride(const std::string_view &filename) -> string
{
  std::string f{filename};
  std::replace(f.begin(), f.end(), '\\', '/');

  return overridefileext(GS::GetFileSystem(), f, ".png");
}

auto pngoverride(const std::string &filename) -> string
{
  std::string f{filename};
  std::replace(f.begin(), f.end(), '\\', '/');
  return overridefileext(GS::GetFileSystem(), f, ".png");
}

void loadmaintextures()
{
  auto &fs = GS::GetFileSystem();

  const auto count = std::count_if(GFXData.begin(), GFXData.end(),
                        [](const auto &d) { return d.Group != GFXG::INTERFACE; });

  mainspritesheet = new tgfxspritesheet(count);
  float scale = 1.5 * renderheight / gameheight;

  for (const auto &i : GFXData)
  {
    if (i.Group != GFXG::INTERFACE)
    {
      tgfxcolor color;
      auto id = i.ID;
      color.color.r = (i.ColorKey & 0xff) >> 0;
      color.color.g = (i.ColorKey & 0xff00) >> 8;
      color.color.b = (i.ColorKey & 0xff0000) >> 16;
      color.color.a = (i.ColorKey & 0xff000000) >> 24;

      auto path = pngoverride(moddir + std::string(i.Path));

      if (!fs.Exists(path))
      {
        path = pngoverride(i.Path);
      }

      imagescale[id] = getimagescale(path);
      SoldatAssert(!path.empty());

      if (CVar::r_optimizetextures)
      {
        mainspritesheet->addimage(path, color, scale / imagescale[id]);
      }
      else
      {
        mainspritesheet->addimage(path, color, 1);
      }
    }
  }

  mainspritesheet->startloading();
}

void loadinterfacetextures(const std::string interfacename)
{
  const std::int32_t custom_first = GFX::INTERFACE_CURSOR;
  const std::int32_t custom_last = GFX::INTERFACE_TITLE_R;
  std::int32_t i;
  std::int32_t count = 0;
  std::int32_t cutlength = 0;
  std::string prefix;
  std::string path;
  tgfxcolor color;
  float scale;
  bool iscustom = !isdefaultinterface(interfacename);
  auto& fs = GS::GetFileSystem();

  if (iscustom)
  {
    cutlength = length(std::string("interface-gfx/"));
    prefix = moddir + "custom-interfaces/" + interfacename + '/';
  }

  for (i = low(GFXData); i <= high(GFXData); i++)
  {
    if (GFXData[i].Group == GFXG::INTERFACE)
    {
      count += 1;
    }
  }

  if (interfacespritesheet != nullptr)
  {
    freeandnullptr(interfacespritesheet);
  }

  interfacespritesheet = new tgfxspritesheet(count);

  if (CVar::r_scaleinterface)
  {
    scale = (float)(renderheight) / gameheight;
  }
  else
  {
    scale = 1;
  }

  for (i = low(GFXData); i <= high(GFXData); i++)
  {
    if (GFXData[i].Group == GFXG::INTERFACE)
    {
      auto id = GFXData[i].ID;
      color.color.r = (GFXData[i].ColorKey & 0xff) >> 0;
      color.color.g = (GFXData[i].ColorKey & 0xff00) >> 8;
      color.color.b = (GFXData[i].ColorKey & 0xff0000) >> 16;
      color.color.a = (GFXData[i].ColorKey & 0xff000000) >> 24;

      if (iscustom && (i >= custom_first) && (i <= custom_last))
      {
        path = prefix + std::string(GFXData[i].Path.data() + cutlength + 1);
        path = pngoverride(path);

        if (!fs.Exists(path))
        {
          path = pngoverride(GFXData[i].Path);
        }
      }
      else
      {
        path = pngoverride(moddir + std::string(GFXData[i].Path));

        if (!fs.Exists(path))
        {
          path = pngoverride(GFXData[i].Path);
        }
      }

      imagescale[id] = getimagescale(path);

      if (CVar::r_optimizetextures)
      {
        interfacespritesheet->addimage(path, color, scale / imagescale[id]);
      }
      else
      {
        interfacespritesheet->addimage(path, color, 1);
      }
    }
  }

  interfacespritesheet->startloading();
}

void loadinterface()
{
  if (loadinterfacedata(gamerenderingparams.interfacename))
  {
    loadinterfacetextures(gamerenderingparams.interfacename);
  }
  else
  {
    loadinterfacetextures("");
  }

  loadedinterfacename = gamerenderingparams.interfacename;
}

auto getfontpath(string fontfile) -> string
{
  std::string result;
  auto p = std::filesystem::path(basedirectory + fontfile);
  if (std::filesystem::exists(p) && !std::filesystem::is_directory(p))
  {
    result = basedirectory + fontfile;
  }
  return result;
}

auto getfontpath(string fallback, string &fontfile) -> string
{
  std::string result;
  if (fontfile == "play-regular.ttf")
  {
    return "play-regular.ttf";
  }

  if (!fontfile.empty())
  {
    result = getfontpath(fontfile);
  }

  if (result.empty())
  {
    result = getfontpath(fontfile);
  }

  if (result.empty())
  {
    fontfile = fallback;
    result = getfontpath(fontfile);
  }
  return result;
}

void loadfonts()
{
  std::array<string, 2> fontfile;
  std::array<string, 2> fontpath;

  fontfile[0] = CVar::font_1_filename;
  fontfile[1] = CVar::font_2_filename;

  fontpath[0] = getfontpath(default_font, fontfile[0]);
  fontpath[1] = getfontpath(default_font, fontfile[1]);

  if ((fontpath[0].empty()) || (fontpath[1].empty()))
  {
    showmessage(("One of the fonts cannot be found. Please check your installation directory."));
    shutdown();
  }

  const std::int32_t w = renderwidth;
  const std::int32_t h = renderheight;
  const float s = iif(CVar::r_scaleinterface, (float)(renderheight) / gameheight, 1.0f);

  fonts[0] = gfxcreatefont(fontpath[0], npot(w / 2), npot(h / 2));
  fonts[1] = gfxcreatefont(fontpath[1], npot(w / 3), npot(h / 3));

  fontstyles[font_small].font = fonts[1];
  fontstyles[font_small].size = s * CVar::font_consolesize;
  fontstyles[font_small].stretch = (float)(CVar::font_2_scale) / 100;
  fontstyles[font_small].flags = 0;

  // bold not supported for now so same as FONT_SMALL
  fontstyles[font_small_bold].font = fonts[1];
  fontstyles[font_small_bold].size = s * CVar::font_consolesize;
  fontstyles[font_small_bold].stretch = (float)(CVar::font_2_scale) / 100;
  fontstyles[font_small_bold].flags = 0;

  fontstyles[font_smallest].font = fonts[1];
  fontstyles[font_smallest].size = s * CVar::font_consolesmallsize;
  fontstyles[font_smallest].stretch = (float)(CVar::font_2_scale) / 100;
  fontstyles[font_smallest].flags = 0;

  fontstyles[font_big].font = fonts[1];
  fontstyles[font_big].size = CVar::font_bigsize;
  fontstyles[font_big].stretch = (float)(CVar::font_1_scale) / 100;
  fontstyles[font_big].flags = 0;

  fontstyles[font_menu].font = fonts[1];
  fontstyles[font_menu].size = s * CVar::font_menusize;
  fontstyles[font_menu].stretch = (float)(CVar::font_1_scale) / 100;
  fontstyles[font_menu].flags = 0;

  fontstyles[font_weapons_menu].font = fonts[1];
  fontstyles[font_weapons_menu].size = s * CVar::font_weaponmenusize;
  ;
  fontstyles[font_weapons_menu].stretch = (float)(CVar::font_2_scale) / 100;
  fontstyles[font_weapons_menu].flags = 0;

  fontstyles[font_world].font = fonts[1];
  fontstyles[font_world].size = 128 * ((float)(renderheight) / gameheight);
  fontstyles[font_world].stretch = (float)(CVar::font_1_scale) / 100;
  fontstyles[font_world].flags = 0;

  for (std::int32_t i = 0; i < fontstyles.size(); i++)
  {
    if (fontstyles[i].size < 10)
    {
      fontstyles[i].flags = gfx_monochrome;
    }

    fontstyles[i].tableindex = gfxsetfont(fontstyles[i].font, fontstyles[i].size,
                                          fontstyles[i].flags, fontstyles[i].stretch);
  }
}

auto initgamegraphics() -> bool
{
  std::vector<std::uint8_t> filebuffer;

  bool result = true;

  if (initialized)
  {
    if (gamerenderingparams.interfacename != loadedinterfacename)
    {
      scaledata.custominterface.clear();
      loadmodinfo();
      loadinterface();
    }

    return result;
  }

  //std::uint32_t windowflags = SDL_WINDOW_OPENGL;
  std::uint32_t windowflags = SDL_WINDOW_VULKAN;

  if (CVar::r_fullscreen == 2)
  {
    NotImplemented("sdl2_port");
    //windowflags = windowflags | SDL_WINDOW_FULLSCREEN_DESKTOP;
  }
  else if (CVar::r_fullscreen == 1)
  {
    windowflags = windowflags | SDL_WINDOW_FULLSCREEN;
  }
  gamewindow = SDL_CreateWindow("Soldat", windowwidth, windowheight, windowflags);

  auto& fs = GS::GetFileSystem();
  filebuffer = fs.ReadFile("/icon.bmp");

  SDL_IOStream *iconfile = SDL_IOFromMem(filebuffer.data(), length(filebuffer));

  IconSurface = SDL_LoadBMP_IO(iconfile, 1);

  SDL_SetWindowIcon(gamewindow, IconSurface);

  if (gamewindow == nullptr)
  {
    showmessage("Error creating sdl3 window");
    result = false;
    return result;
  }

  if (!gfxinitcontext(gamewindow, CVar::r_dithering, CVar::r_compatibility))
  {
    result = false;
    return result;
  }

  startinput();

  if (SDL_GL_SetSwapInterval(CVar::r_swapeffect) == false)
  {
    gfxlog(string("Error while setting SDL_GL_SetSwapInterval:") + SDL_GetError());
  }

  gfxviewport(0, 0, windowwidth, windowheight);

  textures = new pgfxsprite[GFX::END + 1];
  loadmodinfo();
  loadmaintextures();
  loadinterface();
  loadfonts();

  auto &map = GS::GetGame().GetMap();

  map.loadgraphics = &loadmapgraphics;
  if (!gfxframebuffersupported())
  {
    CVar::cl_actionsnap = false;
  }

  if (CVar::cl_actionsnap)
  {
    actionsnaptexture = gfxcreaterendertarget(renderwidth, renderheight, 4, true);
  }

  if (gfxframebuffersupported())
  {
    if ((windowwidth != renderwidth) || (windowheight != renderheight))
    {
      rendertarget = gfxcreaterendertarget(renderwidth, renderheight, 4, true);

      if (rendertarget->samples() > 0)
      {
        rendertargetaa = gfxcreaterendertarget(renderwidth, renderheight, 4, false);

        if (CVar::r_resizefilter >= 2)
        {
          gfxtexturefilter(rendertargetaa, gfx_linear, gfx_linear);
        }
        else
        {
          gfxtexturefilter(rendertargetaa, gfx_nearest, gfx_nearest);
        }
      }
      else
      {
        if (CVar::r_resizefilter >= 2)
        {
          gfxtexturefilter(rendertarget, gfx_linear, gfx_linear);
        }
        else
        {
          gfxtexturefilter(rendertarget, gfx_nearest, gfx_nearest);
        }
      }
    }
  }

  initialized = true;
  return result;
}

void reloadgraphics()
{
  tmapfile mapfile;
  tmapinfo mapinfo;
  bool bgforce;
  std::array<tmapcolor, 2> color;
  tmapgraphics *mg;

  mg = &mapgfx;
  mapinfo = mg->mapinfo;
  bgforce = mg->bgforce;
  color[0] = mg->bgforcedcolor[0];
  color[1] = mg->bgforcedcolor[1];

  freeandnullptr(mainspritesheet);
  freeandnullptr(interfacespritesheet);
  destroymapgraphics();

  gostekdata.clear();
  scaledata.root.clear();
  scaledata.currentmod.clear();
  scaledata.custominterface.clear();

  loadmodinfo();
  loadmaintextures();
  loadinterface();
  dotextureloading(true);

  loadmapfile(GS::GetFileSystem(), mapinfo, mapfile);
  loadmapgraphics(mapfile, bgforce, color[0], color[1]);
}

void destroygamegraphics()
{
  std::int32_t i;

  if (!initialized)
  {
    return;
  }

  SDL_SetWindowIcon(gamewindow, nullptr);
  SDL_DestroySurface(IconSurface);
  IconSurface = nullptr;

  freeandnullptr(mainspritesheet);
  freeandnullptr(interfacespritesheet);

  for (i = low(fonts); i <= high(fonts); i++)
  {
    gfxdeletefont(fonts[i]);
  }

  if (actionsnaptexture != nullptr)
  {
    gfxdeletetexture(actionsnaptexture);
  }

  if (rendertarget != nullptr)
  {
    gfxdeletetexture(rendertarget);
  }

  if (rendertargetaa != nullptr)
  {
    gfxdeletetexture(rendertargetaa);
  }

  destroymapgraphics();
  gfxdestroycontext();

  initialized = false;
}


#if __EMSCRIPTEN__
float lerp(float a, float b, float x)
{
  return a + (b - a) * x;
}

#else
constexpr float lerp(float a, float b, float x)
{
  return a + (b - a) * x;
}

#endif

constexpr auto lerp(const tvector2 &a, const tvector2 &b, float x) -> tvector2
{
  tvector2 lerp_result;
  lerp_result.x = a.x + (b.x - a.x) * x;
  lerp_result.y = a.y + (b.y - a.y) * x;
  return lerp_result;
}

void interpolatestate(float p, tinterpolationstate &s, bool paused)
{
  ZoneScopedN("InterpolateState");
  static const std::set<std::int32_t> kit_styles = {
    object_medical_kit, object_grenade_kit, object_flamer_kit, object_predator_kit,
    object_vest_kit,    object_berserk_kit, object_cluster_kit};
  std::int32_t i;
  std::int32_t j;

  s.camera.x = camerax;
  s.camera.y = cameray;
  s.mouse.x = mx;
  s.mouse.y = my;

  camerax = lerp(cameraprev.x, camerax, p);
  cameray = lerp(cameraprev.y, cameray, p);
  mx = lerp(mouseprev.x, mx, p);
  my = lerp(mouseprev.y, my, p);

  if (paused)
  {
    p = 1.0;
  }

  {
    ZoneScopedN("Sprites");
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      i = sprite.num;
      auto *const sk = &sprite.skeleton;
      tgun *gun = &sprite.weapon;
      std::memcpy(&s.spritepos[i][1], &sk->pos[1], sizeof(tvector2) * length(s.spritepos[i]));

      {
        ZoneScopedN("LerpSpritePos");
        for (j = low(s.spritepos[i]); j <= high(s.spritepos[i]); j++)
        {
          sk->pos[j] = lerp(sk->oldpos[j], sk->pos[j], p);
        }
      }

      gun->reloadtimefloat = lerp(gun->reloadtimeprev, gun->reloadtimecount, p);
      gun->fireintervalfloat = lerp(gun->fireintervalprev, gun->fireintervalcount, p);
      sprite.jetscountfloat = lerp(sprite.jetscountprev, sprite.jetscount, p);
    }
  }

  {
    ZoneScopedN("Bullets");
    for (i = 1; i <= max_bullets; i++)
    {
      auto &b = GS::GetBulletSystem().GetBullets()[i];
      if (b.active or (b.pingadd > 0))
      {
        j = b.num;

        s.bulletpos[i] = GetBulletParts().pos[j];
        s.bulletvel[i] = GetBulletParts().velocity[j];
        s.bullethitmul[i] = b.hitmultiply;

        GetBulletParts().pos[j] = lerp(GetBulletParts().oldpos[j], GetBulletParts().pos[j], p);
        GetBulletParts().velocity[j] = lerp(b.velocityprev, GetBulletParts().velocity[j], p);
        b.hitmultiply = lerp(b.hitmultiplyprev, b.hitmultiply, p);
        b.timeoutfloat = lerp(b.timeoutprev, b.timeout, p);
      }
    }
  }

  {
    ZoneScopedN("Sparks");
    for (i = 1; i <= max_sparks; i++)
    {
      if (spark[i].active)
      {
        j = spark[i].num;
        s.sparkpos[i] = GetSparkParts().pos[j];
        GetSparkParts().pos[j] = lerp(GetSparkParts().oldpos[j], GetSparkParts().pos[j], p);
        spark[i].lifefloat = lerp(spark[i].lifeprev, spark[i].life, p);
      }
    }
  }

  {
    ZoneScopedN("Things");
    auto &things = GS::GetThingSystem().GetThings();
    for (i = 1; i <= max_things; i++)
    {
      if (things[i].active)
      {
        auto *const sk = &things[i].skeleton;
        std::memcpy(&s.thingpos[i][1], &sk->pos[1], sizeof(tvector2) * length(s.thingpos[i]));

        for (j = low(s.thingpos[i]); j <= high(s.thingpos[i]); j++)
        {
          sk->pos[j] = lerp(sk->oldpos[j], sk->pos[j], p);
        }

        if (kit_styles.contains(things[i].style))
        {
          sk->satisfyconstraints();
        }
      }
    }
  }
}

void restorestate(tinterpolationstate &s)
{
  std::int32_t i;

  camerax = s.camera.x;
  cameray = s.camera.y;
  mx = s.mouse.x;
  my = s.mouse.y;

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    i = sprite.num;
    std::memcpy(&sprite.skeleton.pos[1], &s.spritepos[i][1],
                sizeof(tvector2) * length(s.spritepos[i]));
  }

  for (i = 1; i <= max_bullets; i++)
  {
    auto &b = GS::GetBulletSystem().GetBullets()[i];
    if (b.active or (b.pingadd > 0))
    {
      GetBulletParts().pos[b.num] = s.bulletpos[i];
      GetBulletParts().velocity[b.num] = s.bulletvel[i];
      b.hitmultiply = s.bullethitmul[i];
    }
  }

  for (i = 1; i <= max_sparks; i++)
  {
    if (spark[i].active)
    {
      GetSparkParts().pos[spark[i].num] = s.sparkpos[i];
    }
  }

  auto &things = GS::GetThingSystem().GetThings();
  for (i = 1; i <= max_things; i++)
  {
    if (things[i].active)
    {
      std::memcpy(&things[i].skeleton.pos[1], &s.thingpos[i][1],
                  sizeof(tvector2) * length(s.thingpos[i]));
    }
  }
}

void renderframe(double timeelapsed, double framepercent, bool paused)
{
  ZoneScopedN("RenderFrame");
  auto &sprite_system = SpriteSystem::Get();
  tmapgraphics *mg;
  std::int32_t i;
  float dx;
  float dy;
  float w;
  float h;
  float s;
  float u;
  float v;
  tinterpolationstate interpolationstate;
  bool grabactionsnap;
  trect rc;
  tgfxtexture *rt;

  mg = &mapgfx;
  auto &things = GS::GetThingSystem().GetThings();

  // graphics might be destroyed before end of game loop
  if (mg->vertexbuffer == nullptr)
  {
    return;
  }

  if (rendertarget != nullptr)
  {
    gfxtarget(rendertarget);
    gfxviewport(0, 0, renderwidth, renderheight);
  }
  else
  {
    gfxtarget(nullptr);
  }

  if (!screenshotpath.empty())
  {
    gfxsavescreen(screenshotpath, 0, 0, renderwidth, renderheight, screenshotasync);
    screenshotpath = "";
  }

  if ((showscreen != 0u) && actionsnaptaken)
  {
    ZoneScopedN("Render1");
    rc = trect(0, renderheight, renderwidth, 0);
    gfxblit(actionsnaptexture, rendertarget, rc, rc, gfx_nearest);
    gfxtarget(rendertarget);

    w = renderwidth;
    h = renderheight;

    if (CVar::r_scaleinterface)
    {
      w = gamewidth;
      h = gameheight;
    }

    gfxbegin();
    gfxtransform(gfxmat3ortho(0, w, 0, h));
    gfxtextpixelratio(vector2(w / renderwidth, h / renderheight));
    renderactionsnaptext(timeelapsed);
    gfxend();
  }
  else
  {
    ZoneScopedN("Render2");
    grabactionsnap = false;

    if ((CVar::cl_actionsnap) && (capscreen == 0))
    {
      capscreen = 255;
      grabactionsnap = true;
      actionsnaptaken = true;
      gfxtarget(actionsnaptexture);
    }
    else if (capscreen != 255)
    {
      capscreen -= 1;
    }

    interpolatestate(framepercent, interpolationstate, paused);

    w = exp(CVar::r_zoom) * gamewidth;
    h = exp(CVar::r_zoom) * gameheight;

    dx = camerax - w / 2;
    dy = cameray - h / 2;

    if (cameray > 0)
    {
      gfxclear(mapgfx.bgcolorbtm);
    }
    else
    {
      gfxclear(mapgfx.bgcolortop);
    }

    if (CVar::r_animations)
    {
      updateprops(timeelapsed);
    }

    gfxtransform(gfxmat3ortho(0, 1, dy, h + dy));
    gfxbindtexture(nullptr);
    gfxdraw(mg->vertexbuffer, mg->background, mg->backgroundcount);

    gfxtransform(gfxmat3ortho(dx, w + dx, dy, h + dy));

    if (CVar::r_smoothedges && (length(mg->edges[0]) > 0))
    {
      gfxdraw(mg->vertexbuffer, mg->indexbuffer, mg->edges[0].data(), length(mg->edges[0]));
    }

    if (length(mg->polys[0]) > 0)
    {
      gfxdraw(mg->vertexbuffer, mg->polys[0].data(), length(mg->polys[0]));
    }

    gfxsetmipmapbias(CVar::r_mipmapbias);

    if (CVar::r_renderbackground)
    {
      renderprops(0);
    }

    gfxbegin();

    {
      {
        ZoneScopedN("RenderBullet");
        for (i = 1; i <= max_bullets; i++)
        {
          auto &b = GS::GetBulletSystem().GetBullets()[i];
          if (b.active or (b.pingadd > 0))
          {
            b.render(timeelapsed);
          }
        }
      }

      {
        ZoneScopedN("RenderAllGosteks");
        auto &activeSprites = sprite_system.GetActiveSprites();
        std::for_each(std::begin(activeSprites), std::end(activeSprites),
                      [](auto &sprite) { rendergostek(sprite); });
      }

      {
        ZoneScopedN("RenderThings");
        for (i = 1; i <= max_things; i++)
        {
          if (things[i].active)
          {
            things[i].render(timeelapsed);
          }
        }
      }

      {
        ZoneScopedN("RenderSpark");
        for (i = 1; i <= max_sparks; i++)
        {
          if (spark[i].active)
          {
            spark[i].render();
          }
        }
      }
    }

    gfxend();
    renderprops(1);
    gfxbegin();

    for (i = 1; i <= max_things; i++)
    {
      if (things[i].active)
      {
        things[i].polygonsrender();
      }
    }

    gfxend();
    gfxsetmipmapbias(0);

    if (CVar::r_smoothedges && (length(mg->edges[1]) > 0))
    {
      gfxdraw(mg->vertexbuffer, mg->indexbuffer, mg->edges[1].data(), length(mg->edges[1]));
    }

    if (length(mg->polys[1]) > 0)
    {
      gfxdraw(mg->vertexbuffer, mg->polys[1].data(), length(mg->polys[1]));
    }

    gfxsetmipmapbias(CVar::r_mipmapbias);
    renderprops(2);
    gfxsetmipmapbias(0);

    if (!CVar::r_scaleinterface)
    {
      w = renderwidth;
      h = renderheight;
    }

    if (grabactionsnap)
    {
      rc = trect(0, renderheight, renderwidth, 0);
      gfxblit(actionsnaptexture, rendertarget, rc, rc, gfx_nearest);
      gfxtarget(rendertarget);
    }

    if (CVar::r_renderui)
    {
      ZoneScopedN("RenderUI");
      gfxbegin();
      gfxtransform(gfxmat3ortho(0, w, 0, h));
      renderinterface(timeelapsed, w, h);
      gfxend();
    }

    restorestate(interpolationstate);
  }

  if (rendertarget != nullptr)
  {
    ZoneScopedN("Render3");
    rt = rendertarget;

    if (rendertargetaa != nullptr)
    {
      rc = trect(0, renderheight, renderwidth, 0);
      gfxblit(rendertarget, rendertargetaa, rc, rc, gfx_nearest);
      rt = rendertargetaa;
    }

    if (((float)(screenwidth) / screenheight) >= ((float)(renderwidth) / renderheight))
    {
      w = screenheight * ((float)(renderwidth) / renderheight);
      h = screenheight;
    }
    else
    {
      w = screenwidth;
      h = screenwidth * ((float)(renderheight) / renderwidth);
    }

    dx = floor(0.5 * (screenwidth - w));
    dy = floor(0.5 * (screenheight - h));

    if (screenwidth != windowwidth)
    {
      s = (float)(windowwidth) / screenwidth;
      w = w * s;
      dx = (dx - (float)(screenwidth) / 2) * s + (float)(windowwidth) / 2;
    }

    if (screenheight != windowheight)
    {
      s = (float)(windowheight) / screenheight;
      h = h * s;
      dy = (dy - (float)(screenheight) / 2) * s + (float)(windowheight) / 2;
    }

    gfxtarget(nullptr);
    gfxviewport(0, 0, windowwidth, windowheight);
    gfxclear(rgba(0));
    gfxtransform(gfxmat3ortho(0, windowwidth, 0, windowheight));

    u = (float)(renderwidth) / rendertarget->width();
    v = (float)(renderheight) / rendertarget->height();

    gfxbegin();
    gfxdrawquad(rt, gfxvertex(dx + 0, dy + 0, 0, v, rgba(0xffffff)),
                gfxvertex(dx + w, dy + 0, u, v, rgba(0xffffff)),
                gfxvertex(dx + w, dy + h, u, 0, rgba(0xffffff)),
                gfxvertex(dx + 0, dy + h, 0, 0, rgba(0xffffff)));
    gfxend();
  }

  gfxpresent(CVar::r_glfinish);
}

void rendergameinfo(const std::string &textstring)
{
  tgfxrect rc;

  gfxtarget(nullptr);
  gfxviewport(0, 0, windowwidth, windowheight);
  gfxtransform(gfxmat3ortho(0, windowwidth, 0, windowheight));
  gfxclear(49, 61, 79, 255);
  setfontstyle(font_menu);
  gfxtextcolor(rgba(0xffffff));
  gfxtextshadow(1, 1, rgba(0));
  gfxtextpixelratio(vector2(1, 1));
  rc = gfxtextmetrics(textstring);
  gfxbegin();
  gfxdrawtext((float)((windowwidth - rc.width())) / 2, (float)((windowheight - rc.height())) / 2);
  setfontstyle(font_small);
  rc = gfxtextmetrics(("Press ESC to quit the game"));
  gfxdrawtext((float)((windowwidth - rc.width())) / 2,
              ((float)((windowheight - rc.height())) / 2) + 100);
  gfxend();
  gfxpresent(true);
}

template <typename T>
auto arraycontains(const T &list, std::int32_t x) -> bool
{
  std::int32_t i;

  bool result = false;

  for (i = low(list); i <= high(list); i++)
  {
    if (list[i] == x)
    {
      result = true;
      return result;
    }
  }
  return result;
}

auto getsizeconstraint(std::int32_t id, std::int32_t &w, std::int32_t &h) -> bool
{
  const std::array<std::int32_t, 35> weapons_list = {
    {GFX::WEAPONS_AK74,      GFX::WEAPONS_AK74_2,      GFX::WEAPONS_AK74_FIRE,
     GFX::WEAPONS_MINIMI,    GFX::WEAPONS_MINIMI_2,    GFX::WEAPONS_RUGER,
     GFX::WEAPONS_RUGER_2,   GFX::WEAPONS_MP5,         GFX::WEAPONS_MP5_2,
     GFX::WEAPONS_SPAS,      GFX::WEAPONS_SPAS_2,      GFX::WEAPONS_M79,
     GFX::WEAPONS_M79_2,     GFX::WEAPONS_DEAGLES,     GFX::WEAPONS_DEAGLES_2,
     GFX::WEAPONS_N_DEAGLES, GFX::WEAPONS_N_DEAGLES_2, GFX::WEAPONS_STEYR,
     GFX::WEAPONS_STEYR_2,   GFX::WEAPONS_BARRETT,     GFX::WEAPONS_BARRETT_2,
     GFX::WEAPONS_MINIGUN,   GFX::WEAPONS_MINIGUN_2,   GFX::WEAPONS_SOCOM,
     GFX::WEAPONS_SOCOM_2,   GFX::WEAPONS_N_SOCOM,     GFX::WEAPONS_N_SOCOM_2,
     GFX::WEAPONS_BOW,       GFX::WEAPONS_BOW_S,       GFX::WEAPONS_FLAMER,
     GFX::WEAPONS_FLAMER_2,  GFX::WEAPONS_KNIFE,       GFX::WEAPONS_KNIFE2,
     GFX::WEAPONS_CHAINSAW,  GFX::WEAPONS_CHAINSAW2}};

  bool result = false;

  if ((id >= GFX::GOSTEK_STOPA) && (id <= GFX::GOSTEK_TEAM2_LECISTOPA2))
  {
    w = gos_restrict_width;
    h = gos_restrict_height;
    result = true;
  }
  else if (arraycontains(weapons_list, id))
  {
    w = wep_restrict_width;
    h = wep_restrict_height;
    result = true;
  }
  return result;
}

auto dotextureloading(bool finishloading) -> bool
{
  std::int32_t i;
  std::int32_t j;
  std::int32_t w = 0;
  std::int32_t h = 0;
  std::string s;

  bool dotextureloading_result = true; // return true when not loading

  if ((mainspritesheet == nullptr) || (interfacespritesheet == nullptr))
  {
    return dotextureloading_result;
  }

  bool mainloading = mainspritesheet->loading();
  bool interfaceloading = interfacespritesheet->loading();

  if (!(mainloading || interfaceloading))
  {
    return dotextureloading_result;
  }

  if (finishloading)
  {
    mainspritesheet->finishloading();
    interfacespritesheet->finishloading();
  }
  else
  {
    if (mainspritesheet->loading())
    {
      mainspritesheet->continueloading();
    }
    else if (interfacespritesheet->loading())
    {
      interfacespritesheet->continueloading();
    }
  }

  if (mainspritesheet->loading() != mainloading)
  {
    j = 0;

    for (i = low(GFXData); i <= high(GFXData); i++)
    {
      if (GFXData[i].Group != GFXG::INTERFACE)
      {
        auto id = GFXData[i].ID;
        textures[id] = mainspritesheet->getsprite(j);
        textures[id]->scale = textures[id]->scale * ((float)(1) / imagescale[id]);

        if (getsizeconstraint(i, w, h))
        {
          if (((textures[id]->width * textures[id]->scale) > w) ||
              ((textures[id]->height * textures[id]->scale) > h))
          {
            if (((float)(textures[id]->width) / textures[id]->height) > ((float)(w) / h))
            {
              textures[id]->scale = (float)(w) / textures[id]->width;
            }
            else
            {
              textures[id]->scale = (float)(h) / textures[id]->height;
            }
          }
        }

        j += 1;
      }
    }

    s = "";

    for (i = 0; i <= mainspritesheet->texturecount() - 1; i++)
    {
      s = s + fmt::format("{}x{} ", mainspritesheet->gettexture(i)->width(),
                          mainspritesheet->gettexture(i)->height());

      settexturefilter(mainspritesheet->gettexture(i), true);
    }

    s[length(s)] = ')';
    gfxlog(string("Loaded main spritesheet (") + s);

    applygostekconstraints(textures);
  }

  if (interfacespritesheet->loading() != interfaceloading)
  {
    j = 0;

    for (i = 0; i < GFXData.size(); i++)
    {
      if (GFXData[i].Group == GFXG::INTERFACE)
      {
        auto id = GFXData[i].ID;
        textures[id] = interfacespritesheet->getsprite(j);
        textures[id]->scale = textures[id]->scale * ((float)(1) / imagescale[id]);
        j += 1;
      }
    }

    s = "";

    for (i = 0; i <= interfacespritesheet->texturecount() - 1; i++)
    {
      s = s + fmt::format("{}x{} ", interfacespritesheet->gettexture(i)->width(),
                          interfacespritesheet->gettexture(i)->height());

      settexturefilter(interfacespritesheet->gettexture(i), false);
    }

    s[length(s)] = ')';
    gfxlog(string("Loaded interface spritesheet (") + s);
  }

  dotextureloading_result = false;

  if (!(mainspritesheet->loading() || interfacespritesheet->loading()))
  {
    dotextureloading_result = true;
  }
  return dotextureloading_result;
}

void setfontstyle(std::int32_t style)
{
  gfxsetfonttable(fontstyles[style].font, fontstyles[style].tableindex);
}

void setfontstyle(std::int32_t style, float scale)
{
  gfxsetfont(fontstyles[style].font, scale * fontstyles[style].size, fontstyles[style].flags,
             fontstyles[style].stretch);
}

auto fontstylesize(std::int32_t style) -> std::int32_t { return fontstyles[style].size; }

void gfxlogcallback(const std::string &s)
{
  LogDebugG("[GFX] {}", s);
}

// initialization
//  GfxLog := @GfxLogCallback;

#pragma region tests
#include <doctest/doctest.h>
#include <ApprovalTests.hpp>
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include <thread>

#include "SdlApp.hpp"
extern void gfxSetGpuDevice(SDL_GPUDevice* device);

class PngWriter : public ApprovalTests::ApprovalWriter
{

public:
  PngWriter(std::int32_t w, std::int32_t h, std::unique_ptr<std::uint8_t[]> data)
  : m_width{w}, m_height{h}, m_data{std::move(data)}
  {

  }
  std::string getFileExtensionWithDot() const override
  {
      return ".png";
  }

  void write(std::string path) const override
  {
    stbi_write_png(path.c_str(), m_width, m_height, 4, m_data.get(), m_width * 4);
  }

  void cleanUpReceived(std::string receivedPath) const override
  {
    remove(receivedPath.c_str());
  };

private:
  std::int32_t m_width;
  std::int32_t m_height;
  std::unique_ptr<std::uint8_t[]> m_data;
};


extern void initgamemenus();
namespace
{

class GameRenderingFixture
{
public:
  GameRenderingFixture() = default;
  ~GameRenderingFixture() = default;
  GameRenderingFixture(const GameRenderingFixture&) = delete;
protected:
  static constexpr bool opengl = false;
};

using namespace std::chrono_literals;

TEST_SUITE("GameRenderingSuite")
{

TEST_CASE_FIXTURE(GameRenderingFixture, "Render text" * doctest::skip(false))
{
  SdlApp app("GameRenderingTest", 1280, 720, opengl);
  SDL_GetWindowSize(app.GetWindow(), &windowwidth, &windowheight);
  renderwidth = screenwidth = windowwidth;
  renderheight = screenheight = windowheight;
  gfxSetGpuDevice(app.GetDevice());
  gfxinitcontext(app.GetWindow(), false, false);
  loadfonts();
  rendergameinfo("Test");
  auto data = gfxsavescreen(0, 0, renderwidth, renderheight);
  PngWriter writer(renderwidth, renderheight, std::move(data));
  ApprovalTests::Approvals::verify(writer);
  gfxdestroycontext();
}

TEST_CASE_FIXTURE(GameRenderingFixture, "Render frame" * doctest::skip(false))
{
  SdlApp app("GameRenderingTest", 1280, 720, opengl);
  SDL_GetWindowSize(app.GetWindow(), &windowwidth, &windowheight);
  renderwidth = screenwidth = windowwidth;
  renderheight = screenheight = windowheight;
  gamewindow = app.GetWindow();
  gfxSetGpuDevice(app.GetDevice());
  gfxinitcontext(app.GetWindow(), false, false);
  GlobalSystems<Config::CLIENT_MODULE>::Init();
  auto &fs = GS::GetFileSystem();
  const auto userDirectory = FileUtility::GetPrefPath("client");
  const auto baseDirectory = FileUtility::GetBasePath();

  fs.Mount(userDirectory, "/user");
  fs.Mount(baseDirectory + "/soldat.smod", "/");
  textures = new pgfxsprite[GFX::END + 1];
  auto ret = getmapinfo(fs, "ctf_Ash", userDirectory, mapgfx.mapinfo);
  CHECK(ret);
  reloadgraphics();
  loadweaponnames(fs);
  createweaponsbase(GS::GetWeaponSystem().GetGuns());
  initgamemenus();
  loadfonts();
  renderframe(1.0f, 1.0f, true);
  std::this_thread::sleep_for(16ms);
  auto data = gfxsavescreen(0, 0, renderwidth, renderheight);
  PngWriter writer(renderwidth, renderheight, std::move(data));
  ApprovalTests::Approvals::verify(writer);
  destroymapgraphics();
  gfxdestroycontext();
  delete[] textures;
  GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  gamewindow = nullptr;
  CHECK(true);
}

} // end of GameRenderingSuite
} // end of unnamed namespace
#pragma endregion tests