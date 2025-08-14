// automatically converted

#include "NetworkServerSprite.hpp"

#include <memory>
#include <string>

#include "../../server/Server.hpp"
#include "../Game.hpp"
#include "NetworkUtils.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/Parts.hpp"
#include "common/PolyMap.hpp"
#include "common/Vector.hpp"
#include "common/Weapons.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Bullets.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/network/Net.hpp"
#include "shared/network/NetworkServer.hpp"

GlobalStateNetworkServerSprite gGlobalStateNetworkServerSprite{
  .oldmovementmsg{},
  .oldmouseaimmsg{},
  .oldweaponsmsg{},
  .oldhelmetmsg{},
  .oldspritesnapshotmsg{},
  .time_spritesnapshot{},
  .time_spritesnapshot_mov{},
};

float shotdistanceServer;
float shotlifeServer;
float shotricochetServer;

// SERVER SNAPSHOT
void GlobalStateNetworkServerSprite::serverspritesnapshot(std::uint8_t r)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_serverspritesnapshot servermsg;
  tvector2 b;

  // SERVER SPRITES SNAPSHOT
  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    const auto i = sprite.num;
    if (!sprite.deadmeat && sprite.isnotspectator())
    { // active player/sprite
      servermsg.header.id = msgid_serverspritesnapshot;
      // assign sprite values to ServerMsg
      auto &spritePartsPos = sprite_system.GetSpritePartsPos(i);
      auto &spriteVelocity = sprite_system.GetVelocity(i);
      servermsg.pos = spritePartsPos;
      servermsg.velocity = spriteVelocity;
      servermsg.num = sprite.num;
      servermsg.health = sprite.GetHealth();
      servermsg.position = sprite.position;
      servermsg.serverticks = gGlobalStateNetworkServer.servertickcounter;

      encodekeys(sprite, servermsg.keys16);

      servermsg.mouseaimy = sprite.control.mouseaimy;
      servermsg.mouseaimx = sprite.control.mouseaimx;

      servermsg.look = 0;
      if (sprite.wearhelmet == 0)
      {
        servermsg.look = servermsg.look | B1;
      }
      if (sprite.hascigar == 5)
      {
        servermsg.look = servermsg.look | B2;
      }
      if (sprite.hascigar == 10)
      {
        servermsg.look = servermsg.look | B3;
      }
      if (sprite.wearhelmet == 2)
      {
        servermsg.look = servermsg.look | B4;
      }

      servermsg.weaponnum = sprite.weapon.num;
      servermsg.secondaryweaponnum = sprite.secondaryweapon.num;
      servermsg.ammocount = sprite.weapon.ammocount;
      servermsg.grenadecount = sprite.tertiaryweapon.ammocount;
      if (sprite.vest < 0)
      {
        sprite.vest = 0;
      }
      if (sprite.vest > defaultvest)
      {
        sprite.vest = defaultvest;
      }
      servermsg.vest = sprite.vest;

      b = vec2subtract(servermsg.velocity,
                       gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].velocity);

      if (((vec2length(b) > veldelta)) ||
          (GS::GetGame().GetMainTickCounter() -
             gGlobalStateNetworkServerSprite.time_spritesnapshot_mov[i] >
           30) ||
          (GS::GetGame().GetMainTickCounter() -
             gGlobalStateNetworkServerSprite.time_spritesnapshot[i] >
           30) ||
          (servermsg.health != gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].health) ||
          (servermsg.position !=
           gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].position) ||
          (servermsg.keys16 != gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].keys16) ||
          (servermsg.weaponnum !=
           gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].weaponnum) ||
          (servermsg.secondaryweaponnum !=
           gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].secondaryweaponnum) ||
          (servermsg.ammocount !=
           gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].ammocount) ||
          (servermsg.grenadecount !=
           gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].grenadecount) ||
          (servermsg.vest != gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].vest))
      {
        // send to all
        if (r == netw)
        {
          for (auto &sprite : sprite_system.GetActiveSprites())
          {
            if (sprite.player->controlmethod == human)
            {
              gGlobalStateNetworkServer.GetServerNetwork()->SendData(&servermsg, sizeof(servermsg),
                                                                     sprite.player->peer, false);
            }
          }
        }
      }
      if (r == netw)
      {
        gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i] = servermsg;
      }
    }
  }
}

