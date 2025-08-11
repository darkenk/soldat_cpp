// automatically converted

#include "Bullets.hpp"

#ifdef SERVER
#include "../../server/Server.hpp"
#include "../network/NetworkServer.hpp"
#include "../network/NetworkServerBullet.hpp"
#else
#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/GameRendering.hpp"
#include "../../client/Sound.hpp"
#include "../network/NetworkClientBullet.hpp"
#endif
#include <Tracy.hpp>
#include <math.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <limits>
#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <utility>

#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "common/Calc.hpp"
#include "common/Logging.hpp"
#include "common/Util.hpp"
#include "common/gfx.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "client/Gfx.hpp"
#include "common/Anims.hpp"
#include "common/MapFile.hpp"
#include "common/Parts.hpp"
#include "common/PolyMap.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/misc/RandomGenerator.hpp"
#include "common/misc/SafeType.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Sparks.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/mechanics/Things.hpp"
#include "shared/network/Net.hpp"

#if SOLDAT_UTBOT
constexpr auto pi = 3.14159265358979323846;
#else
#include <numbers>

using std::numbers::pi;
#endif // SOLDAT_UTBOT

template <Config::Module M>
auto GetBulletParts() -> particlesystem &
{
  static particlesystem b;
  return b;
}

template particlesystem &GetBulletParts();

template <Config::Module M>
auto createbullet(tvector2 spos, tvector2 svelocity, std::uint8_t snum, std::int32_t sowner,
                  std::uint8_t n, float hitm, bool net, bool mustcreate,
                  std::uint16_t seed) -> std::int32_t // Seed = -1
{
  auto &sprite_system = SpriteSystem::Get();
#ifndef SERVER
  std::int32_t j;
#endif
  float mass;
  std::int16_t weaponindex;
  std::uint8_t sstyle;

  std::int32_t result;
  result = -1;

  auto &guns = GS::GetWeaponSystem().GetGuns();

  LogTraceG("CreateBullet {} {} {}", snum, sowner, n);

#ifndef SERVER
  {
    if (gGlobalStateDemo.demoplayer.active())
    {
      if (!mustcreate)
      {
        return result;
      }
    }
  }
#endif

  weaponindex = weaponnumtoindex(snum, guns);
  sstyle = guns[weaponindex].bulletstyle;

  if (!mustcreate && (sowner > 0))
  {
    if (
#ifndef SERVER
      (!sprite_system.IsPlayerSprite(sowner)) &&
#endif
      (sprite_system.GetSprite(sowner).player->controlmethod != bot) &&
      ((sprite_system.GetSprite(sowner).weapon.fireinterval > fireinterval_net) ||
       (((sstyle == bullet_style_fragnade) || (sstyle == bullet_style_clusternade))
#ifndef SERVER
        && false
#endif
        )))
    {
#ifndef SERVER
      if (bulletcansend(spos.x, spos.y, gGlobalStateClient.camerafollowsprite, svelocity.x))
#endif
      {
        return result;
      }
    }
  }

#ifdef SERVER
  LogTraceG("CreateBullet 2");
#endif

  auto &bullet = GS::GetBulletSystem().GetBullets();

  auto f = [&bullet](const auto n) -> std::int32_t {
    if (n != 255)
    {
      return n;
    }
    for (auto i = 1; i <= max_bullets + 1; i++)
    {
      if (i == max_bullets + 1)
      {
        return -1;
      }
      if (!bullet[i].active)
      {
        return i;
      }
    }
    return -1;
  };
  const auto i = f(n);
  if (i == -1)
  {
    return i;
  }

  // i is now the active sprite
  LogTraceG("CreateBullet 3");

  auto &b = bullet[i];

  // activate sprite
  b.active = true;
#ifndef SERVER
  b.hashit = false;
#endif
  b.style = sstyle;
  b.num = i;
  b.owner = sowner;
  b.timeout = guns[weaponindex].timeout;
#ifndef SERVER // TODO: Check if this should be used also in server
  b.timeoutprev = guns[weaponindex].timeout;
#endif
  b.hitmultiply = hitm;
#ifndef SERVER // TODO: Check if this should be used also in server
  b.hitmultiplyprev = hitm;
#endif
  b.whizzed = false;

  if (
#ifndef SERVER
    (sowner == gGlobalStateClient.camerafollowsprite) ||
#endif
    (sstyle == bullet_style_flamearrow) || (sstyle == bullet_style_flame))
  {
    b.whizzed = true;
  }

  if (sprite_system.GetSprite(sowner).player->controlmethod == human)
  {
    b.ownerpingtick = sprite_system.GetSprite(sowner).player->pingticksb;
  }
  else
  {
    b.ownerpingtick = 0;
  }

  b.ownerweapon = snum;
  b.hitbody = 0;
  b.hitspot.x = 0;
  b.hitspot.y = 0;
  b.tracking = 0;

#ifndef SERVER // TODO: Check if this should be used also in server
  if (sprite_system.GetSprite(sowner).aimdistcoef < defaultaimdist)
  {
    b.tracking = 255;
  }

  b.imagestyle = sprite_system.GetSprite(sowner).weapon.bulletimagestyle;
#else
  b.initial = spos;
#endif
  b.startuptime = GS::GetGame().GetMainTickCounter();
  b.ricochetcount = 0;
#ifndef SERVER // TODO: Check if this should be used also in server
  b.degradecount = 0;
  b.pingadd = 0;
  b.pingaddstart = 0;
#endif

  if (seed == std::numeric_limits<std::uint16_t>::max())
  {
    if (sprite_system.GetSprite(sowner).bulletcount == std::numeric_limits<std::uint16_t>::max())
    {
      sprite_system.GetSprite(sowner).bulletcount = 0;
    }
    else
    {
      sprite_system.GetSprite(sowner).bulletcount += 1;
    }
    seed = sprite_system.GetSprite(sowner).bulletcount;
  }
  b.seed = seed;

#ifdef SERVER
  LogTraceG("CreateBullet 4");
#endif

  mass = 1.0;

  if (!mustcreate)
  {
    if (sstyle == bullet_style_flame)
    {
      spos.x = spos.x + svelocity.x;
      spos.y = spos.y + svelocity.y;
    }
  }

  b.initial = spos;

#ifdef SERVER
  LogTraceG("CreateBullet 5");
#endif
  // activate sprite part
  GetBulletParts().createpart(spos, svelocity, mass, i);

#ifndef SERVER

  // SEND BULLLET THROUGH NETWORK
  if (net)
  {
    if ((sprite_system.IsPlayerSprite(sowner)) &&
        (gGlobalStateClientGame.clientstopmovingcounter > 0))
    {
      if ((sprite_system.GetSprite(sowner).weapon.fireinterval > fireinterval_net) || mustcreate ||
          (b.style == bullet_style_fragnade) || (b.style == bullet_style_clusternade) ||
          (b.style == bullet_style_cluster))
      {
        clientsendbullet(i);
        // Damage multiplier hack was here, they recalled ClientSendBullet
      }
      else if ((sprite_system.GetSprite(sowner).weapon.fireinterval <= fireinterval_net) &&
               (sprite_system.GetSprite(sowner).burstcount == 0) &&
               (GS::GetGame().GetMainTickCounter() >
                gGlobalStateClientGame.lastforceclientspritesnapshotmovtick + fireinterval_net))
      {
        gGlobalStateClientGame.forceclientspritesnapshotmov = true;
        gGlobalStateClientGame.lastforceclientspritesnapshotmovtick =
          GS::GetGame().GetMainTickCounter();
      }
    }
  }

  if (sprite_system.IsPlayerSprite(sowner))
  {
    if (b.style == bullet_style_fragnade)
    {
      if (gGlobalStateClient.wepstats[18].shots == 0)
      {
        gGlobalStateClient.wepstats[18].textureid = GFX::INTERFACE_NADE;
      }
      gGlobalStateClient.wepstats[18].shots = gGlobalStateClient.wepstats[18].shots + 1;
      if (gGlobalStateClient.wepstatsnum == 0)
      {
        gGlobalStateClient.wepstatsnum = 1;
      }
      if (gGlobalStateClient.wepstats[18].name.empty())
      {
        gGlobalStateClient.wepstats[18].name = "Grenade";
      }
    }
    else if ((b.style == bullet_style_cluster) || (b.style == bullet_style_clusternade))
    {
      if (gGlobalStateClient.wepstats[19].shots == 0)
      {
        gGlobalStateClient.wepstats[19].textureid = GFX::INTERFACE_CLUSTER_NADE;
      }
      gGlobalStateClient.wepstats[19].shots = gGlobalStateClient.wepstats[19].shots + 1;
      if (gGlobalStateClient.wepstatsnum == 0)
      {
        gGlobalStateClient.wepstatsnum = 1;
      }
      if (gGlobalStateClient.wepstats[19].name.empty())
      {
        gGlobalStateClient.wepstats[19].name = "Clusters";
      }
    }
    else if (b.style == bullet_style_m2)
    {
      if (gGlobalStateClient.wepstats[20].shots == 0)
      {
        gGlobalStateClient.wepstats[20].textureid = GFX::INTERFACE_GUNS_M2;
      }
      gGlobalStateClient.wepstats[20].shots = gGlobalStateClient.wepstats[20].shots + 1;
      if (gGlobalStateClient.wepstatsnum == 0)
      {
        gGlobalStateClient.wepstatsnum = 1;
      }
      if (gGlobalStateClient.wepstats[20].name.empty())
      {
        gGlobalStateClient.wepstats[20].name = "Stationary gun";
      }
    }
    else if (b.style == bullet_style_punch)
    {
      if (gGlobalStateClient.wepstats[17].shots == 0)
      {
        gGlobalStateClient.wepstats[17].textureid = GFX::INTERFACE_GUNS_FIST;
      }
      gGlobalStateClient.wepstats[17].shots = gGlobalStateClient.wepstats[17].shots + 1;
      if (gGlobalStateClient.wepstatsnum == 0)
      {
        gGlobalStateClient.wepstatsnum = 1;
      }
      if (gGlobalStateClient.wepstats[17].name.empty())
      {
        gGlobalStateClient.wepstats[17].name = "Hands";
      }
    }
    else
    {
      j = sprite_system.GetPlayerSprite().weapon.num;
      if (j == noweapon_num)
      {
        j = 17;
      }

      if (gGlobalStateClient.wepstats[j].shots == 0)
      {
        switch (j)
        {
        case 0:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_SOCOM;
          break;
        case 1:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_DEAGLES;
          break;
        case 2:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_MP5;
          break;
        case 3:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_AK74;
          break;
        case 4:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_STEYR;
          break;
        case 5:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_SPAS;
          break;
        case 6:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_RUGER;
          break;
        case 7:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_M79;
          break;
        case 8:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_BARRETT;
          break;
        case 9:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_MINIMI;
          break;
        case 10:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_MINIGUN;
          break;
        case 11:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_KNIFE;
          break;
        case 12:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_CHAINSAW;
          break;
        case 13:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_LAW;
          break;
        case 14:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_FLAMER;
          break;
        case 15:
        case 16:
          gGlobalStateClient.wepstats[j].textureid = GFX::INTERFACE_GUNS_BOW;
          break;
        }
      }
      gGlobalStateClient.wepstats[j].shots = gGlobalStateClient.wepstats[j].shots + 1;
      if (gGlobalStateClient.wepstatsnum == 0)
      {
        gGlobalStateClient.wepstatsnum = 1;
      }
      if (gGlobalStateClient.wepstats[j].name.empty())
      {
        gGlobalStateClient.wepstats[j].name = sprite_system.GetPlayerSprite().weapon.name;
      }
    }
  }
#else
  if (net)
  {
    LogTraceG("CreateBullet 6");

    if (sprite_system.GetSprite(sowner).weapon.fireinterval > fireinterval_net)
    {
      serverbulletsnapshot(i, 0, false);
    }
    else
    {
      for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
      {
        if ((sprite.player->controlmethod == human) && (sprite.num != b.owner) &&
            (sprite.player->port == 0) && sprite.isspectator())
        {
          serverbulletsnapshot(i, sprite.num, false);
        }
      }
    }
  }
#endif

  // b.Update;
#ifdef SERVER
  LogTraceG("CreateBullet 7");
#endif

  result = i;
  return result;
}
#ifdef SERVER
auto servercreatebullet(tvector2 spos, tvector2 svelocity, std::uint8_t snum, std::int32_t sowner,
                        std::uint8_t n, float hitm, bool net) -> std::int32_t
{
  std::int32_t i;

  std::int32_t result;
  result = -1;
  auto &bullet = GS::GetBulletSystem().GetBullets();

  if ((sowner <= 0) || (sowner >= max_sprites))
  {
    return result;
  }

  i = createbullet(spos, svelocity, snum, sowner, n, hitm, net, true, 0);

  bullet[i].ownerpingtick = 0;
  bullet[i].dontcheat = true;

  if (net)
  {
    serverbulletsnapshot(i, 0, true);
  }

  result = i;
  return result;
}
#endif

