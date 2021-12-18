// automatically converted

#include "NetworkClientHeartbeat.hpp"
#include "../../client/Client.hpp"
#include "../../client/Sound.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../GameStrings.hpp"
#include "NetworkClientConnection.hpp"
#include "NetworkUtils.hpp"
#include "shared/mechanics/SpriteSystem.hpp"

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

void clienthandleheartbeat(SteamNetworkingMessage_t *netmessage)
{
    tmsg_heartbeat *heartbeat;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_heartbeat), netmessage->m_cbSize, msgid_heartbeat))
        return;

    heartbeat = pmsg_heartbeat(netmessage->m_pData);

    auto c = 0;
    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
        if (!sprite.player->demoplayer)
        {
            sprite.active = heartbeat->active[c];
            sprite.player->kills = heartbeat->kills[c];
            sprite.player->flags = heartbeat->caps[c];
            sprite.player->team = heartbeat->team[c];
            sprite.player->deaths = heartbeat->deaths[c];
            sprite.player->flags = heartbeat->flags[c];
            sprite.player->pingticks = heartbeat->ping[c];
            sprite.player->pingtime = sprite.player->pingticks * 1000 / 60;
            sprite.player->realping = heartbeat->realping[c];
            sprite.player->connectionquality = heartbeat->connectionquality[c];
            c++;
        }
    }

    // play bding sound
    if (CVar::sv_gamemode == gamestyle_inf)
        if (heartbeat->teamscore[team_bravo] > teamscore[team_bravo])
            if (heartbeat->teamscore[team_bravo] % 5 == 0)
                playsound(sfx_infilt_point);

    if (CVar::sv_gamemode == gamestyle_htf)
    {
        if (heartbeat->teamscore[team_alpha] > teamscore[team_alpha])
            if (heartbeat->teamscore[team_alpha] % 5 == 0)
                playsound(sfx_infilt_point);
        if (heartbeat->teamscore[team_bravo] > teamscore[team_bravo])
            if (heartbeat->teamscore[team_bravo] % 5 == 0)
                playsound(sfx_infilt_point);
    }

    for (i = team_alpha; i <= team_delta; i++)
        teamscore[i] = heartbeat->teamscore[i - 1];

    // MapID differs, map not changed
    if ((mapchangecounter < 0) && (heartbeat->mapid != 0) && (heartbeat->mapid != map.mapid) &&
        (!demoplayer.active()))
    {
        badmapidcount -= 1;
    }
    else
        badmapidcount = 2;

    if (badmapidcount < 1)
    {
        GetMainConsole().console(_("Wrong map version detected"), server_message_color);
        clientdisconnect();
        mapchangecounter = -60;
        return;
    }

    if (connection == INTERNET)
    {
        if ((maintickcounter - heartbeattime) > 350)
        {
            heartbeattimewarnings += 1;
        }
        else if (heartbeattimewarnings > 0)
        {
            heartbeattimewarnings -= 1;
        }
    }

    heartbeattime = maintickcounter;

    sortplayers();
}