// SERVER SNAPSHOT MAJOR
void GlobalStateNetworkServerSprite::serverspritesnapshotmajor(std::uint8_t r)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_serverspritesnapshot_major servermsg;
  tvector2 b;

  // SERVER SPRITES SNAPSHOT
  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    const auto i = sprite.num;
    if (!sprite.deadmeat && sprite.isnotspectator())
    { // active player/sprite
      servermsg.header.id = msgid_serverspritesnapshot_major;
      // assign sprite values to ServerMsg
      auto &spritePartsPos = sprite_system.GetSpritePartsPos(i);
      auto &spriteVelocity = sprite_system.GetVelocity(i);
      servermsg.pos = spritePartsPos;
      servermsg.velocity = spriteVelocity;
      servermsg.num = sprite.num;
      servermsg.health = sprite.GetHealth();
      servermsg.position = sprite.position;
      servermsg.serverticks = gGlobalStateNetworkServer.servertickcounter;

      encodekeys(sprite, servermsg.keys16);

      servermsg.mouseaimy = sprite.control.mouseaimy;
      servermsg.mouseaimx = sprite.control.mouseaimx;

      b = vec2subtract(servermsg.velocity,
                       gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].velocity);

      if (((vec2length(b) > veldelta)) ||
          ((GS::GetGame().GetMainTickCounter() -
            gGlobalStateNetworkServerSprite.time_spritesnapshot_mov[i]) > 30) ||
          (servermsg.position !=
           gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].position) ||
          (servermsg.health != gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].health) ||
          (servermsg.keys16 != gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].keys16))
      {
        // send to all
        if (r == netw)
        {
          for (auto &sprite : sprite_system.GetActiveSprites())
          {
            if (sprite.player->controlmethod == human)
            {
              gGlobalStateNetworkServer.GetServerNetwork()->SendData(&servermsg, sizeof(servermsg),
                                                                     sprite.player->peer, false);
            }
          }
        }
      }

      if (r == netw)
      {
        gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].keys16 = servermsg.keys16;
        gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].position = servermsg.position;
        gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].pos = servermsg.pos;
        gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[i].velocity = servermsg.velocity;
      }
    }
  }
}

void GlobalStateNetworkServerSprite::serverspritesnapshotmajorfloat(const std::uint8_t who,
                                                                    std::uint8_t r)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_serverspritesnapshot_major servermsg;

  servermsg.header.id = msgid_serverspritesnapshot_major;

  auto &spriteVelocity = sprite_system.GetVelocity(who);
  auto &spritePartsPos = sprite_system.GetSpritePartsPos(who);

  // assign sprite values to ServerMsg
  servermsg.pos = spritePartsPos;
  servermsg.velocity = spriteVelocity;
  servermsg.num = sprite_system.GetSprite(who).num;
  servermsg.health = sprite_system.GetSprite(who).GetHealth();
  servermsg.position = sprite_system.GetSprite(who).position;
  servermsg.serverticks = gGlobalStateNetworkServer.servertickcounter;

  encodekeys(sprite_system.GetSprite(who), servermsg.keys16);

  servermsg.mouseaimy = sprite_system.GetSprite(who).control.mouseaimy;
  servermsg.mouseaimx = sprite_system.GetSprite(who).control.mouseaimx;

  // send to all
  if (r == netw)
  {
    for (auto &sprite : sprite_system.GetActiveSprites())
    {
      if (sprite.player->controlmethod == human)
      {
        gGlobalStateNetworkServer.GetServerNetwork()->SendData(&servermsg, sizeof(servermsg),
                                                               sprite.player->peer, false);
      }
    }
  }

  if (r == netw)
  {
    gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[who].keys16 = servermsg.keys16;
    gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[who].position = servermsg.position;
    gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[who].pos = servermsg.pos;
    gGlobalStateNetworkServerSprite.oldspritesnapshotmsg[who].velocity = servermsg.velocity;
  }
}

