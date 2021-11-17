// automatically converted
#include "NetworkServerThing.hpp"
#include "../../server/Server.hpp"
#include "../Calc.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "NetworkUtils.hpp"

#ifdef SERVER
void serverthingsnapshot(std::uint8_t tonum)
{
    tmsg_serverthingsnapshot thingmsg;
    std::int32_t i, j;
    bool send;

    for (i = 1; i <= max_things; i++)
        if ((thing[i].active) && (thing[i].style != object_parachute) &&
            ((!thing[i].statictype) or
             ((thing[i].style < object_ussocom) || (thing[i].style == object_stationary_gun))) &&
            ((pointvisible(thing[i].skeleton.pos[1].x, thing[i].skeleton.pos[1].y, tonum)) or
             (thing[i].style < object_ussocom)))
        {
            thingmsg.header.id = msgid_serverthingsnapshot;
            // assign thing values to ThingMsg
            thingmsg.num = i;
            for (j = 1; j <= 4; j++)
            {
                [[deprecated("indexing")]] auto jminus1 = j - 1;
                thingmsg.pos[jminus1].x = thing[i].skeleton.pos[j].x;
                thingmsg.pos[jminus1].y = thing[i].skeleton.pos[j].y;
                thingmsg.oldpos[jminus1].x = thing[i].skeleton.oldpos[j].x;
                thingmsg.oldpos[jminus1].y = thing[i].skeleton.oldpos[j].y;
            }

            thingmsg.owner = thing[i].owner;
            thingmsg.style = thing[i].style;
            thingmsg.holdingsprite = thing[i].holdingsprite;

            // send only if moving
            send = false;
            if ((distance(thingmsg.pos[0].x, thingmsg.pos[0].y, thingmsg.oldpos[0].x,
                          thingmsg.oldpos[0].y) > minmovedelta) ||
                (distance(thingmsg.pos[1].x, thingmsg.pos[1].y, thingmsg.oldpos[1].x,
                          thingmsg.oldpos[1].y) > minmovedelta))
                send = true;

            if (maintickcounter % 2 == 0)
            {
                if (thing[i].style < object_ussocom)
                    send = true;
                if (thing[i].style == object_rambo_bow)
                    send = true;
            }

            if (send)
            {
                GetNetwork()->senddata(&thingmsg, sizeof(thingmsg), sprite[tonum].player->peer,
                                       k_nSteamNetworkingSend_Unreliable);
            }
        }
}

void serverthingmustsnapshot(std::uint8_t i)
{
    tmsg_serverthingmustsnapshot thingmsg;
    std::int32_t j;

    if (thing[i].style == object_parachute)
        return;

    thingmsg.header.id = msgid_serverthingmustsnapshot;
    // assign thing values to ThingMsg
    thingmsg.num = i;
    for (j = 1; j <= 4; j++)
    {
        thingmsg.pos[j - 1].x = thing[i].skeleton.pos[j].x;
        thingmsg.pos[j - 1].y = thing[i].skeleton.pos[j].y;
        thingmsg.oldpos[j - 1].x = thing[i].skeleton.oldpos[j].x;
        thingmsg.oldpos[j - 1].y = thing[i].skeleton.oldpos[j].y;
    }
    thingmsg.timeout = thing[i].timeout;
    if (thing[i].timeout < 1)
        thingmsg.timeout = 1;
    thingmsg.owner = thing[i].owner;
    thingmsg.style = thing[i].style;
    thingmsg.holdingsprite = thing[i].holdingsprite;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
            GetNetwork()->senddata(&thingmsg, sizeof(thingmsg), sprite[i].player->peer,
                                   k_nSteamNetworkingSend_Unreliable);
}
#endif

void serverthingmustsnapshotonconnect(std::uint8_t tonum)
{
    tmsg_serverthingmustsnapshot thingmsg;
    std::int32_t i, j;

    for (i = 1; i <= max_things; i++)
        if (thing[i].active)
            if (((thing[i].style < object_ussocom) || (thing[i].style > object_minigun)) &&
                (thing[i].style != object_parachute))
            {
                thingmsg.header.id = msgid_serverthingmustsnapshot;
                // assign thing values to ThingMsg
                thingmsg.num = i;
                for (j = 1; j <= 4; j++)
                {
                    thingmsg.pos[j - 1].x = thing[i].skeleton.pos[j].x;
                    thingmsg.pos[j - 1].y = thing[i].skeleton.pos[j].y;
                    thingmsg.oldpos[j - 1].x = thing[i].skeleton.oldpos[j].x;
                    thingmsg.oldpos[j - 1].y = thing[i].skeleton.oldpos[j].y;
                }
                thingmsg.timeout = std::int16_t(thing[i].timeout);
                if (thing[i].timeout < 1)
                    thingmsg.timeout = 1;
                thingmsg.owner = thing[i].owner;
                thingmsg.style = thing[i].style;
                thingmsg.holdingsprite = thing[i].holdingsprite;

#ifdef SERVER
                GetNetwork()->senddata(&thingmsg, sizeof(thingmsg), sprite[tonum].player->peer,
                                       k_nSteamNetworkingSend_Unreliable);
#else
                demorecorder.saverecord(thingmsg, sizeof(thingmsg));
#endif
            }
}

#ifdef SERVER
void serverthingmustsnapshotonconnectto(std::uint8_t i, std::uint8_t tonum)
{
    tmsg_serverthingmustsnapshot thingmsg;
    std::int32_t j;

    if (thing[i].style == object_parachute)
        return;

    thingmsg.header.id = msgid_serverthingmustsnapshot;
    // assign thing values to ThingMsg
    thingmsg.num = i;
    for (j = 1; j <= 4; j++)
    {
        thingmsg.pos[j].x = thing[i].skeleton.pos[j].x;
        thingmsg.pos[j].y = thing[i].skeleton.pos[j].y;
        thingmsg.oldpos[j].x = thing[i].skeleton.oldpos[j].x;
        thingmsg.oldpos[j].y = thing[i].skeleton.oldpos[j].y;
    }
    thingmsg.timeout = thing[i].timeout;
    if (thing[i].timeout < 1)
        thingmsg.timeout = 1;
    thingmsg.owner = thing[i].owner;
    thingmsg.style = thing[i].style;
    thingmsg.holdingsprite = thing[i].holdingsprite;

    GetNetwork()->senddata(&thingmsg, sizeof(thingmsg), sprite[tonum].player->peer,
                           k_nSteamNetworkingSend_Unreliable);
}

void serverthingtaken(std::uint8_t i, std::uint8_t w)
{
    tmsg_serverthingtaken thingmsg;

    thingmsg.header.id = msgid_thingtaken;
    thingmsg.num = thing[i].num;
    thingmsg.who = w;

    thingmsg.style = thing[i].style;
    thingmsg.ammocount = thing[i].ammocount;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
            GetNetwork()->senddata(&thingmsg, sizeof(thingmsg), sprite[i].player->peer,
                                   k_nSteamNetworkingSend_Unreliable);
}

void serverhandlerequestthing(SteamNetworkingMessage_t *netmessage)
{
    pmsg_requestthing msg;
    tplayer *player;

    if (!verifypacket(sizeof(tmsg_requestthing), netmessage->m_cbSize, msgid_requestthing))
        return;
    msg = pmsg_requestthing(netmessage->m_pData);
    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    serverthingmustsnapshotonconnectto(msg->thingid, player->spritenum);
}
#endif