template <Config::Module M>
auto bulletcansend(float x, float y, const std::int32_t i, float vx) -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  float sx;
  float sy;

  bool result;
  LogTraceG("BulletCanSend");
  result = false;

  if ((i > max_players) || (i < 0))
  {
    return result;
  }

  if (i == 0)
  {
    result = true;
    return result;
  }

  const auto &spritePartsPos = sprite_system.GetSpritePartsPos(i);

  sx = spritePartsPos.x -
       ((float)((spritePartsPos.x - sprite_system.GetSprite(i).control.mouseaimx)) / 2);
  sy = spritePartsPos.y -
       ((float)((spritePartsPos.y - sprite_system.GetSprite(i).control.mouseaimy)) / 2);
#ifdef SERVER
  if ((x > (sx - max_game_width)) && (x < (sx + max_game_width)) && (y > (sy - 480)) &&
      (y < (sy + 480)))
  {
    result = true;
  }
  if ((x > (sx - 1640)) && (x < (sx + 1640)) && (y > (sy - 680)) && (y < (sy + 680)))
  {
    if (((sx < x) && (vx < 0)) || ((sx > x) && (vx > 0)))
    {
      result = true;
    }
  }
#else
  if ((x > (sx - gGlobalStateGame.gamewidth)) && (x < (sx + gGlobalStateGame.gamewidth)) &&
      (y > (sy - gGlobalStateGame.gameheight)) && (y < (sy + gGlobalStateGame.gameheight)))
  {
    result = true;
  }

  if ((x > (sx - 1640)) && (x < (sx + 1640)) && (y > (sy - 680)) && (y < (sy + 680)))
  {
    if (((sx < x) && (vx < 0)) || ((sx > x) && (vx > 0)))
    {
      result = true;
    }
  }
#endif
  return result;
}

template bool bulletcansend<Config::GetModule()>(float x, float y, std::int32_t i, float vx);

#ifndef SERVER
auto canhitspray(std::int32_t victim, std::int32_t attacker) -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  bool result;
  result = false;

  if (sprite_system.IsPlayerSprite(victim))
  {
    if ((victim == attacker) or (CVar::sv_friendlyfire) or
        sprite_system.GetSprite(victim).issolo() or
        sprite_system.GetSprite(victim).isnotinsameteam(sprite_system.GetSprite(attacker)))
    {
      result = true;
    }
  }
  return result;
}

void hitspray()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int16_t bink;

  bink = sprite_system.GetPlayerSprite().weapon.bink;
  if (bink > 0)
  {
    gGlobalStateClient.hitspraycounter = calculatebink(gGlobalStateClient.hitspraycounter, bink);
  }
}

void calculaterecoil(float px, float py, float &cx, float &cy, float da)
{
  auto &sprite_system = SpriteSystem::Get();
  float dx;
  float dy;
  float radius;
  float alpha;
  float displacementx;
  // M: TVector2;

  displacementx = 0;
  // If cursor is on right side displace recoil circle to left,
  // otherwise to right
  if (cx > px) // Test to find out best Result for displacement_x
  {
    px = px - displacementx;
  }
  else
  {
    px = px + displacementx;
  }

  // Calculate delta x/y and radius
  dx = cx - px;
  dy = cy - py;
  radius = sqrt(dx * dx + dy * dy);

  // Depending on is cursor is on left/right side do different things
  if (dx > 0)
  {
    alpha = atan(dy / dx);
    alpha = alpha + da;

    if (alpha > pi / 2)
    {
      alpha = pi / 2;
    }
  }
  else
  {
    alpha = atan(dy / dx) + pi;
    alpha = alpha - da;
    if (alpha < pi / 2)
    {
      alpha = pi / 2;
    }
  }

  // Calculate new x/y for cursor
  // cx := (px + Cos(alpha) * radius);

  // If Soldat uses a coordinate system where y < 0 at top instead of bottom,
  // this needs subtraction instead of addition
  // cy := (py - Sin(alpha) * radius);

  // M.X := (dx / SpriteSystem::Get().GetSprite(CameraFollowSprite).AimDistCoef) * 3;
  // M.Y := (dy / SpriteSystem::Get().GetSprite(CameraFollowSprite).AimDistCoef) * 3;

  cx = px + ((dx + cos(alpha) * radius) /
             sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).aimdistcoef) *
              (sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).aimdistcoef / 2);
  cy = py + ((dy + sin(alpha) * radius) /
             sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).aimdistcoef) *
              (sprite_system.GetSprite(gGlobalStateClient.camerafollowsprite).aimdistcoef / 2);

  // Lastly make sure that cursor is still at the same side as it was first
  if (dx > 0)
  {
    if (cx <= px)
    {
      cx = (px + 0.0001); // We add a floatly small value like 0.001 to
                          // distinguish that the cursos is on right side
                          // (if it was right in the middle it'd be possible to
                          // interpret as it being on left or right side)
    }
  }
  else
  {
    if (cx >= px)
    {
      cx = (px - 0.0001);
    }
  }
}
#endif

// TBULLET
template <Config::Module M>
void Bullet<M>::update()
{
  [[maybe_unused]] auto &sprite_system = SpriteSystem::Get();
  tvector2 oldv;
  tvector2 a;
  float dist;
  tvector2 oldp;
  tvector2 oldop;
  tvector2 hitp;
  tvector2 hitp2;
  tvector2 hitp3;

  LogTraceG("TBullet.Update");

  timeoutprev = timeout;
  hitmultiplyprev = hitmultiply;
  velocityprev = GetBulletParts().velocity[num];

  oldv = GetBulletParts().velocity[num];
  oldp = GetBulletParts().pos[num];
  oldop = GetBulletParts().oldpos[num];
  dist = -1;
  hitp2.x = 0;
  hitp3.x = 0;

  checkoutofbounds();

  // check collision with map
  if (style != bullet_style_fragnade)
  {
    hitp = checkmapcollision(GetBulletParts().pos[num].x, GetBulletParts().pos[num].y);
  }
  else
  {
    hitp = checkmapcollision(GetBulletParts().pos[num].x, GetBulletParts().pos[num].y - 2);
    hitp = checkmapcollision(GetBulletParts().pos[num].x, GetBulletParts().pos[num].y);
  }

  if (!active)
  {
    a = vec2subtract(hitp, oldop);
    dist = vec2length(a);

    GetBulletParts().velocity[num] = oldv;
    ricochetcount -= 1;
    GetBulletParts().pos[num] = oldp;
    GetBulletParts().oldpos[num] = oldop;
  }

  // check if hit collider
  hitp2 = checkcollidercollision(dist);

  if (!active)
  {
    if (hitp2.x == 0)
    {
      a = vec2subtract(hitp, oldop);
    }
    else
    {
      a = vec2subtract(hitp2, oldop);
    }

    dist = vec2length(a);
    GetBulletParts().velocity[num] = oldv;
    GetBulletParts().pos[num] = oldp;
    GetBulletParts().oldpos[num] = oldop;
  }

  // check if hit sprites
  hitp3 = checkspritecollision(dist);

  if (!active)
  {
    if (hitp3.x == 0)
    {
      if (hitp2.x == 0)
      {
        a = vec2subtract(hitp, oldop);
      }
      else
      {
        a = vec2subtract(hitp2, oldop);
      }
    }
    else
    {
      a = vec2subtract(hitp3, oldop);
    }

    dist = vec2length(a);
  }

  // check if hit things
  hitp = checkthingcollision(dist);

  // count Time Out
  timeout -= 1;
  if (timeout == 0)
  {
    switch (style)
    {
    case bullet_style_plain:
    case bullet_style_shotgun:
    case bullet_style_flame:
    case bullet_style_punch:
    case bullet_style_arrow:
    case bullet_style_clusternade:
    case bullet_style_knife:
    case bullet_style_thrownknife:
      kill();
      break;
    case bullet_style_fragnade:
    case bullet_style_m79:
    case bullet_style_flamearrow:
    case bullet_style_law: {
      hit(hit_type_fragnade);
      kill();
    }
    break;
    case bullet_style_cluster: {
      hit(hit_type_cluster);
      kill();
    }
    break;
    case bullet_style_m2: {
      hit(hit_type_flak);
      kill();
    }
    break;
    } // case
  }   // TimeOut = 0

  // lose power on distance
  if (timeout % 6 == 0)
  {
    if ((ownerweapon != barrett_num) && (ownerweapon != m79_num) && (ownerweapon != knife_num) &&
        (ownerweapon != law_num))
    {
      a = vec2subtract(initial, GetBulletParts().pos[num]);
      dist = vec2length(a);

      if (degradecount == 0)
      {
        if (dist > 500)
        {
          hitmultiply = hitmultiply * 0.5;
          degradecount += 1;
        }
      }
      else if (degradecount == 1)
      {
        if (dist > 900)
        {
          hitmultiply = hitmultiply * 0.5;
          degradecount += 1;
        }
      }
    }
  }

#ifndef SERVER
  // Bullet Tracking
  if (sprite_system.IsPlayerSprite(owner))
  {
    if (tracking == 255)
    {
      tracking = owner;
    }
  }

  if (sprite_system.IsPlayerSprite(tracking))
  {
    if (sprite_system.GetSprite(owner).position != pos_stand)
    {
      gGlobalStateClient.camerax =
        GetBulletParts().pos[num].x + 5 * GetBulletParts().velocity[num].x;
      gGlobalStateClient.cameray =
        GetBulletParts().pos[num].y + 5 * GetBulletParts().velocity[num].y;
    }
    else
    {
      tracking = 0;
    }
  }

  // his sound
  if ((timeout == bullet_timeout - 25) && (style != bullet_style_shotgun))
  {
    playsound(SfxEffect::bulletby, GetBulletParts().pos[num]);
  }

  // whiizz above head
  if (!whizzed)
  {
    if (style != bullet_style_punch)
    {
      if (gGlobalStateClient.camerafollowsprite > 0)
      {
        const auto &spritePartsPos =
          sprite_system.GetSpritePartsPos(gGlobalStateClient.camerafollowsprite);
        if ((GetBulletParts().pos[num].x > spritePartsPos.x - 200) &&
            (GetBulletParts().pos[num].x < spritePartsPos.x + 200) &&
            (GetBulletParts().pos[num].y > spritePartsPos.y - 350) &&
            (GetBulletParts().pos[num].y < spritePartsPos.y + 100))
        {
          playsound(SfxEffect::bulletby2 + Random(4), GetBulletParts().pos[num]);
          whizzed = true;
        }
      }
    }
  }

  // fire for flaming arrow
  if (style == bullet_style_flamearrow)
  {
    // smoke
    if (Random(2) == 0)
    {
      createspark(GetBulletParts().pos[num], vector2(0, -0.5), 37, num, 40);
    }

    if (Random(2) == 0)
    {
      createspark(GetBulletParts().pos[num], vector2(0, -0.5), 36, num, 40);
    }
  }

  // law missile smoke
  if (style == bullet_style_law)
  {
    // smoke
    createspark(GetBulletParts().pos[num], vector2(0, -1.5), 59, num, 50);
    if (Random(2) == 0)
    {
      createspark(GetBulletParts().pos[num], GetBulletParts().velocity[num], 2, num, 5);
    }
  }
#endif

  // flame
  if (style == bullet_style_flame)
  {
    GetBulletParts().forces[num].y = GetBulletParts().forces[num].y - 0.15;
  }

#ifndef SERVER
  // bleed
  if (hitbody > 0)
  {
    if (Random(5) == 0)
    {
      createspark(
        GetBulletParts().pos[num],
        vector2(GetBulletParts().velocity[num].x * 0.5, GetBulletParts().velocity[num].y * 0.5), 4,
        owner, 90);
    }
  }
#endif
}