// SERVER SKELETON SNAPSHOT
void GlobalStateNetworkServerSprite::serverskeletonsnapshot(std::uint8_t r)
{
  tmsg_serverskeletonsnapshot skeletonmsg;

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.deadmeat && sprite.isnotspectator())
    { // active player/sprite
      skeletonmsg.header.id = msgid_serverskeletonsnapshot;
      // assign sprite values to SkeletonMsg
      skeletonmsg.num = sprite.num;
      if (sprite.respawncounter > 0)
      {
        skeletonmsg.respawncounter = sprite.respawncounter;
      }
      else
      {
        skeletonmsg.respawncounter = 0;
      }

      // send to all
      if (r == netw)
      {
        for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
        {
          if (sprite.player->controlmethod == human)
          {
            gGlobalStateNetworkServer.GetServerNetwork()->SendData(
              &skeletonmsg, sizeof(skeletonmsg), sprite.player->peer, false);
          }
        }
      }
    }
  }
}

void GlobalStateNetworkServerSprite::serverspritedeath(std::int32_t who, std::int32_t killer,
                                                       std::int32_t bulletnum, std::int32_t where)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_spritedeath spritedeathmsg;

  spritedeathmsg.header.id = msgid_spritedeath;
  // assign sprite values to SpriteDeathMsg
  spritedeathmsg.num = who;
  spritedeathmsg.killer = killer;
  spritedeathmsg.where = where;
  spritedeathmsg.respawncounter = sprite_system.GetSprite(who).respawncounter;
  spritedeathmsg.health = sprite_system.GetSprite(who).GetHealth();
  spritedeathmsg.onfire = sprite_system.GetSprite(who).onfire;
  spritedeathmsg.shotdistance = shotdistanceServer;
  spritedeathmsg.shotlife = shotlifeServer;
  spritedeathmsg.shotricochet = shotricochetServer;

  if (bulletnum == -1)
  {
    spritedeathmsg.killbullet = 250;
  }
  else
  {
    // if Bullet[BulletNum].OwnerWeapon = 0 then
    // SpriteDeathMsg.KillBullet := 250;
    auto &bullet = GS::GetBulletSystem().GetBullets();
    auto &b = bullet[bulletnum];
    spritedeathmsg.killbullet = b.ownerweapon;
    if (b.style == 2)
    {
      spritedeathmsg.killbullet = 222;
    }
    if (b.style == 10)
    {
      spritedeathmsg.killbullet = 210;
    }
    if (b.style == 5)
    {
      spritedeathmsg.killbullet = 205;
    }
    if (b.style == 7)
    {
      spritedeathmsg.killbullet = 207;
    }
    if (b.style == 8)
    {
      spritedeathmsg.killbullet = 208;
    }
    if (b.style == 6)
    {
      spritedeathmsg.killbullet = 206;
    }
    if (b.ownerweapon == knife_num)
    {
      spritedeathmsg.killbullet = 211;
    }
    if (b.ownerweapon == chainsaw_num)
    {
      spritedeathmsg.killbullet = 212;
    }
    if (b.style == 12)
    {
      spritedeathmsg.killbullet = 224;
    }
    if (b.style == 13)
    {
      spritedeathmsg.killbullet = 211;
    }
    if (b.style == 14)
    {
      spritedeathmsg.killbullet = 225;
    }
  }

  for (auto j = 1; j <= 16; j++)
  {
    [[deprecated]] auto jminus1 = j - 1;
    spritedeathmsg.pos[jminus1].x = sprite_system.GetSprite(who).skeleton.pos[j].x;
    spritedeathmsg.pos[jminus1].y = sprite_system.GetSprite(who).skeleton.pos[j].y;
    spritedeathmsg.oldpos[jminus1].x = sprite_system.GetSprite(who).skeleton.oldpos[j].x;
    spritedeathmsg.oldpos[jminus1].y = sprite_system.GetSprite(who).skeleton.oldpos[j].y;
  }

  spritedeathmsg.constraints = 0;
  if (!sprite_system.GetSprite(who).skeleton.constraints[2].active)
  {
    spritedeathmsg.constraints = spritedeathmsg.constraints | B1;
  }
  if (!sprite_system.GetSprite(who).skeleton.constraints[4].active)
  {
    spritedeathmsg.constraints = spritedeathmsg.constraints | B2;
  }
  if (!sprite_system.GetSprite(who).skeleton.constraints[20].active)
  {
    spritedeathmsg.constraints = spritedeathmsg.constraints | B3;
  }
  if (!sprite_system.GetSprite(who).skeleton.constraints[21].active)
  {
    spritedeathmsg.constraints = spritedeathmsg.constraints | B4;
  }
  if (!sprite_system.GetSprite(who).skeleton.constraints[23].active)
  {
    spritedeathmsg.constraints = spritedeathmsg.constraints | B5;
  }

  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
    {
      gGlobalStateNetworkServer.GetServerNetwork()->SendData(
        &spritedeathmsg, sizeof(spritedeathmsg), sprite.player->peer, false);
    }
  }
}

