#include <cstdint>

#include "Client.hpp"
#include "ClientGame.hpp"
#include "ControlGame.hpp"
#include "GameMenus.hpp"
#include "GameRendering.hpp"
#include "Gfx.hpp"
#include "InterfaceGraphics.hpp"
#include "MapGraphics.hpp"
#include "common/Calc.hpp"
#include "common/Console.hpp"
#include "common/Logging.hpp"
#include "common/Util.hpp"
#include "common/gfx.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClient.hpp"
#include <filesystem>
#include <physfs.h>
#include <set>

using string = std::string;

// TODO: fix that later
// NOLINTBEGIN(readability-magic-numbers)

// Chat stuff

// Big Text

// World Text

GlobalStateInterfaceGraphics gGlobalStateInterfaceGraphics{
  .chatmessage{},
  .chatteam{},
  .chatdelay{},
  .bigtext{},
  .bigdelay{},
  .bigx{},
  .bigscale{},
  .bigcolor{},
  .bigposx{},
  .bigposy{},
  .worldtext{},
  .worlddelay{},
  .worldx{},
  .worldscale{},
  .worldcolor{},
  .worldposx{},
  .worldposy{},
  .cursortext{},
  .cursortextlength{},
  .cursorfriendly{},
  .fragsmenushow{},
  .statsmenushow{},
  .coninfoshow{},
  .playernamesshow{},
  .minimapshow{},
  .noobshow{},
  .fragsscrollmax = 0,
  .fragsscrolllev = 0,
  ._rscala{},
  ._iscala{},
  .fragx{},
  .fragy{},
};

struct tinterfacerelinfo
{
  std::int32_t healthbar_rel_x, healthbar_rel_y;
  std::int32_t jetbar_rel_x, jetbar_rel_y;
  std::int32_t ammobar_rel_x, ammobar_rel_y;
  std::int32_t firebar_rel_x, firebar_rel_y;
  std::int32_t nadesbar_rel_x, nadesbar_rel_y;
};

struct tinterface
{
  std::uint8_t alpha;
  bool health, ammo, vest, jet, nades, bullets, weapon, fire, team, ping, status;
  std::int32_t healthico_x, healthico_y;
  std::int32_t healthico_rotate;
  std::int32_t healthbar_x, healthbar_y;
  std::int32_t healthbar_width, healthbar_height;
  std::uint8_t healthbar_pos;
  std::int32_t healthbar_rotate;
  std::int32_t ammoico_x, ammoico_y;
  std::int32_t ammoico_rotate;
  std::int32_t ammobar_x, ammobar_y;
  std::int32_t ammobar_width, ammobar_height;
  std::uint8_t ammobar_pos;
  std::int32_t ammobar_rotate;
  std::int32_t jetico_x, jetico_y;
  std::int32_t jetico_rotate;
  std::int32_t jetbar_x, jetbar_y;
  std::int32_t jetbar_width, jetbar_height;
  std::uint8_t jetbar_pos;
  std::int32_t jetbar_rotate;
  std::int32_t vestbar_x, vestbar_y;
  std::int32_t vestbar_width, vestbar_height;
  std::uint8_t vestbar_pos;
  std::int32_t vestbar_rotate;
  std::int32_t nades_x, nades_y;
  std::int32_t nades_width, nades_height;
  std::uint8_t nades_pos;
  std::int32_t bullets_x, bullets_y;
  std::int32_t weapon_x, weapon_y;
  std::int32_t fireico_x, fireico_y;
  std::int32_t fireico_rotate;
  std::int32_t firebar_x, firebar_y;
  std::int32_t firebar_width, firebar_height;
  std::uint8_t firebar_pos;
  std::int32_t firebar_rotate;
  std::int32_t teambox_x, teambox_y;
  std::int32_t ping_x, ping_y;
  std::int32_t status_x, status_y;
};

using pinterface = tinterface *;

tinterface int_;
tinterfacerelinfo relinfo;
tvector2 pixelsize;
double chatinputtime;
struct
{ // 0: left; 1: right
  std::uint8_t weapon;
  std::uint8_t bullets;
  std::uint8_t healthbar;
  std::uint8_t ammobar;
  std::uint8_t reloadbar;
  std::uint8_t firebar;
  std::uint8_t jetbar;
  std::uint8_t vestbar;
} intalign;

void loadinterfacearchives(const std::string &path, bool firstonly)
{
  if (!std::filesystem::exists(path))
  {
    return;
  }

  for (const auto &f : std::filesystem::directory_iterator(path))
  {
    if (f.path().extension() != ".sint")
    {
      continue;
    }
    std::string name = f.path().stem();
    if (PHYSFS_mount((pchar)(path + f.path().filename().c_str()),
                     (pchar)(string("custom-interfaces/") + name + '/'), 0) != 0)
    {
      LogDebugG("Mounted interface: {}", f.path().c_str());
      if (firstonly)
      {
        CVar::ui_style = name;
        break;
      }
    }
    else
    {
      LogDebugG("Failed to mount interface: {}", f.path().c_str());
    }
  }
}

auto isdefaultinterface(const std::string &interfacename) -> bool
{
  return (interfacename.empty()) || (interfacename == "Default");
}

void loaddefaultinterfacedata()
{
  // Create default INTERFACE
  intalign.weapon = 1;
  intalign.bullets = 1;
  intalign.healthbar = 0;
  intalign.ammobar = 0;
  intalign.reloadbar = 0;
  intalign.firebar = 1;
  intalign.jetbar = 0;
  intalign.vestbar = 0;

  int_.alpha = 255;
  int_.health = true;
  int_.ammo = true;
  int_.vest = true;
  int_.jet = true;
  int_.nades = true;
  int_.bullets = true;
  int_.weapon = true;
  int_.fire = true;
  int_.team = true;
  int_.ping = true;
  int_.status = true;
  int_.healthbar_width = 115;
  int_.healthbar_height = 9;
  int_.healthbar_pos = horizontal;
  int_.healthbar_rotate = 0;
  int_.ammobar_width = 120;
  int_.ammobar_height = 9;
  int_.ammobar_pos = horizontal;
  int_.ammobar_rotate = 0;
  int_.jetbar_width = 115;
  int_.jetbar_height = 9;
  int_.jetbar_pos = horizontal;
  int_.jetbar_rotate = 0;
  int_.vestbar_width = 115;
  int_.vestbar_height = 9;
  int_.vestbar_pos = horizontal;
  int_.vestbar_rotate = 0;
  int_.firebar_width = 57;
  int_.firebar_height = 4;
  int_.firebar_pos = horizontal;
  int_.firebar_rotate = 0;
  int_.healthico_x = 5;
  int_.healthico_y = 445 - 6;
  int_.healthico_rotate = 0;
  int_.ammoico_x = 285 - 10;
  int_.ammoico_y = 445 - 6;
  int_.ammoico_rotate = 0;
  int_.jetico_x = 480;
  int_.jetico_y = 445 - 6;
  int_.jetico_rotate = 0;
  int_.healthbar_x = 45;
  int_.healthbar_y = 455 - 6;
  int_.ammobar_x = 352;
  int_.ammobar_y = 455 - 6;
  int_.bullets_x = 348;
  int_.bullets_y = 451;
  int_.jetbar_x = 520;
  int_.jetbar_y = 455 - 6;
  int_.firebar_x = 402;
  int_.firebar_y = 464;
  int_.firebar_rotate = 0;
  int_.fireico_x = 409;
  int_.fireico_y = 464;
  int_.fireico_rotate = 0;
  int_.nades_x = 305 - 7 + 10;
  int_.nades_y = 468 - 6;
  int_.nades_height = 10;
  int_.nades_width = 10;
  int_.nades_pos = horizontal;
  int_.vestbar_x = 45;
  int_.vestbar_y = 465 - 6;
  int_.teambox_x = 575;
  int_.teambox_y = 330;
  int_.status_x = 575;
  int_.status_y = 421;
  int_.ping_x = 600;
  int_.ping_y = 18;
  int_.weapon_x = 285;
  int_.weapon_y = 454;

  relinfo.healthbar_rel_x = int_.healthico_x;
  relinfo.healthbar_rel_y = int_.healthico_y;
  relinfo.jetbar_rel_x = int_.jetico_x;
  relinfo.jetbar_rel_y = int_.jetico_y;
  relinfo.ammobar_rel_x = int_.ammoico_x;
  relinfo.ammobar_rel_y = int_.ammoico_y;
  relinfo.firebar_rel_x = int_.ammoico_x;
  relinfo.firebar_rel_y = int_.ammoico_y;
  relinfo.nadesbar_rel_x = int_.ammoico_x;
  relinfo.nadesbar_rel_y = int_.ammoico_y;
}

// result is true if it loaded a custom interface
auto loadinterfacedata(const std::string &interfacename) -> bool
{
  std::vector<std::uint8_t> addrfile;
  tinterface addrrec;
  const char custom_interface_dir[] = "custom-intercases/";

  auto& fs = GS::GetFileSystem();

  bool loadinterfacedata_result = false;
  intalign.weapon = 0;
  intalign.bullets = 0;
  intalign.healthbar = 1;
  intalign.ammobar = 1;
  intalign.reloadbar = 0;
  intalign.firebar = 1;
  intalign.jetbar = 1;
  intalign.vestbar = 1;

  // not registered / default interface
  if (isdefaultinterface(interfacename))
  {
    loaddefaultinterfacedata();
    return loadinterfacedata_result;
  }

  if (!fs.Exists(gGlobalStateClient.moddir + custom_interface_dir + interfacename + "/setup.sif"))
  {
    showmessage(("Could not find setup.sif. Loading default interface instead."));
    loaddefaultinterfacedata();
    return loadinterfacedata_result;
  }

  loadinterfacedata_result = true;

  addrfile =
    fs.ReadFile(gGlobalStateClient.moddir + custom_interface_dir + interfacename + "/setup.sif");

  addrrec = *(pinterface)(addrfile.data());
  int_ = addrrec;

  if (fs.Exists(gGlobalStateClient.moddir + custom_interface_dir + interfacename + "/health.bmp"))
  {
    relinfo.healthbar_rel_x = int_.healthico_x;
    relinfo.healthbar_rel_y = int_.healthico_y;
    relinfo.jetbar_rel_x = int_.healthico_x;
    relinfo.jetbar_rel_y = int_.healthico_y;
    relinfo.ammobar_rel_x = int_.healthico_x;
    relinfo.ammobar_rel_y = int_.healthico_y;
    relinfo.firebar_rel_x = int_.healthico_x;
    relinfo.firebar_rel_y = int_.healthico_y;
    relinfo.nadesbar_rel_x = int_.healthico_x;
    relinfo.nadesbar_rel_y = int_.healthico_y;
  }

  if (fs.Exists(gGlobalStateClient.moddir + custom_interface_dir + interfacename + "/jet.bmp"))
  {
    relinfo.healthbar_rel_x = int_.jetico_x;
    relinfo.healthbar_rel_y = int_.jetico_y;
    relinfo.jetbar_rel_x = int_.jetico_x;
    relinfo.jetbar_rel_y = int_.jetico_y;
    relinfo.ammobar_rel_x = int_.jetico_x;
    relinfo.ammobar_rel_y = int_.jetico_y;
    relinfo.firebar_rel_x = int_.jetico_x;
    relinfo.firebar_rel_y = int_.jetico_y;
    relinfo.nadesbar_rel_x = int_.jetico_x;
    relinfo.nadesbar_rel_y = int_.jetico_y;
  }

  if (fs.Exists(gGlobalStateClient.moddir + custom_interface_dir + interfacename + "/ammo.bmp"))
  {
    relinfo.healthbar_rel_x = int_.ammoico_x;
    relinfo.healthbar_rel_y = int_.ammoico_y;
    relinfo.jetbar_rel_x = int_.ammoico_x;
    relinfo.jetbar_rel_y = int_.ammoico_y;
    relinfo.ammobar_rel_x = int_.ammoico_x;
    relinfo.ammobar_rel_y = int_.ammoico_y;
    relinfo.firebar_rel_x = int_.ammoico_x;
    relinfo.firebar_rel_y = int_.ammoico_y;
    relinfo.nadesbar_rel_x = int_.ammoico_x;
    relinfo.nadesbar_rel_y = int_.ammoico_y;
  }

  if (fs.Exists(gGlobalStateClient.moddir + custom_interface_dir + interfacename + "/health.bmp"))
  {
    relinfo.healthbar_rel_x = int_.healthico_x;
    relinfo.healthbar_rel_y = int_.healthico_y;
  }

  if (fs.Exists(gGlobalStateClient.moddir + custom_interface_dir + interfacename + "/jet.bmp"))
  {
    relinfo.jetbar_rel_x = int_.jetico_x;
    relinfo.jetbar_rel_y = int_.jetico_y;
  }

  if (fs.Exists(gGlobalStateClient.moddir + custom_interface_dir + interfacename + "/ammo.bmp"))
  {
    relinfo.ammobar_rel_x = int_.ammoico_x;
    relinfo.ammobar_rel_y = int_.ammoico_y;
  }
  return loadinterfacedata_result;
}

// Does the HUD overlay belong to the player?
auto isinteractiveinterface() -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  return sprite_system.GetPlayerSprite().isnotspectator() or
         ((gGlobalStateClient.camerafollowsprite > 0) && (CVar::sv_advancedspectator));
}

auto pixelalignx(float x) -> float { return pixelsize.x * floor(x / pixelsize.x); }

auto pixelaligny(float y) -> float { return pixelsize.y * floor(y / pixelsize.y); }

void drawline(float x, float y, float w, tgfxcolor color)
{
  float x0;
  float y0;
  float x1;
  float y1;

  x0 = pixelalignx(x);
  y0 = pixelaligny(y);
  x1 = pixelalignx(x0 + w);
  y1 = y0 + pixelsize.y;

  gfxdrawquad(nullptr, gfxvertex(x0, y0, 0, 0, color), gfxvertex(x1, y0, 0, 0, color),
              gfxvertex(x1, y1, 0, 0, color), gfxvertex(x0, y1, 0, 0, color));
}

auto tominimap(const tvector2 pos, float scale = 1) -> tvector2
{
  tvector2 result;
  worldtominimap(pos.x, pos.y, result.x, result.y);

  scale = scale * gGlobalStateGameRendering.textures[GFX::INTERFACE_SMALLDOT]->scale;

  result.x = pixelalignx(
    CVar::ui_minimap_posx * gGlobalStateInterfaceGraphics._rscala.x + result.x -
    (float)(scale * gGlobalStateGameRendering.textures[GFX::INTERFACE_SMALLDOT]->width) / 2);

  result.y = pixelaligny(
    CVar::ui_minimap_posy + result.y -
    (float)(scale * gGlobalStateGameRendering.textures[GFX::INTERFACE_SMALLDOT]->height) / 2);
  return result;
}

void renderbar(std::int32_t t, std::uint8_t postype, std::int32_t x, std::int32_t rx,
               std::int32_t y, std::int32_t ry, std::int32_t w, std::int32_t h, std::int32_t r,
               float p, bool leftalign = true)

{
  float px;
  float py;
  tgfxrect rc;

  if (postype == textstyle)
  {
    return;
  }

  p = max(0.f, min(1.f, p));
  w = gGlobalStateGameRendering.textures[t]->width;
  h = gGlobalStateGameRendering.textures[t]->height;

  px = pixelalignx(rx * gGlobalStateInterfaceGraphics._iscala.x) + (x - rx);
  py = pixelaligny(ry * gGlobalStateInterfaceGraphics._iscala.y) + (y - ry);
  rc.top = 0;
  rc.bottom = h;

  if (leftalign)
  {
    rc.left = 0;
    rc.right = w * p;

    if (postype == vertical)
    {
      rc.right = w;
      rc.top = h * (1 - p);
      py = py + rc.top * gGlobalStateGameRendering.textures[t]->scale;
    }
  }
  else
  {
    rc.left = w * (1 - p);
    rc.right = w;

    if (postype == vertical)
    {
      rc.left = 0;
      rc.bottom = h * p;
      py = py + (h * (1 - p)) * gGlobalStateGameRendering.textures[t]->scale;
    }
  }

  gfxdrawsprite(gGlobalStateGameRendering.textures[t], px, py, 0, 0, degtorad(r),
                rgba(0xffffff, int_.alpha), rc);
}

struct tattr
{
  float cur;
  float def;
  std::string des;
};