#ifndef SERVER
template <Config::Module M>
void Bullet<M>::render(double timeelapsed)
{
  auto &sprite_system = SpriteSystem::Get();
  tvector2 bulletpos;
  tvector2 bulletvel;
  tvector2 _p;
  tvector2 _p2;
  tvector2 _scala;
  tvector2 a;
  tvector2 b;
  float roto;
  float alfa;
  float grenvel = 0.0;
  float ox;
  float oy;
  float sinusvar;

  tgfxspritearray &t = gGlobalStateGameRendering.textures;
  bulletpos = GetBulletParts().pos[num];

  auto &map = GS::GetGame().GetMap();

  if (CVar::sv_realisticmode)
  {
    if ((owner > 0) && (owner < max_sprites + 1))
    {
      if (sprite_system.GetSprite(owner).IsActive())
      {
        if (sprite_system.GetSprite(owner).visible == 0)
        {
          if (map.raycast(bulletpos, sprite_system.GetPlayerSprite().skeleton.pos[9], grenvel,
                          gGlobalStateGame.gamewidth, true))
          {
            return;
          }
        }
      }
    }
  }

  bulletvel = GetBulletParts().velocity[num];
  sinusvar = sin(timeoutfloat + 5.1 * timeelapsed);

  auto &guns = GS::GetWeaponSystem().GetGuns();

  switch (style)
  {
  case bullet_style_plain: // bullet
    if (timeoutfloat < bullet_timeout - 2)
    {
      if ((imagestyle != guns[eagle].bulletimagestyle) &&
          (imagestyle != guns[mp5].bulletimagestyle) &&
          (imagestyle != guns[ak74].bulletimagestyle) &&
          (imagestyle != guns[steyraug].bulletimagestyle) &&
          (imagestyle != guns[ruger77].bulletimagestyle) &&
          (imagestyle != guns[barrett].bulletimagestyle) &&
          (imagestyle != guns[m249].bulletimagestyle) &&
          (imagestyle != guns[minigun].bulletimagestyle) &&
          (imagestyle != guns[colt].bulletimagestyle))
      {
        imagestyle = guns[colt].bulletimagestyle;
      }

      _p.x = bulletpos.x + bulletvel.x;
      _p.y = bulletpos.y + bulletvel.y;
      _p2.x = _p.x - bulletvel.x;
      _p2.y = _p.y - bulletvel.y;
      _scala.x = (float)(vec2length(bulletvel)) / bullettrail;
      _scala.y = 1;
      roto = -angle2points(_p, _p2);

      alfa = ((hitmultiply * _scala.x * _scala.x) / 4.63) * 255;

      if (alfa > 230)
      {
        alfa = 230;
      }
      if (alfa < 50)
      {
        alfa = 50;
      }

      if (pingadd < 1)
      {
        gfxdrawsprite(t[imagestyle], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                      rgba(0xffffff, round(alfa)));
      }

      if (pingadd > 0)
      {
        a.x = bulletpos.x - initial.x;
        a.y = bulletpos.y - initial.y;
        b.x = vec2length(a) *
              min((float)(1) / bulletlength, ((float)((pingadd + 2)) / pingaddstart) / bullettrail);
        b.y = 1;

        if (active)
        {
          alfa = alfa / 6;
        }
        else
        {
          alfa = alfa / 4;
        }
        gfxdrawsprite(t[imagestyle], _p.x, _p.y, b.x, b.y, 0, 0, roto, rgba(0xffffff, round(alfa)));
      }

      if (hitbody > 0)
      {
        if (gGlobalStateClient.trails == 1)
        {
          if (timeoutfloat < bullet_timeout - 7)
          {
            _p.x = bulletpos.x;
            _p.y = bulletpos.y;
            _scala.x = fabs((float)(vec2length(bulletvel)) / 4);
            _scala.y = 1;
            gfxdrawsprite(t[imagestyle], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                          rgba(0xffdddd, bulletalpha / 2));
          }
        }
      }
      else
      {
        if (gGlobalStateClient.trails == 1)
        {
          if (timeoutfloat < bullet_timeout - 7)
          {
            _p.x = bulletpos.x;
            _p.y = bulletpos.y;
            _scala.x = fabs(vec2length(bulletvel) / 3.5);
            _scala.y = 1;
            gfxdrawsprite(t[imagestyle], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                          rgba(0xffffff, bulletalpha / 2));
          }
        }
      }
    }
    break;

  case bullet_style_fragnade: {
    grenvel = vec2length(bulletvel);
    if (gGlobalStateClient.trails == 1)
    {
      if (timeoutfloat < grenade_timeout - 3)
      {
        if (bulletvel.y > 0)
        {
          ox = -1;
        }
        else
        {
          ox = 1;
        }
        if (bulletvel.x > 0)
        {
          oy = 1;
        }
        else
        {
          oy = -1;
        }

        _p.x = bulletpos.x + ox;
        _p.y = bulletpos.y - 3 + oy;
        _p2.x = _p.x - bulletvel.x;
        _p2.y = _p.y - bulletvel.y;
        roto = -angle2points(_p, _p2);
        _scala.x = grenvel / 3;
        _scala.y = 1;
        gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                      rgba(0x64ff64, round(bulletalpha * 0.75)));
      }
    }

    _p.x = bulletpos.x - 1;
    _p.y = bulletpos.y - 4;
    _p2.x = GetBulletParts().oldpos[num].x - 1;
    _p2.y = GetBulletParts().oldpos[num].y - 4;
    gfxdrawsprite(t[GFX::WEAPONS_FRAG_GRENADE], _p.x, _p.y);
  }
  break;

  case bullet_style_shotgun:
    if (timeoutfloat < bullet_timeout - 2)
    {
      _p.x = bulletpos.x + bulletvel.x;
      _p.y = bulletpos.y + bulletvel.y;
      _p2.x = _p.x - bulletvel.x;
      _p2.y = _p.y - bulletvel.y;
      roto = -angle2points(_p, _p2);
      gfxdrawsprite(t[GFX::WEAPONS_SPAS_BULLET], _p.x, _p.y, 0, 0, roto, rgba(0xffffff, 150));

      if (gGlobalStateClient.trails == 1)
      {
        if (timeoutfloat < bullet_timeout - 3)
        {
          _p.x = bulletpos.x;
          _p.y = bulletpos.y;
          _scala.x = fabs((float)(vec2length(bulletvel)) / 9);
          _scala.y = 1;
          gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                        rgba(0xffffff, bulletalpha / 5));
        }
      }
    }
    break;

  case bullet_style_m79:
    if (timeoutfloat < bullet_timeout - 2)
    {
      _p.x = bulletpos.x;
      _p.y = bulletpos.y + 1;
      _p2.x = _p.x - bulletvel.x;
      _p2.y = _p.y - bulletvel.y;
      _scala.x = 1;
      _scala.y = 1;
      roto = -angle2points(_p, _p2);
      gfxdrawsprite(t[GFX::WEAPONS_M79_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0,
                    degtorad(timeoutfloat * 6), rgba(0xffffff, 252));

      if (gGlobalStateClient.trails == 1)
      {
        if (timeoutfloat < bullet_timeout - 4)
        {
          if (bulletvel.y > 0)
          {
            ox = -1;
          }
          else
          {
            ox = 1;
          }
          if (bulletvel.x > 0)
          {
            oy = 1;
          }
          else
          {
            oy = -1;
          }

          _p.x = bulletpos.x + ox;
          _p.y = bulletpos.y + oy;
          _scala.x = fabs((float)(vec2length(bulletvel)) / 4);
          _scala.y = 1.3;
          gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                        rgba(0xffff55, bulletalpha));
        }
      }
    }
    break;

  case bullet_style_flame: {
    _p.x = bulletpos.x - 8;
    _p.y = bulletpos.y - 17;
    if ((timeoutfloat > 0) && (timeoutfloat <= flamer_timeout))
    {
      gfxdrawsprite(t[GFX::SPARKS_FLAMES_EXPLODE16 - (std::int32_t)trunc(timeoutfloat / 2)], _p.x,
                    _p.y);
    }
  }
  break;

  case bullet_style_arrow:
    if (timeoutfloat < bullet_timeout - 2)
    {
      _p.x = bulletpos.x + bulletvel.x;
      _p.y = bulletpos.y + bulletvel.y;
      _p2.x = _p.x - bulletvel.x;
      _p2.y = _p.y - bulletvel.y;
      roto = -angle2points(_p, _p2);
      gfxdrawsprite(t[GFX::WEAPONS_ARROW], _p.x, _p.y, 0, 0, roto);

      if (gGlobalStateClient.trails == 1)
      {
        if (timeoutfloat > arrow_resist)
        {
          _p.x = bulletpos.x;
          _p.y = bulletpos.y;
          _scala.x = fabs((float)(vec2length(bulletvel)) / 3);
          _scala.y = 1;
          gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                        rgba(0xffffff, bulletalpha / 7));
        }
      }
    }
    break;

  case bullet_style_flamearrow:
    if (timeoutfloat < bullet_timeout - 2)
    {
      _p.x = bulletpos.x + bulletvel.x;
      _p.y = bulletpos.y + bulletvel.y;
      _p2.x = _p.x - bulletvel.x;
      _p2.y = _p.y - bulletvel.y;
      roto = -angle2points(_p, _p2);
      gfxdrawsprite(t[GFX::WEAPONS_ARROW], _p.x, _p.y, 0, 0, roto);
    }
    break;

  case bullet_style_clusternade: {
    _p.x = bulletpos.x;
    _p.y = bulletpos.y - 3;
    gfxdrawsprite(t[GFX::WEAPONS_CLUSTER_GRENADE], _p.x, _p.y, 0, 0,
                  degtorad(timeoutfloat * 5) * iif(bulletvel.x < 0, -1, 1));
  }
  break;

  case bullet_style_cluster: {
    _p.x = bulletpos.x;
    _p.y = bulletpos.y - 2;
    gfxdrawsprite(t[GFX::WEAPONS_CLUSTER], _p.x, _p.y);
  }
  break;

  case bullet_style_law:
    if (timeoutfloat < bullet_timeout - 2)
    {
      _p.x = bulletpos.x + bulletvel.x;
      _p.y = bulletpos.y + bulletvel.y;
      _p2.x = _p.x - bulletvel.x;
      _p2.y = _p.y - bulletvel.y;
      roto = -angle2points(_p, _p2);
      gfxdrawsprite(t[GFX::WEAPONS_MISSILE], _p.x, _p.y, 0, 0, roto);

      if (gGlobalStateClient.trails == 1)
      {
        if (timeoutfloat < bullet_timeout - 7)
        {
          _p.x = bulletpos.x;
          _p.y = bulletpos.y;
          _scala.x = fabs((float)(vec2length(bulletvel)) / 3);
          _scala.y = 1;
          gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                        rgba(0xffffff, bulletalpha / 5));
        }
      }
    }
    break;

  case bullet_style_thrownknife: {
    _p.x = bulletpos.x + bulletvel.x;
    _p.y = bulletpos.y + bulletvel.y;
    roto = timeoutfloat / pi;

    if (bulletvel.x >= 0)
    {
      gfxdrawsprite(t[GFX::WEAPONS_KNIFE], _p.x, _p.y, 4, 1, roto);
    }
    else
    {
      gfxdrawsprite(t[GFX::WEAPONS_KNIFE2], _p.x, _p.y, 4, 1, -roto);
    }
  }
  break;

  case bullet_style_m2:
    if (timeoutfloat < m2bullet_timeout - 2)
    {
      _p.x = bulletpos.x + bulletvel.x;
      _p.y = bulletpos.y + bulletvel.y;
      _p2.x = _p.x - bulletvel.x;
      _p2.y = _p.y - bulletvel.y;
      _scala.x = fabs((float)(vec2length(bulletvel)) / bullettrail);
      _scala.y = 1.2;
      roto = -angle2points(_p, _p2);
      gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                    rgba(0xffbf77, bulletalpha * 2));

      if (gGlobalStateClient.trails == 1)
      {
        if (timeoutfloat < m2bullet_timeout - 13)
        {
          _p.x = bulletpos.x;
          _p.y = bulletpos.y;
          _scala.x = fabs((float)(vec2length(bulletvel)) / 3);
          _scala.y = 1;
          gfxdrawsprite(t[GFX::WEAPONS_BULLET], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                        rgba(0xffffff, bulletalpha / 5));

          _scala.x = fabs(vec2length(bulletvel) / (sinusvar + 2.5));
          _scala.y = sinusvar;
          gfxdrawsprite(t[GFX::WEAPONS_SMUDGE], _p.x, _p.y, _scala.x, _scala.y, 0, 0, roto,
                        rgba(0xffffff, bulletalpha / 6));
        }
      }
    }
    break;
  }
}
#endif