// SEND DELTAS OF SPRITE
void GlobalStateNetworkServerSprite::serverspritedeltas(const std::uint8_t i)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_serverspritedelta_movement movementmsg;
  tmsg_serverspritedelta_weapons weaponsmsg;
  tmsg_serverspritedelta_helmet helmetmsg;
  std::int32_t j;
  tvector2 a;
  tvector2 b;

  const auto &spritePartsPos = sprite_system.GetSpritePartsPos(i);
  auto &spriteVelocity = sprite_system.GetVelocity(i);

  helmetmsg.header.id = msgid_delta_helmet;
  helmetmsg.num = i;
  helmetmsg.wearhelmet = sprite_system.GetSprite(i).wearhelmet;

  movementmsg.header.id = msgid_delta_movement;
  movementmsg.num = i;

  movementmsg.velocity = spriteVelocity;
  movementmsg.pos = spritePartsPos;
  movementmsg.servertick = gGlobalStateNetworkServer.servertickcounter;

  encodekeys(sprite_system.GetSprite(i), movementmsg.keys16);

  movementmsg.mouseaimy = sprite_system.GetSprite(i).control.mouseaimy;
  movementmsg.mouseaimx = sprite_system.GetSprite(i).control.mouseaimx;

  weaponsmsg.header.id = msgid_delta_weapons;
  weaponsmsg.num = i;
  weaponsmsg.weaponnum = sprite_system.GetSprite(i).weapon.num;
  weaponsmsg.secondaryweaponnum = sprite_system.GetSprite(i).secondaryweapon.num;
  weaponsmsg.ammocount = sprite_system.GetSprite(i).weapon.ammocount;

  for (j = 1; j <= max_sprites; j++)
  {
    if (sprite_system.GetSprite(j).active &&
        (sprite_system.GetSprite(j).player->controlmethod == human) && (j != i))
    {
      if (GS::GetGame().pointvisible(spritePartsPos.x, spritePartsPos.y,
                                     sprite_system.GetSprite(j).player->camera) or
          (sprite_system.GetSprite(j).isspectator() &&
           (sprite_system.GetSprite(j).player->port == 0))) // visible to sprite
      {
        a = vec2subtract(movementmsg.pos, gGlobalStateNetworkServerSprite.oldmovementmsg[j][i].pos);
        b = vec2subtract(movementmsg.velocity,
                         gGlobalStateNetworkServerSprite.oldmovementmsg[j][i].velocity);
        if ((sprite_system.GetSprite(i).player->controlmethod == human) ||
            (((vec2length(a) > posdelta) || (vec2length(b) > veldelta)) &&
             (movementmsg.keys16 != gGlobalStateNetworkServerSprite.oldmovementmsg[j][i].keys16)))
        {
          gGlobalStateNetworkServer.GetServerNetwork()->SendData(
            &movementmsg, sizeof(movementmsg), sprite_system.GetSprite(j).player->peer, false);
          gGlobalStateNetworkServerSprite.oldmovementmsg[j][i] = movementmsg;
        }
      }
    }
  }

  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    const auto j = sprite.num;
    if ((sprite.player->controlmethod == human) && (j != i))
    {
      if ((weaponsmsg.weaponnum != gGlobalStateNetworkServerSprite.oldweaponsmsg[j][i].weaponnum) ||
          (weaponsmsg.secondaryweaponnum !=
           gGlobalStateNetworkServerSprite.oldweaponsmsg[j][i].secondaryweaponnum))
      {
        if (GS::GetGame().pointvisible(spritePartsPos.x, spritePartsPos.y,
                                       sprite.player->camera) or
            (sprite.isspectator() && (sprite.player->port == 0))) // visible to sprite
        {
          gGlobalStateNetworkServer.GetServerNetwork()->SendData(&weaponsmsg, sizeof(weaponsmsg),
                                                                 sprite.player->peer, false);
          gGlobalStateNetworkServerSprite.oldweaponsmsg[j][i] = weaponsmsg;
        }
      }
    }
  }

  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    const auto j = sprite.num;
    if ((sprite.player->controlmethod == human) && (j != i))
    {
      if (helmetmsg.wearhelmet != gGlobalStateNetworkServerSprite.oldhelmetmsg[j][i].wearhelmet)
      {
        gGlobalStateNetworkServer.GetServerNetwork()->SendData(&helmetmsg, sizeof(helmetmsg),
                                                               sprite.player->peer, false);
        gGlobalStateNetworkServerSprite.oldhelmetmsg[j][i] = helmetmsg;
      }
    }
  }
}

