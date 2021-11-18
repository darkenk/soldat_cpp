// automatically converted

#include "NetworkServerHeartbeat.hpp"

#include "../../server/Server.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include <steam/isteamnetworkingmessages.h>

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

// HEARTBEAT
void serverheartbeat()
{
    tmsg_heartbeat heartbeatmsg;
    heartbeatmsg.header.id = msgid_heartbeat;

    std::fill(std::begin(heartbeatmsg.active), std::end(heartbeatmsg.active), false);
    std::fill(std::begin(heartbeatmsg.kills), std::end(heartbeatmsg.kills), 0);
    std::fill(std::begin(heartbeatmsg.caps), std::end(heartbeatmsg.caps), 0);
    std::fill(std::begin(heartbeatmsg.team), std::end(heartbeatmsg.team), 0);
    std::fill(std::begin(heartbeatmsg.deaths), std::end(heartbeatmsg.deaths), 0);
    std::fill(std::begin(heartbeatmsg.flags), std::end(heartbeatmsg.flags), 0);
    std::fill(std::begin(heartbeatmsg.ping), std::end(heartbeatmsg.ping), 255);

    auto c = 0;
    for (auto &s : sprite)
    {
        if (s.active)
        {
            heartbeatmsg.active[c] = s.active;
            heartbeatmsg.kills[c] = s.player->kills;
            heartbeatmsg.caps[c] = s.player->flags;
            heartbeatmsg.deaths[c] = s.player->deaths;
            heartbeatmsg.team[c] = s.player->team;
            heartbeatmsg.flags[c] = s.player->flags;
            heartbeatmsg.ping[c] = s.player->pingticks;
            heartbeatmsg.realping[c] = s.player->realping;
            heartbeatmsg.connectionquality[c] = s.player->connectionquality;
            c++;
        }
    }

    for (auto j = team_alpha; j <= team_delta; j++)
    {
        [[deprecated("indexing")]] auto jminus1 = j - 1;
        heartbeatmsg.teamscore[jminus1] = teamscore[j];
    }

    heartbeatmsg.mapid = map.mapid;
    if (mapchangecounter > 0)
        heartbeatmsg.mapid = 0;
    if ((CVar::sv_timelimit - timelimitcounter) < 600)
        heartbeatmsg.mapid = 0;
    if ((timelimitcounter) < 600)
        heartbeatmsg.mapid = 0;

    for (auto &s : sprite)
    {
        if ((s.active) && (s.player->controlmethod == human))
        {
            GetServerNetwork()->senddata(&heartbeatmsg, sizeof(heartbeatmsg), s.player->peer,
                                   k_nSteamNetworkingSend_Unreliable);
        }
    }
}
