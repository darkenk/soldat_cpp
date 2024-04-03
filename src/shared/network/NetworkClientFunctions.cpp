// automatically converted
#include "NetworkClientFunctions.hpp"

#include "../../client/GameMenus.hpp"
#include "../../client/InterfaceGraphics.hpp"
#include "../../client/Sound.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "NetworkClientConnection.hpp"
#include "NetworkUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"

void clienthandlevoteon(SteamNetworkingMessage_t *netmessage)
{
  tmsg_voteon *voteonmsg;
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_voteon), netmessage->m_cbSize, msgid_voteon))
    return;

  voteonmsg = pmsg_voteon(netmessage->m_pData);

  i = voteonmsg->who;

  statsmenushow = false;

  GS::GetGame().startvote(i, voteonmsg->votetype, voteonmsg->targetname.data(),
                          voteonmsg->reason.data());
}

void clienthandlevoteoff()
{
  GS::GetGame().stopvote();
}

void clienthandleserversyncmsg(SteamNetworkingMessage_t *netmessage)
{
  tmsg_serversyncmsg *syncmsg;

  if (!verifypacket(sizeof(tmsg_serversyncmsg), netmessage->m_cbSize, msgid_serversyncmsg))
    return;

  syncmsg = pmsg_serversyncmsg(netmessage->m_pData);

  GS::GetGame().SetTimelimitcounter(syncmsg->time);
  if (syncmsg->pause == 1)
  {
    GS::GetGame().SetMapchangecounter(999999999);
    GS::GetGame().SetMapchangename("PAUSE*!*");
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      stopsound(sprite.reloadsoundchannel);
      stopsound(sprite.jetssoundchannel);
      stopsound(sprite.gattlingsoundchannel);
      stopsound(sprite.gattlingsoundchannel2);
    }
  }
  else if (GS::GetGame().GetMapchangecounter() == 999999999)
  {
    GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
  }
}

void clienthandleforceposition(SteamNetworkingMessage_t *netmessage)
{
  tmsg_forceposition *forceposition;

  if (!verifypacket(sizeof(tmsg_forceposition), netmessage->m_cbSize, msgid_forceposition))
    return;

  forceposition = pmsg_forceposition(netmessage->m_pData);

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(forceposition->playerid);

  spritePartsPos = forceposition->pos;
  SpriteSystem::Get().SetSpritePartsOldPos(forceposition->playerid, spritePartsPos);
}

void clienthandleforcevelocity(SteamNetworkingMessage_t *netmessage)
{
  tmsg_forcevelocity *forcevelocity;

  if (!verifypacket(sizeof(tmsg_forcevelocity), netmessage->m_cbSize, msgid_forcevelocity))
    return;

  forcevelocity = pmsg_forcevelocity(netmessage->m_pData);

  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(forcevelocity->playerid);
  spriteVelocity = forcevelocity->vel;
}

void clienthandleforceweapon(SteamNetworkingMessage_t *netmessage)
{
  tmsg_forceweapon *forceweapon;

  if (!verifypacket(sizeof(tmsg_forceweapon), netmessage->m_cbSize, msgid_forceweapon))
    return;

  forceweapon = pmsg_forceweapon(netmessage->m_pData);

  if (mysprite > 0)
  {
    SpriteSystem::Get().GetSprite(mysprite).applyweaponbynum(forceweapon->weaponnum, 1);
    SpriteSystem::Get().GetSprite(mysprite).applyweaponbynum(forceweapon->secondaryweaponnum, 2);
    SpriteSystem::Get().GetSprite(mysprite).weapon.ammocount = forceweapon->ammocount;
    SpriteSystem::Get().GetSprite(mysprite).secondaryweapon.ammocount = forceweapon->secammocount;
  }
}