void GlobalStateNetworkServerSprite::serverspritedeltasmouse(std::uint8_t i)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_serverspritedelta_mouseaim mouseaimmsg;

  mouseaimmsg.header.id = msgid_delta_mouseaim;
  mouseaimmsg.num = i;
  mouseaimmsg.mouseaimy = sprite_system.GetSprite(i).control.mouseaimy;
  mouseaimmsg.mouseaimx = sprite_system.GetSprite(i).control.mouseaimx;

  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    const auto j = sprite.num;
    if ((sprite.player->controlmethod == human) && (j != i))
    {
      gGlobalStateNetworkServer.GetServerNetwork()->SendData(&mouseaimmsg, sizeof(mouseaimmsg),
                                                             sprite.player->peer, false);
      gGlobalStateNetworkServerSprite.oldmouseaimmsg[j][i] = mouseaimmsg;
    }
  }
}

void GlobalStateNetworkServerSprite::serverhandleclientspritesnapshot(tmsgheader *netmessage,
                                                                      std::int32_t size,
                                                                      NetworkServer &network,
                                                                      TServerPlayer *player)
{
  auto &sprite_system = SpriteSystem::Get();
  pmsg_clientspritesnapshot clientmsg;
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_clientspritesnapshot), size, msgid_clientspritesnapshot))
  {
    return;
  }
  clientmsg = pmsg_clientspritesnapshot(netmessage);
  i = player->spritenum;

  auto &sprite = sprite_system.GetSprite(i);

  gGlobalStateNetworkServer.messagesasecnum[i] += 1;

  if (sprite.deadmeat)
  {
    return;
  }

  sprite.player->camera = i;

#ifdef SCRIPT
  forceweaponcalled = false;
  if ((sprite.weapon.num != clientmsg.weaponnum) ||
      (sprite.secondaryweapon.num != clientmsg.secondaryweaponnum))
  {
    // event must be before actual weapon apply.
    // script might've called ForceWeapon, which we should check.
    // if it did, we don't apply snapshot weapon's as they were already applied
    // by force weapon.
    scrptdispatcher.onweaponchange(i, clientmsg.weaponnum, clientmsg.secondaryweaponnum,
                                   clientmsg.ammocount, clientmsg.secondaryammocount);
  }
  if (!forceweaponcalled)
  {
#endif
    if (sprite.weapon.num != clientmsg->weaponnum)
    {
      sprite.applyweaponbynum(clientmsg->weaponnum, 1, clientmsg->ammocount);
    }
    if (sprite.secondaryweapon.num != clientmsg->secondaryweaponnum)
    {
      sprite.applyweaponbynum(clientmsg->secondaryweaponnum, 2, clientmsg->secondaryammocount);
    }
#ifdef SCRIPT
  }