template <Config::Module M>
void Bullet<M>::kill()
{
#ifdef SERVER
  LogTraceG("TBullet.Kill");
#endif

  active = false;
  if (num > 0)
  {
    GetBulletParts().active[num] = false;
  }
  setlength(thingcollisions, 0);
}

template <Config::Module M>
auto Bullet<M>::checkmapcollision(float x, float y) -> tvector2
{
  ZoneScopedN("Bullet::CheckMapCollision");
  auto &sprite_system = SpriteSystem::Get();
  float largestvelocitycomponent;
  std::int32_t b;
  tvector2 pos;
  tvector2 perp;
  tvector2 step;
  tvector2 temp;
  tvector2 temp2;
  float d = 0.0;
  std::int32_t detacc;
  std::int32_t tempint = 0;
  bool teamcol;

  tvector2 result;

#ifdef SERVER
  LogTraceG("TBullet.CheckMapCollision");
#endif
  auto &map = GS::GetGame().GetMap();

  result = pos;

  // make step
  largestvelocitycomponent =
    max(fabs(GetBulletParts().velocity[num].x), fabs(GetBulletParts().velocity[num].y));

  detacc = trunc(largestvelocitycomponent / 2.5);

  if (detacc == 0)
  {
    detacc = 1;
  }
  vec2scale(step, GetBulletParts().velocity[num], (float)(1) / detacc);

  // make steps for accurate collision detection
  for (b = 0; b <= detacc - 1; b++)
  {
    pos.x = x + b * step.x;
    pos.y = y + b * step.y;

    // iterate through maps sector polygons
    const auto sector = map.GetSector(pos);
    if (!sector.IsValid())
    {
      kill();
      return result;
    }

    for (const auto &w : sector.GetPolys())
    {
      const auto &polytype = w.Type;
      teamcol = teamcollides(polytype, sprite_system.GetSprite(owner).player->team, true);
      if (teamcol)
      {
        if ((polytype != poly_type_only_player) && (polytype != poly_type_doesnt) &&
            (polytype != poly_type_only_flaggers) && (polytype != poly_type_not_flaggers) &&
            (polytype != poly_type_background) && (polytype != poly_type_background_transition))
        {
          if (map.pointinpolyedges(pos.x, pos.y, w.Index))
          {
            switch (style)
            {
            case bullet_style_plain:
            case bullet_style_shotgun:
            case bullet_style_punch:
            case bullet_style_knife:
            case bullet_style_m2: {
              GetBulletParts().oldpos[num] = GetBulletParts().pos[num];
              GetBulletParts().pos[num] = vec2subtract(pos, GetBulletParts().velocity[num]);
              temp = GetBulletParts().pos[num];
              temp2 = GetBulletParts().velocity[num];

              perp = vec2subtract(GetBulletParts().pos[num], hitspot);
              d = vec2length(perp);
              // ricochet!
              if (d > 50.0)
              {
                ricochetcount += 1;
                perp = map.closestperpendicular(w.Index, GetBulletParts().pos[num], d, tempint);
                d = vec2length(GetBulletParts().velocity[num]);
                vec2normalize(perp, perp);
                vec2scale(perp, perp, -d);

                GetBulletParts().velocity[num].x =
                  GetBulletParts().velocity[num].x * ((float)(25) / 35) +
                  perp.x * ((float)(10) / 35);
                GetBulletParts().velocity[num].y =
                  GetBulletParts().velocity[num].y * ((float)(25) / 35) +
                  perp.y * ((float)(10) / 35);
                GetBulletParts().pos[num] = pos;
                hitspot = GetBulletParts().pos[num];

                vec2normalize(perp, GetBulletParts().velocity[num]);
                vec2scale(perp, perp, d / 6);
                GetBulletParts().oldpos[num] = GetBulletParts().pos[num];
                pos.x = GetBulletParts().pos[num].x + perp.x;
                pos.y = GetBulletParts().pos[num].y + perp.y;
                const auto sector = map.GetSector(pos);
                if (sector.IsValid())
                {
                  for (const auto &w2 : sector.GetPolys())
                  {
                    const auto &polytype = w2.Type;
                    if ((polytype != poly_type_only_player) && (polytype != poly_type_doesnt) &&
                        (polytype != poly_type_only_flaggers) &&
                        (polytype != poly_type_not_flaggers) &&
                        (polytype != poly_type_background) &&
                        (polytype != poly_type_background_transition) and
                        teamcollides(polytype, sprite_system.GetSprite(owner).player->team, true))
                    {
                      if (map.pointinpolyedges(pos.x, pos.y, w2.Index))
                      {
                        kill();
                        break;
                      }
                    }
                  }
                }
              }
              else
              {
                kill();
              }

              if (active)
              {
                GetBulletParts().pos[num] = temp;
                perp = GetBulletParts().velocity[num];
                GetBulletParts().velocity[num] = temp2;
                hit(hit_type_ricochet);
                GetBulletParts().pos[num] = hitspot;
                GetBulletParts().velocity[num] = perp;
              }
              else
              {
                GetBulletParts().pos[num] = temp;
                perp = GetBulletParts().velocity[num];
                GetBulletParts().velocity[num] = temp2;
                hit(hit_type_wall);
                GetBulletParts().pos[num] = hitspot;
                GetBulletParts().velocity[num] = perp;
              }
            }
            break;
            case bullet_style_arrow: {
              GetBulletParts().pos[num] = vec2subtract(pos, GetBulletParts().velocity[num]);
              GetBulletParts().forces[num].y =
                GetBulletParts().forces[num].y - GetBulletParts().gravity;
              if (timeout > arrow_resist)
              {
                timeout = arrow_resist;
              }
              if (timeout < 20)
              {
                GetBulletParts().forces[num].y =
                  GetBulletParts().forces[num].y + GetBulletParts().gravity;
              }
            }
            break;
            case bullet_style_fragnade:
            case bullet_style_flame: {
#ifndef SERVER
              // bounce sound
              if (style == bullet_style_fragnade)
              {
                if (vec2length(GetBulletParts().velocity[num]) > 1.5)
                {
                  const auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);
                  playsound(SfxEffect::grenade_bounce, spritePartsPos);
                }
              }
#endif

              perp = map.closestperpendicular(w.Index, GetBulletParts().pos[num], d, tempint);

              vec2normalize(perp, perp);
              vec2scale(perp, perp, d);

              GetBulletParts().pos[num] = pos;
              GetBulletParts().velocity[num] = vec2subtract(GetBulletParts().velocity[num], perp);

              vec2scale(GetBulletParts().velocity[num], GetBulletParts().velocity[num],
                        grenade_surfacecoef);

              if (style == bullet_style_flame)
              {
                if (timeout > 16)
                {
                  timeout = 16;
                }
              }
            }
            break;
            case bullet_style_m79:
            case bullet_style_flamearrow:
            case bullet_style_law: {
              GetBulletParts().oldpos[num] = GetBulletParts().pos[num];
              GetBulletParts().pos[num] = vec2subtract(pos, GetBulletParts().velocity[num]);
              temp = GetBulletParts().pos[num];
              temp2 = GetBulletParts().velocity[num];

              perp = vec2subtract(GetBulletParts().pos[num], hitspot);
              d = vec2length(perp);
              // ricochet!
              if (d > 50.0)
              {
                ricochetcount += 1;
                perp = map.closestperpendicular(w.Index, GetBulletParts().pos[num], d, tempint);
                d = vec2length(GetBulletParts().velocity[num]);
                vec2normalize(perp, perp);
                vec2scale(perp, perp, -d);

                GetBulletParts().velocity[num].x =
                  GetBulletParts().velocity[num].x * ((float)(25) / 35) +
                  perp.x * ((float)(10) / 35);
                GetBulletParts().velocity[num].y =
                  GetBulletParts().velocity[num].y * ((float)(25) / 35) +
                  perp.y * ((float)(10) / 35);
                GetBulletParts().pos[num] = pos;
                hitspot = GetBulletParts().pos[num];

                vec2normalize(perp, GetBulletParts().velocity[num]);
                vec2scale(perp, perp, d / 6);

                pos.x = GetBulletParts().pos[num].x + perp.x;
                pos.y = GetBulletParts().pos[num].y + perp.y;
                const auto sector = map.GetSector(pos);
                if (sector.IsValid())
                {
                  for (const auto &w2 : sector.GetPolys())
                  {
                    const auto &polytype = w2.Type;
                    if ((polytype != poly_type_only_player) && (polytype != poly_type_doesnt) &&
                        (polytype != poly_type_only_flaggers) &&
                        (polytype != poly_type_not_flaggers) &&
                        (polytype != poly_type_background) &&
                        (polytype != poly_type_background_transition) and
                        teamcollides(polytype, sprite_system.GetSprite(owner).player->team, true))
                    {
                      if (map.pointinpolyedges(pos.x, pos.y, w2.Index))
                      {
                        kill();
                        break;
                      }
                    }
                  }
                }
              }
              else
              {
                kill();
              }

              if (active)
              {
                GetBulletParts().pos[num] = temp;
                perp = GetBulletParts().velocity[num];
                GetBulletParts().velocity[num] = temp2;
                hit(hit_type_ricochet);
                GetBulletParts().pos[num] = hitspot;
                GetBulletParts().velocity[num] = perp;
              }
              else
              {
                GetBulletParts().pos[num] = temp;
                perp = GetBulletParts().velocity[num];
                GetBulletParts().velocity[num] = temp2;
                hit(hit_type_explode);
                GetBulletParts().pos[num] = hitspot;
                GetBulletParts().velocity[num] = perp;
              }
            }
            break;
            case bullet_style_clusternade: {
              hit(hit_type_clusternade);
              kill();
            }
            break;
            case bullet_style_cluster: {
              hit(hit_type_cluster);
              kill();
            }
            break;
            case bullet_style_thrownknife: {
              GetBulletParts().pos[num] = vec2subtract(pos, GetBulletParts().velocity[num]);

              // create knife thing
#ifdef SERVER
              creatething(GetBulletParts().pos[num], owner, object_combat_knife, 255);
#endif

              hit(hit_type_wall);
              kill();
            }
            break;
            } // case

            result = pos;
            return result;
          } // PointinPolyEdges
        }
      }
    }       // for j
  }         // for b
  return result;
}

