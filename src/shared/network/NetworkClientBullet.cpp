// automatically converted
#include "NetworkClientBullet.hpp"

#include "../../client/Client.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../mechanics/Bullets.hpp"
#include "NetworkClient.hpp"
#include "NetworkUtils.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"

constexpr auto pingticksadd = 2;

namespace
{
std::array<tmsg_bulletsnapshot, max_sprites> oldbulletsnapshotmsg;
} // namespace

void clientsendbullet(std::uint8_t i)
{
  tmsg_clientbulletsnapshot bulletmsg;
  auto &bullet = GS::GetBulletSystem().GetBullets();
  auto &b = bullet[i];

  bulletmsg.header.id = msgid_bulletsnapshot;
  bulletmsg.weaponnum = b.ownerweapon;
  bulletmsg.pos = GetBulletParts().pos[i];
  bulletmsg.velocity = GetBulletParts().velocity[i];
  bulletmsg.clientticks = clienttickcount;
  bulletmsg.seed = b.seed;

  GetNetwork()->SendData(&bulletmsg, sizeof(bulletmsg), false);
  NotImplemented("network");
#if 0
    if (GS::GetDemoRecorder().active())
        serverbulletsnapshot(i, 0, false);
#endif
}

void clienthandlebulletsnapshot(NetworkContext *netmessage)
{
  tmsg_bulletsnapshot *bulletsnap;
  tvector2 a, b, bx;
  tvector2 bstraight;
  tvector2 bnorm;
  float hm;
  std::int32_t pa, c, d;
  std::int16_t weaponindex;
  std::uint8_t style;
  float bulletspread;

  if (!verifypacket(sizeof(tmsg_bulletsnapshot), netmessage->size, msgid_bulletsnapshot))
    return;

  bulletsnap = pmsg_bulletsnapshot(netmessage->packet);

  bulletsnap->Dump();

  if ((bulletsnap->owner < 1) || (bulletsnap->owner > max_sprites))
    return;

  if (!bulletsnap->forced)
    if ((oldbulletsnapshotmsg[bulletsnap->owner].weaponnum == bulletsnap->weaponnum) &&
        (oldbulletsnapshotmsg[bulletsnap->owner].pos.x == bulletsnap->pos.x) &&
        (oldbulletsnapshotmsg[bulletsnap->owner].pos.y == bulletsnap->pos.y) &&
        (oldbulletsnapshotmsg[bulletsnap->owner].velocity.x == bulletsnap->velocity.x) &&
        (oldbulletsnapshotmsg[bulletsnap->owner].velocity.y == bulletsnap->velocity.y))
      return;

  auto &guns = GS::GetWeaponSystem().GetGuns();
  weaponindex = weaponnumtoindex(bulletsnap->weaponnum, guns);
  if (weaponindex == -1)
    return;

  style = guns[weaponindex].bulletstyle;

  a = bulletsnap->pos;
  b = bulletsnap->velocity;

  // FIXME (falcon): Also serialize HitMultiply for CreateBullet()
  // on the other side, how the hell it works now? (because it does)
  hm = SpriteSystem::Get().GetSprite(bulletsnap->owner).weapon.hitmultiply;
  if (style == bullet_style_fragnade)
    hm = guns[fraggrenade].hitmultiply;

  const auto i = createbullet(a, b, bulletsnap->weaponnum, bulletsnap->owner, 255, hm, false, true);

  auto &bullet = GS::GetBulletSystem().GetBullets();
  auto &bul = bullet[i];

  bul.ownerpingtick =
    SpriteSystem::Get().GetSprite(bulletsnap->owner).player->pingticks + pingticksadd;
  pa = SpriteSystem::Get().GetSprite(mysprite).player->pingticks + bul.ownerpingtick;
  bul.pingadd = pa;
  bul.pingaddstart = pa;
  if (!bulletsnap->forced)
  {
    bulletspread = guns[weaponindex].bulletspread;

    if (bulletsnap->weaponnum == eagle_num) // Desert Eagle pellets
    {
      // Undo the bullet spread used on the first pellet that was sent in order to
      // get the "straight" bullet vector. Then re-apply the the same randomness
      NotImplemented("network");
#if 0
            randseed = bulletsnap->seed;
#endif
      bstraight.x = b.x - (float)(Random() * 2 - 1) * bulletspread;
      bstraight.y = b.y - (float)(Random() * 2 - 1) * bulletspread;

      bx.x = bstraight.x + (float)(Random() * 2 - 1) * bulletspread;
      bx.y = bstraight.y + (float)(Random() * 2 - 1) * bulletspread;

      vec2normalize(bnorm, bstraight);
      a.x = a.x - sign(bstraight.x) * fabs(bnorm.y) * 3.0;
      a.y = a.y + sign(bstraight.y) * fabs(bnorm.x) * 3.0;

      const auto k =
        createbullet(a, bx, bulletsnap->weaponnum, bulletsnap->owner, 255, i, false, true);
      auto &b = bullet[k];

      if ((mysprite > 0) && (bulletsnap->owner > 0))
        for (c = 1; c <= pa; c++)
        {
          if (b.active)
          {
            GetBulletParts().doeulertimestepfor(k);
            b.update();
            if (!b.active)
              break;
          }
        }
    }
    else if (style == bullet_style_shotgun) // SPAS-12 pellets
    {
      // Undo the bullet spread used on the first pellet that was sent in order to
      // get the "straight" bullet vector. Then re-apply the the same randomness
      NotImplemented("network");
#if 0
            randseed = bulletsnap->seed;
#endif
      bstraight.x = b.x - (Random() * 2 - 1) * bulletspread;
      bstraight.y = b.y - (Random() * 2 - 1) * bulletspread;

      for (d = 0; d <= 4; d++) // Remaining 5 pellets
      {
        bx.x = bstraight.x + (Random() * 2 - 1) * bulletspread;
        bx.y = bstraight.y + (Random() * 2 - 1) * bulletspread;
        const auto k =
          createbullet(a, bx, bulletsnap->weaponnum, bulletsnap->owner, 255, hm, false, true);

        auto &b = bullet[k];

        if ((mysprite > 0) && (bulletsnap->owner > 0))
        {
          for (c = 1; c <= pa; c++)
            if (b.active)
            {
              GetBulletParts().doeulertimestepfor(k);
              b.update();
              if (!b.active)
                break;
            }
        }
      }
    }

    if ((style != bullet_style_fragnade) && (style != bullet_style_clusternade) &&
        (style != bullet_style_cluster) && (style != bullet_style_thrownknife) &&
        (style != bullet_style_m2))
    {
      SpriteSystem::Get().GetSprite(bulletsnap->owner).fire();
    }
  }

  if (bul.active)
    if ((mysprite > 0) && (bulletsnap->owner > 0))
      for (c = 1; c <= pa; c++)
      {
        GetBulletParts().doeulertimestepfor(i);
        bul.update();
        if (!bul.active)
          break;
      }

  auto &things = GS::GetThingSystem().GetThings();
  // stat gun
  if (!bulletsnap->forced)
    if (style == bullet_style_m2)
      for (auto i = 1; i <= max_things; i++)
        if ((things[i].active) && (things[i].style == object_stationary_gun))
          things[i].checkstationaryguncollision(true);

  oldbulletsnapshotmsg[bulletsnap->owner] = *bulletsnap;
}
