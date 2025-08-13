// automatically converted
#include "NetworkServerBullet.hpp"

#include <math.h>
#include <array>
#include <memory>
#include <string>

#include "../Game.hpp"
#include "NetworkUtils.hpp"
#include "NetworkServer.hpp"
#include "common/Calc.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "../../server/Server.hpp"
#include "common/Anims.hpp"
#include "common/Parts.hpp"
#include "common/PolyMap.hpp"
#include "common/Vector.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/RandomGenerator.hpp"
#include "common/misc/SafeType.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/Constants.cpp.h"
#include "shared/Cvar.hpp"
#include "shared/mechanics/Bullets.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/mechanics/Things.hpp"
#include "shared/network/Net.hpp"

namespace
{
std::array<tmsg_bulletsnapshot, max_sprites> oldbulletsnapshotmsg;
}

void serverbulletsnapshot(const std::uint8_t i, std::uint8_t tonum, bool forced)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_bulletsnapshot bulletmsg;

  auto &bullet = GS::GetBulletSystem().GetBullets();
  auto &b = bullet[i];

  // SERVER BULLETS SNAPSHOT
  bulletmsg.header.id = msgid_bulletsnapshot;
  bulletmsg.owner = b.owner;
  bulletmsg.weaponnum = b.ownerweapon;
  bulletmsg.pos = GetBulletParts().pos[i];
  bulletmsg.velocity = GetBulletParts().velocity[i];
  bulletmsg.seed = b.seed;
  bulletmsg.forced = forced;

#ifdef SERVER
  if (!forced)
  {
    if ((sprite_system.GetSprite(bulletmsg.owner).weapon.ammocount > 0) &&
        (b.style != bullet_style_fragnade) && (b.style != bullet_style_clusternade) &&
        (b.style != bullet_style_cluster))
    {
      sprite_system.GetSprite(bulletmsg.owner).weapon.ammocount -= 1;
    }
  }

  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    if ((sprite.player->controlmethod == human) && (sprite.num != b.owner))
    {
      if ((tonum == 0) || (sprite.num == tonum))
      {
        if (bulletcansend(GetBulletParts().pos[i].x, GetBulletParts().pos[i].y,
                          sprite.player->camera, GetBulletParts().velocity[i].x) or
            forced)
        {
          gGlobalStateNetworkServer.GetServerNetwork()->SendData(&bulletmsg, sizeof(bulletmsg),
                                                                 sprite.player->peer, false);
        }
      }
    }
  }
#else
  GS::GetDemoRecorder().saverecord(bulletmsg, sizeof(bulletmsg));
#endif
}

