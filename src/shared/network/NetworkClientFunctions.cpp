// automatically converted
#include "NetworkClientFunctions.hpp"

#include "../../client/GameMenus.hpp"
#include "../../client/InterfaceGraphics.hpp"
#include "../../client/Sound.hpp"
#include "../Game.hpp"
#include "NetworkClient.hpp"
#include "NetworkClientConnection.hpp"
#include "NetworkUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"

void clienthandlevoteon(NetworkContext *netmessage)
{
  tmsg_voteon *voteonmsg;
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_voteon), netmessage->size, msgid_voteon))
  {
    return;
  }

  voteonmsg = pmsg_voteon(netmessage->packet);

  i = voteonmsg->who;

  statsmenushow = false;

  GS::GetGame().startvote(i, voteonmsg->votetype, voteonmsg->targetname.data(),
                          voteonmsg->reason.data());
}

void clienthandlevoteoff()
{
  GS::GetGame().stopvote();
}

void clienthandleserversyncmsg(NetworkContext *netmessage)
{
  tmsg_serversyncmsg *syncmsg;

  if (!verifypacket(sizeof(tmsg_serversyncmsg), netmessage->size, msgid_serversyncmsg))
  {
    return;
  }

  syncmsg = pmsg_serversyncmsg(netmessage->packet);

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

void clienthandleforceposition(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_forceposition *forceposition;

  if (!verifypacket(sizeof(tmsg_forceposition), netmessage->size, msgid_forceposition))
  {
    return;
  }

  forceposition = pmsg_forceposition(netmessage->packet);

  auto &spritePartsPos = sprite_system.GetSpritePartsPos(forceposition->playerid);

  spritePartsPos = forceposition->pos;
  sprite_system.SetSpritePartsOldPos(forceposition->playerid, spritePartsPos);
}

void clienthandleforcevelocity(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_forcevelocity *forcevelocity;

  if (!verifypacket(sizeof(tmsg_forcevelocity), netmessage->size, msgid_forcevelocity))
  {
    return;
  }

  forcevelocity = pmsg_forcevelocity(netmessage->packet);

  auto &spriteVelocity = sprite_system.GetVelocity(forcevelocity->playerid);
  spriteVelocity = forcevelocity->vel;
}

void clienthandleforceweapon(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_forceweapon *forceweapon;

  if (!verifypacket(sizeof(tmsg_forceweapon), netmessage->size, msgid_forceweapon))
  {
    return;
  }

  forceweapon = pmsg_forceweapon(netmessage->packet);

  if (sprite_system.IsPlayerSpriteValid())
  {
    sprite_system.GetPlayerSprite().applyweaponbynum(forceweapon->weaponnum, 1);
    sprite_system.GetPlayerSprite().applyweaponbynum(forceweapon->secondaryweaponnum, 2);
    sprite_system.GetPlayerSprite().weapon.ammocount = forceweapon->ammocount;
    sprite_system.GetPlayerSprite().secondaryweapon.ammocount = forceweapon->secammocount;
  }
}

void clienthandleweaponactivemessage(NetworkContext *netmessage)
{
  tmsg_weaponactivemessage *wactivemessage;
  std::int32_t i;

  if (!verifypacket(sizeof(tmsg_weaponactivemessage), netmessage->size, msgid_weaponactivemessage))
  {
    return;
  }

  wactivemessage = pmsg_weaponactivemessage(netmessage->packet);
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

void clienthandleclientfreecam(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_clientfreecam *freecammsg;

  if (!verifypacket(sizeof(tmsg_clientfreecam), netmessage->size, msgid_clientfreecam))
  {
    return;
  }

  freecammsg = pmsg_clientfreecam(netmessage->packet);

  if (sprite_system.IsPlayerSpriteValid())
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
void clienthandlejoinserver(NetworkContext *netmessage)
{
  tmsg_joinserver *joinservermsg;

  if (!verifypacket(sizeof(tmsg_joinserver), netmessage->size, msgid_joinserver))
  {
    return;
  }

  joinservermsg = pmsg_joinserver(netmessage->packet);
  NotImplemented("network");
#if 0
    GS::GetMainConsole().console(std::string("Redirecting to... ") +
                            netaddrtostr(in_addr(joinservermsg->ip)) + ':' +
                            inttostr(joinservermsg->port),
                        server_message_color);
#endif

  clientdisconnect(*GetNetwork());

  redirecttoserver = true;
  NotImplemented("network");
#if 0
    redirectip = netaddrtostr(in_addr(joinservermsg->ip));
#endif
  redirectport = joinservermsg->port;
  redirectmsg = trim((pmsg_joinserver(netmessage->packet)->showmsg.data()));

  exittomenu();
}

void clienthandleplaysound(NetworkContext *netmessage)
{
  tmsg_playsound *playsoundmsg;
  auto& fs = GS::GetFileSystem();

  if (!verifypacket(sizeof(tmsg_playsound), netmessage->size, msgid_playsound))
  {
    return;
  }

  playsoundmsg = pmsg_playsound(netmessage->packet);

  if (fs.Exists(std::string(moddir) + std::string("sfx/") + playsoundmsg->name.data()))
  {
    // Name to ID, for easy use for scripters
    [[maybe_unused]] std::int32_t i = soundnametoid(playsoundmsg->name.data());
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
