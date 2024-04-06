// automatically converted

#include "NetworkServerFunctions.hpp"

#include "NetworkServer.hpp"
#include "../Game.hpp"
#include "NetworkUtils.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"

void serversendfreecam(std::uint8_t tonum, bool freecam, tvector2 pos)
{
  tmsg_clientfreecam freecammsg;

  freecammsg.header.id = msgid_clientfreecam;
  freecammsg.freecamon = (std::uint8_t)(freecam);
  freecammsg.targetpos = pos;

  GetServerNetwork()->senddata(&freecammsg, sizeof(freecammsg),
                               SpriteSystem::Get().GetSprite(tonum).player->peer,
                               k_nSteamNetworkingSend_Reliable);
}

void setweaponactive(std::uint8_t id, std::uint8_t weaponnum, bool state)
{
  tmsg_weaponactivemessage wepmsg;

  wepmsg.header.id = msgid_weaponactivemessage;
  wepmsg.weapon = weaponnum;
  wepmsg.active = iif(state, 1, 0);

  if (id == 0)
  {
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      if (sprite.player->controlmethod == human)
      {
        GetServerNetwork()->senddata(&wepmsg, sizeof(wepmsg), sprite.player->peer,
                                     k_nSteamNetworkingSend_Reliable);
      }
    }
  }
  else if ((SpriteSystem::Get().GetSprite(id).active) &&
           (SpriteSystem::Get().GetSprite(id).player->controlmethod == human))
    GetServerNetwork()->senddata(&wepmsg, sizeof(wepmsg),
                                 SpriteSystem::Get().GetSprite(id).player->peer,
                                 k_nSteamNetworkingSend_Reliable);
}

void forceweapon(std::uint8_t id, std::uint8_t primary, std::uint8_t secondary, std::uint8_t ammo,
                 std::uint8_t secammo)
{
  tmsg_forceweapon wepmsg;

  SpriteSystem::Get().GetSprite(id).applyweaponbynum(primary, 1);
  SpriteSystem::Get().GetSprite(id).applyweaponbynum(secondary, 2);
  auto &guns = GS::GetWeaponSystem().GetGuns();

  if (ammo > guns[weaponnumtoindex(primary, guns)].ammo)
    ammo = guns[weaponnumtoindex(primary, guns)].ammo;

  if (secammo > guns[weaponnumtoindex(secondary, guns)].ammo)
    secammo = guns[weaponnumtoindex(secondary, guns)].ammo;

  SpriteSystem::Get().GetSprite(id).weapon.ammo = ammo;
  SpriteSystem::Get().GetSprite(id).secondaryweapon.ammo = secammo;

  if (SpriteSystem::Get().GetSprite(id).player->controlmethod == human)
  {
    wepmsg.header.id = msgid_forceweapon;
    wepmsg.weaponnum = primary;
    wepmsg.secondaryweaponnum = secondary;
    wepmsg.ammocount = ammo;
    wepmsg.secammocount = secammo;

    GetServerNetwork()->senddata(&wepmsg, sizeof(wepmsg),
                                 SpriteSystem::Get().GetSprite(id).player->peer,
                                 k_nSteamNetworkingSend_Reliable);
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
  tmsg_forceposition movemsg;

  if (!SpriteSystem::Get().GetSprite(id).active)
    return;

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(id);

  spritePartsPos.x = x;
  spritePartsPos.y = y;
  SpriteSystem::Get().SetSpritePartsOldPos(id, spritePartsPos);

  movemsg.header.id = msgid_forceposition;

  movemsg.pos = spritePartsPos;
  movemsg.playerid = id;

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
      GetServerNetwork()->senddata(&movemsg, sizeof(movemsg), sprite.player->peer,
                                   k_nSteamNetworkingSend_Reliable);
  }
}

void modifyplayervelocity(const std::uint8_t id, float velx, float vely)
{
  tmsg_forcevelocity velmsg;

  if (!SpriteSystem::Get().GetSprite(id).active)
    return;

  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(id);

  spriteVelocity.x = velx;
  spriteVelocity.y = vely;

  velmsg.header.id = msgid_forcevelocity;

  velmsg.vel = spriteVelocity;
  velmsg.playerid = id;

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if (sprite.player->controlmethod == human)
      GetServerNetwork()->senddata(&velmsg, sizeof(velmsg), sprite.player->peer,
                                   k_nSteamNetworkingSend_Reliable);
  }
}

void forwardclient(std::uint8_t id, std::string targetip, std::int32_t targetport,
                   std::string showmsg)
{
  tmsg_joinserver joinservermsg;

  joinservermsg.header.id = msgid_joinserver;
  joinservermsg.port = targetport;
  NotImplemented("network", "missing strtonetaddr");
#if 0
    joinservermsg.ip = std::uint32_t(strtonetaddr(targetip));
#endif
  stringtoarray(joinservermsg.showmsg.data(), showmsg);

  if ((SpriteSystem::Get().GetSprite(id).active) &&
      (SpriteSystem::Get().GetSprite(id).player->controlmethod == human))
    GetServerNetwork()->senddata(&joinservermsg, sizeof(joinservermsg),
                                 SpriteSystem::Get().GetSprite(id).player->peer,
                                 k_nSteamNetworkingSend_Reliable);
}

void playsound(std::uint8_t id, std::string name, float x, float y)
{
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
      if ((SpriteSystem::Get().GetSprite(id).active) &&
          (SpriteSystem::Get().GetSprite(id).player->controlmethod == human))
        GetServerNetwork()->senddata(&playsoundmsg, sizeof(playsoundmsg),
                                     SpriteSystem::Get().GetSprite(id).player->peer,
                                     k_nSteamNetworkingSend_Reliable);
  }
  else if ((SpriteSystem::Get().GetSprite(id).active) &&
           (SpriteSystem::Get().GetSprite(id).player->controlmethod == human))
    GetServerNetwork()->senddata(&playsoundmsg, sizeof(playsoundmsg),
                                 SpriteSystem::Get().GetSprite(id).player->peer,
                                 k_nSteamNetworkingSend_Reliable);
}

void serverhandleclientfreecam(SteamNetworkingMessage_t *netmessage)
{
  tmsg_clientfreecam *freecammsg;
  tplayer *player;
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_clientfreecam), netmessage->m_cbSize, msgid_clientfreecam))
    return;
  freecammsg = pmsg_clientfreecam(netmessage->m_pData);
  player = GetServerNetwork()->GetPlayer(netmessage);
  i = player->spritenum;
  SpriteSystem::Get().GetSprite(i).targetx = freecammsg->targetpos.x;
  SpriteSystem::Get().GetSprite(i).targety = freecammsg->targetpos.y;
}