#ifdef SERVER
void serverhandlebulletsnapshot(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_clientbulletsnapshot *bulletsnap;
  std::int32_t p;
  std::int32_t d;
  float k;
  std::int32_t i;
  bool onstatgun;
  bool failedbulletcheck;
  std::int16_t weaponindex;
  std::uint8_t style;
  tvector2 a;
  tvector2 b;
  tvector2 bx;
  tvector2 bstraight;
  tvector2 bnorm;
  float bulletspread;
  auto &map = GS::GetGame().GetMap();

  if (!verifypacket(sizeof(tmsg_clientbulletsnapshot), size, msgid_bulletsnapshot))
  {
    return;
  }

  bulletsnap = pmsg_clientbulletsnapshot(netmessage);
  p = player->spritenum;

  auto &guns = GS::GetWeaponSystem().GetGuns();

  gGlobalStateNetworkServer.messagesasecnum[p] += 1;

  auto& sprite = sprite_system.GetSprite(p);

  sprite.player->pingticksb = gGlobalStateNetworkServer.servertickcounter - bulletsnap->clientticks;
  if (sprite.player->pingticksb < 0)
  {
    sprite.player->pingticksb = 0;
  }
  if (sprite.player->pingticksb > max_oldpos)
  {
    sprite.player->pingticksb = max_oldpos;
  }

  weaponindex = weaponnumtoindex(bulletsnap->weaponnum, guns);
  if (weaponindex == -1)
  {
    return;
  }

  style = guns[weaponindex].bulletstyle;

  // Check for duplicated bullets
  // Using a ringbuffer of saved references of old ones
  failedbulletcheck = false;
  for (i = 0; i <= sprite.bulletcheckamount; i++)
  {
    if (sprite.bulletcheck[i] == bulletsnap->seed)
    {
      failedbulletcheck = true;
      break;
    }
  }
  if (failedbulletcheck)
  {
    // ignore duplicate packet
    return;
  }

  if (sprite.bulletcheckindex > bulletcheckarraysize - 1)
  {
    sprite.bulletcheckindex = 0;
  }
  if (sprite.bulletcheckamount < bulletcheckarraysize - 1)
  {
    sprite.bulletcheckamount += 1;
  }
  sprite.bulletcheck[sprite.bulletcheckindex] =
    bulletsnap->seed;
  sprite.bulletcheckindex += 1;

  // spec kill: spectators NEVER send bullet snapshots
  if (sprite.isspectator())
  {
    return;
  }

  // Disable knife-cheat kick for now, until we have timestamped
  // packets, and will do a time-based comparison to sprite snapshot
  // with info about switching primary to secondary
  /*if (BulletSnap.Style <> SpriteSystem::Get().GetSprite(p).Weapon.BulletStyle) then
    if ((BulletSnap.Style = 13) and
      (SpriteSystem::Get().GetSprite(p).Weapon.BulletStyle <> 11) and
      (SpriteSystem::Get().GetSprite(p).Weapon.Num <> NoWeapon.Num)) then
    begin
      KickPlayer(p, True, KICK_CHEAT, DAY, 'Knife Cheat');
      Exit
    end;*/

  auto &weaponSystem = GS::GetWeaponSystem();

  if ((style == bullet_style_thrownknife) && (!weaponSystem.IsEnabled(knife)))
  {
    if (gGlobalStateServer.kickplayer(p, true, kick_cheat, day, "Knife-Spawn Cheat"))
    {
      return;
    }
  }

  auto &things = GS::GetThingSystem().GetThings();
  if (style == bullet_style_m2)
  {
    onstatgun = false;
    for (i = 0; i <= max_things; i++)
    {
      if ((things[i].style == object_stationary_gun) /*Stat Gun*/ && (!onstatgun))
      {
        if (distance(sprite.skeleton.pos[1].x,
                     sprite.skeleton.pos[1].y, things[i].skeleton.pos[1].x,
                     things[i].skeleton.pos[1].y) < stat_radius * 2)
        {
          onstatgun = true;
        }
      }
    }
    if (!onstatgun)
    {
      // KickPlayer(p, True, KICK_CHEAT, DAY, 'StatGun Cheat');
      return;
    }
  }

  map.checkoutofbounds(bulletsnap->pos.x, bulletsnap->pos.y);

  if ((style < bullet_style_plain) || (style > bullet_style_m2))
  {
    return;
  }

  if ((style != bullet_style_fragnade) && (style != bullet_style_punch) &&
      (style != bullet_style_clusternade) && (style != bullet_style_cluster) &&
      (style != bullet_style_thrownknife) && (style != bullet_style_m2) &&
      (sprite.lastweaponstyle != style))
  {
    return;
  }

  if ((oldbulletsnapshotmsg[p].weaponnum == bulletsnap->weaponnum) &&
      (oldbulletsnapshotmsg[p].pos.x == bulletsnap->pos.x) &&
      (oldbulletsnapshotmsg[p].pos.y == bulletsnap->pos.y) &&
      (oldbulletsnapshotmsg[p].velocity.x == bulletsnap->velocity.x) &&
      (oldbulletsnapshotmsg[p].velocity.y == bulletsnap->velocity.y))
  {
    return;
  }

  if ((style != bullet_style_fragnade) && (style != bullet_style_clusternade) &&
      (style != bullet_style_cluster) && (style != bullet_style_thrownknife) &&
      (style != bullet_style_m2))
  {
    if (vec2length(bulletsnap->velocity) >
        sprite.lastweaponspeed + 10 * guns[weaponindex].inheritedvelocity)
    {
      return;
    }
  }

  a.x = sprite.skeleton.pos[15].x - (bulletsnap->velocity.x / 1.33);
  a.y = sprite.skeleton.pos[15].y - 2 - (bulletsnap->velocity.y / 1.33);
  b = vec2subtract(a, bulletsnap->pos);

  if ((style != bullet_style_fragnade) && (style != bullet_style_flame) &&
      (style != bullet_style_clusternade) && (style != bullet_style_cluster) &&
      (style != bullet_style_thrownknife) && (style != bullet_style_m2))
  {
    if (vec2length(b) > 366)
    {
      return;
    }
  }

  if (GS::GetGame().GetMapchangecounter() == 999999999)
  {
    return;
  }

  if (gGlobalStateNetworkServer.bullettime[p] > GS::GetGame().GetMainTickCounter())
  {
    gGlobalStateNetworkServer.bullettime[p] = 0;
  }

  if ((style != bullet_style_fragnade) && (style != bullet_style_punch) &&
      (style != bullet_style_clusternade) && (style != bullet_style_cluster) &&
      (style != bullet_style_thrownknife) && (style != bullet_style_m2))
  {
    if (sprite.weapon.ammo > 1)
    {
      if ((GS::GetGame().GetMainTickCounter() - gGlobalStateNetworkServer.bullettime[p]) <
          ((sprite.lastweaponfire) * 0.85))
      {
        gGlobalStateServer.bulletwarningcount[p] += 1;
      }
      else
      {
        gGlobalStateServer.bulletwarningcount[p] = 0;
      }
    }

    if (sprite.weapon.ammo == 1)
    {
      if ((GS::GetGame().GetMainTickCounter() - gGlobalStateNetworkServer.bullettime[p]) <
          ((sprite.lastweaponreload) * 0.9))
      {
        gGlobalStateServer.bulletwarningcount[p] += 1;
      }
      else
      {
        gGlobalStateServer.bulletwarningcount[p] = 0;
      }
    }

    if (gGlobalStateServer.bulletwarningcount[p] > 2)
    {
      return;
    }

    gGlobalStateNetworkServer.bullettime[p] = GS::GetGame().GetMainTickCounter();
  }

  if (gGlobalStateNetworkServer.grenadetime[p] > GS::GetGame().GetMainTickCounter())
  {
    gGlobalStateNetworkServer.grenadetime[p] = 0;
  }

  if ((style == bullet_style_fragnade) || (style == bullet_style_clusternade))
  {
    if (GS::GetGame().GetMainTickCounter() - gGlobalStateNetworkServer.grenadetime[p] < 6)
    {
      return;
    }

    gGlobalStateNetworkServer.grenadetime[p] = GS::GetGame().GetMainTickCounter();
  }

  if (style == bullet_style_thrownknife)
  {
    if (CVar::sv_warnings_knifecheat == 69)
    {
      if ((!gGlobalStateNetworkServer.knifecan[p]) or
          (sprite.weapon.bulletstyle != bullet_style_thrownknife) or
          (sprite.weapon.bulletstyle != bullet_style_punch))
      {
        sprite.player->knifewarnings += 1;
        if (sprite.player->knifewarnings == 3)
        {
          GS::GetMainConsole().console(std::string("** DETECTED KNIFE CHEATING FROM ") +
                                         sprite.player->name + " **",
                                       server_message_color);
          gGlobalStateServer.kickplayer(p, true, kick_cheat, day, "Knife Throw Cheat");
        }
      }
    }
    gGlobalStateNetworkServer.knifecan[p] = false;
  }

  a = bulletsnap->pos;
  b = bulletsnap->velocity;
  k = sprite.lastweaponhm;

  if (style == bullet_style_punch)
  {
    k = guns[noweapon].hitmultiply;
  }

  if (style == bullet_style_cluster)
  {
    if (sprite.tertiaryweapon.ammocount == 0)
    {
      return;
    }
  }

  if ((style == bullet_style_fragnade) || (style == bullet_style_clusternade))
  {
    k = guns[fraggrenade].hitmultiply;
    if (sprite.tertiaryweapon.ammocount == 0)
    {
      return;
    }
    if (sprite.tertiaryweapon.ammocount > 0)
    {
      sprite.tertiaryweapon.ammocount -= 1;
    }
  }

  if (style == bullet_style_thrownknife)
  {
    k = guns[thrownknife].hitmultiply;
    sprite.bodyapplyanimation(AnimationType::Stand, 1);
  }

  if (style == bullet_style_m2)
  {
    k = guns[m2].hitmultiply;
  }

  createbullet(a, b, bulletsnap->weaponnum, p, 255, k, true, true, bulletsnap->seed);

  bulletspread = guns[weaponindex].bulletspread;

  if (bulletsnap->weaponnum == eagle_num) // Desert Eagle pellets
  {
    // Undo the bullet spread used on the first pellet that was sent in order to
    // get the "straight" bullet vector. Then re-apply the the same randomness
    NotImplemented("No randseed");
#if 0
        randseed = bulletsnap->seed;
#endif
    bstraight.x = b.x - (Random() * 2 - 1) * bulletspread;
    bstraight.y = b.y - (Random() * 2 - 1) * bulletspread;

    bx.x = bstraight.x + (Random() * 2 - 1) * bulletspread;
    bx.y = bstraight.y + (Random() * 2 - 1) * bulletspread;

    vec2normalize(bnorm, bstraight);
    a.x = a.x - sign(bstraight.x) * fabs(bnorm.y) * 3.0;
    a.y = a.y + sign(bstraight.y) * fabs(bnorm.x) * 3.0;

    createbullet(a, bx, bulletsnap->weaponnum, p, 255, k, false, true);
  }
  else if (style == bullet_style_shotgun) // SPAS-12 pellets
  {
    // Undo the bullet spread used on the first pellet that was sent in order to
    // get the "straight" bullet vector. Then re-apply the the same randomness
    NotImplemented("No randseed");
#if 0
        randseed = bulletsnap->seed;
#endif
    bstraight.x = b.x - (Random() * 2 - 1) * bulletspread;
    bstraight.y = b.y - (Random() * 2 - 1) * bulletspread;

    for (d = 0; d <= 4; d++) // Remaining 5 pellets
    {
      bx.x = bstraight.x + (Random() * 2 - 1) * bulletspread;
      bx.y = bstraight.y + (Random() * 2 - 1) * bulletspread;
      createbullet(a, bx, bulletsnap->weaponnum, p, 255, k, false, true);
    }
  }

  if ((style != bullet_style_fragnade) && (style != bullet_style_clusternade) &&
      (style != bullet_style_cluster) && (style != bullet_style_thrownknife) &&
      (style != bullet_style_m2))
  {
    sprite.weapon.fireintervalprev = 1;
    sprite.weapon.fireintervalcount = 1;
    sprite.control.fire = true;
  }
}
#endif