template <Config::Module M>
auto Bullet<M>::checkspritecollision(float lasthitdist) -> tvector2
{
  ZoneScopedN("Bullet::CheckSpriteCollision");
  auto &sprite_system = SpriteSystem::Get();
  const std::array<std::int32_t, 7> bodypartspriority = {{12, 11, 10, 6, 5, 4, 3}};
  tspriteindexes spritesbydistance;
  std::int32_t spritecount;
  std::int32_t spritecounter;
  std::int32_t j;
  tvector2 pos;
  tvector2 norm;
  tvector2 colpos;
  tvector2 a;
  tvector2 col;
  tvector2 bulletvelocity;
  particlesystem *candidateskeleton;
  std::int32_t bodypartpriorityindex;
  std::int32_t bodypartid;
  tvector2 bodypartoffset;
  tvector2 buttstockpositionoffset;
  tvector2 startpoint;
  tvector2 endpoint;
  std::uint8_t where;
  std::uint8_t weaponindex;
  float dist;
  float mindist;
  std::int32_t r;
  tvector2 bulletpush;
  std::int32_t pushtick;
  float hitboxmodifier;
  float speed;
  bool wasdead;
  std::uint8_t nocollision;
#ifndef SERVER
  std::uint8_t clothesshreadstyle;
  std::int32_t i;
  std::int32_t srv = 0;
#endif

  tvector2 result;
  LogTraceG("TBullet.CheckSpriteCollision {} {} {} {} {}", style, owner, ownerweapon, timeout,
            tracking);

  if ((style == bullet_style_arrow) && (timeout <= arrow_resist))
  {
    return result;
  }

  auto &guns = GS::GetWeaponSystem().GetGuns();

  pos.x = 0;
  result = pos;

  bulletvelocity = GetBulletParts().velocity[num];

  LogTraceG("TBullet.CheckSpriteCollision 2");
  // Iterate through sprites
  if (style != bullet_style_clusternade)
  {
    spritecount = filterspritesbydistance(spritesbydistance);
    spritecounter = 0;

    while (spritecounter < spritecount)
    {
      spritecounter = spritecounter + 1;
      j = spritesbydistance[spritecounter];

      col = getspritecollisionpoint(j);

      where = 0;
      if (style != bullet_style_fragnade)
      {
        r = part_radius;
      }
      else
      {
        r = part_radius + 1;
      }

#ifdef SERVER
      LogTraceG("TBullet.CheckSpriteCollision 3 -- {}", j);
#endif

      candidateskeleton = &sprite_system.GetSprite(j).skeleton;

      // Pre-calculate some points if it's a melee weapon
      if ((style == bullet_style_punch) || (style == bullet_style_knife))
      {
        pos = GetBulletParts().pos[num];

        buttstockpositionoffset = sprite_system.GetSprite(owner).gethandsaimdirection();
        vec2scale(buttstockpositionoffset, buttstockpositionoffset, 4);

        startpoint =
          vec2add(sprite_system.GetSprite(owner).skeleton.pos[15], buttstockpositionoffset);
        endpoint = vec2add(pos, bulletvelocity);
      }
      else
      {
        startpoint = GetBulletParts().pos[num];
        endpoint = vec2add(startpoint, bulletvelocity);
      }

      // Check for collision with the body parts
      mindist = std::numeric_limits<float>::max();

      for (bodypartpriorityindex = 0; bodypartpriorityindex <= high(bodypartspriority);
           bodypartpriorityindex++)
      {
        bodypartid = bodypartspriority[bodypartpriorityindex];
        const auto &spritePartsPos = sprite_system.GetSpritePartsPos(j);
        bodypartoffset = vec2subtract(candidateskeleton->pos[bodypartid], spritePartsPos);
        colpos = vec2add(col, bodypartoffset);

        // FIXME(skoskav): Offset player sprite 2px to the left because sprites are glitchy
        // like that (don't need to do it for melee weapons though because their offsets
        // cancel each other out)
        if ((style != bullet_style_punch) && (style != bullet_style_knife))
        {
          colpos.x = colpos.x - 2;
        }

        if (linecirclecollision(startpoint, endpoint, colpos, r, pos))
        {
          dist = sqrdist(startpoint, pos);

          if (dist < mindist)
          {
            where = bodypartid;
            mindist = dist;
          }
        }
      }

#ifdef SERVER
      LogTraceG("TBullet.CheckSpriteCollision 4 ");
#endif

      if (((style != bullet_style_punch) && (style != bullet_style_knife)) || (j != owner))
      {
        if (where > 0)
        {
          // order collision
          if (lasthitdist > -1)
          {
            a = vec2subtract(pos, GetBulletParts().oldpos[num]);
            dist = vec2length(a);

            if (dist > lasthitdist)
            {
              break;
            }
          }

          sprite_system.GetSprite(j).brain.pissedoff = owner;

          norm = vec2subtract(pos, sprite_system.GetSprite(j).skeleton.pos[where]);
          vec2scale(norm, norm, 1.3);
          norm.y = -norm.y;

#ifdef SERVER
          LogTraceG("TBullet.CheckSpriteCollision 5 {}", where);
#endif

          result = pos;

          nocollision = guns[weaponnumtoindex(ownerweapon, guns)].nocollision;

          if (((nocollision & weapon_nocollision_enemy) != 0) and
              (sprite_system.GetSprite(j).isnotinsameteam(sprite_system.GetSprite(owner))))
          {
            continue;
          }

          if (((nocollision & weapon_nocollision_team) != 0) and
              (sprite_system.GetSprite(j).isinsameteam(sprite_system.GetSprite(owner))) &&
              (j != owner))
          {
            continue;
          }

          if (((nocollision & weapon_nocollision_self) != 0) && (owner == j))
          {
            continue;
          }

          if (sprite_system.GetSprite(j).ceasefirecounter < 0)
          {
            weaponindex = getweaponindex();

            // Collision respond
            if (!sprite_system.GetSprite(j).deadmeat)
            {
              if ((style != bullet_style_fragnade) && (style != bullet_style_flame) &&
                  (style != bullet_style_arrow))
              {
                vec2scale(bulletpush, bulletvelocity, guns[weaponindex].push);
                pushtick = sprite_system.GetSprite(j).player->pingticks / 2 + ownerpingtick + 1;
                if (pushtick > max_pushtick)
                {
                  pushtick = max_pushtick;
                }
                sprite_system.GetSprite(j).nextpush[pushtick] =
                  vec2add(sprite_system.GetSprite(j).nextpush[pushtick], bulletpush);
              }
            }

            switch (style)
            {
            case bullet_style_plain:
            case bullet_style_shotgun:
            case bullet_style_punch:
            case bullet_style_knife:
            case bullet_style_m2: {
              GetBulletParts().pos[num] = pos;

#ifndef SERVER
              // Blood spark
              if ((CVar::sv_friendlyfire) or sprite_system.GetSprite(owner).issolo() or
                  sprite_system.GetSprite(owner).isnotinsameteam(sprite_system.GetSprite(j)) or
                  (j == owner))
              {
                hit(hit_type_blood);
              }

              // Shake screen
              if ((sprite_system.IsPlayerSprite(owner)) && (ownerweapon == chainsaw_num))
              {
                gGlobalStateClient.camerax = gGlobalStateClient.camerax - 3 + Random(7);
                gGlobalStateClient.cameray = gGlobalStateClient.cameray - 3 + Random(7);
              }

              // Puff
              vec2normalize(a, bulletvelocity);
              vec2scale(a, a, 3);
              a = vec2add(GetBulletParts().pos[num], a);
              if (CVar::r_maxsparks > (max_sparks - 10))
              {
                createspark(a, a, 50, j, 31);
              }

              // Shread clothes
              if (CVar::r_maxsparks > (max_sparks - 10))
              {
                if (where <= 4)
                {
                  clothesshreadstyle = 49; // Piece of clothing with pants
                }
                else if (where <= 11)
                {
                  clothesshreadstyle = 48; // Piece of clothing
                }
                else
                {
                  clothesshreadstyle = 0;
                }

                if (clothesshreadstyle > 0)
                {
                  for (i = 1; i <= 2; i++)
                  {
                    if (Random(8) == 0)
                    {
                      a.x = sin(Random(100));
                      a.y = cos(Random(100));
                      createspark(pos, a, clothesshreadstyle, j, 120);
                    }
                  }
                }
              }

              // play hit sound
              if (sprite_system.GetSprite(j).vest < 1)
              {
                if (!sprite_system.GetSprite(j).deadmeat)
                {
                  playsound(SfxEffect::hit_arg + Random(3), GetBulletParts().pos[num]);
                }
                else
                {
                  playsound(SfxEffect::dead_hit, GetBulletParts().pos[num]);
                }
              }
              else
              {
                playsound(SfxEffect::vesthit, GetBulletParts().pos[num]);
              }
#endif

              // Head, torso or leg hitbox modifier
              if (where <= 4)
              {
                hitboxmodifier = guns[weaponnumtoindex(ownerweapon, guns)].modifierlegs;
              }
              else if (where <= 11)
              {
                hitboxmodifier = guns[weaponnumtoindex(ownerweapon, guns)].modifierchest;
              }
              else
              {
                hitboxmodifier = guns[weaponnumtoindex(ownerweapon, guns)].modifierhead;
              }

              speed = vec2length(bulletvelocity);

              wasdead = sprite_system.GetSprite(j).deadmeat;

              sprite_system.GetSprite(j).healthhit(
#ifndef SERVER
                srv *
#endif
                  speed * hitmultiply * hitboxmodifier,
                owner, where, num, norm);

#ifndef SERVER
              // hit Spray
              if (canhitspray(j, owner))
              {
                hitspray();
              }
#endif

#ifdef SERVER
              LogTraceG("TBullet.CheckSpriteCollision a ");
#endif

              // drop weapon when punched
              if (style == bullet_style_punch)
              {
                if (sprite_system.GetSprite(j).issolo() or
                    (sprite_system.GetSprite(j).isnotsolo() and
                     sprite_system.GetSprite(j).isnotinsameteam(sprite_system.GetSprite(owner))))
                {
                  if ((sprite_system.GetSprite(j).weapon.num != bow_num) &&
                      (sprite_system.GetSprite(j).weapon.num != bow2_num))
                  {
                    sprite_system.GetSprite(j).bodyapplyanimation(AnimationType::ThrowWeapon, 11);
                  }
                }
              }

              hitbody = j;

              // Pierce check and break to next sprite
              if (wasdead)
              {
                vec2scale(GetBulletParts().velocity[num], bulletvelocity, 0.9);
                bulletvelocity = GetBulletParts().velocity[num];
                hit(hit_type_bodyhit);
                continue;
              }

              if (sprite_system.GetSprite(j).deadmeat or (speed > 23))
              {
                vec2scale(GetBulletParts().velocity[num], bulletvelocity, 0.75);
                bulletvelocity = GetBulletParts().velocity[num];
                hit(hit_type_bodyhit);
                continue;
              }

              if ((speed > 5) && (speed / guns[weaponindex].speed >= 0.9))
              {
                vec2scale(GetBulletParts().velocity[num], bulletvelocity, 0.66);
                bulletvelocity = GetBulletParts().velocity[num];
                hit(hit_type_bodyhit);
                continue;
              }

              // Destroy bullet
              kill();
            }
            break;

            case bullet_style_fragnade: {
              if (!sprite_system.GetSprite(j).deadmeat)
              {
                hit(hit_type_fragnade, j, where);
                kill();
              }
            }
            break;

            case bullet_style_arrow:
              if (timeout > arrow_resist)
              {
                GetBulletParts().pos[num] = vec2subtract(pos, GetBulletParts().velocity[num]);
                GetBulletParts().forces[num].y =
                  GetBulletParts().forces[num].y - GetBulletParts().gravity;
                if (((CVar::sv_friendlyfire) && sprite_system.GetSprite(owner).isnotsolo() and
                     sprite_system.GetSprite(owner).isinsameteam(sprite_system.GetSprite(j))
#ifndef SERVER
                     && (!sprite_system.IsPlayerSprite(num))
#endif
                       ) ||
                    (sprite_system.GetSprite(j).bonusstyle == bonus_flamegod))
                {
                  ;
                }
                else
                {
                  hit(hit_type_blood);
                }
#ifndef SERVER
                // play hit sound
                if (sprite_system.GetSprite(j).vest < 1)
                {
                  if (!sprite_system.GetSprite(j).deadmeat)
                  {
                    playsound(SfxEffect::hit_arg + Random(3), GetBulletParts().pos[num]);
                  }
                  else
                  {
                    playsound(SfxEffect::dead_hit, GetBulletParts().pos[num]);
                  }
                }
                else
                {
                  playsound(SfxEffect::vesthit, GetBulletParts().pos[num]);
                }
#endif

#ifdef SERVER
                LogTraceG("TBullet.CheckSpriteCollision b ");
#endif

                // Head, torso or leg hitbox modifier
                if (where <= 4)
                {
                  hitboxmodifier = guns[weaponnumtoindex(ownerweapon, guns)].modifierlegs;
                }
                else if (where <= 11)
                {
                  hitboxmodifier = guns[weaponnumtoindex(ownerweapon, guns)].modifierchest;
                }
                else
                {
                  hitboxmodifier = guns[weaponnumtoindex(ownerweapon, guns)].modifierhead;
                }

                speed = vec2length(GetBulletParts().velocity[num]);

                sprite_system.GetSprite(j).healthhit(
#ifndef SERVER
                  srv *
#endif
                    speed * hitmultiply * hitboxmodifier,
                  owner, where, num, norm);

                if (!sprite_system.GetSprite(j).deadmeat)
                {
                  sprite_system.GetSprite(j).skeleton.pos[where] = a;
                }

                kill();
              }
              break;

            case bullet_style_m79:
            case bullet_style_flamearrow:
            case bullet_style_law:
              if (!sprite_system.GetSprite(j).deadmeat)
              {
                hit(hit_type_explode, j, where);
                GetBulletParts().pos[num] = pos;
                kill();

                sprite_system.GetSprite(j).healthhit(
#ifndef SERVER
                  srv *
#endif
                    vec2length(GetBulletParts().velocity[num]) * hitmultiply,
                  owner, where, num, norm);

#ifdef SERVER
                LogTraceG("TBullet.CheckSpriteCollision c ");
#endif

                if (!sprite_system.GetSprite(j).deadmeat)
                {
                  sprite_system.GetSprite(j).skeleton.pos[where] = a;
                }
              }
              break;

            case bullet_style_flame:
              if (owner != j)
              {
                const auto &spriteVelocity = sprite_system.GetVelocity(j);
                GetBulletParts().pos[num] = sprite_system.GetSprite(j).skeleton.pos[where];
                if (!sprite_system.GetSprite(j).deadmeat)
                {
                  GetBulletParts().velocity[num] = spriteVelocity;
                }
                else
                {
                  GetBulletParts().velocity[num].x = 0;
                  GetBulletParts().velocity[num].y = 0;
                }

#ifdef SERVER
                if ((timeout < 3) && (ricochetcount < 2))
#else
                if ((timeout < 2) && (ricochetcount < 1))
#endif
                {
                  if (hitmultiply >= guns[flamer].hitmultiply / 3)
                  {
                    timeout = flamer_timeout - 1;
                    ricochetcount += 1;
                    a.x = -spriteVelocity.x;
                    a.y = -spriteVelocity.y;
                    createbullet(sprite_system.GetSprite(j).skeleton.pos[where], a, flamer_num,
                                 owner, 255, (float)(2 * hitmultiply) / 3,
                                 //                        False, {$IFDEF
                                 //                        SERVER}False{$ELSE}True{$ENDIF});
                                 false, true);
                  }

                  if (sprite_system.GetSprite(j).GetHealth() > -1)
                  {
                    sprite_system.GetSprite(j).healthhit(
#ifndef SERVER
                      srv *
#endif
                        hitmultiply,
                      owner, where, num, norm);
                  }
                }
              }
              break;

            case bullet_style_cluster: {
#ifdef SERVER
              LogTraceG("TBullet.CheckSpriteCollision f ");
#endif

              hit(hit_type_cluster, j, where);

              if (!sprite_system.GetSprite(j).deadmeat)
              {
                sprite_system.GetSprite(j).skeleton.pos[where] = a;
              }

              kill();
            }
            break;

            case bullet_style_thrownknife: {
#ifndef SERVER
              // Blood spark
              if ((CVar::sv_friendlyfire) or sprite_system.GetSprite(owner).issolo() or
                  sprite_system.GetSprite(owner).isnotinsameteam(sprite_system.GetSprite(j)) or
                  (j == owner))
              {
                hit(hit_type_blood);
              }

              // Puff
              vec2normalize(a, GetBulletParts().velocity[num]);
              vec2scale(a, a, 3);
              a = vec2add(GetBulletParts().pos[num], a);
              if (CVar::r_maxsparks > (max_sparks - 10))
              {
                createspark(a, a, 50, j, 31);
              }

              // Shread clothes
              if (CVar::r_maxsparks > (max_sparks - 10))
              {
                if (where <= 4)
                {
                  clothesshreadstyle = 49; // Piece of clothing with pants
                }
                else if (where <= 11)
                {
                  clothesshreadstyle = 48; // Piece of clothing
                }
                else
                {
                  clothesshreadstyle = 0;
                }

                if (clothesshreadstyle > 0)
                {
                  for (i = 1; i <= 2; i++)
                  {
                    if (Random(8) == 0)
                    {
                      a.x = sin(Random(100));
                      a.y = cos(Random(100));
                      createspark(pos, a, clothesshreadstyle, j, 120);
                    }
                  }
                }
              }

              // play hit sound
              if (std::find(spritecollisions.begin(), spritecollisions.end(),
                            sprite_system.GetSprite(j).num) != spritecollisions.end())
              {
                NotImplemented("Check spritecollisions");
#if 0
                                include(spritecollisions, SpriteSystem::Get().GetSprite(j).num);
#endif

                if (sprite_system.GetSprite(j).vest < 1)
                {
                  if (!sprite_system.GetSprite(j).deadmeat)
                  {
                    playsound(SfxEffect::hit_arg + Random(3), GetBulletParts().pos[num]);
                  }
                  else
                  {
                    playsound(SfxEffect::dead_hit, GetBulletParts().pos[num]);
                  }
                }
                else
                {
                  playsound(SfxEffect::vesthit, GetBulletParts().pos[num]);
                }
              }
#endif

#ifdef SERVER
              LogTraceG("TBullet.CheckSpriteCollision g ");
#endif

              wasdead = sprite_system.GetSprite(j).deadmeat;

              sprite_system.GetSprite(j).healthhit(
#ifndef SERVER
                srv *
#endif
                  vec2length(GetBulletParts().velocity[num]) * hitmultiply * 0.01,
                owner, where, num, norm);

              if (!sprite_system.GetSprite(j).deadmeat)
              {
                sprite_system.GetSprite(j).skeleton.pos[where] = a;
              }

              if ((!wasdead) or (CVar::sv_realisticmode))
              {
#ifdef SERVER
                // create knife thing
                creatething(GetBulletParts().pos[num], owner, object_combat_knife, 255);
#endif
                kill();
              }
            }
            break;
            } // case

            // Bullet is destroyed, so exit
            return result;
          }
        }
      }

#ifdef SERVER
      LogTraceG("TBullet.CheckSpriteCollision 7 ");
#endif
    } // while SpriteCounter < SpriteCount
  }
  return result;
}

