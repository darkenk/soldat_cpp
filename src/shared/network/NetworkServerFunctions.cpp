// automatically converted

#include "NetworkServerFunctions.hpp"

#include "../../server/Server.hpp"
#include "../Game.hpp"
#include "NetworkUtils.hpp"

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

void serversendfreecam(std::uint8_t tonum, bool freecam, tvector2 pos)
{
    tmsg_clientfreecam freecammsg;

    freecammsg.header.id = msgid_clientfreecam;
    freecammsg.freecamon = (std::uint8_t)(freecam);
    freecammsg.targetpos = pos;

    GetServerNetwork()->senddata(&freecammsg, sizeof(freecammsg), sprite[tonum].player->peer,
                                 k_nSteamNetworkingSend_Reliable);
}

void setweaponactive(std::uint8_t id, std::uint8_t weaponnum, bool state)
{
    tmsg_weaponactivemessage wepmsg;
    std::int32_t i;

    wepmsg.header.id = msgid_weaponactivemessage;
    wepmsg.weapon = weaponnum;
    wepmsg.active = iif(state, 1, 0);

    if (id == 0)
    {
        for (i = 1; i <= max_players; i++)
            if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
                GetServerNetwork()->senddata(&wepmsg, sizeof(wepmsg), sprite[i].player->peer,
                                             k_nSteamNetworkingSend_Reliable);
    }
    else if ((sprite[id].active) && (sprite[id].player->controlmethod == human))
        GetServerNetwork()->senddata(&wepmsg, sizeof(wepmsg), sprite[id].player->peer,
                                     k_nSteamNetworkingSend_Reliable);
}

void forceweapon(std::uint8_t id, std::uint8_t primary, std::uint8_t secondary, std::uint8_t ammo,
                 std::uint8_t secammo)
{
    tmsg_forceweapon wepmsg;

    sprite[id].applyweaponbynum(primary, 1);
    sprite[id].applyweaponbynum(secondary, 2);

    if (ammo > guns[weaponnumtoindex(primary)].ammo)
        ammo = guns[weaponnumtoindex(primary)].ammo;

    if (secammo > guns[weaponnumtoindex(secondary)].ammo)
        secammo = guns[weaponnumtoindex(secondary)].ammo;

    sprite[id].weapon.ammo = ammo;
    sprite[id].secondaryweapon.ammo = secammo;

    if (sprite[id].player->controlmethod == human)
    {
        wepmsg.header.id = msgid_forceweapon;
        wepmsg.weaponnum = primary;
        wepmsg.secondaryweaponnum = secondary;
        wepmsg.ammocount = ammo;
        wepmsg.secammocount = secammo;

        GetServerNetwork()->senddata(&wepmsg, sizeof(wepmsg), sprite[id].player->peer,
                                     k_nSteamNetworkingSend_Reliable);
    }

#ifdef SCRIPT
    // In some places in code, where we apply weapons (sprie snapshot, object collisions),
    // we need to check whenever weapons are already changed, hence this ugly workaround
    // with bool variable.
    forceweaponcalled = true;
#endif
}

void moveplayer(std::uint8_t id, float x, float y)
{
    tmsg_forceposition movemsg;
    std::uint8_t i;

    if (!sprite[id].active)
        return;

    spriteparts.pos[id].x = x;
    spriteparts.pos[id].y = y;
    spriteparts.oldpos[id] = spriteparts.pos[id];

    movemsg.header.id = msgid_forceposition;

    movemsg.pos = spriteparts.pos[id];
    movemsg.playerid = id;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
            GetServerNetwork()->senddata(&movemsg, sizeof(movemsg), sprite[i].player->peer,
                                         k_nSteamNetworkingSend_Reliable);
}

void modifyplayervelocity(std::uint8_t id, float velx, float vely)
{
    tmsg_forcevelocity velmsg;
    std::uint8_t i;

    if (!sprite[id].active)
        return;

    spriteparts.velocity[id].x = velx;
    spriteparts.velocity[id].y = vely;

    velmsg.header.id = msgid_forcevelocity;

    velmsg.vel = spriteparts.velocity[id];
    velmsg.playerid = id;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
            GetServerNetwork()->senddata(&velmsg, sizeof(velmsg), sprite[i].player->peer,
                                         k_nSteamNetworkingSend_Reliable);
}

void forwardclient(std::uint8_t id, std::string targetip, std::int32_t targetport,
                   std::string showmsg)
{
    tmsg_joinserver joinservermsg;

    joinservermsg.header.id = msgid_joinserver;
    joinservermsg.port = targetport;
    NotImplemented(NITag::NETWORK, "missing strtonetaddr");
#if 0
    joinservermsg.ip = std::uint32_t(strtonetaddr(targetip));
#endif
    stringtoarray(joinservermsg.showmsg.data(), showmsg);

    if ((sprite[id].active) && (sprite[id].player->controlmethod == human))
        GetServerNetwork()->senddata(&joinservermsg, sizeof(joinservermsg), sprite[id].player->peer,
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
            if ((sprite[id].active) && (sprite[id].player->controlmethod == human))
                GetServerNetwork()->senddata(&playsoundmsg, sizeof(playsoundmsg),
                                             sprite[id].player->peer,
                                             k_nSteamNetworkingSend_Reliable);
    }
    else if ((sprite[id].active) && (sprite[id].player->controlmethod == human))
        GetServerNetwork()->senddata(&playsoundmsg, sizeof(playsoundmsg), sprite[id].player->peer,
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
    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    i = player->spritenum;
    sprite[i].targetx = freecammsg->targetpos.x;
    sprite[i].targety = freecammsg->targetpos.y;
}
