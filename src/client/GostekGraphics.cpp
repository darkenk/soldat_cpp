// automatically converted

#include "GostekGraphics.hpp"
#include "GameRendering.hpp"
#include "Gfx.hpp"
#include "common/gfx.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <Tracy.hpp>
#include <array>
#include <memory_resource>
#include <numbers>
#include <set>

using std::numbers::pi;

#define IDS 1
#include "GostekGraphics.inc.hpp"
#undef IDS

const std::int32_t color_none = 0;
const std::int32_t color_main = 1;
const std::int32_t color_pants = 2;
const std::int32_t color_skin = 3;
const std::int32_t color_hair = 4;
const std::int32_t color_cygar = 5;
const std::int32_t color_headblood = 6;

const std::int32_t alpha_base = 0;
const std::int32_t alpha_blood = 1;
const std::int32_t alpha_nades = 2;

struct tgosteksprite
{
  std::string id;
  std::uint64_t image;
  std::uint8_t p1, p2;
  float cx, cy;
  float flex;
  bool flip;
  bool team;
  std::uint8_t color;
  std::uint8_t alpha;
};

typedef std::set<std::int32_t> tgostekspriteset;

std::array<tgosteksprite, GOSTEK_LAST> gosteksprites;
tgostekspriteset gostekbase;

void loaddefaults();

static void include(tgostekspriteset &sprite, std::int32_t v)
{
  sprite.emplace(v);
}

static void def(std::string id, std::uint64_t image, std::uint8_t p1, std::uint8_t p2, float cx,
                float cy, std::uint8_t visible, std::uint8_t flip, std::uint8_t team, float flex,
                std::uint8_t color, std::uint8_t alpha)
{
  static std::int32_t defindex = 0;
  gosteksprites[defindex].id = id;
  gosteksprites[defindex].image = image;
  gosteksprites[defindex].p1 = p1;
  gosteksprites[defindex].p2 = p2;
  gosteksprites[defindex].cx = cx;
  gosteksprites[defindex].cy = cy;
  gosteksprites[defindex].flex = flex;
  gosteksprites[defindex].flip = flip == 1;
  gosteksprites[defindex].team = team == 1;
  gosteksprites[defindex].color = color;
  gosteksprites[defindex].alpha = alpha;

  if (visible == 1)
    gostekbase.emplace(defindex);

  defindex += 1;
}

void loaddefaults()
{
  std::int32_t defindex;

  defindex = GOSTEK_FIRST;
  gostekbase.clear();
// TODO: pure evil, do something with this
#define IDS 0
#include "GostekGraphics.inc.hpp"
#undef IDS
}

void loadgostekdata(const TIniFile::Entries &data)
{
  std::int32_t i;
  std::string cx, cy;

  loaddefaults();

  for (i = GOSTEK_FIRST; i < GOSTEK_LAST; i++)
  {
    cx = data.at(gosteksprites[i].id + "_CenterX");
    cy = data.at(gosteksprites[i].id + "_CenterY");
    gosteksprites[i].cx = strtofloatdef(cx, gosteksprites[i].cx);
    gosteksprites[i].cy = strtofloatdef(cy, gosteksprites[i].cy);
  }
}

void applygostekconstraints();

static float texwidth(std::int32_t index)
{
  return fabs(textures[index]->width * textures[index]->scale);
}

static float texheight(std::int32_t index)
{
  return fabs(textures[index]->height * textures[index]->scale);
}

void applygostekconstraints()
{
  std::int32_t i, t2;
  float w, h, cx, cy;

  t2 = GFX::GOSTEK_TEAM2_STOPA - GFX::GOSTEK_STOPA;

  for (i = GOSTEK_FIRST; i < GOSTEK_LAST; i++)
  {
    tgosteksprite &gs = gosteksprites[i];

    if (gs.image != 0)
    {
      w = texwidth(gs.image);
      w = max(w, texwidth(gs.image + ord(gs.flip)));
      w = max(w, texwidth(gs.image + t2 * ord(gs.team)));
      w = max(w, texwidth(gs.image + t2 * ord(gs.team) + ord(gs.flip)));

      h = texheight(gs.image);
      h = max(h, texheight(gs.image + ord(gs.flip)));
      h = max(h, texheight(gs.image + t2 * ord(gs.team)));
      h = max(h, texheight(gs.image + t2 * ord(gs.team) + ord(gs.flip)));

      cx = w * fabs(gs.cx + 0.5);
      cy = h * fabs(gs.cy + 0.5);

      if (cx > (w + gos_restrict_width))
        gs.cx = 0.5 + sign(gs.cx + 0.5) * ((float)((w + gos_restrict_width)) / w);

      if (cy > (h + gos_restrict_height))
        gs.cy = 0.5 + sign(gs.cy + 0.5) * ((float)((h + gos_restrict_height)) / h);
    }
  }
}