template <Config::Module M>
auto Bullet<M>::checkthingcollision(float lasthitdist) -> tvector2
{
  std::int32_t i;
  std::int32_t j;
  tvector2 startpoint;
  tvector2 endpoint;
  tvector2 pos;
  tvector2 colpos;
  tvector2 a;
  std::uint8_t where;
  float dist;
  tvector2 thingvel;
  tvector2 veldiff;
  tvector2 bulletpush;
  std::uint8_t newindex;
  bool skipcollision;

  tvector2 result;
#ifdef SERVER
  LogTraceG("TBullet.CheckThingCollision");
#endif

  pos.x = 0;
  result = pos;
  auto &guns = GS::GetWeaponSystem().GetGuns();
  auto &things = GS::GetThingSystem().GetThings();

  // iterate through Things
  if (style != bullet_style_fragnade)
  {
    for (j = 1; j <= max_things; j++)
    {
      if (things[j].active && (timeout < bullet_timeout - 1) && things[j].collidewithbullets and
          (owner != things[j].holdingsprite) &&
          (((CVar::sv_gamemode == gamestyle_inf) && (things[j].style == object_bravo_flag)) ||
           (CVar::sv_gamemode != gamestyle_inf)) &&
          (things[j].style != object_stationary_gun))
      {
        startpoint = GetBulletParts().pos[num];
        endpoint = vec2add(startpoint, GetBulletParts().velocity[num]);

        where = 0;

        for (i = 1; i <= 2; i++)
        {
          colpos = things[j].skeleton.pos[i];

          if (linecirclecollision(startpoint, endpoint, colpos, flag_part_radius, pos))
          {
            where = i;
            break;
          }
        }

        if ((where == 1) || (where == 2))
        {
          // order collision
          if (lasthitdist > -1)
          {
            a = vec2subtract(pos, GetBulletParts().oldpos[num]);
            dist = vec2length(a);
            if (dist > lasthitdist)
            {
              break;
            }
          }

          // Thing push cooldown from this bullet
          skipcollision = false;

          for (i = 0; i <= high(thingcollisions); i++)
          {
            if (thingcollisions[i].thingnum == things[j].num)
            {
              if (GS::GetGame().GetMainTickCounter() < thingcollisions[i].cooldownend)
              {
                skipcollision = true;
                break;
              }
            }
          }

          if (skipcollision)
          {
            break;
          }

          // TODO: has high performance impact? -> do alloc like std vector
          newindex = length(thingcollisions);
          setlength(thingcollisions, newindex + 1);
          thingcollisions[newindex] = thingcollision(
            things[j].num, GS::GetGame().GetMainTickCounter() + thing_collision_cooldown);

          // collision respond
          thingvel = vec2subtract(things[j].skeleton.pos[where], things[j].skeleton.oldpos[where]);
          veldiff = vec2subtract(GetBulletParts().velocity[num], thingvel);

          vec2scale(bulletpush, veldiff, guns[getweaponindex()].push * thing_push_multiplier);

          things[j].skeleton.pos[where] = vec2add(things[j].skeleton.pos[where], bulletpush);

          result = pos;
          things[j].statictype = false;

          switch (style)
          {
          case bullet_style_plain:
          case bullet_style_fragnade:
          case bullet_style_shotgun: {
            hit(hit_type_thing);
          }
          break;
          }

          break;
        }
      } // for j
    }
  }
  return result;
}

template <Config::Module M>
auto Bullet<M>::checkcollidercollision(float lasthitdist) -> tvector2
{
  ZoneScopedN("Bullet::CheckColliderCollision");
  std::int32_t j;
#ifndef SERVER
  std::int32_t i;
#endif
  tvector2 startpoint;
  tvector2 endpoint;
  tvector2 pos;
  tvector2 colpos;
  tvector2 a;
  float dist;

  LogTraceG("TBullet.CheckColliderCollision");

  tvector2 result;

  auto &map = GS::GetGame().GetMap();

  pos.x = 0;
  result = pos;

  // iterate through colliders
  for (j = 1; j <= 128; j++)
  {
    if (map.collider[j].active)
    {
      startpoint = GetBulletParts().pos[num];
      endpoint = vec2add(startpoint, GetBulletParts().velocity[num]);
      // make steps for accurate collision detection

      colpos.x = map.collider[j].x;
      colpos.y = map.collider[j].y;

      if (linecirclecollision(startpoint, endpoint, colpos, map.collider[j].radius / 1.7, pos))
      {
        // order collision
        if (lasthitdist > -1)
        {
          a = vec2subtract(pos, GetBulletParts().oldpos[num]);
          dist = vec2length(a);
          if (dist > lasthitdist)
          {
            break;
          }
        }

        switch (style)
        {
        case bullet_style_plain:
        case bullet_style_shotgun:
        case bullet_style_punch:
        case bullet_style_knife:
        case bullet_style_thrownknife:
        case bullet_style_m2: {
          GetBulletParts().pos[num] = vec2subtract(pos, GetBulletParts().velocity[num]);

#ifndef SERVER
          // dirt
          if (CVar::r_maxsparks > (max_sparks - 10))
          {
            for (i = 1; i <= 2; i++)
            {
              if (Random(4) == 0)
              {
                a.x = sin(Random(100));
                a.y = cos(Random(100));
                createspark(pos, a, 44 + Random(4), j, 120);
              }
            }
          }
#endif

              // create knife thing
#ifdef SERVER
          if (style == bullet_style_thrownknife)
          {
            creatething(GetBulletParts().pos[num], owner, object_combat_knife, 255);
          }
#endif

#ifndef SERVER
          playsound(SfxEffect::colliderhit, GetBulletParts().pos[num]);
#endif

          hit(hit_type_wall);
          kill();
        }
        break;
        case bullet_style_fragnade: // frag grenade exploded
          if (timeout < grenade_timeout - 2)
          {
            hit(hit_type_fragnade);
            kill();
          }
          break;
        case bullet_style_flame: {
          kill();
        }
        break;
        case bullet_style_arrow:
          if (timeout > arrow_resist)
          {
            GetBulletParts().forces[num].y =
              GetBulletParts().forces[num].y - GetBulletParts().gravity;
            hit(hit_type_wall);

            kill();
          }
          break;
        case bullet_style_m79:
        case bullet_style_flamearrow:
        case bullet_style_law: {
          hit(hit_type_explode); // plays m79 explosion sound
          kill();
        }
        break;
        case bullet_style_clusternade: {
          hit(hit_type_clusternade);
          kill();
        }
        break;
        case bullet_style_cluster: {
          hit(hit_type_cluster);
          kill();
        }
        break;
        } // case

        result = pos;
        return result;
      }
    } // for j
  }
  return result;
}