void getweaponattribs(std::int32_t i, std::vector<tattr> &attrs)
{
  auto &curgun = GS::GetWeaponSystem().GetGuns()[i];
  auto &defgun = GS::GetWeaponSystem().GetDefaultGuns()[i];

  attrs[0].cur =
    (curgun.hitmultiply * (curgun.modifierlegs + curgun.modifierchest + curgun.modifierhead)) / 3;
  attrs[0].def =
    (defgun.hitmultiply * (defgun.modifierlegs + defgun.modifierchest + defgun.modifierhead)) / 3;
  attrs[0].des = ("Damage");

  attrs[1].cur = curgun.ammo;
  attrs[1].def = defgun.ammo;
  attrs[1].des = ("Ammo");

  attrs[2].cur = curgun.reloadtime;
  attrs[2].def = defgun.reloadtime;
  attrs[2].des = ("Reload");

  attrs[3].cur = curgun.speed;
  attrs[3].def = defgun.speed;
  attrs[3].des = ("Speed");

  attrs[4].cur = curgun.fireinterval;
  attrs[4].def = defgun.fireinterval;
  attrs[4].des = ("Rate");

  attrs[5].cur = curgun.movementacc;
  attrs[5].def = defgun.movementacc;
  attrs[5].des = ("Acc.");

  attrs[6].cur = curgun.bink;
  attrs[6].def = defgun.bink;
  attrs[6].des = ("Bink");

  attrs[7].cur = curgun.startuptime;
  attrs[7].def = defgun.startuptime;
  attrs[7].des = ("Delay");

  attrs[8].cur = curgun.bulletspread;
  attrs[8].def = defgun.bulletspread;
  attrs[8].des = ("Spread");

  attrs[9].cur = curgun.recoil;
  attrs[9].def = defgun.recoil;
  attrs[9].des = ("Recoil");

  attrs[10].cur = curgun.push;
  attrs[10].def = defgun.push;
  attrs[10].des = ("Push");

  attrs[11].cur = curgun.bulletstyle;
  attrs[11].def = defgun.bulletstyle;
  attrs[11].des = ("Style");

  attrs[12].cur = curgun.inheritedvelocity;
  attrs[12].def = defgun.inheritedvelocity;
  attrs[12].des = ("Inh. Speed");
}

auto getweaponattribdesc(tattr &attr) -> std::string
{
  return string("    |-") + attr.des + " : " +
         (attr.def != 0 ? inttostr(round(attr.cur / attr.def * 100)) + "%" : "NEW") + " (" +
         formatfloat("0.####", attr.cur) + '/' + formatfloat("0.####", attr.def) + ')';
}

void renderweaponmenutext()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  std::int32_t cursoronindex;
  float x;
  float y;
  float tipy;
  struct tgamebutton *btn;
  std::vector<tattr> attrs{13};
  std::string str1;

  cursoronindex = 0;

  setfontstyle(font_small);
  gfxtextshadow(1, 1, rgba(0));

  gfxtextcolor(rgba(234, 234, 234, 255));
  gfxdrawtext(("Primary Weapon:"), 65, 142);

  gfxtextverticalalign(gfx_baseline);
  gfxtextcolor(rgba(214, 214, 214, 255));
  gfxdrawtext(("Secondary Weapon:"), 65, 349);

  if (GS::GetWeaponSystem().GetLoadedWMChecksum() != GS::GetWeaponSystem().GetDefaultWMChecksum())
  {
    gfxtextcolor(rgba(204, 94, 94, 205));
    gfxdrawtext(45 + 252 - rectwidth(gfxtextmetrics(("Weapons Mod"))), 139);
  }

  gfxtextverticalalign(gfx_top);

  for (i = low(gGlobalStateGameMenus.limbomenu->button);
       i <= high(gGlobalStateGameMenus.limbomenu->button); i++)
  {
    btn = &gGlobalStateGameMenus.limbomenu->button[i];

    if (btn->active)
    {
      if (btn == gGlobalStateGameMenus.hoveredbutton)
      {
        cursoronindex = i;
      }

      x = btn->x1 + 85;
      y = btn->y1 + (float)((btn->y2 - btn->y1)) / 2 - 2;

      gfxtextcolor(rgba(0xffffff, 230));

      if (((i + 1) == sprite_system.GetPlayerSprite().selweapon) ||
          ((i + 1) == 11 + CVar::cl_player_secwep))
      {
        if (btn == gGlobalStateGameMenus.hoveredbutton)
        {
          gfxtextcolor(rgba(85, 105, 55, 230));
        }
        else
        {
          gfxtextcolor(rgba(55, 165, 55, 230));
        }
      }
      else if (btn == gGlobalStateGameMenus.hoveredbutton)
      {
        x = x + 1;
        y = y - 1;
      }

      gfxdrawtext(btn->caption, x, y);
    }
  }

  setfontstyle(font_weapons_menu);

  if (cursoronindex >= 0)
  {
    btn = &gGlobalStateGameMenus.limbomenu->button[cursoronindex];
    x = btn->x1;
    tipy = btn->y1 + (float)((btn->y2 - btn->y1)) / 2;

    btn = &gGlobalStateGameMenus.limbomenu->button[min(9, cursoronindex)];
    y = btn->y1 + (float)((btn->y2 - btn->y1)) / 2;

    if (GS::GetWeaponSystem().GetLoadedWMChecksum() != GS::GetWeaponSystem().GetDefaultWMChecksum())
    {
      tipy = y - 26;

      getweaponattribs(cursoronindex + 1, attrs);
      gfxtextcolor(rgba(215, 215, 155, 230));

      str1 = gGlobalStateGameMenus.limbomenu->button[cursoronindex].caption + '\12' +
             ("   Settings      : change% (present/default)") + '\12';

      for (i = low(attrs); i <= high(attrs); i++)
      {
        if (attrs[i].cur != attrs[i].def)
        {
          str1 = str1 + getweaponattribdesc(attrs[i]);
        }

        str1 = str1 + '\12';
      }

      gfxdrawtext(str1, x + 245, y - 16);
    }

    if (CVar::cl_runs < 4)
    {
      switch (cursoronindex + 1)
      {
      case 8:
        str1 = ("Hold fire to shoot, inaccurate while moving");
        break;
      case 12:
        str1 = ("Can be thrown by holding throw weapon button");
        break;
      case 14:
        str1 = ("Hold fire to shoot, while crouching or prone");
        break;
      default:
        i = -1;
      }

      if (i != -1)
      {
        gfxtextcolor(rgba(225, 195, 195, 250));
        gfxdrawtext(str1, x + 245, tipy - 2);
      }
    }
  }
}

void renderescmenutext(float w, float h)
{
  std::int32_t i;
  float x;
  float y;
  float sx;
  float sy;
  float dx;
  float dy;
  struct tgamebutton *btn;

  sx = (float)(gGlobalStateGameMenus.escmenu->w) / background_width;
  sy = (float)(gGlobalStateGameMenus.escmenu->h) / background_width;

  dx = (w / 2 - ((float)(gGlobalStateGameMenus.escmenu->w) / 2)) - gGlobalStateGameMenus.escmenu->x;
  dy = (h / 2 - ((float)(gGlobalStateGameMenus.escmenu->h) / 2)) - gGlobalStateGameMenus.escmenu->y;

  gfxdrawsprite(gGlobalStateGameRendering.textures[GFX::INTERFACE_BACK],
                gGlobalStateGameMenus.escmenu->x + dx, gGlobalStateGameMenus.escmenu->y + dy, sx,
                sy, rgba(0xffffff, CVar::ui_status_transparency * 0.56));

  setfontstyle(font_small);
  gfxtextshadow(1, 1, rgba(0));

  gfxtextcolor(rgba(250, 245, 255, 240));
  gfxdrawtext(("ESC - return to game"), gGlobalStateGameMenus.escmenu->x + dx + 20,
              gGlobalStateGameMenus.escmenu->y + gGlobalStateGameMenus.escmenu->h + dy - 45);

  gfxtextcolor(rgba(230, 235, 255, 190));
  gfxtextverticalalign(gfx_bottom);
  NotImplemented("rendering", "No soldat version");
#if 0
    gfxdrawtext(escmenu->x + escmenu->w + dx - 2 -
                    rectwidth(gfxtextmetrics(string("Soldat ") + soldat_version)),
                escmenu->y + escmenu->h + dy);
#endif
  gfxtextverticalalign(gfx_top);

  setfontstyle(font_menu);
  gfxtextcolor(rgba(0xffffff, 250));
  for (i = low(gGlobalStateGameMenus.escmenu->button);
       i <= high(gGlobalStateGameMenus.escmenu->button); i++)
  {
    btn = &gGlobalStateGameMenus.escmenu->button[i];

    if (btn->active)
    {
      x = btn->x1 + dx + ord(btn == gGlobalStateGameMenus.hoveredbutton) + 10;
      y = btn->y1 + dy - ord(btn == gGlobalStateGameMenus.hoveredbutton) +
          (float)((btn->y2 - btn->y1)) / 2 - rectheight(gfxtextmetrics(btn->caption)) / 2;

      gfxdrawtext(x, y);
    }
  }
}

void renderteammenutext()
{
  std::int32_t i;
  std::uint8_t alpha;
  float x;
  float y;
  struct tgamebutton *btn;
  std::array<std::array<tgfxcolor, 2>, 6> colors;

  if (gGlobalStateInterfaceGraphics.fragsmenushow || gGlobalStateInterfaceGraphics.statsmenushow)
  {
    alpha = 80;
  }
  else
  {
    alpha = 255;
  }

  colors[0][0] = rgba(255, 255, 255, alpha); // DM, etc.
  colors[0][1] = rgba(255, 255, 255, 250);
  colors[1][0] = rgba(210, 15, 5, alpha); // Alpha
  colors[1][1] = rgba(210, 15, 5, 250);
  colors[2][0] = rgba(5, 15, 205, alpha); // Bravo
  colors[2][1] = rgba(5, 15, 205, 250);
  colors[3][0] = rgba(210, 210, 5, alpha); // Charlie
  colors[3][1] = rgba(210, 210, 5, 250);
  colors[4][0] = rgba(5, 210, 5, alpha); // Delta
  colors[4][1] = rgba(5, 210, 5, 250);
  colors[5][0] = rgba(210, 210, 105, alpha); // Spectator
  colors[5][1] = rgba(210, 210, 105, 250);

  setfontstyle(font_menu);
  gfxtextshadow(1, 1, rgba(0));
  gfxtextcolor(rgba(234, 234, 234, alpha));
  gfxdrawtext(("Select Team:"), 55, 165);

  for (i = low(gGlobalStateGameMenus.teammenu->button);
       i <= high(gGlobalStateGameMenus.teammenu->button); i++)
  {
    btn = &gGlobalStateGameMenus.teammenu->button[i];

    if (btn->active)
    {
      if (i == 2)
      {
        gfxtextshadow(1, 1, rgba(0x333333));
      }
      else
      {
        gfxtextshadow(1, 1, rgba(0));
      }

      gfxtextcolor(colors[i][ord(btn == gGlobalStateGameMenus.hoveredbutton)]);

      x = btn->x1 + 10 + ord(btn == gGlobalStateGameMenus.hoveredbutton);
      y = btn->y1 - ord(btn == gGlobalStateGameMenus.hoveredbutton) +
          (float)((btn->y2 - btn->y1)) / 2 - rectheight(gfxtextmetrics(btn->caption)) / 2;

      gfxdrawtext(x, y);

      x = 269 + ord(btn == gGlobalStateGameMenus.hoveredbutton);

      if ((i > 0) && (i < 5))
      {
        gfxdrawtext(string("(") + inttostr(GS::GetGame().GetPlayersTeamNum(i)) + ')', x, y);
      }
    }
  }
}

void renderkickwindowtext()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  float x;
  float y;
  struct tgamebutton *btn;

  gfxdrawsprite(gGlobalStateGameRendering.textures[GFX::INTERFACE_BACK],
                gGlobalStateGameMenus.kickmenu->x, gGlobalStateGameMenus.kickmenu->y,
                (float)(gGlobalStateGameMenus.kickmenu->w) / background_width,
                (float)(gGlobalStateGameMenus.kickmenu->h) / background_width,
                rgba(0xffffff, CVar::ui_status_transparency * 0.56));

  setfontstyle(font_menu);
  gfxtextshadow(1, 1, rgba(0));

  if ((gGlobalStateGameMenus.kickmenuindex > 0) &&
      (gGlobalStateGameMenus.kickmenuindex < max_sprites + 1))
  {
    auto &sprite = sprite_system.GetSprite(gGlobalStateGameMenus.kickmenuindex);
    if (sprite.IsActive())
    {
      btn = gGlobalStateGameMenus.kickmenu->button.data();
      gfxtextcolor(argb(sprite.player->shirtcolor));
      gfxdrawtext(sprite.player->name, btn->x1, btn->y1 - 15);
    }
  }

  gfxtextcolor(rgba(0xffffff, 250));

  for (i = low(gGlobalStateGameMenus.kickmenu->button);
       i <= high(gGlobalStateGameMenus.kickmenu->button); i++)
  {
    btn = &gGlobalStateGameMenus.kickmenu->button[i];

    if (btn->active)
    {
      x = btn->x1 + 10 + std::int32_t(btn == gGlobalStateGameMenus.hoveredbutton);
      y = btn->y1 + (float)((btn->y2 - btn->y1)) / 2 -
          std::int32_t(btn == gGlobalStateGameMenus.hoveredbutton) -
          rectheight(gfxtextmetrics(btn->caption)) / 2;

      gfxdrawtext(x, y);
    }
  }
}

void rendermapwindowtext()
{
  std::int32_t i;
  float x;
  float y;
  std::string str1;
  struct tgamebutton *btn;

  gfxdrawsprite(gGlobalStateGameRendering.textures[GFX::INTERFACE_BACK],
                gGlobalStateGameMenus.mapmenu->x, gGlobalStateGameMenus.mapmenu->y,
                (float)(gGlobalStateGameMenus.mapmenu->w) / background_width,
                (float)(gGlobalStateGameMenus.mapmenu->h) / background_width,
                rgba(0xffffff, CVar::ui_status_transparency * 0.56));

  str1 = gGlobalStateNetworkClient.votemapname;

  setfontstyle(font_menu);
  gfxtextshadow(1, 1, rgba(0));

  if (gGlobalStateGameMenus.mapmenuindex > -1)
  {
    btn = gGlobalStateGameMenus.mapmenu->button.data();
    gfxtextcolor(rgba(135, 235, 135, 230));
    gfxdrawtext(str1, btn->x1, btn->y1 - 15);
  }

  gfxtextcolor(rgba(0xffffff, 250));

  for (i = low(gGlobalStateGameMenus.mapmenu->button);
       i <= high(gGlobalStateGameMenus.mapmenu->button); i++)
  {
    btn = &gGlobalStateGameMenus.mapmenu->button[i];

    if (btn->active)
    {
      x = btn->x1 + 10 + std::int32_t(btn == gGlobalStateGameMenus.hoveredbutton);
      y = btn->y1 + (float)((btn->y2 - btn->y1)) / 2 -
          std::int32_t(btn == gGlobalStateGameMenus.hoveredbutton) -
          rectheight(gfxtextmetrics(btn->caption)) / 2;

      gfxdrawtext(x, y);
    }
  }
}

void rendergamemenutexts(float w, float h)
{
  if (gGlobalStateGameMenus.limbomenu->active)
  {
    renderweaponmenutext();
  }

  if (gGlobalStateGameMenus.escmenu->active)
  {
    renderescmenutext(w, h);
  }

  if (gGlobalStateGameMenus.teammenu->active)
  {
    renderteammenutext();
  }

  if (gGlobalStateGameMenus.kickmenu->active)
  {
    renderkickwindowtext();
  }

  if (gGlobalStateGameMenus.mapmenu->active)
  {
    rendermapwindowtext();
  }
}

