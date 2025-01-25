// automatically converted

#include "Sparks.hpp"
#include "../../client/Client.hpp"
#include "../../client/GameRendering.hpp"
#include "../../client/Gfx.hpp"
#include "../../client/Sound.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "common/Util.hpp"
#include "common/gfx.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <Tracy.hpp>
#include <numbers>
#include <set>

template <Config::Module M>
auto GetSparkParts() -> particlesystem &
{
  static particlesystem b;
  return b;
}

template particlesystem &GetSparkParts();

std::int32_t sparkscount;

using std::numbers::pi;

auto createspark(tvector2 spos, tvector2 svelocity, std::uint8_t sstyle, std::uint8_t sowner,
                 std::int32_t life) -> std::int32_t
{
  ZoneScopedN("CreateSpark");
  std::int32_t i;
  float m;

  std::int32_t result;
  result = 0;

  if (camerafollowsprite > 0)
  {
    if (SpriteSystem::Get().IsPlayerSprite(camerafollowsprite))
    {
      if (!GS::GetGame().pointvisible(spos.x, spos.y, camerafollowsprite) && (sstyle != 38))
      {
        result = 0;
        return result;
      }
    }

    if (!SpriteSystem::Get().IsPlayerSprite(camerafollowsprite))
    {
      if (!GS::GetGame().pointvisible2(spos.x, spos.y, camerafollowsprite) && (sstyle != 38))
      {
        result = 0;
        return result;
      }
    }
  }

  for (i = 1; i <= CVar::r_maxsparks + 1; i++)
  {
    if ((sparkscount > CVar::r_maxsparks - 50) &&
        ((sstyle == 3) || (sstyle == 4) || (sstyle == 26) || (sstyle == 27) || (sstyle == 59) ||
         (sstyle == 2)))
    {
      return result;
    }
    if ((sparkscount > CVar::r_maxsparks - 40) && (sstyle == 1))
    {
      return result;
    }
    if ((sparkscount > CVar::r_maxsparks - 30) && (sstyle == 24))
    {
      return result;
    }

    if (i == CVar::r_maxsparks)
    {
      result = Random(CVar::r_maxsparks / 3) + 1;
      break;
    }
    if (!spark[i].active && (spark[i].style == 0) && !GetSparkParts().active[i])
    {
      result = i;
      break;
    }
  }
  // i is now the active sprite
  i = result;

  // activate sprite
  spark[i].active = true;
  spark[i].life = life;
  spark[i].style = sstyle;
  spark[i].num = i;
  spark[i].owner = sowner;
  spark[i].collidecount = 0;

  m = 1;

  // activate sprite part
  GetSparkParts().createpart(spos, svelocity, m, i);

  result = i;
  return result;
}

