// automatically converted
#include "NetworkServerThing.hpp"
#include "../../server/Server.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "NetworkUtils.hpp"
#include "common/Calc.hpp"
#include "shared/mechanics/SpriteSystem.hpp"

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

#ifdef SERVER
void serverthingsnapshot(std::uint8_t tonum)
{
    tmsg_serverthingsnapshot thingmsg;
    std::int32_t i, j;
    bool send;

    for (i = 1; i <= max_things; i++)
        if ((things[i].active) && (things[i].style != object_parachute) &&
            ((!things[i].statictype) or
             ((things[i].style < object_ussocom) || (things[i].style == object_stationary_gun))) &&
            ((pointvisible(things[i].skeleton.pos[1].x, things[i].skeleton.pos[1].y, tonum)) or
             (things[i].style < object_ussocom)))
        {
            thingmsg.header.id = msgid_serverthingsnapshot;
            // assign thing values to ThingMsg
            thingmsg.num = i;
            for (j = 1; j <= 4; j++)
            {
                [[deprecated("indexing")]] auto jminus1 = j - 1;
                thingmsg.pos[jminus1].x = things[i].skeleton.pos[j].x;
                thingmsg.pos[jminus1].y = things[i].skeleton.pos[j].y;
                thingmsg.oldpos[jminus1].x = things[i].skeleton.oldpos[j].x;
                thingmsg.oldpos[jminus1].y = things[i].skeleton.oldpos[j].y;
            }

            thingmsg.owner = things[i].owner;
            thingmsg.style = things[i].style;
            thingmsg.holdingsprite = things[i].holdingsprite;

            // send only if moving
            send = false;
            if ((distance(thingmsg.pos[0].x, thingmsg.pos[0].y, thingmsg.oldpos[0].x,
                          thingmsg.oldpos[0].y) > minmovedelta) ||
                (distance(thingmsg.pos[1].x, thingmsg.pos[1].y, thingmsg.oldpos[1].x,
                          thingmsg.oldpos[1].y) > minmovedelta))
                send = true;

            if (maintickcounter % 2 == 0)
            {
                if (things[i].style < object_ussocom)
                    send = true;
                if (things[i].style == object_rambo_bow)
                    send = true;
            }

            if (send)
            {
                GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg),
                                             SpriteSystem::Get().GetSprite(tonum).player->peer,
                                             k_nSteamNetworkingSend_Unreliable);
            }
        }
}

void serverthingmustsnapshot(std::uint8_t i)
{
    tmsg_serverthingmustsnapshot thingmsg;
    std::int32_t j;

    if (things[i].style == object_parachute)
        return;

    thingmsg.header.id = msgid_serverthingmustsnapshot;
    // assign thing values to ThingMsg
    thingmsg.num = i;
    for (j = 1; j <= 4; j++)
    {
        thingmsg.pos[j - 1].x = things[i].skeleton.pos[j].x;
        thingmsg.pos[j - 1].y = things[i].skeleton.pos[j].y;
        thingmsg.oldpos[j - 1].x = things[i].skeleton.oldpos[j].x;
        thingmsg.oldpos[j - 1].y = things[i].skeleton.oldpos[j].y;
    }
    thingmsg.timeout = things[i].timeout;
    if (things[i].timeout < 1)
        thingmsg.timeout = 1;
    thingmsg.owner = things[i].owner;
    thingmsg.style = things[i].style;
    thingmsg.holdingsprite = things[i].holdingsprite;

    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
        if (sprite.player->controlmethod == human)
        {
            GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg), sprite.player->peer,
                                         k_nSteamNetworkingSend_Unreliable);
        }
    }
}
#endif

void serverthingmustsnapshotonconnect(std::uint8_t tonum)
{
    tmsg_serverthingmustsnapshot thingmsg;
    std::int32_t i, j;

    for (i = 1; i <= max_things; i++)
        if (things[i].active)
            if (((things[i].style < object_ussocom) || (things[i].style > object_minigun)) &&
                (things[i].style != object_parachute))
            {
                thingmsg.header.id = msgid_serverthingmustsnapshot;
                // assign thing values to ThingMsg
                thingmsg.num = i;
                for (j = 1; j <= 4; j++)
                {
                    thingmsg.pos[j - 1].x = things[i].skeleton.pos[j].x;
                    thingmsg.pos[j - 1].y = things[i].skeleton.pos[j].y;
                    thingmsg.oldpos[j - 1].x = things[i].skeleton.oldpos[j].x;
                    thingmsg.oldpos[j - 1].y = things[i].skeleton.oldpos[j].y;
                }
                thingmsg.timeout = std::int16_t(things[i].timeout);
                if (things[i].timeout < 1)
                    thingmsg.timeout = 1;
                thingmsg.owner = things[i].owner;
                thingmsg.style = things[i].style;
                thingmsg.holdingsprite = things[i].holdingsprite;

#ifdef SERVER
                GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg),
                                             SpriteSystem::Get().GetSprite(tonum).player->peer,
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

    if (things[i].style == object_parachute)
        return;

    thingmsg.header.id = msgid_serverthingmustsnapshot;
    // assign thing values to ThingMsg
    thingmsg.num = i;
    for (j = 1; j <= 4; j++)
    {
        thingmsg.pos[j].x = things[i].skeleton.pos[j].x;
        thingmsg.pos[j].y = things[i].skeleton.pos[j].y;
        thingmsg.oldpos[j].x = things[i].skeleton.oldpos[j].x;
        thingmsg.oldpos[j].y = things[i].skeleton.oldpos[j].y;
    }
    thingmsg.timeout = things[i].timeout;
    if (things[i].timeout < 1)
        thingmsg.timeout = 1;
    thingmsg.owner = things[i].owner;
    thingmsg.style = things[i].style;
    thingmsg.holdingsprite = things[i].holdingsprite;

    GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg),
                                 SpriteSystem::Get().GetSprite(tonum).player->peer,
                                 k_nSteamNetworkingSend_Unreliable);
}

void serverthingtaken(std::uint8_t i, std::uint8_t w)
{
    tmsg_serverthingtaken thingmsg;

    thingmsg.header.id = msgid_thingtaken;
    thingmsg.num = things[i].num;
    thingmsg.who = w;

    thingmsg.style = things[i].style;
    thingmsg.ammocount = things[i].ammocount;

    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
        if (sprite.player->controlmethod == human)
        {
            GetServerNetwork()->senddata(&thingmsg, sizeof(thingmsg), sprite.player->peer,
                                         k_nSteamNetworkingSend_Unreliable);
        }
    }
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