void drawgosteksprite(pgfxsprite sprite, float x, float y, float sx, float sy, float cx, float cy,
                      float r, const tgfxcolor &color)
{
  ZoneScopedN("DrawGostekSprite");
  tgfxmat3 m;
  float c, s, w, h, u0, v0, u1, v1;
  std::array<tgfxvertex, 4> buff;
  std::pmr::monotonic_buffer_resource res(buff.data(), buff.size() * sizeof(tgfxvertex));
  std::pmr::vector<tgfxvertex> v{4, &res};

  std::array<tgfxvertex, 4> buff2;
  std::pmr::monotonic_buffer_resource res2(buff2.data(), buff2.size() * sizeof(tvector2));
  std::pmr::vector<tvector2> p{4, &res2};

  c = cos(r);
  s = sin(r);

  m[0] = c * sx;
  m[3] = -s * sy;
  m[6] = x - cy * m[3] - cx * m[0];
  m[1] = s * sx;
  m[4] = c * sy;
  m[7] = y - cy * m[4] - cx * m[1];
  m[2] = 0;
  m[5] = 0;
  m[8] = 1;

  w = sprite->scale * sprite->width;
  h = sprite->scale * sprite->height;

  u0 = sprite->texcoords.left;
  v0 = sprite->texcoords.top;
  u1 = sprite->texcoords.right;
  v1 = sprite->texcoords.bottom;

  p[0] = gfxmat3mul(m, 0, 0);
  p[1] = gfxmat3mul(m, w, 0);
  p[2] = gfxmat3mul(m, w, h);
  p[3] = gfxmat3mul(m, 0, h);

  v[0] = gfxvertex(p[0].x, p[0].y, u0, v0, color);
  v[1] = gfxvertex(p[1].x, p[1].y, u1, v0, color);
  v[2] = gfxvertex(p[2].x, p[2].y, u1, v1, color);
  v[3] = gfxvertex(p[3].x, p[3].y, u0, v1, color);

  gfxdrawquad(sprite->texture, v);
}

