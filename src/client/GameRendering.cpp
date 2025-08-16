// automatically converted

#include "GameRendering.hpp"

#include <Tracy.hpp>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <math.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <stdio.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <set>
#include <string>
#include <chrono>
#include <cstring>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

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
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/Bullets.hpp"
#include "shared/mechanics/Sparks.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/mechanics/Things.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/gfx.hpp"
#include "client/Gfx.hpp"
#include "common/FileUtility.hpp"
#include "common/MapFile.hpp"
#include "common/Parts.hpp"
#include "common/PolyMap.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/misc/SafeType.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/Constants.cpp.h"

GlobalStateGameRendering gGlobalStateGameRendering{

};

using string = std::string;

struct ttextureloaddata
{
  std::int32_t id;
  std::int32_t group;
  std::string path;
  std::uint32_t colorkey;
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

void GlobalStateGameRendering::loadmodinfo()
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

  auto modinistream = ReadAsFileStream(fs, gGlobalStateClient.moddir + "mod.ini");

  if (gostekdata.empty() or scaledata.currentmod.empty())
  {
    modini = std::make_unique<TIniFile>(std::move(modinistream));
  }

  auto interfaceinistream = ReadAsFileStream(
    fs, (string("custom-interfaces/") +
         gGlobalStateGameRendering.gamerenderingparams.interfacename + "/mod.ini"));

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

auto GlobalStateGameRendering::getimagescale(const std::string &imagepath) -> float
{
  std::string scale;
  std::string key;

  std::string intdir = string("/custom-interfaces/") +
                       lowercase(gGlobalStateGameRendering.gamerenderingparams.interfacename) + '/';

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

void GlobalStateGameRendering::takescreenshot(string filename, bool async)
{
  screenshotpath = filename;
  screenshotasync = async;
}

auto GlobalStateGameRendering::pngoverride(const std::string_view &filename) -> string
{
  std::string f{filename};
  std::replace(f.begin(), f.end(), '\\', '/');

  return overridefileext(GS::GetFileSystem(), f, ".png");
}

auto GlobalStateGameRendering::pngoverride(const std::string &filename) -> string
{
  std::string f{filename};
  std::replace(f.begin(), f.end(), '\\', '/');
  return overridefileext(GS::GetFileSystem(), f, ".png");
}

void GlobalStateGameRendering::loadmaintextures()
{
  auto &fs = GS::GetFileSystem();

  const auto count = std::count_if(GFXData.begin(), GFXData.end(),
                        [](const auto &d) { return d.Group != GFXG::INTERFACE; });

  mainspritesheet = new tgfxspritesheet(count);
  float scale = 1.5 * gGlobalStateClientGame.renderheight / gGlobalStateGame.gameheight;

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

      auto path = pngoverride(gGlobalStateClient.moddir + std::string(i.Path));

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

void GlobalStateGameRendering::loadinterfacetextures(const std::string interfacename)
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
  bool iscustom = !gGlobalStateInterfaceGraphics.isdefaultinterface(interfacename);
  auto& fs = GS::GetFileSystem();

  if (iscustom)
  {
    cutlength = length(std::string("interface-gfx/"));
    prefix = gGlobalStateClient.moddir + "custom-interfaces/" + interfacename + '/';
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
    scale = (float)(gGlobalStateClientGame.renderheight) / gGlobalStateGame.gameheight;
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
        path = pngoverride(gGlobalStateClient.moddir + std::string(GFXData[i].Path));

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

void GlobalStateGameRendering::loadinterface()
{
  if (gGlobalStateInterfaceGraphics.loadinterfacedata(
        gGlobalStateGameRendering.gamerenderingparams.interfacename))
  {
    loadinterfacetextures(gGlobalStateGameRendering.gamerenderingparams.interfacename);
  }
  else
  {
    loadinterfacetextures("");
  }

  loadedinterfacename = gGlobalStateGameRendering.gamerenderingparams.interfacename;
}

auto GlobalStateGameRendering::getfontpath(string fontfile) -> string
{
  std::string result;
  auto p = std::filesystem::path(gGlobalStateClient.basedirectory + fontfile);
  if (std::filesystem::exists(p) && !std::filesystem::is_directory(p))
  {
    result = gGlobalStateClient.basedirectory + fontfile;
  }
  return result;
}

auto GlobalStateGameRendering::getfontpath(string fallback, string &fontfile) -> string
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

void GlobalStateGameRendering::loadfonts()
{
  std::array<string, 2> fontfile;
  std::array<string, 2> fontpath;

  fontfile[0] = CVar::font_1_filename;
  fontfile[1] = CVar::font_2_filename;

  fontpath[0] = getfontpath(default_font, fontfile[0]);
  fontpath[1] = getfontpath(default_font, fontfile[1]);

  if ((fontpath[0].empty()) || (fontpath[1].empty()))
  {
    gGlobalStateClient.showmessage(
      ("One of the fonts cannot be found. Please check your installation directory."));
    gGlobalStateClient.shutdown();
  }

  const std::int32_t w = gGlobalStateClientGame.renderwidth;
  const std::int32_t h = gGlobalStateClientGame.renderheight;
  const float s =
    iif(CVar::r_scaleinterface,
        (float)(gGlobalStateClientGame.renderheight) / gGlobalStateGame.gameheight, 1.0f);

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
  fontstyles[font_world].size =
    128 * ((float)(gGlobalStateClientGame.renderheight) / gGlobalStateGame.gameheight);
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

auto GlobalStateGameRendering::initgamegraphics() -> bool
{
  bool result = true;

  if (initialized)
  {
    if (gGlobalStateGameRendering.gamerenderingparams.interfacename != loadedinterfacename)
    {
      scaledata.custominterface.clear();
      loadmodinfo();
      loadinterface();
    }

    return result;
  }

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
  gGlobalStateInput.gamewindow = SDL_CreateWindow("Soldat", gGlobalStateClientGame.windowwidth,
                                                  gGlobalStateClientGame.windowheight, windowflags);

  auto& fs = GS::GetFileSystem();
  {
    std::vector<std::uint8_t> filebuffer;
    filebuffer = fs.ReadFile("/icon.bmp");

    SDL_IOStream *iconfile = SDL_IOFromMem(filebuffer.data(), length(filebuffer));

    auto icon_surface = SDL_LoadBMP_IO(iconfile, 1);
    SDL_SetWindowIcon(gGlobalStateInput.gamewindow, icon_surface);
    SDL_DestroySurface(icon_surface);
  }

  if (gGlobalStateInput.gamewindow == nullptr)
  {
    gGlobalStateClient.showmessage("Error creating sdl3 window");
    result = false;
    return result;
  }

  if (!gfxinitcontext(gGlobalStateInput.gamewindow, CVar::r_dithering, CVar::r_compatibility))
  {
    result = false;
    return result;
  }

  gGlobalStateInput.startinput();

  if (SDL_GL_SetSwapInterval(CVar::r_swapeffect) == false)
  {
    gfxlog(string("Error while setting SDL_GL_SetSwapInterval:") + SDL_GetError());
  }

  gfxviewport(0, 0, gGlobalStateClientGame.windowwidth, gGlobalStateClientGame.windowheight);

  textures = new pgfxsprite[GFX::END + 1];
  loadmodinfo();
  loadmaintextures();
  loadinterface();
  loadfonts();
  
  {
    auto &map = GS::GetGame().GetMap();
    map.loadgraphics = [](tmapfile &mapfile, bool bgforce, tmapcolor bgcolortop, tmapcolor bgcolorbtm) {
      gGlobalStateMapGraphics.loadmapgraphics(mapfile, bgforce, bgcolortop, bgcolorbtm);
    };
  }
  if (!gfxframebuffersupported())
  {
    CVar::cl_actionsnap = false;
  }

  if (CVar::cl_actionsnap)
  {
    actionsnaptexture = gfxcreaterendertarget(gGlobalStateClientGame.renderwidth,
                                              gGlobalStateClientGame.renderheight, 4, true);
  }

  if (gfxframebuffersupported())
  {
    if ((gGlobalStateClientGame.windowwidth != gGlobalStateClientGame.renderwidth) ||
        (gGlobalStateClientGame.windowheight != gGlobalStateClientGame.renderheight))
    {
      rendertarget = gfxcreaterendertarget(gGlobalStateClientGame.renderwidth,
                                           gGlobalStateClientGame.renderheight, 4, true);

      if (rendertarget->samples() > 0)
      {
        rendertargetaa = gfxcreaterendertarget(gGlobalStateClientGame.renderwidth,
                                               gGlobalStateClientGame.renderheight, 4, false);

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

void GlobalStateGameRendering::reloadgraphics()
{
  tmapfile mapfile;
  tmapinfo mapinfo;
  bool bgforce;
  std::array<tmapcolor, 2> color;
  tmapgraphics *mg;

  mg = &gGlobalStateMapGraphics.mapgfx;
  mapinfo = mg->mapinfo;
  bgforce = mg->bgforce;
  color[0] = mg->bgforcedcolor[0];
  color[1] = mg->bgforcedcolor[1];

  freeandnullptr(mainspritesheet);
  freeandnullptr(interfacespritesheet);
  gGlobalStateMapGraphics.destroymapgraphics();

  gostekdata.clear();
  scaledata.root.clear();
  scaledata.currentmod.clear();
  scaledata.custominterface.clear();

  loadmodinfo();
  loadmaintextures();
  loadinterface();
  dotextureloading(true);

  loadmapfile(GS::GetFileSystem(), mapinfo, mapfile);
  gGlobalStateMapGraphics.loadmapgraphics(mapfile, bgforce, color[0], color[1]);
}

void GlobalStateGameRendering::destroygamegraphics()
{
  std::int32_t i;

  if (!initialized)
  {
    return;
  }

  SDL_SetWindowIcon(gGlobalStateInput.gamewindow, nullptr);

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

  gGlobalStateMapGraphics.destroymapgraphics();
  gfxdestroycontext();

  initialized = false;
}

constexpr auto lerp(const tvector2 &a, const tvector2 &b, float x)
  -> tvector2
{
  tvector2 lerp_result;
  lerp_result.x = a.x + (b.x - a.x) * x;
  lerp_result.y = a.y + (b.y - a.y) * x;
  return lerp_result;
}

void GlobalStateGameRendering::interpolatestate(float p, tinterpolationstate &s, bool paused)
{
  ZoneScopedN("InterpolateState");
  static const std::set<std::int32_t> kit_styles = {object_medical_kit,  object_grenade_kit, object_flamer_kit,
                                       object_predator_kit, object_vest_kit,    object_berserk_kit,
                                       object_cluster_kit};
  std::int32_t i;
  std::int32_t j;

  s.camera.x = gGlobalStateClient.camerax;
  s.camera.y = gGlobalStateClient.cameray;
  s.mouse.x = gGlobalStateClientGame.mx;
  s.mouse.y = gGlobalStateClientGame.my;

  gGlobalStateClient.camerax = lerp(gGlobalStateClient.cameraprev.x, gGlobalStateClient.camerax, p);
  gGlobalStateClient.cameray = lerp(gGlobalStateClient.cameraprev.y, gGlobalStateClient.cameray, p);
  gGlobalStateClientGame.mx =
    lerp(gGlobalStateClientGame.mouseprev.x, gGlobalStateClientGame.mx, p);
  gGlobalStateClientGame.my =
    lerp(gGlobalStateClientGame.mouseprev.y, gGlobalStateClientGame.my, p);

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
      if (gGlobalStateGame.spark[i].active)
      {
        j = gGlobalStateGame.spark[i].num;
        s.sparkpos[i] = gGlobalStateSparks.GetSparkParts().pos[j];
        gGlobalStateSparks.GetSparkParts().pos[j] =
          lerp(gGlobalStateSparks.GetSparkParts().oldpos[j],
               gGlobalStateSparks.GetSparkParts().pos[j], p);
        gGlobalStateGame.spark[i].lifefloat =
          lerp(gGlobalStateGame.spark[i].lifeprev, gGlobalStateGame.spark[i].life, p);
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

void GlobalStateGameRendering::restorestate(tinterpolationstate &s)
{
  std::int32_t i;

  gGlobalStateClient.camerax = s.camera.x;
  gGlobalStateClient.cameray = s.camera.y;
  gGlobalStateClientGame.mx = s.mouse.x;
  gGlobalStateClientGame.my = s.mouse.y;

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
    if (gGlobalStateGame.spark[i].active)
    {
      gGlobalStateSparks.GetSparkParts().pos[gGlobalStateGame.spark[i].num] = s.sparkpos[i];
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

void GlobalStateGameRendering::renderframe(double timeelapsed, double framepercent, bool paused)
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

  mg = &gGlobalStateMapGraphics.mapgfx;
  auto &things = GS::GetThingSystem().GetThings();

  // graphics might be destroyed before end of game loop
  if (mg->vertexbuffer == nullptr)
  {
    return;
  }

  if (rendertarget != nullptr)
  {
    gfxtarget(rendertarget);
    gfxviewport(0, 0, gGlobalStateClientGame.renderwidth, gGlobalStateClientGame.renderheight);
  }
  else
  {
    gfxtarget(nullptr);
  }

  if (!screenshotpath.empty())
  {
    gfxsavescreen(screenshotpath, 0, 0, gGlobalStateClientGame.renderwidth,
                  gGlobalStateClientGame.renderheight, screenshotasync);
    screenshotpath = "";
  }

  if ((gGlobalStateClientGame.showscreen != 0u) && gGlobalStateClientGame.actionsnaptaken)
  {
    ZoneScopedN("Render1");
    rc = trect(0, gGlobalStateClientGame.renderheight, gGlobalStateClientGame.renderwidth, 0);
    gfxblit(actionsnaptexture, rendertarget, rc, rc, gfx_nearest);
    gfxtarget(rendertarget);

    w = gGlobalStateClientGame.renderwidth;
    h = gGlobalStateClientGame.renderheight;

    if (CVar::r_scaleinterface)
    {
      w = gGlobalStateGame.gamewidth;
      h = gGlobalStateGame.gameheight;
    }

    gfxbegin();
    gfxtransform(gfxmat3ortho(0, w, 0, h));
    gfxtextpixelratio(
      vector2(w / gGlobalStateClientGame.renderwidth, h / gGlobalStateClientGame.renderheight));
    gGlobalStateInterfaceGraphics.renderactionsnaptext(timeelapsed);
    gfxend();
  }
  else
  {
    ZoneScopedN("Render2");
    grabactionsnap = false;

    if ((CVar::cl_actionsnap) && (gGlobalStateClientGame.capscreen == 0))
    {
      gGlobalStateClientGame.capscreen = 255;
      grabactionsnap = true;
      gGlobalStateClientGame.actionsnaptaken = true;
      gfxtarget(actionsnaptexture);
    }
    else if (gGlobalStateClientGame.capscreen != 255)
    {
      gGlobalStateClientGame.capscreen -= 1;
    }

    interpolatestate(framepercent, interpolationstate, paused);

    w = exp(CVar::r_zoom) * gGlobalStateGame.gamewidth;
    h = exp(CVar::r_zoom) * gGlobalStateGame.gameheight;

    dx = gGlobalStateClient.camerax - w / 2;
    dy = gGlobalStateClient.cameray - h / 2;

    if (gGlobalStateClient.cameray > 0)
    {
      gfxclear(gGlobalStateMapGraphics.mapgfx.bgcolorbtm);
    }
    else
    {
      gfxclear(gGlobalStateMapGraphics.mapgfx.bgcolortop);
    }

    if (CVar::r_animations)
    {
      gGlobalStateMapGraphics.updateprops(timeelapsed);
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
      gGlobalStateMapGraphics.renderprops(0);
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
          if (gGlobalStateGame.spark[i].active)
          {
            gGlobalStateGame.spark[i].render();
          }
        }
      }
    }

    gfxend();
    gGlobalStateMapGraphics.renderprops(1);
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
    gGlobalStateMapGraphics.renderprops(2);
    gfxsetmipmapbias(0);

    if (!CVar::r_scaleinterface)
    {
      w = gGlobalStateClientGame.renderwidth;
      h = gGlobalStateClientGame.renderheight;
    }

    if (grabactionsnap)
    {
      rc = trect(0, gGlobalStateClientGame.renderheight, gGlobalStateClientGame.renderwidth, 0);
      gfxblit(actionsnaptexture, rendertarget, rc, rc, gfx_nearest);
      gfxtarget(rendertarget);
    }

    if (CVar::r_renderui)
    {
      ZoneScopedN("RenderUI");
      gfxbegin();
      gfxtransform(gfxmat3ortho(0, w, 0, h));
      gGlobalStateInterfaceGraphics.renderinterface(timeelapsed, w, h);
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
      rc = trect(0, gGlobalStateClientGame.renderheight, gGlobalStateClientGame.renderwidth, 0);
      gfxblit(rendertarget, rendertargetaa, rc, rc, gfx_nearest);
      rt = rendertargetaa;
    }

    if (((float)(gGlobalStateClientGame.screenwidth) / gGlobalStateClientGame.screenheight) >=
        ((float)(gGlobalStateClientGame.renderwidth) / gGlobalStateClientGame.renderheight))
    {
      w = gGlobalStateClientGame.screenheight *
          ((float)(gGlobalStateClientGame.renderwidth) / gGlobalStateClientGame.renderheight);
      h = gGlobalStateClientGame.screenheight;
    }
    else
    {
      w = gGlobalStateClientGame.screenwidth;
      h = gGlobalStateClientGame.screenwidth *
          ((float)(gGlobalStateClientGame.renderheight) / gGlobalStateClientGame.renderwidth);
    }

    dx = floor(0.5 * (gGlobalStateClientGame.screenwidth - w));
    dy = floor(0.5 * (gGlobalStateClientGame.screenheight - h));

    if (gGlobalStateClientGame.screenwidth != gGlobalStateClientGame.windowwidth)
    {
      s = (float)(gGlobalStateClientGame.windowwidth) / gGlobalStateClientGame.screenwidth;
      w = w * s;
      dx = (dx - (float)(gGlobalStateClientGame.screenwidth) / 2) * s +
           (float)(gGlobalStateClientGame.windowwidth) / 2;
    }

    if (gGlobalStateClientGame.screenheight != gGlobalStateClientGame.windowheight)
    {
      s = (float)(gGlobalStateClientGame.windowheight) / gGlobalStateClientGame.screenheight;
      h = h * s;
      dy = (dy - (float)(gGlobalStateClientGame.screenheight) / 2) * s +
           (float)(gGlobalStateClientGame.windowheight) / 2;
    }

    gfxtarget(nullptr);
    gfxviewport(0, 0, gGlobalStateClientGame.windowwidth, gGlobalStateClientGame.windowheight);
    gfxclear(rgba(0));
    gfxtransform(
      gfxmat3ortho(0, gGlobalStateClientGame.windowwidth, 0, gGlobalStateClientGame.windowheight));

    u = (float)(gGlobalStateClientGame.renderwidth) / rendertarget->width();
    v = (float)(gGlobalStateClientGame.renderheight) / rendertarget->height();

    gfxbegin();
    gfxdrawquad(rt, gfxvertex(dx + 0, dy + 0, 0, v, rgba(0xffffff)),
                gfxvertex(dx + w, dy + 0, u, v, rgba(0xffffff)),
                gfxvertex(dx + w, dy + h, u, 0, rgba(0xffffff)),
                gfxvertex(dx + 0, dy + h, 0, 0, rgba(0xffffff)));
    gfxend();
  }

  gfxpresent(CVar::r_glfinish);
}

void GlobalStateGameRendering::rendergameinfo(const std::string &textstring)
{
  tgfxrect rc;

  gfxtarget(nullptr);
  gfxviewport(0, 0, gGlobalStateClientGame.windowwidth, gGlobalStateClientGame.windowheight);
  gfxtransform(
    gfxmat3ortho(0, gGlobalStateClientGame.windowwidth, 0, gGlobalStateClientGame.windowheight));
  gfxclear(49, 61, 79, 255);
  setfontstyle(font_menu);
  gfxtextcolor(rgba(0xffffff));
  gfxtextshadow(1, 1, rgba(0));
  gfxtextpixelratio(vector2(1, 1));
  rc = gfxtextmetrics(textstring);
  gfxbegin();
  gfxdrawtext((float)((gGlobalStateClientGame.windowwidth - rc.width())) / 2,
              (float)((gGlobalStateClientGame.windowheight - rc.height())) / 2);
  setfontstyle(font_small);
  rc = gfxtextmetrics(("Press ESC to quit the game"));
  gfxdrawtext((float)((gGlobalStateClientGame.windowwidth - rc.width())) / 2,
              ((float)((gGlobalStateClientGame.windowheight - rc.height())) / 2) + 100);
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

auto GlobalStateGameRendering::getsizeconstraint(std::int32_t id, std::int32_t &w, std::int32_t &h)
  -> bool
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

auto GlobalStateGameRendering::dotextureloading(bool finishloading) -> bool
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

      gGlobalStateMapGraphics.settexturefilter(mainspritesheet->gettexture(i), true);
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

      gGlobalStateMapGraphics.settexturefilter(interfacespritesheet->gettexture(i), false);
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

void GlobalStateGameRendering::setfontstyle(std::int32_t style)
{
  gfxsetfonttable(fontstyles[style].font, fontstyles[style].tableindex);
}

void GlobalStateGameRendering::setfontstyle(std::int32_t style, float scale)
{
  gfxsetfont(fontstyles[style].font, scale * fontstyles[style].size, fontstyles[style].flags,
             fontstyles[style].stretch);
}

auto GlobalStateGameRendering::fontstylesize(std::int32_t style) -> std::int32_t
{
  return fontstyles[style].size;
}

void GlobalStateGameRendering::gfxlogcallback(const std::string &s) { LogDebugG("[GFX] {}", s); }

// initialization
//  GfxLog := @GfxLogCallback;

#pragma region tests
#include <doctest/doctest.h>
#include <stb_image_write.h>
#include <ApprovalTests/Approvals.h>
#include <ApprovalTests/core/ApprovalWriter.h>
#include <thread>
#include "GameMenus.hpp"

#include "SdlApp.hpp"

extern void gfxSetGpuDevice(SDL_GPUDevice* device);

class PngWriter : public ApprovalTests::ApprovalWriter
{

public:
  PngWriter(std::int32_t w, std::int32_t h, std::unique_ptr<std::uint8_t[]> data)
  : m_width{w}, m_height{h}, m_data{std::move(data)}
  {

  }
  [[nodiscard]] std::string getFileExtensionWithDot() const override
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
  SDL_GetWindowSize(app.GetWindow(), &gGlobalStateClientGame.windowwidth,
                    &gGlobalStateClientGame.windowheight);
  gGlobalStateClientGame.renderwidth = gGlobalStateClientGame.screenwidth =
    gGlobalStateClientGame.windowwidth;
  gGlobalStateClientGame.renderheight = gGlobalStateClientGame.screenheight =
    gGlobalStateClientGame.windowheight;
  gfxSetGpuDevice(app.GetDevice());
  gfxinitcontext(app.GetWindow(), false, false);
  gGlobalStateGameRendering.loadfonts();
  gGlobalStateGameRendering.rendergameinfo("Test");
  auto data =
    gfxsavescreen(0, 0, gGlobalStateClientGame.renderwidth, gGlobalStateClientGame.renderheight);
  PngWriter writer(gGlobalStateClientGame.renderwidth, gGlobalStateClientGame.renderheight,
                   std::move(data));
  ApprovalTests::Approvals::verify(writer);
  gfxdestroycontext();
}

TEST_CASE_FIXTURE(GameRenderingFixture, "Render frame" * doctest::skip(true))
{
  SdlApp app("GameRenderingTest", 1280, 720, opengl);
  SDL_GetWindowSize(app.GetWindow(), &gGlobalStateClientGame.windowwidth,
                    &gGlobalStateClientGame.windowheight);
  gGlobalStateClientGame.renderwidth = gGlobalStateClientGame.screenwidth =
    gGlobalStateClientGame.windowwidth;
  gGlobalStateClientGame.renderheight = gGlobalStateClientGame.screenheight =
    gGlobalStateClientGame.windowheight;
  gGlobalStateInput.gamewindow = app.GetWindow();
  gfxSetGpuDevice(app.GetDevice());
  gfxinitcontext(app.GetWindow(), false, false);
  GlobalSystems<Config::CLIENT_MODULE>::Init();
  auto &fs = GS::GetFileSystem();
  const auto userDirectory = FileUtility::GetPrefPath("client");
  const auto baseDirectory = FileUtility::GetBasePath();

  fs.Mount(userDirectory, "/user");
  fs.Mount(baseDirectory + "/soldat.smod", "/");
  gGlobalStateGameRendering.textures = new pgfxsprite[GFX::END + 1];
  auto ret = getmapinfo(fs, "ctf_Ash", userDirectory, gGlobalStateMapGraphics.mapgfx.mapinfo);
  CHECK(ret);
  gGlobalStateGameRendering.reloadgraphics();
  gGlobalStateClient.loadweaponnames(fs, gGlobalStateClient.gundisplayname,
                                             gGlobalStateClient.moddir);
  createweaponsbase(GS::GetWeaponSystem().GetGuns());
  gGlobalStateGameMenus.initgamemenus();
  gGlobalStateGameRendering.loadfonts();
  gGlobalStateGameRendering.renderframe(1.0f, 1.0f, true);
  std::this_thread::sleep_for(16ms);
  auto data =
    gfxsavescreen(0, 0, gGlobalStateClientGame.renderwidth, gGlobalStateClientGame.renderheight);
  PngWriter writer(gGlobalStateClientGame.renderwidth, gGlobalStateClientGame.renderheight,
                   std::move(data));
  ApprovalTests::Approvals::verify(writer);
  gGlobalStateMapGraphics.destroymapgraphics();
  gfxdestroycontext();
  delete[] gGlobalStateGameRendering.textures;
  GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  gGlobalStateInput.gamewindow = nullptr;
  CHECK(true);
}

} // end of GameRenderingSuite
} // end of unnamed namespace
#pragma endregion tests