// automatically converted

#include "NetworkServerFunctions.hpp"

#include <array>
#include <memory>

#include "NetworkServer.hpp"
#include "NetworkUtils.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/Util.hpp"
#include "common/Vector.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/misc/SafeType.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/network/Net.hpp"

void serversendfreecam(std::uint8_t tonum, bool freecam, tvector2 pos)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_clientfreecam freecammsg;

  freecammsg.header.id = msgid_clientfreecam;
  freecammsg.freecamon = (std::uint8_t)(freecam);
  freecammsg.targetpos = pos;

  gGlobalStateNetworkServer.GetServerNetwork()->SendData(
    &freecammsg, sizeof(freecammsg), sprite_system.GetSprite(tonum).player->peer, true);
}

void setweaponactive(std::uint8_t id, std::uint8_t weaponnum, bool state)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_weaponactivemessage wepmsg;

  wepmsg.header.id = msgid_weaponactivemessage;
  wepmsg.weapon = weaponnum;
  wepmsg.active = iif(state, 1, 0);

  if (id == 0)
  {
    for (auto &sprite : sprite_system.GetActiveSprites())
    {
      if (sprite.player->controlmethod == human)
      {
        gGlobalStateNetworkServer.GetServerNetwork()->SendData(&wepmsg, sizeof(wepmsg),
                                                               sprite.player->peer, true);
      }
    }
  }
  else if ((sprite_system.GetSprite(id).active) &&
           (sprite_system.GetSprite(id).player->controlmethod == human))
  {
    gGlobalStateNetworkServer.GetServerNetwork()->SendData(
      &wepmsg, sizeof(wepmsg), sprite_system.GetSprite(id).player->peer, true);
  }
}

void forceweapon(std::uint8_t id, std::uint8_t primary, std::uint8_t secondary, std::uint8_t ammo,
                 std::uint8_t secammo)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_forceweapon wepmsg;

  sprite_system.GetSprite(id).applyweaponbynum(primary, 1);
  sprite_system.GetSprite(id).applyweaponbynum(secondary, 2);
  auto &guns = GS::GetWeaponSystem().GetGuns();

  if (ammo > guns[weaponnumtoindex(primary, guns)].ammo)
  {
    ammo = guns[weaponnumtoindex(primary, guns)].ammo;
  }

  if (secammo > guns[weaponnumtoindex(secondary, guns)].ammo)
  {
    secammo = guns[weaponnumtoindex(secondary, guns)].ammo;
  }

  sprite_system.GetSprite(id).weapon.ammo = ammo;
  sprite_system.GetSprite(id).secondaryweapon.ammo = secammo;

  if (sprite_system.GetSprite(id).player->controlmethod == human)
  {
    wepmsg.header.id = msgid_forceweapon;
    wepmsg.weaponnum = primary;
    wepmsg.secondaryweaponnum = secondary;
    wepmsg.ammocount = ammo;
    wepmsg.secammocount = secammo;

    gGlobalStateNetworkServer.GetServerNetwork()->SendData(
      &wepmsg, sizeof(wepmsg), sprite_system.GetSprite(id).player->peer, true);
  }

#ifdef SCRIPT
  // In some places in code, where we apply weapons (sprie snapshot, object collisions),
  // we need to check whenever weapons are already changed, hence this ugly workaround
  // with bool variable.
  forceweaponcalled = true;
#endif
}

void moveplayer(const std::uint8_t id, float x, float y)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_forceposition movemsg;

  if (!sprite_system.GetSprite(id).active)
  {
    return;
  }

  auto &spritePartsPos = sprite_system.GetSpritePartsPos(id);

  spritePartsPos.x = x;
  spritePartsPos.y = y;
  sprite_system.SetSpritePartsOldPos(id, spritePartsPos);

  movemsg.header.id = msgid_forceposition;

  movemsg.pos = spritePartsPos;
  movemsg.playerid = id;

  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
    {
      gGlobalStateNetworkServer.GetServerNetwork()->SendData(&movemsg, sizeof(movemsg),
                                                             sprite.player->peer, true);
    }
  }
}

void modifyplayervelocity(const std::uint8_t id, float velx, float vely)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_forcevelocity velmsg;

  if (!sprite_system.GetSprite(id).active)
  {
    return;
  }

  auto &spriteVelocity = sprite_system.GetVelocity(id);

  spriteVelocity.x = velx;
  spriteVelocity.y = vely;

  velmsg.header.id = msgid_forcevelocity;

  velmsg.vel = spriteVelocity;
  velmsg.playerid = id;

  for (auto &sprite : sprite_system.GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
    {
      gGlobalStateNetworkServer.GetServerNetwork()->SendData(&velmsg, sizeof(velmsg),
                                                             sprite.player->peer, true);
    }
  }
}

void forwardclient(std::uint8_t id, const std::string &targetip, std::int32_t targetport,
                   const std::string &showmsg)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_joinserver joinservermsg;

  joinservermsg.header.id = msgid_joinserver;
  joinservermsg.port = targetport;
  NotImplemented("network", "missing strtonetaddr");
#if 0
    joinservermsg.ip = std::uint32_t(strtonetaddr(targetip));
#endif
  stringtoarray(joinservermsg.showmsg.data(), showmsg);

  if ((sprite_system.GetSprite(id).active) &&
      (sprite_system.GetSprite(id).player->controlmethod == human))
  {
    gGlobalStateNetworkServer.GetServerNetwork()->SendData(
      &joinservermsg, sizeof(joinservermsg), sprite_system.GetSprite(id).player->peer, true);
  }
}

void playsound(std::uint8_t id, const std::string &name, float x, float y)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_playsound playsoundmsg;
  tvector2 pos;

  playsoundmsg.header.id = msgid_playsound;
  pos.x = x;
  pos.y = y;
  playsoundmsg.emitter = pos;
  stringtoarray(playsoundmsg.name.data(), name);
  // PlaySoundMsg.Name := Name;

  if (id == 0)
  {
    for (id = 1; id <= max_players; id++)
    {
      if ((sprite_system.GetSprite(id).active) &&
          (sprite_system.GetSprite(id).player->controlmethod == human))
      {
        gGlobalStateNetworkServer.GetServerNetwork()->SendData(
          &playsoundmsg, sizeof(playsoundmsg), sprite_system.GetSprite(id).player->peer, true);
      }
    }
  }
  else if ((sprite_system.GetSprite(id).active) &&
           (sprite_system.GetSprite(id).player->controlmethod == human))
  {
    gGlobalStateNetworkServer.GetServerNetwork()->SendData(
      &playsoundmsg, sizeof(playsoundmsg), sprite_system.GetSprite(id).player->peer, true);
  }
}

void serverhandleclientfreecam(tmsgheader* netmessage, std::int32_t size, NetworkServer& network, TServerPlayer* player)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_clientfreecam *freecammsg;
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_clientfreecam), size, msgid_clientfreecam))
  {
    return;
  }
  freecammsg = pmsg_clientfreecam(netmessage);
  i = player->spritenum;
  sprite_system.GetSprite(i).targetx = freecammsg->targetpos.x;
  sprite_system.GetSprite(i).targety = freecammsg->targetpos.y;
}
