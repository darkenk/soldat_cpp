// automatically converted

#include "NetworkServerHeartbeat.hpp"

#include "../../server/Server.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include <steam/isteamnetworkingmessages.h>

// HEARTBEAT
void serverheartbeat()
{
    tmsg_heartbeat heartbeatmsg;
    std::int32_t j, c;

    heartbeatmsg.header.id = msgid_heartbeat;

    c = 0;

    for (j = 1; j <= max_players; j++)
    {
        heartbeatmsg.active[j] = false;
        heartbeatmsg.kills[j] = 0;
        heartbeatmsg.caps[j] = 0;
        heartbeatmsg.team[j] = 0;
        heartbeatmsg.deaths[j] = 0;
        heartbeatmsg.flags[j] = 0;
        heartbeatmsg.ping[j] = 255;
    }

    for (j = 1; j <= max_players; j++)
        if (sprite[j].active)
        {
            c += 1;
            heartbeatmsg.active[c] = sprite[j].active;
            heartbeatmsg.kills[c] = sprite[j].player->kills;
            heartbeatmsg.caps[c] = sprite[j].player->flags;
            heartbeatmsg.deaths[c] = sprite[j].player->deaths;
            heartbeatmsg.team[c] = sprite[j].player->team;
            heartbeatmsg.flags[c] = sprite[j].player->flags;
            heartbeatmsg.ping[c] = sprite[j].player->pingticks;
            heartbeatmsg.realping[c] = sprite[j].player->realping;
            heartbeatmsg.connectionquality[c] = sprite[j].player->connectionquality;
        }

    for (j = team_alpha; j <= team_delta; j++)
        heartbeatmsg.teamscore[j] = teamscore[j];

    heartbeatmsg.mapid = map.mapid;
    if (mapchangecounter > 0)
        heartbeatmsg.mapid = 0;
    if ((CVar::sv_timelimit - timelimitcounter) < 600)
        heartbeatmsg.mapid = 0;
    if ((timelimitcounter) < 600)
        heartbeatmsg.mapid = 0;

    for (j = 1; j <= max_players; j++)
        if ((sprite[j].active) && (sprite[j].player->controlmethod == human))
            udp->senddata(&heartbeatmsg, sizeof(heartbeatmsg), sprite[j].player->peer,
                          k_nSteamNetworkingSend_Unreliable);
}