#endif

  if (sprite.weapon.num == colt_num)
  {
    sprite.player->secwep = 0;
  }
  if (sprite.weapon.num == knife_num)
  {
    sprite.player->secwep = 1;
  }
  if (sprite.weapon.num == chainsaw_num)
  {
    sprite.player->secwep = 2;
  }
  if (sprite.weapon.num == law_num)
  {
    sprite.player->secwep = 3;
  }

  sprite.weapon.ammocount = clientmsg->ammocount;
  sprite.secondaryweapon.ammocount = clientmsg->secondaryammocount;

  // Toggle prone if it was activated or deactivated
  sprite.control.prone = (clientmsg->position == pos_prone) ^ (sprite.position == pos_prone);

  if (checkweaponnotallowed(i))
  {
    gGlobalStateServer.kickplayer(i, true, kick_cheat, day, "Not allowed weapon");
    return;
  }

  serverspritedeltas(i);

  gGlobalStateNetworkServerSprite.time_spritesnapshot[i] = GS::GetGame().GetMainTickCounter();
}
void GlobalStateNetworkServerSprite::serverhandleclientspritesnapshot_mov(tmsgheader *netmessage,
                                                                          std::int32_t size,
                                                                          NetworkServer &network,
                                                                          TServerPlayer *player)
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_clientspritesnapshot_mov), size, msgid_clientspritesnapshot_mov))
  {
    return;
  }

  tmsg_clientspritesnapshot_mov &clientmovmsg = *pmsg_clientspritesnapshot_mov(netmessage);
  i = player->spritenum;

  gGlobalStateNetworkServer.messagesasecnum[i] += 1;

  auto &sprite = sprite_system.GetSprite(i);

  if (sprite.deadmeat)
  {
    return;
  }

  auto &map = GS::GetGame().GetMap();
  map.checkoutofbounds(clientmovmsg.pos.x, clientmovmsg.pos.y);
  map.checkoutofbounds(clientmovmsg.velocity.x, clientmovmsg.velocity.y);

  sprite.player->camera = i;

  auto &spritePartsPos = sprite_system.GetSpritePartsPos(i);
  auto &spriteVelocity = sprite_system.GetVelocity(i);

  spritePartsPos = clientmovmsg.pos;
  spriteVelocity = clientmovmsg.velocity;

  map.checkoutofbounds(clientmovmsg.mouseaimx, clientmovmsg.mouseaimy);

  sprite.control.mouseaimx = clientmovmsg.mouseaimx;
  sprite.control.mouseaimy = clientmovmsg.mouseaimy;

  decodekeys(sprite, clientmovmsg.keys16);

  if (!sprite.control.throwweapon)
  {
    sprite.player->knifewarnings = 0;
  }

  serverspritedeltas(i);

  gGlobalStateNetworkServerSprite.time_spritesnapshot_mov[i] = GS::GetGame().GetMainTickCounter();
}

void GlobalStateNetworkServerSprite::serverhandleclientspritesnapshot_dead(tmsgheader *netmessage,
                                                                           std::int32_t size,
                                                                           NetworkServer &network,
                                                                           TServerPlayer *player)
{
  auto &sprite_system = SpriteSystem::Get();
  pmsg_clientspritesnapshot_dead clientdeadmsg;
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_clientspritesnapshot_dead), size, msgid_clientspritesnapshot_dead))
  {
    return;
  }
  clientdeadmsg = pmsg_clientspritesnapshot_dead(netmessage);
  i = player->spritenum;

  gGlobalStateNetworkServer.messagesasecnum[i] += 1;

  auto &sprite = sprite_system.GetSprite(i);

  if (!sprite.deadmeat)
  {
    return;
  }

  // assign received sprite info to sprite
  if (clientdeadmsg->camerafocus < max_sprites + 1)
  {
    sprite.player->camera = clientdeadmsg->camerafocus;
  }
}