void renderplayerinterfacetexts(std::int32_t playerindex)
{
  auto &sprite_system = SpriteSystem::Get();
  tsprite *me;
  std::int32_t i;
  std::int32_t pos;
  float x;
  float y;
  float t;
  std::string str1;

  me = &sprite_system.GetSprite(playerindex);

  auto &map = GS::GetGame().GetMap();

  if (!me->deadmeat)
  {
    setfontstyle(font_menu);
    gfxtextcolor(rgba(0xffffff, int_.alpha));

    // health
    if (int_.health && (int_.healthbar_pos == textstyle))
    {
      x = relinfo.healthbar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
          (int_.healthbar_x - relinfo.healthbar_rel_x);
      y = relinfo.healthbar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
          (int_.healthbar_y - relinfo.healthbar_rel_y);

      t = me->GetHealth() / GS::GetGame().GetStarthealth();
      gfxdrawtext(inttostr(trunc(t * 100)) + '%', x, y);
    }

    // weapon reload
    if (int_.ammo && (int_.ammobar_pos == textstyle) && (me->weapon.ammocount == 0) &&
        (me->weapon.num != spas12_num))
    {
      x = relinfo.ammobar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
          (int_.ammobar_x - relinfo.ammobar_rel_x);
      y = relinfo.ammobar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
          (int_.ammobar_y - relinfo.ammobar_rel_y);

      t = (float)(me->weapon.reloadtimecount) / me->weapon.reloadtime;
      gfxdrawtext(inttostr(trunc(100 - t * 100)) + '%', x, y);
    }

    // jet
    if (int_.jet && (int_.jetbar_pos == textstyle) && (map.startjet > 0))
    {
      x = relinfo.jetbar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
          (int_.jetbar_x - relinfo.jetbar_rel_x);
      y = relinfo.jetbar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
          (int_.jetbar_y - relinfo.jetbar_rel_y);

      t = (float)(me->jetscount) / map.startjet;
      gfxdrawtext(inttostr(trunc(t * 100)) + '%', x, y);
    }

    // vest
    if (int_.vest && (int_.vestbar_pos == textstyle) && (me->vest > 0))
    {
      x = relinfo.healthbar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
          (int_.vestbar_x - relinfo.healthbar_rel_x);
      y = relinfo.healthbar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
          (int_.vestbar_y - relinfo.healthbar_rel_y);

      t = me->vest / defaultvest;
      gfxdrawtext(inttostr(trunc(t * 100)) + '%', x, y);
    }

    // nades
    if (int_.nades && (int_.nades_pos == textstyle))
    {
      x = relinfo.nadesbar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
          (int_.nades_x - relinfo.nadesbar_rel_x);
      y = relinfo.nadesbar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
          (int_.nades_y - relinfo.nadesbar_rel_y);

      gfxdrawtext(inttostr(me->tertiaryweapon.ammocount), x, y);
    }

    // bullets
    if (int_.bullets)
    {
      x = relinfo.ammobar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
          (int_.bullets_x - relinfo.ammobar_rel_x);
      y = relinfo.ammobar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
          (int_.bullets_y - relinfo.ammobar_rel_y);

      gfxtextcolor(rgba(242, 244, 40, int_.alpha));

      if (intalign.bullets == 1)
      {
        gfxdrawtext(x - rectwidth(gfxtextmetrics((inttostr(me->weapon.ammocount)))), y);
      }
      else
      {
        gfxdrawtext(inttostr(me->weapon.ammocount), x, y);
      }
    }

    // weapon
    if (int_.weapon &&
        isdoubleweaponindex(weaponnumtoindex(me->weapon.num, GS::GetWeaponSystem().GetGuns())))
    {
      x = relinfo.ammobar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
          (int_.weapon_x - relinfo.ammobar_rel_x);
      y = relinfo.ammobar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
          (int_.weapon_y - relinfo.ammobar_rel_y);

      setfontstyle(font_weapons_menu);
      gfxtextcolor(rgba(255, 245, 177, int_.alpha));

      if (intalign.weapon == 1)
      {
        gfxdrawtext(
          x - rectwidth(gfxtextmetrics((gGlobalStateClient.gundisplayname[me->weapon.num]))), y);
      }
      else
      {
        gfxdrawtext(gGlobalStateClient.gundisplayname[me->weapon.num], x, y);
      }
    }
  } // not DeadMeat

  // kills
  if (int_.status)
  {
    setfontstyle(font_small);

    x = int_.status_x * gGlobalStateInterfaceGraphics._iscala.x;
    y = int_.status_y * gGlobalStateInterfaceGraphics._iscala.y;

    pos = 0;

    for (i = 1; i <= GS::GetGame().GetPlayersNum(); i++)
    {
      if (GS::GetGame().GetSortedPlayers(i).playernum == playerindex)
      {
        pos = i;
        break;
      }
    }

    if ((pos > 0) && (pos <= (GS::GetGame().GetPlayersNum() - GS::GetGame().GetSpectatorsNum())))
    {
      str1 = (inttostr(pos)) + '/' +
             (inttostr(GS::GetGame().GetPlayersNum() - GS::GetGame().GetSpectatorsNum()));
      gfxtextcolor(rgba(88, 255, 90, int_.alpha));
      gfxdrawtext(str1, x, y);
    }

    gfxtextcolor(rgba(255, 55, 50, int_.alpha));

    if ((pos == 1) && ((GS::GetGame().GetPlayersNum() - GS::GetGame().GetSpectatorsNum()) > 1))
    {
      i = me->player->kills - GS::GetGame().GetSortedPlayers(2).kills;
      str1 = (iif(i > 0, "+", ""));
      str1 = wideformat("{} ({}{})", me->player->kills, str1, i);
      gfxdrawtext(str1, x, y + 10);
    }
    else
    {
      i = me->player->kills - GS::GetGame().GetSortedPlayers(1).kills;
      str1 = (inttostr(me->player->kills)) + " (" + (inttostr(i)) + ')';
      gfxdrawtext(str1, x, y + 10);
    }

    gfxtextcolor(rgba(114, 120, 255, int_.alpha));
    gfxdrawtext(inttostr(CVar::sv_killlimit), x, y + 20);
  }

  // bonus
  str1 = "";

  switch (me->bonusstyle)
  {
  case bonus_flamegod:
    str1 = ("Flame God");
    break;
  case bonus_predator:
    str1 = ("Predator");
    break;
  case bonus_berserker:
    str1 = ("Berserker");
    break;
  }

  if (!str1.empty())
  {
    str1 = str1 + " - " + (floattostrf((float)(me->bonustime) / 60, fffixed, 7, 1));

    setfontstyle(font_menu);
    gfxtextcolor(rgba(245, 40, 50));
    gfxdrawtext(str1, 190 * gGlobalStateInterfaceGraphics._iscala.x,
                435 * gGlobalStateInterfaceGraphics._iscala.y);
  }
}

void renderteamscoretexts()
{
  std::int32_t i;
  std::int32_t teamcount;
  std::int32_t spacing;
  float x;
  float y;

  setfontstyle(font_menu);

  x = int_.teambox_x * gGlobalStateInterfaceGraphics._iscala.x + 2;
  y = int_.teambox_y * gGlobalStateInterfaceGraphics._iscala.y + 25;

  teamcount = 2;
  spacing = 40;

  if (CVar::sv_gamemode == gamestyle_teammatch)
  {
    teamcount = 4;
    spacing = 24;
    y = y - 25;
  }

  if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf) ||
      (CVar::sv_gamemode == gamestyle_htf) || (CVar::sv_gamemode == gamestyle_teammatch))
  {
    for (i = 1; i <= teamcount; i++)
    {
      gfxtextcolor(argb(gGlobalStateGame.sortedteamscore[i].color));
      gfxdrawtext(inttostr(gGlobalStateGame.sortedteamscore[i].kills), x, y + spacing * (i - 1));
    }
  }
}

void renderendgametexts(float fragmenubottom)
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  float y;

  i = GS::GetGame().GetSortedPlayers(1).playernum;

  if (GS::GetGame().isteamgame())
  {
    gfxtextverticalalign(gfx_bottom);

    y = fragmenubottom + gGlobalStateInterfaceGraphics.fragy -
        (gGlobalStateInterfaceGraphics.fragsscrolllev * 20);

    if (gGlobalStateGame.sortedteamscore[1].kills == gGlobalStateGame.sortedteamscore[2].kills)
    {
      // tie
      gfxtextcolor(rgba(245, 245, 245));
      gfxdrawtext(("It's a tie"), gGlobalStateInterfaceGraphics.fragx + 137, y);
    }
    else
    {
      // team wins
      i = gGlobalStateGame.sortedteamscore[1].playernum;

      switch (i)
      {
      case 1:
        gfxtextcolor(rgba(210, 15, 5));
        break;
      case 2:
        gfxtextcolor(rgba(5, 15, 205));
        break;
      case 3:
        gfxtextcolor(rgba(210, 210, 5));
        break;
      case 4:
        gfxtextcolor(rgba(5, 210, 5));
        break;
      }

      switch (i)
      {
      case 1:
        gfxdrawtext(("Alpha team wins"), gGlobalStateInterfaceGraphics.fragx + 50, y);
        break;
      case 2:
        gfxdrawtext(("Bravo team wins"), gGlobalStateInterfaceGraphics.fragx + 50, y);
        break;
      case 3:
        gfxdrawtext(("Charlie team wins"), gGlobalStateInterfaceGraphics.fragx + 50, y);
        break;
      case 4:
        gfxdrawtext(("Delta team wins"), gGlobalStateInterfaceGraphics.fragx + 50, y);
        break;
      }
    }

    gfxtextverticalalign(gfx_top);
  }
  else if (sprite_system.GetSprite(i).player->kills > 0)
  {
    // player wins
    gfxtextcolor(rgba(185, 250, 138));
    gfxdrawtext(wideformat(("{} wins"), sprite_system.GetSprite(i).player->name),
                gGlobalStateInterfaceGraphics.fragx + 107,
                gGlobalStateInterfaceGraphics.fragy + 24);
  }
}

void renderweaponstatstexts()
{
  std::int32_t i;
  std::int32_t j;
  struct tweaponstat *stat;

  setfontstyle(font_small);
  gfxtextcolor(rgba(170, 160, 200, 230));
  gfxdrawtext(("% = Accuracy"), gGlobalStateInterfaceGraphics.fragx + 465,
              gGlobalStateInterfaceGraphics.fragy + 15); // % = Accuracy
  gfxdrawtext(("HS = Headshots"), gGlobalStateInterfaceGraphics.fragx + 465,
              gGlobalStateInterfaceGraphics.fragy + 25); // HS = Headshots

  setfontstyle(font_menu);
  gfxtextcolor(rgba(255, 255, 230));
  gfxdrawtext(("Weapon:"), gGlobalStateInterfaceGraphics.fragx + 70,
              gGlobalStateInterfaceGraphics.fragy + 40); // Weapon:
  gfxdrawtext((" %"), gGlobalStateInterfaceGraphics.fragx + 240,
              gGlobalStateInterfaceGraphics.fragy + 40); // %
  gfxdrawtext(("Shots:"), gGlobalStateInterfaceGraphics.fragx + 290,
              gGlobalStateInterfaceGraphics.fragy + 40); // Shots:
  gfxdrawtext(("Hits:"), gGlobalStateInterfaceGraphics.fragx + 390,
              gGlobalStateInterfaceGraphics.fragy + 40); // Hits:
  gfxdrawtext(("Kills (HS):"), gGlobalStateInterfaceGraphics.fragx + 470,
              gGlobalStateInterfaceGraphics.fragy + 40); // Kills (HS):

  setfontstyle(font_small);

  j = 0;
  gfxtextcolor(rgba(0xffffff));

  for (i = 1; i <= 20; i++)
  {
    stat = &gGlobalStateClient.wepstats[i];

    if (stat->shots > 0)
    {
      j += 1;

      gfxdrawtext(stat->name, gGlobalStateInterfaceGraphics.fragx + 90,
                  gGlobalStateInterfaceGraphics.fragy + j * 20 + 50);
      gfxdrawtext(inttostr(round((float)((stat->hits * 100)) / stat->shots)) + '%',
                  gGlobalStateInterfaceGraphics.fragx + 245,
                  gGlobalStateInterfaceGraphics.fragy + j * 20 + 50);
      gfxdrawtext(numberformat(stat->shots), gGlobalStateInterfaceGraphics.fragx + 295,
                  gGlobalStateInterfaceGraphics.fragy + j * 20 + 50);
      gfxdrawtext(numberformat(stat->hits), gGlobalStateInterfaceGraphics.fragx + 395,
                  gGlobalStateInterfaceGraphics.fragy + j * 20 + 50);
      gfxdrawtext(inttostr(stat->kills) + " (" + inttostr(stat->headshots) + ')',
                  gGlobalStateInterfaceGraphics.fragx + 475,
                  gGlobalStateInterfaceGraphics.fragy + j * 20 + 50);
    }
  }

  gfxtextcolor(rgba(255, 255, 230, 100));
  gfxdrawtext(("(Updated every 10 seconds)"), gGlobalStateInterfaceGraphics.fragx + 230,
              gGlobalStateInterfaceGraphics.fragy + (j + 1) * 20 + 50);
}

