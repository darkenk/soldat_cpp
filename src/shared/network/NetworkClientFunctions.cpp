// automatically converted
#include "NetworkClientFunctions.hpp"

#include "../../client/Client.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/InterfaceGraphics.hpp"
#include "../../client/Sound.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../misc/PortUtilsSoldat.hpp"
#include "NetworkClientConnection.hpp"
#include "NetworkUtils.hpp"
#include <physfs.h>

void clienthandlevoteon(SteamNetworkingMessage_t *netmessage)
{
    tmsg_voteon *voteonmsg;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_voteon), netmessage->m_cbSize, msgid_voteon))
        return;

    voteonmsg = pmsg_voteon(netmessage->m_pData);

    i = voteonmsg->who;

    statsmenushow = false;

    startvote(i, voteonmsg->votetype, voteonmsg->targetname.data(), voteonmsg->reason.data());
}

void clienthandlevoteoff()
{
    stopvote();
}

void clienthandleserversyncmsg(SteamNetworkingMessage_t *netmessage)
{
    tmsg_serversyncmsg *syncmsg;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_serversyncmsg), netmessage->m_cbSize, msgid_serversyncmsg))
        return;

    syncmsg = pmsg_serversyncmsg(netmessage->m_pData);

    timelimitcounter = syncmsg->time;
    if (syncmsg->pause == 1)
    {
        mapchangecounter = 999999999;
        mapchangename = "PAUSE*!*";
        for (i = 1; i <= max_players; i++)
            if (sprite[i].active)
            {
                stopsound(sprite[i].reloadsoundchannel);
                stopsound(sprite[i].jetssoundchannel);
                stopsound(sprite[i].gattlingsoundchannel);
                stopsound(sprite[i].gattlingsoundchannel2);
            }
    }
    else if (mapchangecounter == 999999999)
        mapchangecounter = -60;
}

void clienthandleforceposition(SteamNetworkingMessage_t *netmessage)
{
    tmsg_forceposition *forceposition;

    if (!verifypacket(sizeof(tmsg_forceposition), netmessage->m_cbSize, msgid_forceposition))
        return;

    forceposition = pmsg_forceposition(netmessage->m_pData);

    spriteparts.pos[forceposition->playerid] = forceposition->pos;
    spriteparts.oldpos[forceposition->playerid] = spriteparts.pos[forceposition->playerid];
}

void clienthandleforcevelocity(SteamNetworkingMessage_t *netmessage)
{
    tmsg_forcevelocity *forcevelocity;

    if (!verifypacket(sizeof(tmsg_forcevelocity), netmessage->m_cbSize, msgid_forcevelocity))
        return;

    forcevelocity = pmsg_forcevelocity(netmessage->m_pData);

    spriteparts.velocity[forcevelocity->playerid] = forcevelocity->vel;
}

void clienthandleforceweapon(SteamNetworkingMessage_t *netmessage)
{
    tmsg_forceweapon *forceweapon;

    if (!verifypacket(sizeof(tmsg_forceweapon), netmessage->m_cbSize, msgid_forceweapon))
        return;

    forceweapon = pmsg_forceweapon(netmessage->m_pData);

    if (mysprite > 0)
    {
        sprite[mysprite].applyweaponbynum(forceweapon->weaponnum, 1);
        sprite[mysprite].applyweaponbynum(forceweapon->secondaryweaponnum, 2);
        sprite[mysprite].weapon.ammocount = forceweapon->ammocount;
        sprite[mysprite].secondaryweapon.ammocount = forceweapon->secammocount;
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

    if ((wactivemessage->weapon > 0) && (wactivemessage->weapon <= main_weapons))
    {
        weaponsel[mysprite][wactivemessage->weapon] = wactivemessage->active;

        for (i = 1; i <= main_weapons; i++)
            if (weaponactive[i] == 1)
                limbomenu->button[i - 1].active = (bool)(weaponsel[mysprite][i]);
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
    NotImplemented(NITag::NETWORK);
#if 0
    mainconsole.console(std::string("Redirecting to... ") +
                            netaddrtostr(in_addr(joinservermsg->ip)) + ':' +
                            inttostr(joinservermsg->port),
                        server_message_color);
#endif

    clientdisconnect();

    redirecttoserver = true;
    NotImplemented(NITag::NETWORK);
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

    if (!verifypacket(sizeof(tmsg_playsound), netmessage->m_cbSize, msgid_playsound))
        return;

    playsoundmsg = pmsg_playsound(netmessage->m_pData);

    if (PHYSFS_exists(
            (pchar)(std::string(moddir) + std::string("sfx/") + playsoundmsg->name.data())))
    {
        // Name to ID, for easy use for scripters
        i = soundnametoid(playsoundmsg->name.data());
        NotImplemented(NITag::NETWORK);
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
#endif
        if (playsoundmsg->emitter.x == 0)
            playsoundmsg->emitter.x = spriteparts.pos[mysprite].x;
        if (playsoundmsg->emitter.y == 0)
            playsoundmsg->emitter.y = spriteparts.pos[mysprite].y;
        playsound(i, playsoundmsg->emitter);
    }
}