void tspark::update()
{
  static const std::set<std::int32_t> noneuler_style = {12, 13, 14, 15, 17, 24, 25, 28,
                                                        29, 31, 36, 37, 50, 54, 56, 60};

  static const std::set<std::int32_t> collidable_style = {
    2,  4,  5,  6,  7,  8,  9,  10, 11, 13, 16, 18, 19, 20, 21, 22, 23, 30, 32, 33, 34,
    40, 41, 42, 43, 48, 49, 51, 52, 57, 62, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73};
  std::int32_t wobble;
  std::int32_t wobblex;
  std::int32_t wobbley;

  if (!(noneuler_style.contains(style)))
  {
    GetSparkParts().doeulertimestepfor(num);
  }

  checkoutofbounds();

  // check collision with map
  if (collidable_style.contains(style))
  {
    checkmapcollision(GetSparkParts().pos[num].x, GetSparkParts().pos[num].y);
  }

  // wobble the screen when explosion
  if ((SpriteSystem::Get().IsPlayerSpriteValid()) && (camerafollowsprite > 0) &&
      (!demoplayer.active()))
  {
    if ((style == 17) || (style == 12) || (style == 14) || (style == 15) || (style == 28))
    {
      if (GS::GetGame().pointvisible(GetSparkParts().pos[num].x, GetSparkParts().pos[num].y,
                                     camerafollowsprite))
      {
        if (life > explosion_anims * 2.3)
        // if ((Style = 17) and (Life > EXPLOSION_ANIMS * 2.5)) or
        // ((Style <> 17) and (Life > EXPLOSION_ANIMS * 2.4)) then
        {
          wobble = life / 6;
          wobblex = Random(2 * wobble + 1);
          wobbley = Random(2 * wobble);
          camerax = camerax - wobble + wobblex;
          cameray = cameray - wobble + wobbley;
        }
      }
    }
  }

  // smoke luska
  if ((CVar::r_maxsparks > (max_sparks - 10)) && (style > 64) && (life > 235) && (Random(32) == 0))
  {
    createspark(GetSparkParts().pos[num], GetSparkParts().velocity[num], 31, owner, 40);
  }

  // smoke m79 luska
  if ((CVar::r_maxsparks > (max_sparks - 10)) && (style == 52))
  {
    if ((life > 235) && (Random(6) == 0))
    {
      createspark(GetSparkParts().pos[num], GetSparkParts().velocity[num], 31, owner, 40);
    }

    if ((life > 85) && (life < 235) && (Random(15) == 0))
    {
      createspark(GetSparkParts().pos[num], GetSparkParts().velocity[num], 31, owner, 35);
    }

    if ((life < 85) && (Random(24) == 0))
    {
      createspark(GetSparkParts().pos[num], GetSparkParts().velocity[num], 31, owner, 30);
    }
  }

  // iskry
  if ((CVar::r_maxsparks > (max_sparks - 10)) && (style == 2) && (Random(8) == 0))
  {
    createspark(GetSparkParts().pos[num], vector2(0, 0), 26, owner, 35);
  }

  lifeprev = life;
  life = life - 1;
  if (life == 0)
  {
    kill();
  }
}