void renderfragsmenutexts(float fragmenubottom)
{
  auto &sprite_system = SpriteSystem::Get();
  tgfxcolor color;
  float x;
  float y;
  float py;
  std::int32_t i;
  std::int32_t j;
  std::int32_t k;
  std::int32_t z;
  std::int32_t nextitemstep;
  std::array<std::int32_t, 6> ids;
  std::array<std::int32_t, 6> totalteamkills;
  std::array<tvector2, 6> lines;
  std::string str1;

  x = gGlobalStateInterfaceGraphics.fragx;
  y = gGlobalStateInterfaceGraphics.fragy - (gGlobalStateInterfaceGraphics.fragsscrolllev * 20);

  // team lines
  if (GS::GetGame().isteamgame())
  {
    nextitemstep = 0;
    k = 0;

    for (j = 1; j <= 6; j++)
    {
      // iteration order for i: 1,2,3,4,0,5
      // i=5 means spectators
      // i=0 means TEAM_NONE but goes after float teams

      i = iif(j < 6, j % 5, 5);
      if ((CVar::ui_hidespectators) && (i == 5))
      {
        z = 0;
      }
      else if (i == 5)
      {
        z = GS::GetGame().GetSpectatorsNum();
      }
      else
      {
        z = GS::GetGame().GetTeamplayersnum(i);
      }

      if (z > 0)
      {
        lines[i].x = x + 35;
        lines[i].y = y + 50 + nextitemstep + k * fragsmenu_player_height;
        nextitemstep += 20;
        k += z;

        switch (i)
        {
        case 0:
          color = argb(enter_message_color);
          break;
        case 1:
          color = rgba(255, 0, 0);
          break;
        case 2:
          color = rgba(0, 0, 255);
          break;
        case 3:
          color = argb(charliej_message_color);
          break;
        case 4:
          color = argb(deltaj_message_color);
          break;
        case 5:
          color = rgba(129, 52, 118);
          break;
        }

        drawline(lines[i].x, lines[i].y + 15, 565, color);
      }
    }
  }
  else
  {
    lines[0].y = y + 40 + fragsmenu_player_height;
    lines[5].y = y + 40 +
                 (GS::GetGame().GetPlayersNum() - GS::GetGame().GetSpectatorsNum() + 1) *
                   fragsmenu_player_height;
  }

  // columns
  if ((CVar::sv_gamemode == gamestyle_deathmatch) || (CVar::sv_gamemode == gamestyle_teammatch))
  {
    str1 = ("Kills:");
  }
  else
  {
    str1 = ("Points:");
  }

  j = iif(length(str1) > 7, 80, 0);

  setfontstyle(font_menu);
  gfxtextcolor(rgba(255, 255, 230));
  gfxdrawtext(str1, x + 280 - j, y + 40);
  gfxdrawtext(("Deaths:"), x + 390, y + 40);
  gfxdrawtext(("Ping:"), x + 530, y + 40);

  // server name
  setfontstyle(font_small_bold);
  gfxtextcolor(rgba(233, 180, 12));
  gfxdrawtext(CVar::sv_hostname, x + 30, y + 15);

  // time left && computer time
  str1 = wideformat("{} {}:{}", ("Time"), GS::GetGame().GetTimeleftmin(),
                    GS::GetGame().GetTimeleftsec());
  setfontstyle(font_small);
  gfxtextcolor(rgba(170, 160, 200, 230));
  gfxdrawtext(str1, x + 485, y + 15);
  NotImplemented("rendering", "No time");
#if 0
    gfxdrawtext(formatdatetime("h:nn:ss ampm", get_time()), x + 485, y + 30);
#endif

  // server info message
  gfxtextcolor(rgba(200, 150, 0));
  gfxdrawtext(CVar::sv_info, x + 30, y + 30);

  // demo name
  if (GS::GetDemoRecorder().active())
  {
    gfxtextcolor(rgba(0, 128, 0, fabs(round(sin(GS::GetGame().GetSinusCounter() / 2) * 255))));
    gfxdrawtext(GS::GetDemoRecorder().name(), x + 280, y + fragmenubottom - 10);
  }

  // players count
  gfxtextcolor(rgba(200, 190, 180, 240));
  gfxdrawtext(("Players"), x + 330, y + 15);

  if (GS::GetGame().isteamgame())
  {
    j = iif(CVar::sv_gamemode == gamestyle_teammatch, 4, 2);

    for (i = 1; i <= j; i++)
    {
      switch (i)
      {
      case 1:
        gfxtextcolor(rgba(233, 0, 0, 240));
        break;
      case 2:
        gfxtextcolor(rgba(0, 0, 233, 240));
        break;
      case 3:
        gfxtextcolor(rgba(233, 233, 0, 240));
        break;
      case 4:
        gfxtextcolor(rgba(0, 233, 0, 240));
        break;
      }

      gfxdrawtext(inttostr(GS::GetGame().GetTeamplayersnum(i)), x + 440 + 20 * ((i - 1) / 2),
                  y + 10 + 10 * ((i - 1) % 2));
    }
  }
  else
  {
    gfxdrawtext(inttostr(GS::GetGame().GetPlayersNum()), x + 450, y + 15);
  }

  // players
  fillchar(ids.data(), sizeof(ids), 0);
  fillchar(totalteamkills.data(), sizeof(totalteamkills), 0);

  for (j = 1; j <= GS::GetGame().GetPlayersNum(); j++)
  {
    k = 0;
    i = GS::GetGame().GetSortedPlayers(j).playernum;

    if (i <= 0)
    {
      continue;
    }

    if (CVar::ui_hidespectators && sprite_system.GetSprite(i).isspectator())
    {
      continue;
    }

    if (sprite_system.GetSprite(i).isspectator())
    {
      k = 5;
    }
    else if (GS::GetGame().isteamgame())
    {
      k = sprite_system.GetSprite(i).player->team;
    }

    if (k == 5)
    {
      gfxtextcolor(rgba(220, 50, 200, 113));
    }
    else
    {
      gfxtextcolor(rgba(sprite_system.GetSprite(i).player->shirtcolor, 255));
    }

    py = lines[k].y + 20 + fragsmenu_player_height * ids[k];

    gfxdrawtext(sprite_system.GetSprite(i).player->name, x + 44, py);
    gfxdrawtext(inttostr(sprite_system.GetSprite(i).player->kills), x + 284, py);
    gfxdrawtext(inttostr(sprite_system.GetSprite(i).player->deaths), x + 394, py);

    if (sprite_system.GetSprite(i).player->flags > 0)
    {
      gfxdrawtext(string("x") + inttostr(sprite_system.GetSprite(i).player->flags), x + 348, py);
    }

    if ((sprite_system.GetSprite(i).player->jetcolor & 0xff000000) != color_transparency_bot)
    {
      gfxdrawtext(inttostr(sprite_system.GetSprite(i).player->realping), x + 534, py);
    }

    if ((!gGlobalStateClientGame.chattext.empty()) && (gGlobalStateClientGame.chattext[1] == '/'))
    {
      gfxtextcolor(rgba(245, 255, 230, 155));
      gfxdrawtext(x + 20 - rectwidth(gfxtextmetrics((inttostr(sprite_system.GetSprite(i).num)))),
                  py);
    }

    ids[k] += 1;
    totalteamkills[k] += sprite_system.GetSprite(i).player->kills;
  }

  // team captions
  if (GS::GetGame().isteamgame())
  {
    for (j = 1; j <= 6; j++)
    {
      // see above ("team lines") to know what's up with this
      i = iif(j < 6, j % 5, 5);
      if (i == 5)
      {
        z = GS::GetGame().GetSpectatorsNum();
      }
      else
      {
        z = GS::GetGame().GetTeamplayersnum(i);
      }

      if (z > 0)
      {
        switch (i)
        {
        case 0:
          gfxtextcolor(argb(enter_message_color));
          break;
        case 1:
          gfxtextcolor(rgba(255, 0, 0));
          break;
        case 2:
          gfxtextcolor(rgba(0, 0, 255));
          break;
        case 3:
          gfxtextcolor(argb(charliej_message_color));
          break;
        case 4:
          gfxtextcolor(argb(deltaj_message_color));
          break;
        case 5:
          gfxtextcolor(rgba(129, 52, 118));
          break;
        }

        setfontstyle(font_small_bold);

        switch (i)
        {
        case 0:
          gfxdrawtext(("Player"), lines[i].x, lines[i].y);
          break;
        case 1:
          gfxdrawtext(("Alpha"), lines[i].x, lines[i].y);
          break;
        case 2:
          gfxdrawtext(("Bravo"), lines[i].x, lines[i].y);
          break;
        case 3:
          gfxdrawtext(("Charlie"), lines[i].x, lines[i].y);
          break;
        case 4:
          gfxdrawtext(("Delta"), lines[i].x, lines[i].y);
          break;
        case 5:
          gfxdrawtext(("Spectator"), lines[i].x, lines[i].y);
          break;
        }

        if (j < 5)
        {
          switch (i)
          {
          case 1:
            gfxtextcolor(rgba(0xd20f05, 0xdd));
            break;
          case 2:
            gfxtextcolor(rgba(0x151fd9, 0xdd));
            break;
          case 3:
            gfxtextcolor(rgba(0xd2d205, 0xdd));
            break;
          case 4:
            gfxtextcolor(rgba(0x5d205, 0xdd));
            break;
          }

          setfontstyle(font_small);
          gfxdrawtext(inttostr(totalteamkills[i]), x + 284, lines[i].y + 3);
        }
      }
    }
  }
}

void renderconsoletexts(float w)
{
  auto *console =
    !gGlobalStateClientGame.chattext.empty() ? &GetBigConsole() : &GS::GetMainConsole();

  setfontstyle(font_small);

  float l = CVar::font_consolelineheight * pixelsize.y * fontstylesize(font_small);
  std::uint8_t alpha = 255;
  bool tiny = false;

  if (gGlobalStateInterfaceGraphics.fragsmenushow || gGlobalStateInterfaceGraphics.statsmenushow or
      gGlobalStateGameMenus.teammenu->active or
      ((console == &GetBigConsole()) && gGlobalStateGameMenus.limbomenu->active) or
      ((console == &GS::GetMainConsole()) && gGlobalStateGameMenus.escmenu->active &&
       gGlobalStateInterfaceGraphics.noobshow))
  {
    alpha = 60;
  }

  for (std::int32_t i = 1; i <= console->GetCount(); i++)
  {
    if (console->GetTextMessage(i).empty())
    {
      continue;
    }

    gfxtextcolor(rgba(console->GetTextMessageColor(i), alpha));

    if ((rectwidth(gfxtextmetrics(console->GetTextMessage(i))) > (w - 10)) != tiny)
    {
      tiny = !tiny;

      if (tiny)
      {
        setfontstyle(font_smallest);
      }
      else
      {
        setfontstyle(font_small);
      }

      gfxdrawtext(console->GetTextMessage(i), 5, 1 + (i - 1) * l);
    }
    else
    {
      gfxdrawtext(5, 1 + (i - 1) * l);
    }
  }
}

void renderkillconsoletexts(float w)
{
  std::uint8_t alpha = 245;
  bool tiny = false;
  float dy = 0;

  setfontstyle(font_weapons_menu);

  if (w < 1024)
  {
    if (gGlobalStateInterfaceGraphics.fragsmenushow || gGlobalStateInterfaceGraphics.statsmenushow)
    {
      alpha = 80;
    }

    if (length(gGlobalStateClientGame.chattext) > 0)
    {
      alpha = 180;
    }
  }

  for (std::int32_t i = 1; i <= GetKillConsole().GetCount(); i++)
  {
    if (GetKillConsole().GetTextMessage(i).empty())
    {
      continue;
    }

    if (GetKillConsole().GetNumMessage(i) > -255)
    {
      dy = dy + killconsole_separate_height;
    }

    if ((length(GetKillConsole().GetTextMessage(i)) > 14) != tiny)
    {
      tiny = !tiny;

      if (tiny)
      {
        setfontstyle(font_smallest);
      }
      else
      {
        setfontstyle(font_weapons_menu);
      }
    }

    float x = 595 * gGlobalStateInterfaceGraphics._iscala.x -
              rectwidth(gfxtextmetrics(GetKillConsole().GetTextMessage(i)));
    float y = 60 + (i - 1) * (CVar::font_weaponmenusize + 2) + dy;

    gfxtextcolor(rgba(GetKillConsole().GetTextMessageColor(i), alpha));
    gfxdrawtext(x, y);
  }
}

void renderchattexts()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  float x;
  float y;
  float dx;
  float dy;
  bool hide;
  std::string str1;

  setfontstyle(font_small);
  gfxtextverticalalign(gfx_bottom);

  for (i = 1; i <= max_sprites; i++)
  {
    if ((!sprite_system.GetSprite(i).typing) && (gGlobalStateInterfaceGraphics.chatdelay[i] <= 0))
    {
      continue;
    }

    hide = (CVar::sv_realisticmode) && (sprite_system.GetSprite(i).visible == 0) and
           sprite_system.GetPlayerSprite().isnotinsameteam(sprite_system.GetSprite(i)) &&
           sprite_system.GetPlayerSprite().isnotspectator();

    x = (sprite_system.GetSprite(i).skeleton.pos[12].x - gGlobalStateClient.camerax +
         0.5 * gGlobalStateGame.gamewidth) *
        gGlobalStateInterfaceGraphics._rscala.x;
    y = (sprite_system.GetSprite(i).skeleton.pos[12].y - gGlobalStateClient.cameray +
         0.5 * gGlobalStateGame.gameheight) *
        gGlobalStateInterfaceGraphics._rscala.y;
    dy = -25 * gGlobalStateInterfaceGraphics._rscala.y;

    if (sprite_system.GetSprite(i).typing && !hide)
    {
      dx = (-rectwidth(gfxtextmetrics("..."))) / 2;
      NotImplemented("rendering");
#if 0
            str1 =  (copy("...", 1, GS::GetGame().GetMainTickCounter() / 30 % 3 + 1));
#endif

      gfxtextcolor(rgba(abovechat_message_color));
      gfxdrawtext(str1, x + dx, y + dy);

      dy = dy - 15 * gGlobalStateInterfaceGraphics._rscala.y;
    }

    if ((gGlobalStateInterfaceGraphics.chatdelay[i] > 0) &&
        (length(gGlobalStateInterfaceGraphics.chatmessage[i]) < morechattext) &&
        !(hide && gGlobalStateInterfaceGraphics.chatteam[i]))
    {
      dx = (-rectwidth(gfxtextmetrics(gGlobalStateInterfaceGraphics.chatmessage[i]))) / 2;

      gfxtextcolor(rgba(abovechat_message_color,
                        max(0, min(255, 9 * gGlobalStateInterfaceGraphics.chatdelay[i]))));
      gfxdrawtext(x + dx, y + dy);
    }
  }

  gfxtextverticalalign(gfx_top);
}

void renderchatinput(float w, float h, double t)
{
  std::string str1;
  tgfxrect rc;
  std::string strhalf;
  float x;
  float y;

  if (gGlobalStateClientGame.chattype == msgtype_pub)
  {
    str1 = "Say:";
  }
  else if (gGlobalStateClientGame.chattype == msgtype_team)
  {
    str1 = "Team Say:";
  }
  else if (gGlobalStateClientGame.chattype == msgtype_cmd)
  {
    str1 = "Cmd:";
  }

  if (!str1.empty())
  {
    setfontstyle(font_small);

    if (gGlobalStateClientGame.chattype == msgtype_pub)
    {
      gfxtextcolor(rgba(chat_message_color));
    }
    else if (gGlobalStateClientGame.chattype == msgtype_team)
    {
      gfxtextcolor(rgba(teamchat_message_color));
    }
    else if (gGlobalStateClientGame.chattype == msgtype_cmd)
    {
      gfxtextcolor(rgba(enter_message_color));
    }

    if (gGlobalStateClientGame.chatchanged)
    {
      chatinputtime = t;
      gGlobalStateClientGame.chatchanged = false;
    }

    t = t - chatinputtime;

    str1 =
      str1 + iif(gGlobalStateClientGame.chattext[length(gGlobalStateClientGame.chattext)] == ' ',
                 gGlobalStateClientGame.chattext + ' ', gGlobalStateClientGame.chattext);
    rc = gfxtextmetrics(str1);

    if (rectwidth(rc) >= (w - 80))
    {
      setfontstyle(font_smallest);
    }

    gfxtextverticalalign(gfx_baseline);
    gfxdrawtext(str1, 5, 420 * gGlobalStateInterfaceGraphics._iscala.y);
    strhalf = str1;

    if (gGlobalStateClientGame.cursorposition < length(gGlobalStateClientGame.chattext))
    {
      strhalf.resize(gGlobalStateClientGame.cursorposition + 4 +
                     ord(gGlobalStateClientGame.chattext[1] == '/'));
    }

    x = rectwidth(gfxtextmetrics(strhalf));

    if ((t - floor(t)) <= 0.5)
    {
      x = pixelalignx(5 + x) + 2 * pixelsize.x;
      y = pixelaligny(420 * gGlobalStateInterfaceGraphics._iscala.y - 1.2 * rectheight(rc));
      w = pixelsize.x;
      h = pixelaligny(1.2 * 1.2 * rectheight(rc));

      gfxdrawquad(nullptr, gfxvertex(x + 0, y + 0, 0, 0, rgba(255, 230, 170)),
                  gfxvertex(x + w, y + 0, 0, 0, rgba(255, 230, 170)),
                  gfxvertex(x + w, y + h, 0, 0, rgba(255, 230, 170)),
                  gfxvertex(x + 0, y + h, 0, 0, rgba(255, 230, 170)));
    }

    gfxtextverticalalign(gfx_top);
  }
}

void renderrespawnandsurvivaltexts()
{
  auto &sprite_system = SpriteSystem::Get();
  tsprite *me;
  float p;
  std::string str1;

  me = &sprite_system.GetPlayerSprite();

  auto &game = GS::GetGame();

  if (me->isnotspectator())
  {
    if (me->deadmeat || game.GetSurvivalEndRound())
    {
      gfxdrawsprite(gGlobalStateGameRendering.textures[GFX::INTERFACE_BACK],
                    180 * gGlobalStateInterfaceGraphics._iscala.x,
                    gGlobalStateInterfaceGraphics._iscala.y, (float)(300) / background_width,
                    (float)(22) / background_width,
                    rgba(0xffffff, CVar::ui_status_transparency * 0.56));
    }

    if ((!CVar::sv_survivalmode) && (me->respawncounter > 0))
    {
      p = (float)(me->respawncounter) / 60;
      str1 = wideformat(("Respawn in... {}"), floattostrf(p, fffixed, 7, 1));
      gfxtextcolor(rgba(255, 65, 55));
    }
    else if ((CVar::sv_survivalmode) && me->deadmeat && !game.GetSurvivalEndRound())
    {
      gfxtextcolor(rgba(115, 255, 100));

      if (!GS::GetGame().isteamgame())
      {
        str1 = (inttostr(game.GetAlivenum())) + ' ' + ("players left");
      }
      else
      {
        str1 = (inttostr(game.GetTeamAliveNum(me->player->team))) + ' ' + ("team players left");
      }
    }
    else if (game.GetSurvivalEndRound())
    {
      if (me->deadmeat)
      {
        p = (float)(me->respawncounter) / 60;
        str1 = ("End of round...") + (floattostrf(p, fffixed, 7, 1));
        gfxtextcolor(rgba(115, 255, 100));
      }
      else
      {
        str1 = ("You have survived");
        gfxtextcolor(rgba(155, 245, 100));
      }
    }

    setfontstyle(font_menu);
    gfxdrawtext(str1, 200 * gGlobalStateInterfaceGraphics._iscala.x,
                4 * gGlobalStateInterfaceGraphics._iscala.y);
  }
  else if (game.GetSurvivalEndRound())
  {
    setfontstyle(font_menu);
    gfxtextcolor(rgba(115, 255, 100));
    gfxdrawtext(("End of round..."), 240 * gGlobalStateInterfaceGraphics._iscala.x,
                400 * gGlobalStateInterfaceGraphics._iscala.y);
  }
}