void clienthandleweaponactivemessage(SteamNetworkingMessage_t *netmessage)
{
  tmsg_weaponactivemessage *wactivemessage;
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_weaponactivemessage), netmessage->m_cbSize,
                    msgid_weaponactivemessage))
    return;

  wactivemessage = pmsg_weaponactivemessage(netmessage->m_pData);
  auto &weaponSystem = GS::GetWeaponSystem();
  auto &weaponsel = GS::GetGame().GetWeaponsel();

  if ((wactivemessage->weapon > 0) && (wactivemessage->weapon <= main_weapons))
  {
    weaponsel[mysprite][wactivemessage->weapon] = wactivemessage->active;

    for (i = 1; i <= main_weapons; i++)
    {
      if (weaponSystem.IsEnabled(i))
      {
        limbomenu->button[i - 1].active = (bool)(weaponsel[mysprite][i]);
      }
    }
  }
}

void clienthandleclientfreecam(SteamNetworkingMessage_t *netmessage)
{
  tmsg_clientfreecam *freecammsg;

  if (!verifypacket(sizeof(tmsg_clientfreecam), netmessage->m_cbSize, msgid_clientfreecam))
    return;

  freecammsg = pmsg_clientfreecam(netmessage->m_pData);

  if (mysprite > 0)
  {
    if (freecammsg->freecamon == 1)
    {
      camerafollowsprite = 0;
      targetmode = true;
    }
    else
    {
      camerafollowsprite = mysprite;
      targetmode = false;
    }

    if ((freecammsg->targetpos.x != 0.0) && (freecammsg->targetpos.y != 0.0))
    {
      camerax = freecammsg->targetpos.x;
      cameray = freecammsg->targetpos.y;
    }
  }
}

// Server tells client to join another server
void clienthandlejoinserver(SteamNetworkingMessage_t *netmessage)
{
  tmsg_joinserver *joinservermsg;

  if (!verifypacket(sizeof(tmsg_joinserver), netmessage->m_cbSize, msgid_joinserver))
    return;

  joinservermsg = pmsg_joinserver(netmessage->m_pData);
  NotImplemented("network");
#if 0
    GS::GetMainConsole().console(std::string("Redirecting to... ") +
                            netaddrtostr(in_addr(joinservermsg->ip)) + ':' +
                            inttostr(joinservermsg->port),
                        server_message_color);
#endif

  clientdisconnect();

  redirecttoserver = true;
  NotImplemented("network");
#if 0
    redirectip = netaddrtostr(in_addr(joinservermsg->ip));
#endif
  redirectport = joinservermsg->port;
  redirectmsg = trim((pmsg_joinserver(netmessage->m_pData)->showmsg.data()));

  exittomenu();
}

void clienthandleplaysound(SteamNetworkingMessage_t *netmessage)
{
  tmsg_playsound *playsoundmsg;
  std::int32_t i;
  auto& fs = GS::GetFileSystem();

  if (!verifypacket(sizeof(tmsg_playsound), netmessage->m_cbSize, msgid_playsound))
    return;

  playsoundmsg = pmsg_playsound(netmessage->m_pData);

  if (fs.Exists(std::string(moddir) + std::string("sfx/") + playsoundmsg->name.data()))
  {
    // Name to ID, for easy use for scripters
    i = soundnametoid(playsoundmsg->name.data());
    NotImplemented("network");
#if 0
        // Sound downloaded, but not initialized. So intialize it
        if (i == -1)
        {
            setlength(scriptsamp, high(scriptsamp) + 2);
            i = high(scriptsamp);
            scriptsamp[i].name = playsoundmsg->name;
            scriptsamp[i].samp = loadsample((pchar)(moddir + "sfx/" + playsoundmsg->name.data()),
                                            scriptsamp[i].samp);
        }

        auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(mysprite);
        if (playsoundmsg->emitter.x == 0)
            playsoundmsg->emitter.x = spritePartsPos.x;
        if (playsoundmsg->emitter.y == 0)
            playsoundmsg->emitter.y = spritePartsPos.y;
        playsound(i, playsoundmsg->emitter);
#endif
  }
}