void rendergostek(tsprite &soldier)
{
  ZoneScopedN("RenderGostek");
  std::int32_t i, n, index;
  bool showclip, grabbed;
  tgostekspriteset visible;
  PascalArray<tgfxcolor, color_none, color_headblood> color;
  PascalArray<std::uint8_t, alpha_base, alpha_nades> alpha;
  tgfxmat3 m{0};
  tvector2 v;
  float x1, y1, x2, y2, cx, cy, r, sx, sy;
  std::int32_t tex, team2offset;

  if ((soldier.style != 1) || (soldier.ceasefirecounter > GS::GetGame().GetCeasefiretime() - 5) ||
      ((CVar::sv_realisticmode) && (soldier.visible == 0)) or (soldier.isspectator()) or
      (soldier.player->name == "") or (soldier.player->demoplayer))
    return;

  {
    ZoneScopedN("CopyGostekBase");
    visible = gostekbase;
  }

  // setup colors

  color[color_none] = rgba(0xffffff);
  color[color_main] = rgba(soldier.player->shirtcolor);
  color[color_pants] = rgba(soldier.player->pantscolor);
  color[color_skin] = rgba(soldier.player->skincolor);
  color[color_hair] = rgba(soldier.player->haircolor);
  color[color_cygar] = rgba(0xffffff);
  color[color_headblood] = rgba(0xaca9a8);

  if (soldier.hascigar == 5)
    color[color_cygar] = rgba(0x616161);

  alpha[alpha_base] = soldier.alpha;
  alpha[alpha_blood] = max(0.0f, min(255.0f, 200 - round(soldier.GetHealth())));

  if (soldier.GetHealth() > (90 - 40 * CVar::sv_realisticmode))
    alpha[alpha_blood] = 0;

  if ((CVar::sv_realisticmode) && (soldier.visible > 0) && (soldier.visible < 45) &&
      (soldier.alpha > 60))
  {
    soldier.alpha = 3 * soldier.visible;
    alpha[alpha_base] = soldier.alpha;
    alpha[alpha_blood] = 0;
  }

  alpha[alpha_nades] = trunc(0.75 * alpha[alpha_base]);

  // blood
  if (alpha[alpha_blood] > 0)
  {
    visible.emplace(GOSTEK_LEFT_THIGH_DMG);
    visible.emplace(GOSTEK_LEFT_LOWERLEG_DMG);
    visible.emplace(GOSTEK_LEFT_FOREARM_DMG);
    visible.emplace(GOSTEK_LEFT_ARM_DMG);
    visible.emplace(GOSTEK_CHEST_DMG);
    visible.emplace(GOSTEK_HIP_DMG);
    visible.emplace(GOSTEK_HEAD_DMG);
    visible.emplace(GOSTEK_RIGHT_THIGH_DMG);
    visible.emplace(GOSTEK_RIGHT_LOWERLEG_DMG);
    visible.emplace(GOSTEK_RIGHT_FOREARM_DMG);
    visible.emplace(GOSTEK_RIGHT_ARM_DMG);
  }

  // jets
  if ((soldier.control.jetpack) && (soldier.jetscount > 0))
  {
    auto it = visible.find(GOSTEK_LEFT_FOOT);
    if (it != visible.end())
    {
      visible.erase(it);
    }
    it = visible.find(GOSTEK_RIGHT_FOOT);
    if (it != visible.end())
    {
      visible.erase(it);
    }
    visible.emplace(GOSTEK_LEFT_JETFOOT);
    visible.emplace(GOSTEK_RIGHT_JETFOOT);
  }

  // vest
  if (soldier.vest > 0)
    include(visible, GOSTEK_VEST);

  // grenades
  if (soldier.tertiaryweapon.num == fraggrenade_num)
    index = GOSTEK_FRAG_GRENADE1;
  else
    index = GOSTEK_CLUSTER_GRENADE1;

  n = soldier.tertiaryweapon.ammocount - ord(soldier.bodyanimation.id == AnimationType::Throw);

  for (i = 0; i <= min(5, n) - 1; i++)
    include(visible, index + i);

  // chain
  switch (soldier.player->chain)
  {
  case 1:
    visible.emplace(GOSTEK_SILVER_LCHAIN);
    visible.emplace(GOSTEK_SILVER_RCHAIN);
    visible.emplace(GOSTEK_SILVER_PENDANT);
    break;
  case 2:
    visible.emplace(GOSTEK_GOLDEN_LCHAIN);
    visible.emplace(GOSTEK_GOLDEN_RCHAIN);
    visible.emplace(GOSTEK_GOLDEN_PENDANT);
    break;
  }

  // cygar
  if ((soldier.hascigar == 5) || (soldier.hascigar == 10))
    include(visible, GOSTEK_CIGAR);

  // head & hair
  if (soldier.deadmeat)
  {
    auto it = visible.find(GOSTEK_HEAD);
    if (it != visible.end())
    {
      visible.erase(it);
    }
    it = visible.find(GOSTEK_HEAD_DMG);
    if (it != visible.end())
    {
      visible.erase(it);
    }
    visible.emplace(GOSTEK_HEAD_DEAD);
    visible.emplace(GOSTEK_HEAD_DEAD_DMG);
  }

  if (soldier.weapon.num == bow_num || bow2_num == soldier.weapon.num)
  {
    include(visible, GOSTEK_RAMBO_BADGE);
  }
  else
  {
    grabbed = (AnimationType::Wipe == soldier.bodyanimation.id ||
               AnimationType::TakeOff == soldier.bodyanimation.id) &&
              (soldier.bodyanimation.currframe > 4);

    if (soldier.wearhelmet == 1)
    {
      switch (soldier.player->headcap)
      {
      case GFX::GOSTEK_HELM:
        if (grabbed)
        {
          include(visible, GOSTEK_GRABBED_HELMET);
        }
        else
        {
          include(visible, GOSTEK_HELMET);
        }
        break;
      case GFX::GOSTEK_KAP:
        if (grabbed)
        {
          include(visible, GOSTEK_GRABBED_HAT);
        }
        else
        {
          include(visible, GOSTEK_HAT);
        }
        break;
      }
    }

    if (grabbed || (soldier.wearhelmet != 1) || (soldier.player->hairstyle == 3))
    {
      switch (soldier.player->hairstyle)
      {
      case 1:
        for (i = 0; i <= 5; i++)
          include(visible, GOSTEK_HAIR_DREADLOCKS + i);
        break;
      case 2:
        include(visible, GOSTEK_HAIR_PUNK);
        break;
      case 3:
        include(visible, GOSTEK_MR_T);
        break;
      case 4:
        include(visible, GOSTEK_HAIR_NORMAL);
        break;
      }
    }
  }

  // secondary weapon (on the back)

  index = weaponnumtoindex(soldier.secondaryweapon.num, GS::GetWeaponSystem().GetGuns());

  if ((index >= eagle) && (index <= flamer))
    include(visible, GOSTEK_SECONDARY_FIRST + index - eagle);

  // primary weapon

  if (soldier.weapon.num == minigun_num)
  {
    include(visible, GOSTEK_PRIMARY_MINIGUN);

    showclip = (soldier.weapon.ammocount > 0) ||
               ((soldier.weapon.ammocount == 0) && (soldier.weapon.reloadtimecount < 65));

    if (showclip)
      include(visible, GOSTEK_PRIMARY_MINIGUN_CLIP);

    if (soldier.fired > 0)
      include(visible, GOSTEK_PRIMARY_MINIGUN_FIRE);
  }
  else if (soldier.weapon.num == bow_num || bow2_num == soldier.weapon.num)
  {
    if (soldier.weapon.ammocount == 0)
      include(visible, GOSTEK_PRIMARY_BOW_ARROW_RELOAD);
    else
      include(visible, GOSTEK_PRIMARY_BOW_ARROW);

    if (soldier.bodyanimation.id == AnimationType::ReloadBow)
    {
      visible.emplace(GOSTEK_PRIMARY_BOW_RELOAD);
      visible.emplace(GOSTEK_PRIMARY_BOW_STRING_RELOAD);
    }
    else
    {
      visible.emplace(GOSTEK_PRIMARY_BOW);
      visible.emplace(GOSTEK_PRIMARY_BOW_STRING);
    }

    if (soldier.fired > 0)
      include(visible, GOSTEK_PRIMARY_BOW_FIRE);
  }
  else if (!soldier.deadmeat)
  {
    index = weaponnumtoindex(soldier.weapon.num, GS::GetWeaponSystem().GetGuns());

    if ((index >= eagle) && (index <= flamer))
    {
      if (index == flamer)
        index = GOSTEK_PRIMARY_FLAMER - GOSTEK_PRIMARY_FIRST;
      else
        index = 3 * (index - eagle);

      include(visible, GOSTEK_PRIMARY_FIRST + index);

      showclip = (soldier.weapon.cliptexturenum > 0) &&
                 ((soldier.weapon.ammocount > 0) ||
                  ((soldier.weapon.ammocount == 0) &&
                   ((soldier.weapon.reloadtimecount < soldier.weapon.clipintime) ||
                    (soldier.weapon.reloadtimecount > soldier.weapon.clipouttime))));

      if (showclip)
        include(visible, GOSTEK_PRIMARY_FIRST + index + 1);

      if (soldier.fired > 0)
        include(visible, GOSTEK_PRIMARY_FIRST + index + 2);
    }
  }

  // draw

  team2offset = 0;

  if (soldier.player->team == team_bravo || team_delta == soldier.player->team)
    team2offset = GFX::GOSTEK_TEAM2_STOPA - GFX::GOSTEK_STOPA;

  if (visible.contains(GOSTEK_HAIR_DREADLOCKS))
  {
    x1 = soldier.skeleton.pos[gosteksprites[GOSTEK_HEAD].p1].x;
    y1 = soldier.skeleton.pos[gosteksprites[GOSTEK_HEAD].p1].y;
    x2 = soldier.skeleton.pos[gosteksprites[GOSTEK_HEAD].p2].x;
    y2 = soldier.skeleton.pos[gosteksprites[GOSTEK_HEAD].p2].y;
    r = atan2(y2 - y1, x2 - x1) - pi / 2;
    m = gfxmat3rot(r);
  }

  for (i = GOSTEK_FIRST; i <= GOSTEK_LAST; i++)
  {
    tgosteksprite &gs = gosteksprites[i];

    if ((visible.contains(i)) && (gs.image != 0))
    {
      tex = gs.image;

      if (gs.team)
        tex = tex + team2offset;

      x1 = soldier.skeleton.pos[gs.p1].x;
      y1 = soldier.skeleton.pos[gs.p1].y;
      x2 = soldier.skeleton.pos[gs.p2].x;
      y2 = soldier.skeleton.pos[gs.p2].y;
      r = atan2(y2 - y1, x2 - x1);
      cx = gs.cx;
      cy = gs.cy;
      sx = 1;
      sy = 1;

      if (soldier.direction != 1)
      {
        if (gs.flip)
        {
          cy = 1 - gs.cy;
          tex = tex + 1;
        }
        else
          sy = -1;
      }

      cx = cx * textures[tex]->width * textures[tex]->scale;
      cy = cy * textures[tex]->height * textures[tex]->scale;
      color[gs.color].a = alpha[gs.alpha];

      if ((i >= GOSTEK_HAIR_DREADLOCK1) && (i <= GOSTEK_HAIR_DREADLOCK5))
      {
        v = gfxmat3mul(m, -cy * soldier.direction, cx);
        x1 = x1 + v.x;
        y1 = y1 + v.y;
        cx = 0;
        cy = 0.5 * textures[tex]->height * textures[tex]->scale;
        sx = 0.75 + (1 - 0.75) / 5 * (i - GOSTEK_HAIR_DREADLOCK1);
      }
      else if (gs.flex > 0)
      {
        sx = min(1.5f, sqrt(sqr(x2 - x1) + sqr(y2 - y1)) / gs.flex);
      }

      drawgosteksprite(textures[tex], x1, y1 + 1, sx, sy, cx, cy, r, color[gs.color]);
    }
  }
}

// initialization
//  LoadDefaults();