void renderradiomenutexts()
{
  std::set<std::int32_t> radio_gamestyles = {gamestyle_ctf, gamestyle_inf, gamestyle_htf};
  std::string s;
  std::uint8_t alpha;
  float sx;
  float sy;
  std::array<tgfxcolor, 2> color;

  if (!(radio_gamestyles.contains(CVar::sv_gamemode)))
  {
    return;
  }

  color[0] = rgba(0xffffff, CVar::ui_status_transparency * 0.56);
  sx = (float)(180) / background_width;
  sy = (float)(80) / background_width;

  gfxdrawsprite(gGlobalStateGameRendering.textures[GFX::INTERFACE_BACK], 5, 250, sx, sy, color[0]);

  if (gGlobalStateClient.rmenustate[0] != ' ')
  {
    gfxdrawsprite(gGlobalStateGameRendering.textures[GFX::INTERFACE_BACK], 185, 250, sx, sy,
                  color[0]);
  }

  alpha =
    iif(gGlobalStateInterfaceGraphics.fragsmenushow || gGlobalStateInterfaceGraphics.statsmenushow,
        80, 230);

  setfontstyle(font_menu);
  gfxtextcolor(rgba(0xffffff, alpha));
  gfxdrawtext("Radio:", 10, 252);

  setfontstyle(font_small);
  color[0] = rgba(200, 200, 200, alpha);
  color[1] = rgba(210, 210, 5, alpha);

  gfxtextcolor(color[(std::int32_t)(gGlobalStateClient.rmenustate[0] == '1')]);
  gfxdrawtext(string("1: ") + gGlobalStateClient.radiomenu["Menu1EFC"], 10, 270);
  gfxtextcolor(color[(std::int32_t)(gGlobalStateClient.rmenustate[0] == '2')]);
  gfxdrawtext(string("2: ") + gGlobalStateClient.radiomenu["Menu1FFC"], 10, 282);
  gfxtextcolor(color[(std::int32_t)(gGlobalStateClient.rmenustate[0] == '3')]);
  gfxdrawtext(string("3: ") + gGlobalStateClient.radiomenu["Menu1ES"], 10, 294);

  if (gGlobalStateClient.rmenustate[0] != ' ')
  {
    NotImplemented("rendering", "rmenustate to int?");
#if 0
        s = choose(strtoint(rmenustate[0]) - 1, {"EFC", "FFC", "ES"});
#endif
    gfxtextcolor(color[0]);
    gfxdrawtext(string("1: ") + gGlobalStateClient.radiomenu[string("Menu2") + s + "U"], 190, 270);
    gfxdrawtext(string("2: ") + gGlobalStateClient.radiomenu[string("Menu2") + s + 'M'], 190, 282);
    gfxdrawtext(string("3: ") + gGlobalStateClient.radiomenu[string("Menu2") + s + 'D'], 190, 294);
  }
}

void rendervotemenutexts()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  float x;
  float y;
  std::array<std::string, 2> str1;

  if (GS::GetGame().IsVoteActive())
  {
    setfontstyle(font_weapons_menu);

    x = 45 * gGlobalStateInterfaceGraphics._iscala.x;
    y = 400 * gGlobalStateInterfaceGraphics._iscala.y;
    if (GS::GetGame().GetVoteType() == vote_kick)
    {
      str1[0] = ("Kick");
    }
    else
    {
      str1[0] = ("Map");
    }

    str1[1] = (GS::GetGame().GetVoteTarget());

    if (GS::GetGame().GetVoteType() == vote_kick)
    {
      i = strtoint(GS::GetGame().GetVoteTarget());

      if ((i > 0) && (i <= max_sprites))
      {
        str1[1] = (sprite_system.GetSprite(i).player->name);
      }
    }

    gfxtextcolor(rgba(254, 104, 104, 225));
    gfxdrawtext(str1[0], x + 30, y);

    gfxtextcolor(rgba(244, 244, 244, 225));
    gfxdrawtext(str1[1], x + 65, y);

    gfxtextcolor(rgba(224, 218, 244, 205));
    gfxdrawtext(("Voter: ") + (GS::GetGame().GetVoteStarter()), x + 10, y + 11);
    gfxdrawtext(("Reason: ") + (GS::GetGame().GetVoteReason()), x + 10, y + 20);

    gfxtextcolor(rgba(234, 234, 114, 205));
    gfxdrawtext(("F12 - Yes   F11 - No"), x + 50, y + 31);
  }

  if (gGlobalStateControlGame.votekickreasontype)
  {
    setfontstyle(font_small);
    gfxtextcolor(rgba(254, 124, 124));
    gfxdrawtext(("Type reason for vote:"), 5, 390 * gGlobalStateInterfaceGraphics._iscala.y);
  }
}

void renderplayername(float width, float height, std::int32_t i, bool onlyoffscreen)
{
  auto &sprite_system = SpriteSystem::Get();
  std::uint8_t alpha;
  tgfxrect rc;
  float x;
  float y;
  float w;
  float h;
  float dx;
  float dy;
  auto &things = GS::GetThingSystem().GetThings();

  dy = iif(onlyoffscreen, -10, 5) + 15;
  rc = gfxtextmetrics((sprite_system.GetSprite(i).player->name));
  w = rectwidth(rc);
  h = rectheight(rc);
  x = (sprite_system.GetSprite(i).skeleton.pos[7].x - gGlobalStateClient.camerax +
       0.5 * gGlobalStateGame.gamewidth) *
      gGlobalStateInterfaceGraphics._rscala.x;
  y = (sprite_system.GetSprite(i).skeleton.pos[7].y - gGlobalStateClient.cameray +
       0.5 * gGlobalStateGame.gameheight + dy) *
      gGlobalStateInterfaceGraphics._rscala.y;

  if (!onlyoffscreen || (x < 0) || (x > width) || (y < 0) || (y > height))
  {
    x = max(0.f, min(width - w, x - w / 2));
    y = max(0.f, min(height - h, y - (float)((std::int32_t)(!onlyoffscreen) * h) / 2));

    dx = fabs(sprite_system.GetPlayerSprite().skeleton.pos[7].x -
              sprite_system.GetSprite(i).skeleton.pos[7].x);
    dy = fabs(sprite_system.GetPlayerSprite().skeleton.pos[7].y -
              sprite_system.GetSprite(i).skeleton.pos[7].y);

    alpha = min(255.0, 50.0 + round(100000 / (dx + dy / 2)));

    if ((sprite_system.GetSprite(i).holdedthing > 0) &&
        (things[sprite_system.GetSprite(i).holdedthing].style < 4))
    {
      gfxtextcolor(rgba(outofscreenflag_message_color, alpha));
    }
    else if (sprite_system.GetSprite(i).deadmeat)
    {
      gfxtextcolor(rgba(outofscreendead_message_color, alpha));
    }
    else
    {
      gfxtextcolor(rgba(outofscreen_message_color, alpha));
    }

    gfxdrawtext(x, y);
  }
}

void renderplayernames(float width, float height)
{
  auto &sprite_system = SpriteSystem::Get();
  setfontstyle(font_weapons_menu);

  if (sprite_system.GetPlayerSprite().isspectator())
  {
    if (!gGlobalStateDemo.demoplayer.active())
    {
      for (auto &sprite : sprite_system.GetActiveSprites())
      {
        if (sprite.isinteam() and (!(CVar::sv_realisticmode) || (sprite.visible != 0)))
        {
          renderplayername(width, height, sprite.num, false);
        }
      }
    }
  }
  else if (sprite_system.GetPlayerSprite().isinteam())
  {
    for (auto &sprite : sprite_system.GetActiveSprites())
    {
      if ((!sprite_system.IsPlayerSprite(sprite.num)) &&
          sprite.isinsameteam(sprite_system.GetPlayerSprite()))
      {
        renderplayername(width, height, sprite.num, true);
      }
    }
  }
}

void renderceasefirecounter()
{
  auto &sprite_system = SpriteSystem::Get();
  float x;
  float y;

  auto &sprite = sprite_system.GetPlayerSprite();
  x = sprite.skeleton.pos[9].x - 2;
  y = sprite.skeleton.pos[9].y - 15;
  x = (x - gGlobalStateClient.camerax + 0.5 * gGlobalStateGame.gamewidth) *
      gGlobalStateInterfaceGraphics._rscala.x;
  y = (y - gGlobalStateClient.cameray + 0.5 * gGlobalStateGame.gameheight) *
      gGlobalStateInterfaceGraphics._rscala.y;

  setfontstyle(font_small);
  gfxtextcolor(rgba(game_message_color));
  gfxdrawtext(inttostr(sprite.ceasefirecounter / 60 + 1), x, y);
}

void renderactionsnaptext(double t)
{
  std::string str1;

  str1 = string("[[ ") + ("Press F4 to Save Screen Cap") + " ]]     [[ " + ("Press F5 to Cancel") +
         " ]]";

  setfontstyle(font_small);
  gfxtextcolor(rgba(230, 65, 60, 150 + fabs(round(sin(5.1 * t) * 100))));
  gfxdrawtext(str1, 30 * gGlobalStateInterfaceGraphics._iscala.x,
              412 * gGlobalStateInterfaceGraphics._iscala.y);
}

void renderinterface(float timeelapsed, float width, float height)
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  std::int32_t j;
  std::int32_t k;
  std::int32_t z;
  std::int32_t l;
  std::int32_t l2;
  std::int32_t spectnumber;
  tvector2 p;
  tvector2 _scala;
  float scale;
  float maxsize;
  float roto;
  float dx;
  float dy;
  float x;
  float y;
  float fragmenubottom;
  float inaccuracy;
  tvector2 cursorsize;
  float cursorscale;
  tvector2 cursorscaledoffset;
  float cursorbinkscale;
  // CursorBinkOffset: TVector2;
  std::int32_t cursorcolor;
  std::int32_t alfa;
  tvector2 characteroffset;
  tvector2 indicatoroffset;
  std::uint64_t dotcolor;
  std::int32_t f1;
  std::int32_t f2;
  std::array<std::int32_t, 6> ids;
  std::uint64_t nextitemstep;
  std::array<std::uint64_t, 6> teamposstep; // = (0,0,0,0,0,0);
  float moveacc;
  tsprite *spriteme;
  tgfxcolor color;
  tgun *weapon;
  pgfxsprite spr;
  bool widescreencut;
  std::string str1;
  NotImplemented("network");
#if 0
    steamnetworkingquickconnectionstatus networkstats;
#endif
  auto &things = GS::GetThingSystem().GetThings();

  spriteme = nullptr;
  tgfxspritearray &t = gGlobalStateGameRendering.textures;

  pixelsize.x = width / gGlobalStateClientGame.renderwidth;
  pixelsize.y = height / gGlobalStateClientGame.renderheight;

  gfxtextpixelratio(pixelsize);

  /*$IF DEFINED(TESTING) or DEFINED(RELEASE_CANDIDATE)*/
  setfontstyle(font_small);
  gfxtextcolor(rgba(250, 245, 255, 150));
  NotImplemented("rendering", "soldat_version_long");
#if 0
    gfxdrawtext(soldat_version_long, 565 * _iscala.x, 465 * _iscala.y);