template <Config::Module M>
void Bullet<M>::hit(std::int32_t t, std::int32_t spritehit, std::int32_t where)
{
  tvector2 a;
  tvector2 b;
  std::int32_t i;

#ifdef SERVER
  LogTraceG("TBullet.Hit");
#endif

  auto &guns = GS::GetWeaponSystem().GetGuns();

  switch (t)
  {
  case hit_type_wall: // wall hit
  {
#ifndef SERVER
    b = GetBulletParts().velocity[num];
    a = vec2add(GetBulletParts().pos[num], GetBulletParts().velocity[num]);
    vec2scale(b, b, -0.06);
    b.y = b.y - 1.0;

    b.x = b.x * (0.6 + (float)(Random(8)) / 10);
    b.y = b.y * (0.8 + (float)(Random(4)) / 10);
    createspark(a, b, 3, owner, 60);

    b.x = b.x * (0.8 + (float)(Random(4)) / 10);
    b.y = b.y * (0.6 + (float)(Random(8)) / 10);
    createspark(a, b, 3, owner, 65);

    vec2scale(b, b, 0.4 + (float)(Random(4)) / 10);
    createspark(a, b, 1, owner, 60);

    b.x = b.x * (0.5 + (float)(Random(4)) / 10);
    b.y = b.y * (0.7 + (float)(Random(8)) / 10);
    createspark(a, b, 3, owner, 50);

    b.x = 0;
    b.y = 0;
    if (CVar::r_maxsparks > (max_sparks - 5))
    {
      createspark(a, b, 56, owner, 22);
    }

    if (timeout < bullet_timeout - 5)
    {
      playsound(SfxEffect::ric + Random(4), GetBulletParts().pos[num]);
    }
#endif
  }
  break;

  case hit_type_blood: // body hit -blood
  {
#ifndef SERVER
    b = GetBulletParts().velocity[num];
    a = GetBulletParts().pos[num];
    vec2scale(b, b, 0.025);
    // Vec3Normalize(b, b);
    b.x = b.x * 1.2;
    b.y = b.y * 0.85;
    createspark(a, b, 4, owner, 70);

    b.x = b.x * 0.745;
    b.y = b.y * 1.1;
    createspark(a, b, 4, owner, 75);
    b.x = b.x * 0.9;
    b.y = b.y * 0.85;
    if (Random(2) == 0)
    {
      createspark(a, b, 4, owner, 75);
    }

    b.x = b.x * 1.2;
    b.y = b.y * 0.85;
    createspark(a, b, 5, owner, 80);

    b.x = b.x * 1;
    b.y = b.y * 1;
    createspark(a, b, 5, owner, 85);

    b.x = b.x * 0.5;
    b.y = b.y * 1.05;
    if (Random(2) == 0)
    {
      createspark(a, b, 5, owner, 75);
    }

    for (i = 1; i <= 7; i++)
    {
      if (Random(6) == 0)
      {
        b.x = sin(Random(100)) * 1.6;
        b.y = cos(Random(100)) * 1.6;
        createspark(a, b, 4, owner, 55);
      }
    }
#endif
  }
  break;

  case hit_type_explode: // 40mm grenade explode
  {
#ifndef SERVER
    a = vector2(0.0, 0.0);

    if (CVar::r_maxsparks > (max_sparks - 10))
    {
      createspark(GetBulletParts().pos[num], a, 60, owner, 255);
    }

    if (CVar::r_maxsparks > (max_sparks - 10))
    {
      createspark(GetBulletParts().pos[num], a, 54, owner, smoke_anims * 4 + 10);
    }

    createspark(GetBulletParts().pos[num], a, 12, owner, explosion_anims * 3);
    playsound(SfxEffect::m79_explosion, GetBulletParts().pos[num]);
#endif

    explosionhit(hit_type_explode, spritehit, where);
  }
  break;

  case hit_type_fragnade: {
#ifndef SERVER
    a = vector2(0.0, 0.0);

    if (CVar::r_maxsparks > (max_sparks - 10))
    {
      createspark(GetBulletParts().pos[num], a, 60, owner, 190);
    }

    if (CVar::r_maxsparks > (max_sparks - 10))
    {
      createspark(GetBulletParts().pos[num], a, 54, owner, smoke_anims * 4 + 10);
    }

    createspark(GetBulletParts().pos[num], a, 17, owner, explosion_anims * 3);
    playsound(SfxEffect::grenade_explosion, GetBulletParts().pos[num]);
#endif

    explosionhit(hit_type_fragnade, spritehit, where);
  }
  break;

  case hit_type_thing: {
#ifndef SERVER
    b = GetBulletParts().velocity[num];
    a = vec2add(GetBulletParts().pos[num], GetBulletParts().velocity[num]);
    vec2scale(b, b, -0.02);
    vec2scale(b, b, 0.4 + (float)(Random(4)) / 10);
    createspark(a, b, 1, owner, 70);

    playsound(SfxEffect::bodyfall, GetBulletParts().pos[num]);
#endif
  }
  break;

  case hit_type_clusternade: {
#ifndef SERVER
    b.x = 0;
    b.y = 0;
    createspark(GetBulletParts().pos[num], b, 29, owner, 55);

    playsound(SfxEffect::clustergrenade, GetBulletParts().pos[num]);
#endif

    a = vec2subtract(GetBulletParts().pos[num], GetBulletParts().velocity[num]);

    for (i = 1; i <= 5; i++)
    {
      b = GetBulletParts().velocity[num];
      vec2scale(b, b, -0.75);
      b.x = -b.x - 2.5 + ((float)(Random(50)) / 10.0f);
      b.y = b.y - 2.5 + ((float)(Random(25)) / 10.0f);
      createbullet(a, b, cluster_num, owner, 255, guns[fraggrenade].hitmultiply / 2.0f, true,
                   false);
    }
  }
  break;

  case hit_type_cluster: {
#ifndef SERVER
    a = vector2(0.0, 0.0);
    createspark(GetBulletParts().pos[num], a, 28, owner, explosion_anims * 3);
    playsound(SfxEffect::cluster_explosion, GetBulletParts().pos[num]);
#endif

    explosionhit(hit_type_cluster, spritehit, where);
  }
  break;

  case hit_type_flak: {
#ifndef SERVER
    a = vector2(0.0, 0.0);
    createspark(GetBulletParts().pos[num], a, 29, owner, 55);
    playsound(SfxEffect::m2explode, GetBulletParts().pos[num]);
#endif

    explosionhit(hit_type_flak, spritehit, where);
  }
  break;

#ifndef SERVER
  case hit_type_bodyhit: {
    if (CVar::r_maxsparks < (max_sparks - 10))
    {
      return;
    }

    b = GetBulletParts().velocity[num];
    a = GetBulletParts().pos[num];
    vec2scale(b, b, 0.075);

    b.x = b.x * 1.2;
    b.y = b.y * 0.85;
    createspark(a, b, 4, owner, 60);
    b.x = b.x * 0.745;
    b.y = b.y * 1.1;
    createspark(a, b, 4, owner, 65);
    b.x = b.x * 1.5;
    b.y = b.y * 0.4;
    createspark(a, b, 5, owner, 70);
    b.x = b.x * 1;
    b.y = b.y * 1;
    createspark(a, b, 5, owner, 75);
    b.x = b.x * 0.4;
    b.y = b.y * 1.15;

    for (i = 1; i <= 4; i++)
    {
      if (Random(6) == 0)
      {
        b.x = sin(Random(100)) * 1.2;
        b.y = cos(Random(100)) * 1.2;
        createspark(a, b, 4, owner, 50);
      }
    }
  }
  break;

  case hit_type_ricochet: // ricochet hit
  {
    a = vec2add(GetBulletParts().pos[num], GetBulletParts().velocity[num]);

    b.x = (-2.0 + (float)(Random(40)) / 10.0);
    b.y = (-2.0 + (float)(Random(40)) / 10.0);
    createspark(a, b, 26, owner, 35);
    b.x = (-2.0 + (float)(Random(40)) / 10.0);
    b.y = (-2.0 + (float)(Random(40)) / 10.0);
    createspark(a, b, 26, owner, 35);
    b.x = (-3.0 + (float)(Random(60)) / 10.0);
    b.y = (-3.0 + (float)(Random(60)) / 10.0);
    createspark(a, b, 26, owner, 35);
    b.x = (-3.0 + (float)(Random(60)) / 10.0);
    b.y = (-3.0 + (float)(Random(60)) / 10.0);
    createspark(a, b, 26, owner, 35);
    b.x = (-3.0 + (float)(Random(60)) / 10.0);
    b.y = (-3.0 + (float)(Random(60)) / 10.0);
    createspark(a, b, 26, owner, 35);
    b.x = (-3.0 + (float)(Random(60)) / 10.0);
    b.y = (-3.0 + (float)(Random(60)) / 10.0);
    createspark(a, b, 27, owner, 35);

    playsound(SfxEffect::ric5 + Random(3), GetBulletParts().pos[num]);
  }
  break;
#endif
  } // case
}

template <Config::Module M>
void Bullet<M>::explosionhit(std::int32_t typ, std::int32_t spritehit, std::int32_t where)
{
  auto &sprite_system = SpriteSystem::Get();
  const float after_explosion_radius2 = after_explosion_radius * after_explosion_radius;
  const std::array<std::int32_t, 7> bodyparts = {{12, 11, 10, 6, 5, 4, 3}};
#ifndef SERVER
  const std::int32_t srv = 0;
#endif
  tvector2 a;
  tvector2 col;
  std::int32_t i;
  std::int32_t j;
  std::int32_t w;
  std::int32_t igun;
  std::int32_t pushtick;
  bool parthit;
  float s;
  float explosionradius;
  float explosionradius2;
  float hitboxmodifier;
  std::uint8_t nocollision;
  float s2 = 0.0;
#ifndef SERVER
  tvector2 b;
  std::int32_t n, rnd;
#endif

  auto &guns = GS::GetWeaponSystem().GetGuns();

  switch (typ)
  {
  case hit_type_fragnade: {
    igun = fraggrenade;
    explosionradius = fraggrenade_explosion_radius;
  }
  break;
  case hit_type_explode: {
    igun = m79;
    explosionradius = m79grenade_explosion_radius;
  }
  break;
  case hit_type_cluster:
  case hit_type_flak: {
    igun = fraggrenade;
    explosionradius = clustergrenade_explosion_radius;
  }
  break;
  default:
    return;
  }

  explosionradius2 = explosionradius * explosionradius;

  // check explosion collision with sprites
  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    if (sprite.isspectator())
    {
      continue;
    }

    nocollision = guns[weaponnumtoindex(ownerweapon, guns)].nocollision;

    if (((nocollision & weapon_nocollision_exp_enemy) != 0) and
        (sprite.isnotinsameteam(sprite_system.GetSprite(owner))))
    {
      continue;
    }

    if (((nocollision & weapon_nocollision_exp_team) != 0) and
        (sprite.isinsameteam(sprite_system.GetSprite(owner))) && (sprite.num != owner))
    {
      continue;
    }

    if (((nocollision & weapon_nocollision_exp_self) != 0) && (owner == sprite.num))
    {
      continue;
    }

    if (!sprite.deadmeat)
    {
      col = getspritecollisionpoint(sprite.num);
      hitboxmodifier = 1.0;
      const auto &spritePartsPos = sprite_system.GetSpritePartsPos(sprite.num);

      // if hitpoint is not given find closest one
      w = where;
      if ((sprite.num != spritehit) || (where == 0))
      {
        s = std::numeric_limits<float>::max();
        for (j = low(bodyparts); j <= high(bodyparts); j++)
        {
          a.x = col.x + (sprite.skeleton.pos[bodyparts[j]].x - spritePartsPos.x);
          a.y = col.y + (sprite.skeleton.pos[bodyparts[j]].y - spritePartsPos.y);
          a = vec2subtract(GetBulletParts().pos[num], a);
          s2 = vec2length2(a);

          if (s2 < s)
          {
            s = s2;           // squared distance
            w = bodyparts[j]; // hitpoint index
          }
        }
      }

      if (w <= 4)
      {
        hitboxmodifier = guns[igun].modifierlegs;
      }
      else if (w <= 11)
      {
        hitboxmodifier = guns[igun].modifierchest;
      }
      else
      {
        hitboxmodifier = guns[igun].modifierhead;
      }

      col.x = col.x + (sprite.skeleton.pos[w].x - spritePartsPos.x);
      col.y = col.y + (sprite.skeleton.pos[w].y - spritePartsPos.y);

      a = vec2subtract(GetBulletParts().pos[num], col);
      s = vec2length2(a);

      if (s < explosionradius2)
      {
        s = sqrt(s);

#ifndef SERVER
        createspark(spritePartsPos, vector2(0, -0.01), 5, owner, 80);
        playsound(SfxEffect::explosion_erg, spritePartsPos);
#endif

        // collision respond
        a.x = (a.x * ((float)(1) / (s + 1)) * explosion_impact_multiply);
        a.y = (a.y * ((float)(1) / (s + 1)) * explosion_impact_multiply);

        if (typ == hit_type_fragnade || typ == hit_type_explode)
        {
          a.y *= 2.0f;
        }
        else
        {
          hitboxmodifier *= 0.5f; // cluster/flak is halved
        }

        pushtick = sprite.player->pingticks / 2 + ownerpingtick + 1;
        pushtick = min(pushtick, max_pushtick);
        sprite.nextpush[pushtick].x -= a.x;
        sprite.nextpush[pushtick].y -= a.y;

        if (sprite.ceasefirecounter < 0)
        {
          s = ((float)(1) / (s + 1)) * guns[igun].hitmultiply * hitboxmodifier;
          sprite.healthhit(
#ifndef SERVER
            srv *
#endif
              s,
            owner, 1, num, a);
        }

#ifndef SERVER
        if (canhitspray(sprite.num, owner))
        {
          hitspray();
        }
#endif
      } // s < explosion radius
    }   // not DeadMeat

    if (sprite.deadmeat)
    {
      parthit = false;

      for (j = 1; j <= 16; j++)
      {
        a = vec2subtract(GetBulletParts().pos[num], sprite.skeleton.pos[j]);
        s = vec2length2(a);

        if (s < explosionradius2)
        {
          s = sqrt(s);
          vec2scale(a, a, ((float)(1) / (s + 1)) * explosion_deadimpact_multiply);
          sprite.skeleton.oldpos[j].x += a.x;
          sprite.skeleton.oldpos[j].y += a.y;

          parthit = true;
          s2 = s;
        }
      }

      if (parthit)
      {
        hitboxmodifier = 1.0;

        if (typ == hit_type_explode)
        {
          s2 = max(s2, 20.0000001f);
        }
        else if (typ == hit_type_cluster || typ == hit_type_flak)
        {
          hitboxmodifier = 0.5;
        }

        s2 = ((float)(1) / (s2 + 1)) * guns[igun].hitmultiply * hitboxmodifier;
        sprite.healthhit(
#ifndef SERVER
          srv *
#endif
            s2,
          owner, 1, num, a);
      }
    }
  } // for SpriteSystem::Get().GetSprite(i)
  auto &things = GS::GetThingSystem().GetThings();

  // check explosion collision with things
  for (i = 1; i <= max_things; i++)
  {
    if (!things[i].active or !things[i].collidewithbullets)
    {
      continue;
    }

    for (j = 1; j <= 4; j++)
    {
      a = vec2subtract(GetBulletParts().pos[num], things[i].skeleton.pos[j]);
      s = vec2length2(a);

      if (s < explosionradius2)
      {
        s = sqrt(s);
        vec2scale(a, a, 0.5 * ((float)(1) / (s + 1)) * explosion_impact_multiply);
        things[i].skeleton.oldpos[j].x += a.x;
        things[i].skeleton.oldpos[j].y += a.y;
        things[i].statictype = false;
      }
    }
  } // for Thing[i]

  if (typ != hit_type_fragnade && typ != hit_type_explode)
  {
    return;
  }

  // check explosion collision with bullets
  active = false;
  auto &bullet = GS::GetBulletSystem().GetBullets();
  for (i = 1; i <= max_bullets; i++)
  {
    auto &b = bullet[i];
    if ((i != num) && b.active &&
        ((b.style == bullet_style_fragnade) || (b.style == bullet_style_m79) ||
         (b.style == bullet_style_law)))
    {
      a = vec2subtract(GetBulletParts().pos[num], GetBulletParts().pos[i]);
      s = vec2length2(a);

      if (s < after_explosion_radius2)
      {
        switch (b.style)
        {
        case bullet_style_fragnade:
          b.hit(hit_type_fragnade);
          break;
        case bullet_style_m79:
          b.hit(hit_type_explode);
          break;
        case bullet_style_law:
          b.hit(hit_type_explode);
          break;
        }

        b.kill();
      }
    }
  }