void tspark::render() const
{
  tvector2 _p;
  tvector2 _scala;
  float grenvel = 0.0;
  float l;
  std::int32_t i;
  auto &map = GS::GetGame().GetMap();

  tgfxspritearray &t = textures;
  if (CVar::sv_realisticmode)
  {
    if ((owner > 0) && (owner < max_sprites + 1))
    {
      if (SpriteSystem::Get().GetSprite(owner).active)
      {
        if (SpriteSystem::Get().GetSprite(owner).visible == 0)
        {
          if (map.raycast(GetSparkParts().pos[num],
                          SpriteSystem::Get().GetPlayerSprite().skeleton.pos[9], grenvel, gamewidth,
                          true) or
              (SpriteSystem::Get().GetSprite(owner).visible == 0))
          {
            return;
          }
        }
      }
    }
  }

  _p.x = GetSparkParts().pos[num].x;
  _p.y = GetSparkParts().pos[num].y;
  l = lifefloat;

  switch (style)
  {
  case 1:
    gfxdrawsprite(t[GFX::SPARKS_SMOKE], _p.x, _p.y, rgba(0xffffff, l + 10));
    break;
  case 2:
    gfxdrawsprite(t[GFX::SPARKS_LILFIRE], _p.x, _p.y, rgba(0xffffff, l));
    break;
  case 3:
    gfxdrawsprite(t[GFX::SPARKS_ODPRYSK], _p.x, _p.y, rgba(0xffffff, l * 3 + 10));
    break;
  case 4:
    gfxdrawsprite(t[GFX::SPARKS_LILBLOOD], _p.x, _p.y, 0.75, 0.75, 0, 0, degtorad(l * 10),
                  rgba(0xffffff, l * 2 + 65));
    break;
  case 5: {
    _scala.x = iif(l > 10, 0.33 + (float)(10) / l, 1.0);
    _scala.y = _scala.x;
    gfxdrawsprite(t[GFX::SPARKS_BLOOD], _p.x, _p.y, _scala.x, _scala.y, 0, 0, degtorad(l * 2),
                  rgba(0xffffff, l * 2 + 85));
  }
  break;
  case 6:
    if (SpriteSystem::Get().GetSprite(owner).player->headcap > 0)
    {
      gfxdrawsprite(t[SpriteSystem::Get().GetSprite(owner).player->headcap], _p.x, _p.y, 0, 0,
                    degtorad(l * 2), rgba(SpriteSystem::Get().GetSprite(owner).player->shirtcolor));
    }
    break;
  case 7:
    gfxdrawsprite(t[GFX::WEAPONS_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 8:
    gfxdrawsprite(t[GFX::WEAPONS_SHELL], _p.x, _p.y, 1.1, 1.2, 0, 0, degtorad(l * 3.5),
                  rgba(0xff3333));
    break;
  case 9:
    gfxdrawsprite(t[GFX::WEAPONS_AK74_CLIP], _p.x + 8, _p.y, 0, 0, pi);
    break;
  case 10:
    gfxdrawsprite(t[GFX::WEAPONS_MINIMI_CLIP], _p.x + 8, _p.y, 0, 0, pi);
    break;
  case 11:
    gfxdrawsprite(t[GFX::WEAPONS_MP5_CLIP], _p.x + 8, _p.y, 0, 0, pi);
    break;
  case 12: {
    _p.x = _p.x - 19;
    _p.y = _p.y - 38;

    i = GFX::SPARKS_EXPLOSION_EXPLODE16 - round(l / 4);

    if ((i - 1) >= GFX::SPARKS_EXPLOSION_EXPLODE1)
    {
      gfxdrawsprite(t[i - 1], _p.x, _p.y, 0.75, rgba(0xadadad, 100));
    }

    gfxdrawsprite(t[i], _p.x, _p.y, 0.75, rgba(0xffffff, 255 - (explosion_anims * 5) + l));
  }
  break;
  case 13: {
    _p.x = _p.x - 8;
    _p.y = _p.y - 17;
    gfxdrawsprite(t[GFX::SPARKS_EXPLOSION_EXPLODE16 - (std::int32_t)round(l / 3)], _p.x, _p.y, 0.3,
                  rgba(0xffffff, 255 - 2 * l));
  }
  break;
  case 14: {
    _p.x = _p.x - 50;
    _p.y = _p.y - 100;

    i = GFX::SPARKS_EXPLOSION_EXPLODE16 - round(l / 3);

    if ((i - 1) >= GFX::SPARKS_EXPLOSION_EXPLODE1)
    {
      gfxdrawsprite(t[i - 1], _p.x, _p.y, 2, rgba(0xadadad, 100));
    }

    gfxdrawsprite(t[i], _p.x, _p.y, 2, rgba(0xffffff, 255 - 2 * l));
  }
  break;
  case 15: {
    _p.x = _p.x - 75;
    _p.y = _p.y - 150;

    i = GFX::SPARKS_EXPLOSION_EXPLODE16 - round(l / 3);

    if ((i - 1) >= GFX::SPARKS_EXPLOSION_EXPLODE1)
    {
      gfxdrawsprite(t[i - 1], _p.x, _p.y, 3, rgba(0xadadad, 100));
    }

    gfxdrawsprite(t[i], _p.x, _p.y, 3, rgba(0xffffff, 255 - 2 * l));
  }
  break;
  case 16:
    gfxdrawsprite(t[GFX::WEAPONS_SHELL], _p.x, _p.y, 1, 2, 0, 0, degtorad(l * 3.5), rgba(0x8877ff));
    break;
  case 17: {
    _p.x = _p.x - 25;
    _p.y = _p.y - 50;

    i = GFX::SPARKS_EXPLOSION_EXPLODE16 - round(l / 4);

    if ((i - 1) >= GFX::SPARKS_EXPLOSION_EXPLODE1)
    {
      gfxdrawsprite(t[i - 1], _p.x, _p.y, rgba(0xaaaaaa, 100));
    }

    gfxdrawsprite(t[i], _p.x, _p.y, rgba(0xffffff, 255 - (explosion_anims * 5) + l));
  }
  break;
  case 18:
    gfxdrawsprite(t[GFX::WEAPONS_DEAGLES_CLIP], _p.x + 8, _p.y, 0, 0, pi);
    break;
  case 19:
    gfxdrawsprite(t[GFX::WEAPONS_STEYR_CLIP], _p.x + 8, _p.y, 0, 0, pi);
    break;
  case 20:
    gfxdrawsprite(t[GFX::WEAPONS_BARRETT_CLIP], _p.x + 8, _p.y, 0, 0, pi);
    break;
  case 21:
    gfxdrawsprite(t[GFX::WEAPONS_SHELL], _p.x, _p.y, 1.1, 1, 0, 0, degtorad(l * 3.77));
    break;
  case 22:
    gfxdrawsprite(t[GFX::WEAPONS_SHELL], _p.x, _p.y, 1.3, 1, 0, 0, degtorad(l * 3.5));
    break;
  case 23:
    gfxdrawsprite(t[GFX::WEAPONS_SOCOM_CLIP], _p.x + 8, _p.y, 0, 0, pi);
    break;
  case 24: {
    _scala.x = 0.6 + ((float)(75) / l) / 126;
    _scala.y = 0.6 + ((float)(75) / l) / 120;
    _p.x = _p.x - 22 * _scala.x;
    _p.y = _p.y - 64 + l / 2;
    gfxdrawsprite(t[GFX::SPARKS_BIGSMOKE], _p.x, _p.y, _scala.x, _scala.y,
                  rgba(0xffffff, trunc(3 * l)));
  }
  break;
  case 25:
    gfxdrawsprite(
      t[GFX::SPARKS_SPAWNSPARK], _p.x - 20, _p.y - 20, 0, 0, degtorad(l),
      rgba(SpriteSystem::Get().GetSprite(owner).player->shirtcolor, min(6.0 * l, 255.0)));
    break;
  case 26:
    gfxdrawsprite(t[GFX::SPARKS_ODPRYSK], _p.x, _p.y, rgba(0xfffe35, min(l * 3 + 154.0, 255.0)));
    break;
  case 27:
    gfxdrawsprite(t[GFX::SPARKS_ODPRYSK], _p.x, _p.y, rgba(0xaaaaaa, min(l * 3.0 + 154, 255.0)));
    break;
  case 28:
    gfxdrawsprite(t[GFX::SPARKS_EXPLOSION_EXPLODE16 - (std::int32_t)round(l / 3)], _p.x - 15,
                  _p.y - 37, 0.5, rgba(0xffffff, 255 - 2 * l));
    break;
  case 29: {
    _scala.x = 0.5 * (0.6 + ((float)(75) / l) / 96);
    _scala.y = 0.5 * (0.6 + ((float)(75) / l) / 90);
    _p.x = _p.x - 22 * _scala.x;
    _p.y = _p.y - 48 + l / 1.5;
    gfxdrawsprite(t[GFX::SPARKS_BIGSMOKE], _p.x, _p.y, _scala.x, _scala.y,
                  rgba(0xffffff, trunc(2.5 * l)));
  }
  break;
  case 30:
    gfxdrawsprite(t[GFX::SPARKS_PIN], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 31:
    gfxdrawsprite(t[GFX::SPARKS_LILSMOKE], _p.x, _p.y, rgba(0xffffff, l + 10));
    break;
  case 32:
    gfxdrawsprite(t[GFX::SPARKS_STUFF], _p.x, _p.y, rgba(0xffffff, l + 10));
    break;
  case 33:
    gfxdrawsprite(t[GFX::WEAPONS_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4), rgba(0xbbaaa9));
    break;
  case 34:
    gfxdrawsprite(t[GFX::SPARKS_CYGARO], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 35:
    gfxdrawsprite(t[GFX::SPARKS_LILSMOKE], _p.x, _p.y, rgba(0xffffff, l * 13));
    break;
  case 36: {
    _scala.x = l / 35;
    _scala.y = l / 35;
    _p.y = _p.y - ((float)(1) / _scala.y);
    gfxdrawsprite(t[GFX::SPARKS_PLOMYK], _p.x, _p.y, _scala.x, _scala.y,
                  rgba(0xffffff, min(l * 2 + 185.0, 255.0)));
  }
  break;
  case 37: {
    _scala.x = l / 75;
    _scala.y = l / 75;
    _p.y = _p.y - ((float)(1) / _scala.y);
    gfxdrawsprite(t[GFX::SPARKS_BLACKSMOKE], _p.x, _p.y, _scala.x, _scala.y, rgba(0xffffff, l * 3));
  }
  break;
  case 38:
    gfxdrawsprite(t[GFX::SPARKS_RAIN], _p.x, _p.y, rgba(0xffffff, 105));
    break;
  case 39:
    gfxdrawsprite(t[GFX::SPARKS_SAND], _p.x, _p.y, rgba(0xffffff, 105));
    break;
  case 40:
    gfxdrawsprite(t[GFX::SPARKS_ODLAMEK1], _p.x, _p.y, 0, 0, degtorad(l * 8),
                  rgba(0xffffff, trunc(l + 10)));
    break;
  case 41:
    gfxdrawsprite(t[GFX::SPARKS_ODLAMEK2], _p.x, _p.y, 0, 0, degtorad(l * 8),
                  rgba(0xffffff, trunc(l + 10)));
    break;
  case 42:
    gfxdrawsprite(t[GFX::SPARKS_ODLAMEK3], _p.x, _p.y, 0, 0, degtorad(l * 8),
                  rgba(0xffffff, trunc(l + 10)));
    break;
  case 43:
    gfxdrawsprite(t[GFX::SPARKS_ODLAMEK4], _p.x, _p.y, 0, 0, degtorad(l * 8),
                  rgba(0xffffff, trunc(l + 10)));
    break;
  case 44:
    gfxdrawsprite(t[GFX::SPARKS_ODLAMEK1], _p.x, _p.y, 0.7, rgba(0xffffff, trunc(l * 2) + 15));
    break;
  case 45:
    gfxdrawsprite(t[GFX::SPARKS_ODLAMEK2], _p.x, _p.y, 0.7, rgba(0xffffff, trunc(l * 2) + 15));
    break;
  case 46:
    gfxdrawsprite(t[GFX::SPARKS_ODLAMEK3], _p.x, _p.y, 0.7, rgba(0xffffff, trunc(l * 2) + 15));
    break;
  case 47:
    gfxdrawsprite(t[GFX::SPARKS_ODLAMEK4], _p.x, _p.y, 0.7, rgba(0xffffff, trunc(l * 2) + 15));
    break;
  case 48:
    gfxdrawsprite(t[GFX::SPARKS_SKRAWEK], _p.x, _p.y, 0, 0, degtorad(l * 5),
                  rgba(SpriteSystem::Get().GetSprite(owner).player->shirtcolor, trunc(l * 2) + 15));
    break;
  case 49:
    gfxdrawsprite(t[GFX::SPARKS_SKRAWEK], _p.x, _p.y, 0, 0, degtorad(l * 5),
                  rgba(SpriteSystem::Get().GetSprite(owner).player->pantscolor, trunc(l * 2) + 15));
    break;
  case 50:
    gfxdrawsprite(t[GFX::SPARKS_PUFF], _p.x, _p.y, rgba(0xffffff, trunc(l) + 5));
    break;
  case 51:
    gfxdrawsprite(t[GFX::WEAPONS_SPAS_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 3.77));
    break;
  case 52:
    gfxdrawsprite(t[GFX::WEAPONS_M79_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 3.77));
    break;
  case 53:
    gfxdrawsprite(t[GFX::SPARKS_SNOW], _p.x, _p.y, rgba(0xffffff, 105));
    break;
  case 54:
    if (l <= smoke_anims * 4)
    {
      _p.x = _p.x - 26;
      _p.y = _p.y - 48;

      i = GFX::SPARKS_MINISMOKE - round(l / 4);

      if ((i - 1) >= GFX::SPARKS_EXPLOSION_SMOKE1)
      {
        gfxdrawsprite(t[i - 1], _p.x, _p.y, rgba(0xcccccc, 2 * l + 10.0));
      }

      gfxdrawsprite(t[i], _p.x, _p.y, rgba(0xdddddd, 3 * l + 10));
    }
    break;
  case 55: {
    _scala.x = iif(l > 20, 0.63 + (float)(10) / l, 1.0);
    _scala.y = _scala.x;
    gfxdrawsprite(t[GFX::SPARKS_SPLAT], _p.x, _p.y, _scala.x, _scala.y, 0, 0, degtorad(l),
                  rgba(0xffffff, min(l * 2 + 55.0, 255.0)));
  }
  break;
  case 56:
    gfxdrawsprite(t[GFX::SPARKS_MINISMOKE], _p.x - 3, _p.y - 3, rgba(0xffffff, trunc(2.5 * l)));
    break;
  case 57:
    gfxdrawsprite(t[GFX::SPARKS_ODPRYSK], _p.x, _p.y, rgba(0xffff00, l * 2 + 10));
    break;
  case 58:
    gfxdrawsprite(t[GFX::SPARKS_ODPRYSK], _p.x, _p.y, rgba(0xffff00, l * 3 + 10));
    break;
  case 59: {
    _p.y = _p.y - (50 - l);
    _scala.x = 1.5 + ((float)(620) / l) / 50;
    _scala.y = _scala.x;
    gfxdrawsprite(t[GFX::SPARKS_SMOKE], _p.x, _p.y, _scala.x, _scala.y, rgba(0xffffff, l * 2));
  }
  break;
  case 60: {
    _scala.x = 0.5 + ((float)(16) / (l + 50));
    _scala.y = _scala.x;
    _p.x = _p.x - 14 * _scala.x;
    _p.y = _p.y - 30;
    gfxdrawsprite(t[GFX::SPARKS_BIGSMOKE], _p.x, _p.y, _scala.x, _scala.y,
                  rgba(0xffffff, trunc(l / 3.3)));

    if (l > 30)
    {
      gfxdrawsprite(t[GFX::SPARKS_BIGSMOKE2], _p.x, _p.y, _scala.x, _scala.y,
                    rgba(0x666666, trunc((float)((255 - l)) / 9)));
    }
    else
    {
      gfxdrawsprite(t[GFX::SPARKS_BIGSMOKE2], _p.x, _p.y, _scala.x, _scala.y,
                    rgba(0xbfbfbf, trunc(l * 1.0)));
    }
  }
  break;
  case 61:
    gfxdrawsprite(t[GFX::SPARKS_SPAWNSPARK], _p.x - 10, _p.y - 10, 0.5, 0.5, 0, 0, degtorad(l),
                  rgba(SpriteSystem::Get().GetSprite(owner).player->shirtcolor, 14 * l));
    break;
  case 62:
    gfxdrawsprite(t[GFX::SPARKS_JETFIRE], _p.x, _p.y, 0, 0, degtorad(l),
                  rgba(SpriteSystem::Get().GetSprite(owner).player->jetcolor, l * 5));
    break;
  case 64: {
    _scala.x = l / 35;
    _scala.y = l / 35;
    _p.y = _p.y - ((float)(1) / _scala.y);
    gfxdrawsprite(t[GFX::SPARKS_PLOMYK], _p.x, _p.y, _scala.x, _scala.y,
                  rgba(0xffffff, l * 2 + 185));
  }
  break;
  case 65:
    gfxdrawsprite(t[GFX::WEAPONS_COLT_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 66:
    gfxdrawsprite(t[GFX::WEAPONS_DEAGLES_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 67:
    gfxdrawsprite(t[GFX::WEAPONS_MP5_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 68:
    gfxdrawsprite(t[GFX::WEAPONS_AK74_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 69:
    gfxdrawsprite(t[GFX::WEAPONS_STEYR_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 70:
    gfxdrawsprite(t[GFX::WEAPONS_RUGER_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 71:
    gfxdrawsprite(t[GFX::WEAPONS_BARRETT_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 3.5));
    break;
  case 72:
    gfxdrawsprite(t[GFX::WEAPONS_MINIMI_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  case 73:
    gfxdrawsprite(t[GFX::WEAPONS_MINIGUN_SHELL], _p.x, _p.y, 0, 0, degtorad(l * 4));
    break;
  } // case
}

auto tspark::checkmapcollision(float x, float y) -> bool
{
  ZoneScopedN("TSpark::CheckMapCollision");
  std::int32_t b = 0;
  tvector2 pos;
  tvector2 perp;
  float d = 0.0;
  bool teamcol;

  bool result;
  result = false;

  pos.x = x - 8;
  pos.y = y - 1;
  auto &map = GS::GetGame().GetMap();

  /*iterate through maps sector polygons*/
  auto const sector = map.GetSector(pos);
  if (!sector.IsValid())
  {
    return result;
  }

  for (const auto &w : sector.GetPolys())
  {
    if ((owner < 1) || (owner > 32))
    {
      return result;
    }

    const auto polytype = w.Type;

    teamcol = teamcollides(polytype, SpriteSystem::Get().GetSprite(owner).player->team, false);

    if (teamcol)
    {
      if ((polytype != poly_type_bouncy) || (SpriteSystem::Get().GetSprite(owner).holdedthing != 0))
      {
        if ((polytype != poly_type_only_bullets) && (polytype != poly_type_only_player) &&
            (polytype != poly_type_doesnt) && (polytype != poly_type_background) &&
            (polytype != poly_type_background_transition))
        {
          if (map.pointinpolyedges(pos.x, pos.y, w.Index))
          {
            perp = map.closestperpendicular(w.Index, pos, d, b);

            vec2normalize(perp, perp);
            vec2scale(perp, perp, d);

            GetSparkParts().velocity[num] = vec2subtract(GetSparkParts().velocity[num], perp);

            vec2scale(GetSparkParts().velocity[num], GetSparkParts().velocity[num],
                      spark_surfacecoef);

            switch (style)
            {
            case 2:
            case 62: {
              vec2scale(perp, perp, 2.5);
              perp.x = perp.x - 0.5 + (float)(Random(11)) / 10;
              perp.y = -perp.y;
              if (Random(2) == 0)
              {
                if (Random(2) == 0)
                {
                  createspark(pos, perp, 26, owner, 35);
                }
                else
                {
                  createspark(pos, perp, 27, owner, 35);
                }

                playsound(SfxEffect::ts, GetSparkParts().pos[num]);
              }
            }
            break;
            case 33:
            case 34: {
              vec2scale(perp, perp, 2.5);
              perp.x = perp.x - 0.5 + (float)(Random(11)) / 10;
              perp.y = -perp.y;
              if (Random(7) == 0)
              {
                createspark(pos, perp, 26, owner, 35);
              }
              else
              {
                createspark(pos, perp, 27, owner, 35);
              }

              if (collidecount > 4)
              {
                kill();
              }
            }
            break;
            case 4:
            case 5: {
              if (style == 5)
              {
                createspark(GetSparkParts().pos[num], GetSparkParts().velocity[num], 55, owner, 30);
              }

              if (collidecount > 1)
              {
                kill();
              }
            }
            break;
            case 6: {
              if ((collidecount == 0) || (collidecount == 2) || (collidecount == 4))
              {
                playsound(SfxEffect::clipfall, GetSparkParts().pos[num]);
              }

              if (collidecount > 4)
              {
                kill();
              }
            }
            break;
            case 7:
            case 21:
            case 22:
            case 16:
            case 30:
            case 52:
            case 65:
            case 66:
            case 67:
            case 68:
            case 69:
            case 70:
            case 71:
            case 72:
            case 73: {
              if ((collidecount == 0) || (collidecount == 2) || (collidecount == 4))
              {
                playsound(SfxEffect::shell + Random(2), GetSparkParts().pos[num]);
              }
              if (collidecount > 4)
              {
                kill();
              }
            }
            break;
            case 51: {
              playsound(SfxEffect::gaugeshell, GetSparkParts().pos[num]);
              if (collidecount > 4)
              {
                kill();
              }
            }
            break;
            case 32:
            case 48:
            case 49: {
              if (collidecount > 2)
              {
                kill();
              }
            }
            break;
            case 9:
            case 10:
            case 11:
            case 18:
            case 19:
            case 20:
            case 23: {
              if ((collidecount == 0) || (collidecount == 4))
              {
                playsound(SfxEffect::clipfall, GetSparkParts().pos[num]);
              }

              if (collidecount > 4)
              {
                kill();
              }
            }
            break;
            case 57: {
              vec2scale(perp, perp, 0.75);
              perp.x = perp.x - 0.5 + (float)(Random(11)) / 10;
              perp.y = -perp.y;
              if (Random(2) == 0)
              {
                createspark(pos, perp, 58, owner, 50);
              }
              else
              {
                createspark(pos, perp, 58, owner, 50);
              }
            }
            break;
            }

            collidecount += 1;

            result = true;
            return result;
          } // PointinPolyEdges
        }
      }
    }
  }         // for j
  return result;
}

void tspark::kill()
{
  active = false;
  style = 0;
  if (num > 0)
  {
    GetSparkParts().active[num] = false;
  }
}

void tspark::checkoutofbounds()
{
  std::int32_t bound;
  tvector2 *sparkpartspos;
  auto &map = GS::GetGame().GetMap();

  bound = map.sectorsnum * map.GetSectorsDivision() - 10;
  sparkpartspos = &GetSparkParts().pos[num];

  if ((fabs(sparkpartspos->x) > bound) || (fabs(sparkpartspos->y) > bound))
  {
    kill();
  }
}