#endif
  /*#include "fend.pas"*/

  auto &map = GS::GetGame().GetMap();

  if (sprite_system.IsPlayerSpriteValid())
  {
    spriteme = &sprite_system.GetPlayerSprite();
  }

  widescreencut = (CVar::sv_bullettime) && (gGlobalStateClient.notexts == 0) &&
                  (GS::GetGame().GetGoalTicks() < default_goalticks) &&
                  (GS::GetGame().GetMapchangecounter() < 0);

  // Big messages
  if (gGlobalStateClient.notexts == 0)
  {
    maxsize = 0.8 * npot(gGlobalStateClientGame.renderheight / 2);

    for (i = 0; i < max_big_messages; i++)
    {
      if (gGlobalStateInterfaceGraphics.bigdelay[i] > 0)
      {
        dy = 0;
        alfa = (std::uint32_t)gGlobalStateInterfaceGraphics.bigcolor[i] >> 24;
        // somebody might have defined color as $RRGGBB not $AARRGGBB
        // effectively leaving AA component 0
        if (alfa == 0)
        {
          alfa = 255;
        }
        alfa = max(min(3 * gGlobalStateInterfaceGraphics.bigdelay[i] + 25, alfa), 0);
        scale = gGlobalStateInterfaceGraphics.bigscale[i] *
                ((float)(gGlobalStateClientGame.renderheight) / 480) * 4.8;

        if (scale * fontstylesize(font_big) > maxsize)
        {
          y = scale;
          scale = maxsize / fontstylesize(font_big);
          gfxtextscale(y / scale);
        }

        if (i == 1)
        {
          gfxtextverticalalign(gfx_baseline);

          if (widescreencut)
          {
            dy = -30 * gGlobalStateInterfaceGraphics._iscala.y;
          }
        }

        setfontstyle(font_big, scale);
        gfxtextcolor(rgba(gGlobalStateInterfaceGraphics.bigcolor[i], alfa));
        gfxtextshadow(1, 1, rgba(0, power((float)(alfa) / 255, 4) * alfa));
        gfxdrawtext(gGlobalStateInterfaceGraphics.bigtext[i],
                    gGlobalStateInterfaceGraphics.bigposx[i],
                    gGlobalStateInterfaceGraphics.bigposy[i] + dy);
        gfxtextverticalalign(gfx_top);
        gfxtextscale(1);
      }

      if (gGlobalStateInterfaceGraphics.worlddelay[i] > 0)
      {
        alfa = (unsigned long)gGlobalStateInterfaceGraphics.worldcolor[i] >> 24;
        // somebody might have defined color as $RRGGBB not $AARRGGBB
        // effectively leaving AA component 0
        if (alfa == 0)
        {
          alfa = 255;
        }
        alfa = max(min(3 * gGlobalStateInterfaceGraphics.worlddelay[i] + 25, alfa), 0);
        x = gGlobalStateInterfaceGraphics.worldposx[i] +
            (0.5 * gGlobalStateGame.gamewidth - gGlobalStateClient.camerax) *
              gGlobalStateInterfaceGraphics._rscala.x;
        y = gGlobalStateInterfaceGraphics.worldposy[i] +
            (0.5 * gGlobalStateGame.gameheight - gGlobalStateClient.cameray) *
              gGlobalStateInterfaceGraphics._rscala.y;

        scale = gGlobalStateInterfaceGraphics.worldscale[i];

        if (gGlobalStateInterfaceGraphics.worldscale[i] * fontstylesize(font_world) > maxsize)
        {
          scale = maxsize / fontstylesize(font_world);
        }

        if (scale != gGlobalStateInterfaceGraphics.worldscale[i])
        {
          gfxtextscale((float)(gGlobalStateInterfaceGraphics.worldscale[i]) / scale);
        }

        setfontstyle(font_world, scale);
        gfxtextcolor(rgba(gGlobalStateInterfaceGraphics.worldcolor[i], alfa));
        gfxtextshadow(1, 1, rgba(0, power((float)(alfa) / 255, 4) * alfa));
        gfxdrawtext(gGlobalStateInterfaceGraphics.worldtext[i], x, y);
        gfxtextscale(1);
      }
    }
  }

  if ((sprite_system.IsPlayerSpriteValid()) && (gGlobalStateClient.notexts == 0))
  {
    if (sprite_system.GetPlayerSprite().isspectator() &&
        (gGlobalStateClient.camerafollowsprite > 0) and (CVar::sv_advancedspectator))
    {
      spectnumber = gGlobalStateClient.mysprite;
      gGlobalStateClient.mysprite = gGlobalStateClient.camerafollowsprite;
      spriteme = &sprite_system.GetPlayerSprite();
    }

    // Bonus all colored
    if (CVar::ui_bonuscolors)
    {
      color.color.a = 0;

      switch (spriteme->bonusstyle)
      {
      case bonus_flamegod:
        color = rgba(0xffff00, 62);
        break;
      case bonus_predator:
        color = rgba(0xfe00dc, 82);
        break;
      case bonus_berserker:
        color = rgba(0xfe0000, 82);
        break;
      }

      if (color.color.a > 0)
      {
        _scala.x = t[GFX::INTERFACE_OVERLAY]->scale;
        _scala.y = t[GFX::INTERFACE_OVERLAY]->scale;
        _scala.x = width / (t[GFX::INTERFACE_OVERLAY]->width * _scala.x);
        _scala.y = height / (t[GFX::INTERFACE_OVERLAY]->height * _scala.y);
        gfxdrawsprite(t[GFX::INTERFACE_OVERLAY], 0, 0, _scala.x, _scala.y, color);
      }
    }

    color = rgba(0xffffff, int_.alpha);
    weapon = &spriteme->weapon;

    if (isinteractiveinterface())
    {
      if (int_.health)
      {
        x = pixelalignx(int_.healthico_x * gGlobalStateInterfaceGraphics._iscala.x);
        y = pixelaligny(int_.healthico_y * gGlobalStateInterfaceGraphics._iscala.y);

        gfxdrawsprite(t[GFX::INTERFACE_HEALTH], x, y, 0, 0, degtorad(int_.healthico_rotate), color);

        renderbar(GFX::INTERFACE_HEALTH_BAR, int_.healthbar_pos, int_.healthbar_x,
                  relinfo.healthbar_rel_x, int_.healthbar_y, relinfo.healthbar_rel_y,
                  int_.healthbar_width, int_.healthbar_height, int_.healthbar_rotate,
                  spriteme->GetHealth() / GS::GetGame().GetStarthealth(), intalign.healthbar == 0);
      }

      if (int_.vest && (spriteme->vest > 0))
      {
        renderbar(GFX::INTERFACE_VEST_BAR, int_.vestbar_pos, int_.vestbar_x,
                  relinfo.healthbar_rel_x, int_.vestbar_y, relinfo.healthbar_rel_y,
                  int_.vestbar_width, int_.vestbar_height, int_.vestbar_rotate,
                  spriteme->vest / defaultvest, intalign.vestbar == 0);
      }

      if (int_.ammo)
      {
        x = pixelalignx(int_.ammoico_x * gGlobalStateInterfaceGraphics._iscala.x);
        y = pixelaligny(int_.ammoico_y * gGlobalStateInterfaceGraphics._iscala.y);

        gfxdrawsprite(t[GFX::INTERFACE_AMMO], x, y, 0, 0, degtorad(int_.ammoico_rotate), color);

        if ((weapon->ammocount == 0) && (weapon->num != spas12_num))
        {
          renderbar(GFX::INTERFACE_RELOAD_BAR, int_.ammobar_pos, int_.ammobar_x,
                    relinfo.ammobar_rel_x, int_.ammobar_y, relinfo.ammobar_rel_y,
                    int_.ammobar_width, int_.ammobar_height, int_.ammobar_rotate,
                    1 - weapon->reloadtimefloat / weapon->reloadtime, intalign.reloadbar == 0);
        }
        else if (weapon->ammocount > 0)
        {
          renderbar(GFX::INTERFACE_RELOAD_BAR, int_.ammobar_pos, int_.ammobar_x,
                    relinfo.ammobar_rel_x, int_.ammobar_y, relinfo.ammobar_rel_y,
                    int_.ammobar_width, int_.ammobar_height, int_.ammobar_rotate,
                    (float)(weapon->ammocount) / weapon->ammo, intalign.ammobar == 0);
        }
      }

      if (int_.fire)
      {
        x = pixelalignx(relinfo.firebar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
                        (int_.firebar_x - relinfo.firebar_rel_x));

        y = pixelaligny(relinfo.firebar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
                        (int_.firebar_y - relinfo.firebar_rel_y));

        gfxdrawsprite(t[GFX::INTERFACE_FIRE_BAR_R], x, y, 0, 0, degtorad(int_.firebar_rotate),
                      color);

        renderbar(GFX::INTERFACE_FIRE_BAR, int_.firebar_pos, int_.fireico_x, relinfo.firebar_rel_x,
                  int_.fireico_y, relinfo.firebar_rel_y, int_.firebar_width, int_.firebar_height,
                  int_.fireico_rotate, weapon->fireintervalfloat / weapon->fireinterval,
                  intalign.firebar == 0);
      }

      if (int_.jet)
      {
        x = pixelalignx(int_.jetico_x * gGlobalStateInterfaceGraphics._iscala.x);
        y = pixelaligny(int_.jetico_y * gGlobalStateInterfaceGraphics._iscala.y);

        gfxdrawsprite(t[GFX::INTERFACE_JET], x, y, 0, 0, degtorad(int_.jetico_rotate), color);

        if (map.startjet > 0)
        {
          renderbar(GFX::INTERFACE_JET_BAR, int_.jetbar_pos, int_.jetbar_x, relinfo.jetbar_rel_x,
                    int_.jetbar_y, relinfo.jetbar_rel_y, int_.jetbar_width, int_.jetbar_height,
                    int_.jetbar_rotate, spriteme->jetscountfloat / map.startjet,
                    intalign.jetbar == 0);
        }
      }

      if (int_.nades && (int_.nades_pos != textstyle))
      {
        i = 0;

        if (spriteme->tertiaryweapon.num == fraggrenade_num)
        {
          i = GFX::INTERFACE_NADE;
        }
        else if (spriteme->tertiaryweapon.num == clustergrenade_num)
        {
          i = GFX::INTERFACE_CLUSTER_NADE;
        }

        if (i != 0)
        {
          dx = t[i]->width * t[i]->scale;
          dy = t[i]->height * t[i]->scale;

          for (j = 1; j <= spriteme->tertiaryweapon.ammocount; j++)
          {
            if (int_.nades_pos == horizontal)
            {
              x = pixelalignx(relinfo.nadesbar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
                              dx * j + (int_.nades_x - relinfo.nadesbar_rel_x));
              y = pixelaligny(relinfo.nadesbar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
                              (int_.nades_y - relinfo.nadesbar_rel_y));
            }
            else if (int_.nades_pos == vertical)
            {
              x = pixelalignx(relinfo.nadesbar_rel_x * gGlobalStateInterfaceGraphics._iscala.x +
                              (int_.nades_x - relinfo.nadesbar_rel_x));
              y = pixelaligny(relinfo.nadesbar_rel_y * gGlobalStateInterfaceGraphics._iscala.y +
                              (int_.nades_y - relinfo.nadesbar_rel_y) - dy * j + dy * 6);
            }

            gfxdrawsprite(t[i], x, y, color);
          }
        }
      }
    }

    // Aim cursor
    if (!(gGlobalStateGameMenus.limbomenu->active || gGlobalStateGameMenus.teammenu->active ||
          gGlobalStateGameMenus.escmenu->active) &&
        !spriteme->deadmeat &&
        ((GS::GetGame().GetMapchangecounter() < 0) ||
         (GS::GetGame().GetMapchangecounter() == 999999999)) &&
        !(gGlobalStateDemo.demoplayer.active() && (!CVar::demo_showcrosshair)) &&
        ((spectnumber <= 0) || (spectnumber > 32) ||
         !(!sprite_system.GetSprite(spectnumber).player->demoplayer)))
    {
      // Set base scale for the crosshair
      if (gGlobalStateClient.sniperline_client_hpp == 1)
      {
        cursorscale = 0.5;
      }
      else
      {
        cursorscale = 1;
      }

      cursorsize.x = t[GFX::INTERFACE_CURSOR]->width * t[GFX::INTERFACE_CURSOR]->scale;
      cursorsize.y = t[GFX::INTERFACE_CURSOR]->height * t[GFX::INTERFACE_CURSOR]->scale;

      // Base crosshair offset. Larger offset if interface scaling is enabled
      cursorscaledoffset.x =
        (float)(cursorsize.x) / 2 * cursorscale / gGlobalStateInterfaceGraphics._rscala.x;
      cursorscaledoffset.y =
        (float)(cursorsize.y) / 2 * cursorscale / gGlobalStateInterfaceGraphics._rscala.y;

      moveacc = spriteme->getmoveacc();
      inaccuracy = gGlobalStateClient.hitspraycounter + moveacc * 100;

      // Embiggen the crosshair when binked, keeping it centered
      if (inaccuracy > 0)
      {
        cursorbinkscale = power(inaccuracy, 0.6) / 20 * cursorscale;
        // CursorBinkOffset.x := CursorSize.x / 2 * CursorBinkScale / _rscala.x;
        // CursorBinkOffset.y := CursorSize.y / 2 * CursorBinkScale / _rscala.y;
        // TODO: Finish
        cursorscale = cursorscale + cursorbinkscale;
      }

      // Color and alpha for crosshair
      if (gGlobalStateInterfaceGraphics.cursortextlength > 0)
      {
        alfa = CVar::ui_status_transparency - 50;

        if (gGlobalStateInterfaceGraphics.cursorfriendly)
        {
          cursorcolor = 0x33ff33;
        }
        else
        {
          cursorcolor = 0xff3333;
        }
      }
      else
      {
        if (gGlobalStateClient.sniperline_client_hpp == 1)
        {
          alfa = CVar::ui_status_transparency / 2;
        }
        else
        {
          alfa = CVar::ui_status_transparency;
        }

        cursorcolor = 0xffffff;
      }

      if (gGlobalStateClient.sniperline_client_hpp == 1)
      {
        characteroffset.x = gGlobalStateGame.gamewidthhalf - gGlobalStateClient.camerax +
                            spriteme->skeleton.pos[15].x;
        characteroffset.y = gGlobalStateGame.gameheighthalf - gGlobalStateClient.cameray +
                            spriteme->skeleton.pos[15].y;

        roto = vec2length(vec2subtract(
          vector2(gGlobalStateClientGame.mx, gGlobalStateClientGame.my), characteroffset));

        if (roto < 1200)
        {
          x = pixelalignx(characteroffset.x * gGlobalStateInterfaceGraphics._rscala.x);
          y = pixelaligny(characteroffset.y * gGlobalStateInterfaceGraphics._rscala.y);

          gfxdrawsprite(
            t[GFX::INTERFACE_SIGHT], x - 1, y - 1,
            (roto / 240) * gGlobalStateInterfaceGraphics._rscala.x,
            (roto / 480) * gGlobalStateInterfaceGraphics._rscala.y, 1, 1,
            -angle2points(
              vector2(x, y),
              vector2(gGlobalStateClientGame.mx * gGlobalStateInterfaceGraphics._rscala.x,
                      gGlobalStateClientGame.my * gGlobalStateInterfaceGraphics._rscala.y)),
            rgba(0xffffff, round((roto / 240) * gGlobalStateInterfaceGraphics._rscala.x * 32)));
        }
      }

      if (!gGlobalStateDemo.demoplayer.active())
      {
        x = pixelalignx((gGlobalStateClientGame.mx - cursorscaledoffset.x) *
                        gGlobalStateInterfaceGraphics._rscala.x);
        y = pixelaligny((gGlobalStateClientGame.my - cursorscaledoffset.y) *
                        gGlobalStateInterfaceGraphics._rscala.y);
      }
      else
      {
        x = pixelalignx((gGlobalStateGame.gamewidthhalf - gGlobalStateClient.camerax) *
                          gGlobalStateInterfaceGraphics._rscala.x +
                        spriteme->control.mouseaimx);
        y = pixelaligny((gGlobalStateGame.gameheighthalf - gGlobalStateClient.cameray) *
                          gGlobalStateInterfaceGraphics._rscala.y +
                        spriteme->control.mouseaimy);
      }

      gfxdrawsprite(t[GFX::INTERFACE_CURSOR], x, y, cursorscale, rgba(cursorcolor, alfa));

#ifdef DEBUGCURSORS
      for (auto &sprite : SpriteSystem::Get().GetActiveSprites)
      {
        if (!(sprite.deadmeat))
        {
          x = (gamewidthhalf - camerax) * _rscala.x + sprite.control.mouseaimx;
          y = (gameheighthalf - cameray) * _rscala.y + sprite.control.mouseaimy;

          gfxdrawsprite(t[GFX::INTERFACE_CURSOR], x, y, _rscala.x, _rscala.y, rgba(0xff00ff, alfa));
        }
      }
#endif
    }

    // Player indicator
    if (CVar::ui_playerindicator && spriteme->isnotspectator())
    {
      characteroffset.x =
        gGlobalStateGame.gamewidthhalf - gGlobalStateClient.camerax + spriteme->skeleton.pos[12].x;
      characteroffset.y =
        gGlobalStateGame.gameheighthalf - gGlobalStateClient.cameray + spriteme->skeleton.pos[12].y;

      x = t[GFX::INTERFACE_ARROW]->width * t[GFX::INTERFACE_ARROW]->scale;
      y = t[GFX::INTERFACE_ARROW]->height * t[GFX::INTERFACE_ARROW]->scale;

      indicatoroffset.x = x / 2 / gGlobalStateInterfaceGraphics._rscala.x;
      indicatoroffset.y = y / 2 / gGlobalStateInterfaceGraphics._rscala.y;

      x = characteroffset.x - indicatoroffset.x;
      y = characteroffset.y - indicatoroffset.y - 15;

      if ((spriteme->alpha < 255) && (!CVar::sv_survivalmode))
      {
        alfa = spriteme->ceasefirecounter * 2 + 75;
      }
      else
      {
        alfa = 100;
        y = y + 2 * sin(5.1 * timeelapsed);
      }

      x = x * gGlobalStateInterfaceGraphics._rscala.x;
      y = y * gGlobalStateInterfaceGraphics._rscala.y;

      gfxdrawsprite(t[GFX::INTERFACE_ARROW], x, y, rgba(0xffffff, alfa));
    }

    if ((spectnumber > 0) && (spectnumber <= 32) &&
        sprite_system.GetSprite(spectnumber).isspectator() && (CVar::sv_advancedspectator))
    {
      gGlobalStateClient.mysprite = spectnumber;
      spriteme = &sprite_system.GetPlayerSprite();
    }

    // Ping dot
    if (int_.ping && gGlobalStateInterfaceGraphics.playernamesshow)
    {
      x = int_.ping_x * gGlobalStateInterfaceGraphics._iscala.x;
      y = int_.ping_y * gGlobalStateInterfaceGraphics._iscala.y;
      _scala.x = 0.5 + (float)(spriteme->player->realping) / 600;
      _scala.y = 0.45 + (float)(spriteme->player->realping) / 600;

      if (spriteme->player->realping <= 50)
      {
        dotcolor = 0xff00;
      }
      else if (spriteme->player->realping <= 100)
      {
        dotcolor = 0x22ff00;
      }
      else if (spriteme->player->realping <= 150)
      {
        dotcolor = 0x54c700;
      }
      else if (spriteme->player->realping <= 200)
      {
        dotcolor = 0x76a700;
      }
      else if (spriteme->player->realping <= 250)
      {
        dotcolor = 0x938800;
      }
      else if (spriteme->player->realping <= 300)
      {
        dotcolor = 0xa17700;
      }
      else if (spriteme->player->realping <= 350)
      {
        dotcolor = 0xcc4800;
      }
      else
      {
        dotcolor = 0xff0000;
      }

      if (spriteme->player->realping > 255)
      {
        alfa = 255;
      }
      else
      {
        alfa = spriteme->player->realping;
      }

      gfxdrawsprite(t[GFX::INTERFACE_DOT], x, y, _scala.x, _scala.y, rgba(dotcolor, alfa));
    }
  } // (MySprite > 0) and (NoTexts = 0)

  // Kill console weapons
  if (CVar::ui_killconsole)
  {
    if (gGlobalStateClient.notexts == 0)
    {
      alfa = 255;
      if (gGlobalStateClientGame.renderwidth < 1024)
      {
        if (gGlobalStateInterfaceGraphics.fragsmenushow ||
            gGlobalStateInterfaceGraphics.statsmenushow)
        {
          alfa = 50;
        }
        if (length(gGlobalStateClientGame.chattext) > 0)
        {
          alfa = 150;
        }
      }

      l2 = 0;
      for (j = 1; j <= GetKillConsole().GetCount(); j++)
      {
        if (GetKillConsole().GetTextMessage(j) > "")
        {
          if (GetKillConsole().GetNumMessage(j) > -255)
          {
            x = 605;
            y = (j - 1) * (CVar::font_weaponmenusize + 2) + 59;

            l2 = l2 + killconsole_separate_height;

            x = x * gGlobalStateInterfaceGraphics._iscala.x;
            y = y + l2;
            _scala.x = 0.8;
            _scala.y = 0.8;

            gfxdrawsprite(t[GetKillConsole().GetNumMessage(j)], x, y, _scala.x, _scala.y,
                          rgba(0xffffff, alfa));
          }
        }
      }
    }
  }

  // Minimap

  if (gGlobalStateInterfaceGraphics.minimapshow)
  {
    renderminimap(pixelalignx(CVar::ui_minimap_posx * gGlobalStateInterfaceGraphics._rscala.x),
                  pixelaligny(CVar::ui_minimap_posy), round(CVar::ui_status_transparency * 0.85));
  }

  if (gGlobalStateInterfaceGraphics.minimapshow && (!CVar::sv_minimap))
  {
    alfa = CVar::ui_minimap_transparency;

    f1 = GS::GetGame().GetTeamFlag(1);
    f2 = GS::GetGame().GetTeamFlag(2);

    if ((f1 > 0) && (f2 > 0) &&
        ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf)))
    {
      if ((things[f1].inbase) && (things[f1].holdingsprite == 0))
      {
        p = tominimap(things[f1].skeleton.pos[1]);
        gfxdrawsprite(t[GFX::INTERFACE_SMALLDOT], p.x, p.y, rgba(0xff0000, alfa));
      }

      if ((things[f2].inbase) && (things[f2].holdingsprite == 0))
      {
        p = tominimap(things[f2].skeleton.pos[1]);
        gfxdrawsprite(t[GFX::INTERFACE_SMALLDOT], p.x, p.y, rgba(0x1313ff, alfa));
      }
    }

    if ((f1 > 0) && (CVar::sv_gamemode == gamestyle_htf) && (things[f1].holdingsprite == 0))
    {
      p = tominimap(things[f1].skeleton.pos[1]);
      gfxdrawsprite(t[GFX::INTERFACE_SMALLDOT], p.x, p.y, rgba(0xffff00, alfa));
    }

    if ((gGlobalStateClient.gamethingtarget > 0) && (CVar::sv_gamemode == gamestyle_rambo) &&
        (things[gGlobalStateClient.gamethingtarget].holdingsprite == 0))
    {
      p = tominimap(things[gGlobalStateClient.gamethingtarget].skeleton.pos[1]);
      gfxdrawsprite(t[GFX::INTERFACE_SMALLDOT], p.x, p.y, rgba(0xffffff, alfa));
    }

    if (sprite_system.IsPlayerSpriteValid())
    {
      for (auto &sprite : sprite_system.GetActiveSprites())
      {
        if (sprite.isspectator() || (spriteme->isnotspectator() && !spriteme->isinsameteam(sprite)))
        {
          continue;
        }

        if ((sprite.holdedthing > 0) && (things[sprite.holdedthing].style < 4))
        {
          p = tominimap(sprite.skeleton.pos[7]);
          gfxdrawsprite(t[GFX::INTERFACE_SMALLDOT], p.x, p.y, rgba(0xffff00, alfa));
        }
        else if (((sprite.num == gGlobalStateClient.camerafollowsprite) &&
                  spriteme->isspectator()) or
                 (sprite_system.IsPlayerSprite(sprite.num)))
        {
          p = tominimap(sprite.skeleton.pos[7], 0.8);
          gfxdrawsprite(t[GFX::INTERFACE_SMALLDOT], p.x, p.y, 0.8, rgba(0xffffff, alfa));
        }
        else if (!sprite.issolo() && (!sprite_system.IsPlayerSprite(sprite.num)))
        {
          color = rgba(0);
          p = tominimap(sprite.skeleton.pos[7], 0.65);

          if (!sprite.deadmeat)
          {
            switch (sprite.player->team)
            {
            case team_alpha:
              color = rgba(0xff0000, alfa);
              break;
            case team_bravo:
              color = rgba(0x1313ff, alfa);
              break;
            case team_charlie:
              color = rgba(0xffff00, alfa);
              break;
            case team_delta:
              color = rgba(0xff00, alfa);
              break;
            }
          }

          gfxdrawsprite(t[GFX::INTERFACE_SMALLDOT], p.x, p.y, 0.65, color);

          // Chat indicator
          if (gGlobalStateInterfaceGraphics.chatdelay[sprite.num] > 0)
          {
            p.x = sprite.skeleton.pos[7].x;
            p.y = sprite.skeleton.pos[7].y - 40;
            p = tominimap(p, 0.5);
            gfxdrawsprite(t[GFX::INTERFACE_SMALLDOT], p.x, p.y, 0.5, rgba(0xffffff, alfa));
          }
        }
      }
    }
  }

  // Background for self Weapon Stats
  if (gGlobalStateInterfaceGraphics.statsmenushow && !gGlobalStateInterfaceGraphics.fragsmenushow)
  {
    _scala.x = (float)(590) / background_width;
    _scala.y = (float)(((gGlobalStateClient.wepstatsnum * 20) + 85)) / background_width;
    x = 25 + gGlobalStateInterfaceGraphics.fragx;
    y = 5 + gGlobalStateInterfaceGraphics.fragy;

    gfxdrawsprite(t[GFX::INTERFACE_BACK], x, y, _scala.x, _scala.y,
                  rgba(0xffffff, round(CVar::ui_status_transparency * 0.56)));

    z = 0;

    for (j = 1; j <= original_weapons; j++)
    {
      if (gGlobalStateClient.wepstats[j].shots > 0)
      {
        // Draw the weapons image
        z += 1;
        x = 30 + gGlobalStateInterfaceGraphics.fragx;
        y = ((z * 20) + 50) + gGlobalStateInterfaceGraphics.fragy;

        gfxdrawsprite(t[gGlobalStateClient.wepstats[j].textureid], x, y);
      }
    }

    gGlobalStateClient.wepstatsnum = z;
  }

  auto &game = GS::GetGame();

  // Background For Frags Stats
  if (gGlobalStateInterfaceGraphics.fragsmenushow)
  {
    if (gGlobalStateClient.notexts == 0)
    {
      x = 25 + gGlobalStateInterfaceGraphics.fragx;
      y = 5 + gGlobalStateInterfaceGraphics.fragy;
      i = 0;

      if (game.GetTeamplayersnum(1) > 0)
      {
        i = i + 15;
      }
      if (game.GetTeamplayersnum(2) > 0)
      {
        i = i + 15;
      }
      if (game.GetTeamplayersnum(3) > 0)
      {
        i = i + 15;
      }
      if (game.GetTeamplayersnum(4) > 0)
      {
        i = i + 15;
      }
      if (game.GetTeamplayersnum(0) > 0)
      {
        i = i + 15;
      }
      if (GS::GetGame().GetSpectatorsNum() > 0)
      {
        i = i + 15;
      }

      if (CVar::ui_hidespectators)
      {
        fragmenubottom = 70 +
                         ((GS::GetGame().GetPlayersNum() - GS::GetGame().GetSpectatorsNum() + 1) *
                          fragsmenu_player_height) +
                         i;
      }
      else
      {
        fragmenubottom = 70 + ((GS::GetGame().GetPlayersNum() + 1) * fragsmenu_player_height) + i;
      }

      _scala.x = (float)(590) / background_width;
      _scala.y = fragmenubottom / background_width;
      y = y - (gGlobalStateInterfaceGraphics.fragsscrolllev * 20);

      gfxdrawsprite(t[GFX::INTERFACE_BACK], x, y, _scala.x, _scala.y,
                    rgba(0xffffff, round(CVar::ui_status_transparency * 0.56)));

      if ((_scala.y * background_width) > height - 80)
      {
        gGlobalStateInterfaceGraphics.fragsscrollmax =
          round((float)((_scala.y * background_width - height + 80)) / 20);
      }
      else
      {
        gGlobalStateInterfaceGraphics.fragsscrollmax = 0;
      }

      if (gGlobalStateInterfaceGraphics.fragsscrollmax != 0)
      {
        x = 580 + gGlobalStateInterfaceGraphics.fragx;
        y = height / 2;
        gfxdrawsprite(t[GFX::INTERFACE_SCROLL], x, y,
                      rgba(0xffffff, fabs(round(sin(5.1 * timeelapsed) * 255))));
      }

      nextitemstep = 0;
      ids[0] = 0;
      ids[1] = 0;
      ids[2] = 0;
      ids[3] = 0;
      ids[4] = 0;
      ids[5] = 0;

      if (game.GetTeamplayersnum(1) > 0)
      {
        teamposstep[1] = nextitemstep;
        nextitemstep = nextitemstep + 20;
      }
      if (game.GetTeamplayersnum(2) > 0)
      {
        teamposstep[2] = nextitemstep;
        nextitemstep = nextitemstep + 20;
      }
      if (game.GetTeamplayersnum(3) > 0)
      {
        teamposstep[3] = nextitemstep;
        nextitemstep = nextitemstep + 20;
      }
      if (game.GetTeamplayersnum(4) > 0)
      {
        teamposstep[4] = nextitemstep;
        nextitemstep = nextitemstep + 20;
      }
      if (game.GetTeamplayersnum(0) > 0)
      {
        teamposstep[0] = nextitemstep;
        nextitemstep = nextitemstep + 20;
      }
      if (GS::GetGame().GetSpectatorsNum() > 0)
      {
        teamposstep[5] = nextitemstep;
      }

      for (j = 1; j <= GS::GetGame().GetPlayersNum(); j++)
      {
        if (GS::GetGame().GetSortedPlayers(j).playernum > 0)
        {
          if (!sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                 .player->demoplayer)
          {
            x = 32 + gGlobalStateInterfaceGraphics.fragx;
            y = 61 + j * fragsmenu_player_height + gGlobalStateInterfaceGraphics.fragy;

            if (CVar::ui_hidespectators and
                sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum).isspectator())
            {
              continue;
            }

            if (GS::GetGame().isteamgame())
            {
              // New team based score board
              if (sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                    .player->team == team_alpha)
              {
                y = 70 + (ids[1] * 15) + teamposstep[1] + gGlobalStateInterfaceGraphics.fragy;
                ids[1] = ids[1] + 1;
              }
              else if (sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                         .player->team == team_bravo)
              {
                y = 70 + game.GetTeamplayersnum(1) * fragsmenu_player_height + teamposstep[2] +
                    gGlobalStateInterfaceGraphics.fragy + (ids[2] * 15);
                ids[2] = ids[2] + 1;
              }
              // if sv_gamemode.IntValue = GAMESTYLE_TEAMMATCH then
              // begin
              else if (sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                         .player->team == team_charlie)
              {
                y = 70 +
                    (game.GetTeamplayersnum(1) + game.GetTeamplayersnum(2)) *
                      fragsmenu_player_height +
                    teamposstep[3] + gGlobalStateInterfaceGraphics.fragy + (ids[3] * 15);
                ids[3] = ids[3] + 1;
              }
              else if (sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                         .player->team == team_delta)
              {
                y = 70 +
                    (game.GetTeamplayersnum(1) + game.GetTeamplayersnum(2) +
                     game.GetTeamplayersnum(3)) *
                      fragsmenu_player_height +
                    teamposstep[4] + gGlobalStateInterfaceGraphics.fragy + (ids[4] * 15);
                ids[4] = ids[4] + 1;
              }
              // end;
              else if (sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                         .player->team == team_none)
              {
                y = 70 +
                    (game.GetTeamplayersnum(1) + game.GetTeamplayersnum(2) +
                     game.GetTeamplayersnum(3) + game.GetTeamplayersnum(4)) *
                      fragsmenu_player_height +
                    teamposstep[0] + gGlobalStateInterfaceGraphics.fragy + (ids[0] * 15);
                ids[0] = ids[0] + 1;
              }
              else if (sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                         .player->team == team_spectator)
              {
                y = 70 +
                    (game.GetTeamplayersnum(1) + game.GetTeamplayersnum(2) +
                     game.GetTeamplayersnum(3) + game.GetTeamplayersnum(4) +
                     game.GetTeamplayersnum(0)) *
                      fragsmenu_player_height +
                    teamposstep[5] + gGlobalStateInterfaceGraphics.fragy + (ids[5] * 15);
                ids[5] = ids[5] + 1;
              }
            }
            y = y - (gGlobalStateInterfaceGraphics.fragsscrolllev * 20);

            if (sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum).deadmeat and
                sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                  .isnotspectator())
            {
              gfxdrawsprite(t[GFX::INTERFACE_DEADDOT], pixelalignx(x), pixelaligny(y + 1),
                            rgba(0xffffff, CVar::ui_status_transparency));
            }

            x = 31 + gGlobalStateInterfaceGraphics.fragx;

            if (sprite_system.IsPlayerSprite(GS::GetGame().GetSortedPlayers(j).playernum))
            {
              gfxdrawsprite(t[GFX::INTERFACE_SMALLDOT], pixelalignx(x), pixelaligny(y + 1),
                            rgba(0xffffff, CVar::ui_status_transparency));
            }

            x = 30 + gGlobalStateInterfaceGraphics.fragx;
#ifdef STEAM
            // Steam Friend
            if (!SpriteSystem::Get()
                   .GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                   .player->steamfriend)
              gfxdrawsprite(t[GFX::interface_friend], pixelalignx(fragx + 240), pixelaligny(y),
                            color);
#endif

            // reg star
            l = 0;
            if ((sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                   .player->jetcolor &
                 0xff000000) == color_transparency_registered)
            {
              l = GFX::INTERFACE_STAR;
            }
            else if ((sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                        .player->jetcolor &
                      0xff000000) == color_transparency_special)
            {
              l = GFX::INTERFACE_PROT;
            }

            color = rgba(0xffffff, CVar::ui_status_transparency);

            if (l != 0)
            {
              gfxdrawsprite(t[l], pixelalignx(gGlobalStateInterfaceGraphics.fragx + 259),
                            pixelaligny(y - 1), color);
            }

            // flag icon
            if ((sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                   .player->flags > 0) and
                sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                  .isnotspectator())
            {
              gfxdrawsprite(t[GFX::INTERFACE_FLAG],
                            pixelalignx(gGlobalStateInterfaceGraphics.fragx + 337),
                            pixelaligny(y - 1), color);
            }

            // mute sign
            if (sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum).muted or
                gGlobalStateClient.muteall)
            {
              gfxdrawsprite(t[GFX::INTERFACE_MUTE],
                            pixelalignx(gGlobalStateInterfaceGraphics.fragx + 246),
                            pixelaligny(y - 1), color);
            }

            // bot icon
            if ((sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                   .player->jetcolor &
                 0xff000000) == color_transparency_bot)
            {
              gfxdrawsprite(t[GFX::INTERFACE_BOT],
                            pixelalignx(gGlobalStateInterfaceGraphics.fragx + 534), pixelaligny(y),
                            color);
            }

            gfxdrawsprite(
              t[GFX::INTERFACE_CONNECTION], pixelalignx(gGlobalStateInterfaceGraphics.fragx + 520),
              pixelaligny(y + 2),
              rgba(
                (std::uint8_t)((
                  (255 * (100 - sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                                  .player->connectionquality)) /
                  100)),
                (std::uint8_t)((255 *
                                sprite_system.GetSprite(GS::GetGame().GetSortedPlayers(j).playernum)
                                  .player->connectionquality) /
                               100),
                0, CVar::ui_status_transparency));
          }
        }
      }
    }
  }

  if (gGlobalStateGameMenus.teammenu->active)
  {
    gfxdrawsprite(t[GFX::INTERFACE_BACK], 45, 140, (float)(262) / background_width,
                  (float)(250) / background_width,
                  rgba(0xffffff, round(CVar::ui_status_transparency * 0.56)));
  }

  if (gGlobalStateGameMenus.limbomenu->active)
  {
    // draw weapon sprites in weapons menu
    _scala.x = (float)(252) / background_width;

    gfxdrawsprite(t[GFX::INTERFACE_BACK], 45, 140, _scala.x, (float)(210) / background_width,
                  rgba(0xffffff, round(CVar::ui_status_transparency * 0.56)));

    gfxdrawsprite(t[GFX::INTERFACE_BACK], 45, 350, _scala.x, (float)(80) / background_width,
                  rgba(0xffffff, round(CVar::ui_status_transparency * 0.56)));

    // draw guns on limbo menu
    x = pixelalignx(55);
    y = 157;

    auto &weaponSystem = GS::GetWeaponSystem();

    for (k = 1; k <= primary_weapons; k++)
    {
      if (weaponSystem.IsEnabled(k))
      {
        spr = t[GFX::INTERFACE_GUNS_DEAGLES + k - 1];
        dy = (float)(max(0.f, 18 - spr->height * spr->scale)) / 2;
        gfxdrawsprite(spr, x, pixelaligny(y + 18 * (k - 1) + dy),
                      rgba(0xffffff, CVar::ui_status_transparency));
      }
    }

    for (k = primary_weapons + 1; k <= main_weapons; k++)
    {
      if (weaponSystem.IsEnabled(k))
      {
        i = k - primary_weapons - 1;
        spr = t[GFX::INTERFACE_GUNS_SOCOM + i];
        dy = (float)(max(0.f, 18 - spr->height * spr->scale)) / 2;

        if (CVar::cl_player_secwep == i)
        {
          gfxdrawsprite(spr, x, pixelaligny(y + k * 18 + dy),
                        rgba(0xffffff, CVar::ui_status_transparency));
        }
        else
        {
          gfxdrawsprite(spr, x, pixelaligny(y + k * 18 + dy),
                        rgba(0xffffff, round(CVar::ui_status_transparency * 0.5)));
        }
      }
    }
  }

  // vote on
  if (GS::GetGame().IsVoteActive())
  {
    gfxdrawsprite(t[GFX::INTERFACE_BACK], 45 * gGlobalStateInterfaceGraphics._iscala.x,
                  400 * gGlobalStateInterfaceGraphics._iscala.y, (float)(252) / background_width,
                  (float)(40) / background_width,
                  rgba(0xffffff, round(CVar::ui_status_transparency * 0.36)));
  }

  // Team Box
  if (int_.team && !gGlobalStateDemo.demoplayer.active() && GS::GetGame().isteamgame())
  {
    x = int_.teambox_x * gGlobalStateInterfaceGraphics._iscala.x;
    y = int_.teambox_y * gGlobalStateInterfaceGraphics._iscala.y;

    gfxdrawsprite(t[GFX::INTERFACE_BACK], x, y, (float)(57) / background_width,
                  (float)(88) / background_width, rgba(0xffffff, round(int_.alpha * 0.56)));

    // Draw captured flags in Team Box
    if ((GS::GetGame().GetTeamFlag(1) > 0) && (GS::GetGame().GetTeamFlag(2) > 0))
    {
      if (CVar::sv_gamemode == gamestyle_ctf)
      {
        x = pixelalignx((int_.teambox_x + 4) * gGlobalStateInterfaceGraphics._iscala.x);
        y = pixelaligny((int_.teambox_y + 5) * gGlobalStateInterfaceGraphics._iscala.y);

        if (!things[GS::GetGame().GetTeamFlag(1)].inbase)
        {
          gfxdrawsprite(t[GFX::INTERFACE_NOFLAG], x, y, rgba(0xff0000, round(int_.alpha)));
        }

        x = pixelalignx(x + 31);

        if (!things[GS::GetGame().GetTeamFlag(2)].inbase)
        {
          gfxdrawsprite(t[GFX::INTERFACE_NOFLAG], x, y, rgba(0xff, round(int_.alpha)));
        }
      }
      else if (CVar::sv_gamemode == gamestyle_inf)
      {
        if (!things[GS::GetGame().GetTeamFlag(2)].inbase)
        {
          x = pixelalignx((int_.teambox_x + 19) * gGlobalStateInterfaceGraphics._iscala.x);
          y = pixelaligny((int_.teambox_y + 3) * gGlobalStateInterfaceGraphics._iscala.y);

          gfxdrawsprite(t[GFX::INTERFACE_NOFLAG], x, y, rgba(0xff, round(int_.alpha)));
        }
      }
    }
    else if ((GS::GetGame().GetTeamFlag(1) > 0) && (CVar::sv_gamemode == gamestyle_htf))
    {
      if (things[GS::GetGame().GetTeamFlag(1)].holdingsprite > 0)
      {
        if (sprite_system.GetSprite(things[GS::GetGame().GetTeamFlag(1)].holdingsprite)
              .player->team == team_alpha)
        {
          x = pixelalignx((int_.teambox_x + 19) * gGlobalStateInterfaceGraphics._iscala.x);
          y = pixelaligny((int_.teambox_y + 3) * gGlobalStateInterfaceGraphics._iscala.y);

          gfxdrawsprite(t[GFX::INTERFACE_NOFLAG], x, y, rgba(0xff0000, round(int_.alpha)));
        }
        else
        {
          x = pixelalignx((int_.teambox_x + 19) * gGlobalStateInterfaceGraphics._iscala.x);
          y = pixelaligny((int_.teambox_y + 3) * gGlobalStateInterfaceGraphics._iscala.y);

          gfxdrawsprite(t[GFX::INTERFACE_NOFLAG], x, y, rgba(0xff, round(int_.alpha)));
        }
      }
    }
  }

  // text
  gfxtextshadow(1, 1, rgba(0));

  if (gGlobalStateClient.notexts == 0)
  {
    if (sprite_system.IsPlayerSpriteValid())
    {
      if (sprite_system.GetPlayerSprite().isspectator() &&
          (gGlobalStateClient.camerafollowsprite > 0) && (CVar::sv_advancedspectator))
      {
        renderplayerinterfacetexts(gGlobalStateClient.camerafollowsprite);
      }
      else
      {
        renderplayerinterfacetexts(gGlobalStateClient.mysprite);
      }
    }

    if (int_.team)
    {
      renderteamscoretexts();
    }

    if (GS::GetGame().GetMapchangecounter() > 0)
    {
      if (gGlobalStateInterfaceGraphics.fragsmenushow && (GS::GetGame().GetPlayersNum() > 1) &&
          (GS::GetGame().GetMapchangecounter() < 999999999))
      {
        renderendgametexts(fragmenubottom);
      }

      if (GS::GetGame().GetMapchangecounter() > 99999999)
      {
        // game paused
        gfxtextcolor(rgba(185, 250, 138));
        gfxdrawtext(("Game paused"), 197 + gGlobalStateInterfaceGraphics.fragx,
                    24 + gGlobalStateInterfaceGraphics.fragy);
      }
    }

    if (gGlobalStateInterfaceGraphics.statsmenushow)
    {
      renderweaponstatstexts();
    }

    if (gGlobalStateInterfaceGraphics.fragsmenushow)
    {
      renderfragsmenutexts(fragmenubottom);
    }

    if (CVar::ui_console)
    {
      renderconsoletexts(width);
    }

    if (sprite_system.IsPlayerSpriteValid())
    {
      renderrespawnandsurvivaltexts();
    }

    rendervotemenutexts();

    if (gGlobalStateClient.showradiomenu && (CVar::sv_radio) &&
        !gGlobalStateGameMenus.escmenu->active)
    {
      renderradiomenutexts();
    }

    if (!gGlobalStateClientGame.chattext.empty())
    {
      renderchatinput(width, height, timeelapsed);
    }

    if (CVar::ui_killconsole)
    {
      renderkillconsoletexts(width);
    }

#ifdef STEAM
    if (CVar::cl_voicechat)
      rendervoicechattexts();
#endif
  }

  if (sprite_system.IsPlayerSpriteValid())
  {
    renderchattexts();

    if (gGlobalStateInterfaceGraphics.playernamesshow)
    {
      renderplayernames(width, height);
    }

    if ((CVar::sv_survivalmode) && sprite_system.GetPlayerSprite().active and
        (sprite_system.GetPlayerSprite().ceasefirecounter > 0))
    {
      renderceasefirecounter();
    }
  }

  if (gGlobalStateClient.notexts == 0)
  {
    setfontstyle(font_small);

    // cursor text
    if ((gGlobalStateInterfaceGraphics.cursortextlength > 0) &&
        (GS::GetGame().GetMapchangecounter() < 0) && !gGlobalStateGameMenus.teammenu->active &&
        !gGlobalStateGameMenus.escmenu->active && !gGlobalStateDemo.demoplayer.active())
    {
      x = gGlobalStateClientGame.mx * gGlobalStateInterfaceGraphics._rscala.x -
          0.5 * rectwidth(gfxtextmetrics((gGlobalStateInterfaceGraphics.cursortext)));
      y = (gGlobalStateClientGame.my + 10) * gGlobalStateInterfaceGraphics._rscala.y;

      gfxtextcolor(rgba(0xffffff, 0x77));
      gfxdrawtext(x, y);
    }

    // free camera / following player
    if (gGlobalStateClient.camerafollowsprite == 0)
    {
      x = (width - rectwidth(gfxtextmetrics(("Free Camera")))) / 2;
      gfxtextcolor(rgba(205, 205, 205));
      gfxdrawtext(x, 430 * gGlobalStateInterfaceGraphics._iscala.y);
    }
    else if ((gGlobalStateClient.camerafollowsprite > 0) &&
             (gGlobalStateClient.camerafollowsprite <= max_sprites) &&
             (!sprite_system.IsPlayerSprite(gGlobalStateClient.camerafollowsprite)))
    {
      i = (std::int32_t)(sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).deadmeat);
      x = (width -
           rectwidth(gfxtextmetrics(
             ("Following " +
              (sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).player->name))))) /
          2;

      gfxtextcolor(rgba(205, 205 - i * 105, 205 - i * 105));
      gfxdrawtext(x, 430 * gGlobalStateInterfaceGraphics._iscala.y);
    }

    // FPS && connection info
    if (gGlobalStateInterfaceGraphics.coninfoshow)
    {
      gfxtextcolor(rgba(239, 170, 200));

      x = gGlobalStateInterfaceGraphics._iscala.x;
      y = gGlobalStateInterfaceGraphics._iscala.y;

      gfxdrawtext(string("FPS: ") + inttostr(getgamefps()), 460 * x, 10 * y);

      if (sprite_system.IsPlayerSpriteValid())
      {
        gfxdrawtext(string("Ping: ") + inttostr(sprite_system.GetPlayerSprite().player->realping),
                    550 * x, 10 * y);
      }

      if (gGlobalStateDemo.demoplayer.active())
      {
        gfxdrawtext(wideformat("Demo: %.2d:%.2d / %.2d:%.2d (%d / %d)",
                               round((GS::GetGame().GetMainTickCounter() / 60) / 60),
                               round((GS::GetGame().GetMainTickCounter() / 60) % 60),
                               round((gGlobalStateDemo.demoplayer.header().ticksnum / 60) / 60),
                               round((gGlobalStateDemo.demoplayer.header().ticksnum / 60) % 60),
                               GS::GetGame().GetMainTickCounter(),
                               gGlobalStateDemo.demoplayer.header().ticksnum),
                    460 * x, 80 * y);
      }
      else
      {
        NotImplemented("network");
#if 0
                networkstats = udp.getquickconnectionstatus(udp.peer);
                setfontstyle(font_smallest);
                gfxdrawtext(string("Ping: ") + networkstats.m_nping.tostring, 460 * x, 40 * y);
                gfxdrawtext(string("Quality: Local ") +
                                float(networkstats.m_flconnectionqualitylocal * 100)
                                    .tostring(fffixed, 7, 0) +
                                "% Remote -  " +
                                float(networkstats.m_flconnectionqualityremote * 100)
                                    .tostring(fffixed, 7, 0) +
                                " %",
                            460 * x, 50 * y);
                gfxdrawtext(string("Traffic Out: ") +
                                networkstats.m_floutpacketspersec.tostring(fffixed, 7, 1) +
                                " pkt/s " +
                                networkstats.m_floutbytespersec.tostring(fffixed, 7, 1) + " B/s",
                            460 * x, 60 * y);
                gfxdrawtext(string("Traffic In: ") +
                                networkstats.m_flinpacketspersec.tostring(fffixed, 7, 1) +
                                " pkt/s " + networkstats.m_flinbytespersec.tostring(fffixed, 7, 1) +
                                " B/s",
                            460 * x, 70 * y);
                gfxdrawtext(string("Max Send Rate: ") +
                                networkstats.m_nsendratebytespersecond.tostring + " B/s",
                            460 * x, 80 * y);
                gfxdrawtext(string("Pending Packets: ") +
                                networkstats.m_cbpendingunreliable.tostring + " / " +
                                networkstats.m_cbpendingunreliable.tostring,
                            460 * x, 90 * y);
                gfxdrawtext(string("Queue Time: ") + networkstats.m_usecqueuetime.tostring + " ms",
                            460 * x, 100 * y);

                setfontstyle(font_smallest);
                gfxdrawtext(udp.getdetailedconnectionstatus(udp.peer), 100 * x, 10 * y);
                setfontstyle(font_small);
#endif
      }
    }

    // REC
    if (GS::GetDemoRecorder().active())
    {
      gfxtextcolor(rgba(195, 0, 0, fabs(sin(5.1 * timeelapsed / 2) * 255)));
      gfxdrawtext("REC", 612 * gGlobalStateInterfaceGraphics._iscala.x,
                  1 * gGlobalStateInterfaceGraphics._iscala.y);
    }

    // default keys help
    if (gGlobalStateInterfaceGraphics.noobshow && gGlobalStateGameMenus.escmenu->active &&
        (gGlobalStateClientGame.chattext.empty()))
    {
      setfontstyle(font_smallest);
      gfxtextcolor(rgba(250, 90, 95));
      gfxdrawtext(("Default keys (shown for first 3 game runs)"),
                  30 * gGlobalStateInterfaceGraphics._rscala.x,
                  28 * gGlobalStateInterfaceGraphics._rscala.y);
      setfontstyle(font_small);

      for (i = 1; i <= 8; i++)
      {
        gfxtextcolor(rgba(230, 232 - 2 * i, 255));
        switch (i)
        {
        case 1:
          gfxdrawtext(("[A]/[D] move left/right"), 30 * gGlobalStateInterfaceGraphics._rscala.x,
                      (28 + 12 * i) * gGlobalStateInterfaceGraphics._rscala.y);
          break;
        case 2:
          gfxdrawtext(("[W]/[S]/[X] jump / crouch / lie down"),
                      30 * gGlobalStateInterfaceGraphics._rscala.x,
                      (28 + 12 * i) * gGlobalStateInterfaceGraphics._rscala.y);
          break;
        case 3:
          gfxdrawtext(("[Left Mouse] fire!"), 30 * gGlobalStateInterfaceGraphics._rscala.x,
                      (28 + 12 * i) * gGlobalStateInterfaceGraphics._rscala.y);
          break;
        case 4:
          gfxdrawtext(("[Right Mouse] jet boots"), 30 * gGlobalStateInterfaceGraphics._rscala.x,
                      (28 + 12 * i) * gGlobalStateInterfaceGraphics._rscala.y);
          break;
        case 5:
          gfxdrawtext(("hold [E] to toss grenade"), 30 * gGlobalStateInterfaceGraphics._rscala.x,
                      (28 + 12 * i) * gGlobalStateInterfaceGraphics._rscala.y);
          break;
        case 6:
          gfxdrawtext(("[R] reloads weapon"), 30 * gGlobalStateInterfaceGraphics._rscala.x,
                      (28 + 12 * i) * gGlobalStateInterfaceGraphics._rscala.y);
          break;
        case 7:
          gfxdrawtext(("[Q] change weapon / [F] throw weapon"),
                      30 * gGlobalStateInterfaceGraphics._rscala.x,
                      (28 + 12 * i) * gGlobalStateInterfaceGraphics._rscala.y);
          break;
        case 8:
          gfxdrawtext(("[T] chat / [Y] team chat"), 30 * gGlobalStateInterfaceGraphics._rscala.x,
                      (28 + 12 * i) * gGlobalStateInterfaceGraphics._rscala.y);
          break;
        }
      }
    }

    // action snap
    if ((gGlobalStateClientGame.screencounter < 255) &&
        (!static_cast<bool>(gGlobalStateClientGame.showscreen)) && (CVar::cl_actionsnap))
    {
      gfxtextcolor(rgba(230, 65, 60, 150 + fabs(round(sin(5.1 * timeelapsed) * 100))));
      gfxdrawtext(string("[[ ") + ("Press F5 to View Screen Cap") + " ]]",
                  30 * gGlobalStateInterfaceGraphics._iscala.x,
                  412 * gGlobalStateInterfaceGraphics._iscala.y);
    }

    // shot distance
    if (gGlobalStateClient.shotdistanceshow > 0)
    {
      gfxtextcolor(rgba(230, 65, 60, 150 + fabs(round(sin(5.1 * timeelapsed) * 100))));

      str1 =
        string("DISTANCE: ") + (floattostrf(gGlobalStateClient.shotdistance, fffixed, 12, 2)) + 'm';
      gfxdrawtext(str1, 390 * gGlobalStateInterfaceGraphics._iscala.x,
                  431 * gGlobalStateInterfaceGraphics._iscala.y);

      str1 = string("AIRTIME: ") + (floattostrf(gGlobalStateClient.shotlife, fffixed, 12, 2)) + 's';
      gfxdrawtext(str1, 228 * gGlobalStateInterfaceGraphics._iscala.x,
                  431 * gGlobalStateInterfaceGraphics._iscala.y);

      if (gGlobalStateClient.shotricochet > 0)
      {
        str1 = string("RICOCHETS: ") + (inttostr(gGlobalStateClient.shotricochet));
        gfxdrawtext(str1, 62 * gGlobalStateInterfaceGraphics._iscala.x,
                    431 * gGlobalStateInterfaceGraphics._iscala.y);
      }
    }
  }

  // Bullet time cut to wide screen
  if (widescreencut)
  {
    gfxdrawquad(nullptr, gfxvertex(0, 0, 0, 0, rgba(0)), gfxvertex(width, 0, 0, 0, rgba(0)),
                gfxvertex(width, 80 * gGlobalStateInterfaceGraphics._rscala.y, 0, 0, rgba(0)),
                gfxvertex(0, 80 * gGlobalStateInterfaceGraphics._rscala.y, 0, 0, rgba(0)));

    gfxdrawquad(
      nullptr, gfxvertex(0, height - 80 * gGlobalStateInterfaceGraphics._rscala.y, 0, 0, rgba(0)),
      gfxvertex(width, height - 80 * gGlobalStateInterfaceGraphics._rscala.y, 0, 0, rgba(0)),
      gfxvertex(width, height, 0, 0, rgba(0)), gfxvertex(0, height, 0, 0, rgba(0)));
  }

  rendergamemenutexts(width, height);

  // Menu cursor
  alfa = gGlobalStateClient.mysprite;
  if (alfa < 1)
  {
    alfa = max_players;
  }
  if (gGlobalStateGameMenus.escmenu->active or gGlobalStateGameMenus.limbomenu->active or
      gGlobalStateGameMenus.teammenu->active or sprite_system.GetSprite(alfa).deadmeat)
  {
    if (!gGlobalStateDemo.demoplayer.active() or gGlobalStateGameMenus.escmenu->active)
    {
      x = pixelalignx(gGlobalStateClientGame.mx * gGlobalStateInterfaceGraphics._rscala.x);
      y = pixelaligny(gGlobalStateClientGame.my * gGlobalStateInterfaceGraphics._rscala.y);

      gfxdrawsprite(t[GFX::INTERFACE_MENUCURSOR], x, y,
                    rgba(0xffffff, CVar::ui_status_transparency));
    }
  }
}
// NOLINTEND(readability-magic-numbers)