#ifndef SERVER
  // Grenade Effect
  if (CVar::snd_effects_explosions)
  {
    if (sprite_system.IsPlayerSpriteValid())
    {
      const auto &spritePartsPos =
        sprite_system.GetSpritePartsPos(gGlobalStateClient.mysprite);
      if (sprite_system.GetPlayerSprite().GetHealth() > -50)
      {
        if (distance(GetBulletParts().pos[num], spritePartsPos) < grenadeeffect_dist)
        {
          gGlobalStateClient.grenadeeffecttimer = 320;
          playsound(SfxEffect::hum);
        }
      }
    }
  }

  a = vec2subtract(GetBulletParts().pos[num], GetBulletParts().velocity[num]);

  // dirt fly
  if (CVar::r_maxsparks > (max_sparks - 10))
  {
    n = iif(typ == hit_type_fragnade, 6, 7);
    s = iif(typ == hit_type_fragnade, -0.2, -0.15);

    for (i = 1; i <= n; i++)
    {
      vec2scale(b, GetBulletParts().velocity[num], s);
      b.x = -b.x - 3.5 + ((float)(Random(70)) / 10.0);
      b.y = b.y - 3.5 + ((float)(Random(65)) / 10.0);
      if (Random(4) == 0)
      {
        createspark(a, b, 40, owner, 180 + Random(50));
      }
      if (Random(4) == 0)
      {
        createspark(a, b, 41, owner, 180 + Random(50));
      }
      if (Random(4) == 0)
      {
        createspark(a, b, 42, owner, 180 + Random(50));
      }
      if (Random(4) == 0)
      {
        createspark(a, b, 43, owner, 180 + Random(50));
      }
    }
  }

  // smaller dirt fly
  if (CVar::r_maxsparks > (max_sparks - 10))
  {
    n = iif(typ == hit_type_fragnade, 7, 5);
    s = iif(typ == hit_type_fragnade, -0.2, -0.15);
    rnd = iif(typ == hit_type_fragnade, 4, 3);

    for (i = 1; i <= n; i++)
    {
      vec2scale(b, GetBulletParts().velocity[num], s);
      b.x = -b.x - 3.5 + ((float)(Random(70)) / 10.0);
      b.y = b.y - 3.5 + ((float)(Random(65)) / 10.0);
      if (Random(rnd) == 0)
      {
        createspark(a, b, 44, owner, 120);
      }
      if (Random(rnd) == 0)
      {
        createspark(a, b, 45, owner, 120);
      }
      if (Random(rnd) == 0)
      {
        createspark(a, b, 46, owner, 120);
      }
      if (Random(rnd) == 0)
      {
        createspark(a, b, 47, owner, 120);
      }
    }
  }

  // iskry fly
  if (CVar::r_maxsparks > (max_sparks - 10))
  {
    n = iif(typ == hit_type_fragnade, 3, 4);
    rnd = iif(typ == hit_type_fragnade, 23, 22);

    for (i = 1; i <= n; i++)
    {
      vec2scale(b, GetBulletParts().velocity[num], -0.3);
      b.x = -b.x - 3.5 + ((float)(Random(70)) / 10.0);
      b.y = b.y - 3.5 + ((float)(Random(65)) / 10.0);
      if (Random(rnd) == 0)
      {
        createspark(a, b, 2, owner, 120);
      }
      if (Random(rnd) == 0)
      {
        createspark(a, b, 2, owner, 120);
      }
      if (Random(rnd) == 0)
      {
        createspark(a, b, 2, owner, 120);
      }
    }
  }

  // plomyki
  if (CVar::r_maxsparks > (max_sparks - 10))
  {
    n = iif(typ == hit_type_fragnade, 3, 4);
    j = iif(typ == hit_type_fragnade, 25, 20);
    rnd = iif(typ == hit_type_fragnade, 50, 40);
    s = iif(typ == hit_type_fragnade, -0.05, -0.1);

    for (i = 1; i <= n; i++)
    {
      a.x = a.x - j + Random(rnd);
      a.y = a.y - j + Random(rnd);
      vec2scale(b, GetBulletParts().velocity[num], s);
      b.x = -b.x - 3.5 + ((float)(Random(70)) / 10);
      b.y = b.y - 3.5 + ((float)(Random(65)) / 10);
      createspark(a, b, 64, owner, 35);
    }
  }
#endif
}

template <Config::Module M>
void Bullet<M>::checkoutofbounds()
{
  std::int32_t bound;
  tvector2 *bulletpartspos;

#ifdef SERVER
  LogTraceG("TBullet.CheckOutOfBounds");
#endif
  auto &map = GS::GetGame().GetMap();

  bound = map.sectorsnum * map.GetSectorsDivision() - 10;
  bulletpartspos = &GetBulletParts().pos[num];

  if ((fabs(bulletpartspos->x) > bound) || (fabs(bulletpartspos->y) > bound))
  {
    kill();
  }
}

template <Config::Module M>
auto Bullet<M>::filterspritesbydistance(tspriteindexes &spriteindexes) -> std::int32_t
{
  ZoneScopedN("Bullet::FilterSpritesByDistance");
  std::int32_t i;
  std::int32_t j;
  std::int32_t spritecount;
  float roughdistance;
  tspritedistances distances;

  std::int32_t result;
  spritecount = 0;

  for (i = 1; i <= max_sprites; i++)
  {
    if (targetablesprite(i))
    {
      // Get a representation of the distance between this bullet and the sprite
      roughdistance = getcomparablespritedistance(i);

      // Add the sprite index at the appropriate place using a variant of Insertion sort
      spritecount = spritecount + 1;
      j = spritecount;

      while ((j > 1) && (roughdistance < distances[j - 1]))
      {
        j = j - 1;
      }

      std::memmove(&distances[j + 1], &distances[j], (spritecount - j) * sizeof(float));
      distances[j] = roughdistance;

      std::memmove(&spriteindexes[j + 1], &spriteindexes[j],
                   (spritecount - j) * sizeof(std::int32_t));
      spriteindexes[j] = i;
    }
  }

  result = spritecount;
  return result;
}

template <Config::Module M>
auto Bullet<M>::targetablesprite(std::int32_t i) -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t ownervulnerabletime;

  bool result;
  if (style == bullet_style_fragnade)
  {
    ownervulnerabletime = grenade_timeout - 50;
  }
  else if (style == bullet_style_m2)
  {
    ownervulnerabletime = m2bullet_timeout - 20;
  }
  else if (style == bullet_style_flame)
  {
    ownervulnerabletime = flamer_timeout;
  }
  else
  {
    ownervulnerabletime = bullet_timeout - 20;
  }

  // Check whether a sprite can be hit by this bullet
  result = sprite_system.GetSprite(i).active && ((owner != i) || (timeout < ownervulnerabletime)) &&
           (hitbody != i) && sprite_system.GetSprite(i).isnotspectator();
  return result;
}

template <Config::Module M>
auto Bullet<M>::getcomparablespritedistance(std::int32_t i) -> float
{
  tvector2 spritecol;
  tvector2 distance;

  float result;
  spritecol = getspritecollisionpoint(i);
  distance = vec2subtract(GetBulletParts().pos[num], spritecol);

  // Faster Euclidean distance calc used for comparisons.
  // Sqrt can be skipped because if "Sqrt[a] < Sqrt[b]" then "a < b"
  result = distance.x * distance.x + distance.y * distance.y;
  return result;
}

template <Config::Module M>
auto Bullet<M>::getspritecollisionpoint(std::int32_t i) -> tvector2
{
  // Why is this an exception to the usual rule??
  auto &sprite_system = SpriteSystem::Get();
  tvector2 result;
  const auto &spritePartsPos = sprite_system.GetSpritePartsPos(i);
#ifndef SERVER
  if ((style == bullet_style_flame) && (timeout > flamer_timeout - 2))
  {
    return spritePartsPos;
  }
#endif

  // Ping compensation to get a more accurate collision point
#ifndef SERVER
  auto &sprite = sprite_system.GetSprite(i);
  if (sprite.player->controlmethod != bot)
  {
    result = sprite.GetOldSpritePos(ownerpingtick);
  }
  else
#endif
  {
    result = spritePartsPos;
  }
  return result;
}

template <Config::Module M>
auto Bullet<M>::getweaponindex() -> std::uint8_t
{
  std::uint8_t weaponindex;

  auto &guns = GS::GetWeaponSystem().GetGuns();

  std::uint8_t result;
  for (weaponindex = 1; weaponindex <= high(guns); weaponindex++)
  {
    if (ownerweapon == guns[weaponindex].num)
    {
      result = weaponindex;
      return result;
    }
  }
  result = 0; // Not possible
  return result;
}

template class Bullet<Config::GetModule()>;
