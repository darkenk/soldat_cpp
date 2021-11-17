// automatically converted
#include "NetworkServerGame.hpp"
#include "../../server/Server.hpp"
#include "../../server/ServerHelper.hpp"
#include "../Game.hpp"
#include "../Util.hpp"
#include "NetworkServerConnection.hpp"
#include "NetworkServerMessages.hpp"
#include "NetworkUtils.hpp"

#include <string>

void serverhandleplayerdisconnect(SteamNetworkingMessage_t *netmessage)
{
    tmsg_playerdisconnect *playermsg;
    tplayer *player;
    std::int32_t i, j;

    if (!verifypacket(sizeof(tmsg_playerdisconnect), netmessage->m_cbSize, msgid_playerdisconnect))
        return;

    playermsg = pmsg_playerdisconnect(netmessage->m_pData);
    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    i = player->spritenum;

    for (j = 0; j <= sprite[i].bulletcheckamount; j++)
        sprite[i].bulletcheck[j] = 0;

    sprite[i].bulletcheckindex = 0;
    sprite[i].bulletcheckamount = 0;

    messagesasecnum[playermsg->num] += 1;

    if ((voteactive) && (votetype == vote_kick))
        if (strtoint(votetarget) == i)
        {
            kickplayer(i, true, kick_voted, five_minutes, "Vote Kicked (Left game)");
            stopvote();
            return;
        }

    switch (sprite[i].player->team)
    {
    case team_none:
        GetServerMainConsole().console(sprite[i].player->name + " has left the game.",
                                       enter_message_color);
        break;
    case team_alpha:
        GetServerMainConsole().console(sprite[i].player->name + " has left alpha team.",
                                       alphaj_message_color);
        break;
    case team_bravo:
        GetServerMainConsole().console(sprite[i].player->name + " has left bravo team.",
                                       bravoj_message_color);
        break;
    case team_charlie:
        GetServerMainConsole().console(sprite[i].player->name + " has left charlie team.",
                                       charliej_message_color);
        break;
    case team_delta:
        GetServerMainConsole().console(sprite[i].player->name + " has left delta team.",
                                       deltaj_message_color);
        break;
    case team_spectator:
        GetServerMainConsole().console(sprite[i].player->name + " has left spectators",
                                       deltaj_message_color);
        break;
    }

    for (j = 1; j <= max_players; j++)
        if ((trim(tklist[j]) == "") || (tklist[j] == sprite[i].player->ip))
        {
            tklistkills[j] = sprite[i].player->tkwarnings;
            tklist[j] = sprite[i].player->ip;
            break;
        }

    serverplayerdisconnect(i, kick_leftgame);

    if (sprite[i].isnotspectator())
        sprite[i].dropweapon();
    sprite[i].player->muted = 0;
    sprite[i].player->tkwarnings = 0;

#ifdef SCRIPT
    scrptdispatcher.onleavegame(sprite[i].num, false);
#endif

    // NOTE: no disconnect event is generated by disconnect_now, hence we destroy Player here
    sprite[i].kill();
    sprite[i].player = &dummyplayer;
    NotImplemented(NITag::OTHER, "Check if &player is used properly to remove player");
    players.erase(std::remove(players.begin(), players.end(), player), players.end());

    GetNetwork()->NetworkingSocket().CloseConnection(netmessage->m_conn, 0, "", false);

    dobalancebots(1, sprite[i].player->team);
}

void servermapchange(std::uint8_t id)
{
    tmsg_mapchange mapchangemsg;
    std::int32_t i;
    tplayer dstplayer;

    mapchangemsg.header.id = msgid_mapchange;
    mapchangemsg.counter = mapchangecounter;
    strcpy(mapchangemsg.mapname.data(), mapchange.name.data());
    mapchecksum = getmapchecksum(mapchange);
    mapchangemsg.mapchecksum = mapchecksum;

    for (i = 1; i <= max_players; i++)
        if (sprite[i].active)
        {
            sprite[i].player->tkwarnings = 0;
            tklist[i] = "";
            tklistkills[i] = 0;
        }

    if (id == 0)
    {
        ;
        // NOTE we send to pending players too, otherwise there is a small window where they miss
        // the map change NOTE also that we're using the CONNECTION channel, which is required for
        // all packets that can be sent before a sprite is assigned to the player for proper
        // sequencing with encryption commands.
        //    for DstPlayer in Players do
        //      udp->senddata(&MapChangeMsg, sizeof(MapChangeMsg), DstPlayer.peer,
        //      k_nSteamNetworkingSend_Reliable);
    }
    else if ((sprite[id].active) && (sprite[id].player->controlmethod == human))
        GetNetwork()->senddata(&mapchangemsg, sizeof(mapchangemsg), sprite[id].player->peer,
                      k_nSteamNetworkingSend_Reliable);
}

void serverflaginfo(std::uint8_t style, std::uint8_t who)
{
    tmsg_serverflaginfo flagmsg;
    std::int32_t i;

    flagmsg.header.id = msgid_flaginfo;
    flagmsg.style = style;
    flagmsg.who = who;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
            GetNetwork()->senddata(&flagmsg, sizeof(flagmsg), sprite[i].player->peer,
                          k_nSteamNetworkingSend_Reliable);
}

void serveridleanimation(std::uint8_t num, std::int16_t style)
{
    tmsg_idleanimation idlemsg;
    std::int32_t i;

    idlemsg.header.id = msgid_idleanimation;
    idlemsg.num = num;
    idlemsg.idlerandom = style;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
            GetNetwork()->senddata(&idlemsg, sizeof(idlemsg), sprite[i].player->peer,
                          k_nSteamNetworkingSend_Reliable);
}

void serversendvoteon(std::uint8_t votestyle, std::int32_t voter, std::string targetname,
                      std::string reason)
{
    tmsg_voteon votemsg;
    std::int32_t i;

    votemsg.header.id = msgid_voteon;
    votemsg.votetype = votestyle;
    votemsg.timer = votetimeremaining;
    votemsg.who = voter;
    stringtoarray(votemsg.targetname.data(), targetname);
    stringtoarray(votemsg.reason.data(), reason);

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
            GetNetwork()->senddata(&votemsg, sizeof(votemsg), sprite[i].player->peer,
                          k_nSteamNetworkingSend_Reliable);
}

void serversendvoteoff()
{
    tmsg_voteoff votemsg;
    std::int32_t i;

    votemsg.header.id = msgid_voteoff;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
            GetNetwork()->senddata(&votemsg, sizeof(votemsg), sprite[i].player->peer,
                          k_nSteamNetworkingSend_Reliable);
}

void serverhandlevotekick(SteamNetworkingMessage_t *netmessage)
{
    tmsg_votekick *votekickmsg;
    tplayer *player;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_votekick), netmessage->m_cbSize, msgid_votekick))
        return;

    votekickmsg = pmsg_votekick(netmessage->m_pData);
    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    i = player->spritenum;

    if (voteactive)
    {
        // if a vote against a player is in progress,
        // don't allow that player to vote against himself.
        if (votetype != vote_kick)
            return;
        if (strtoint(votetarget) == i)
        {
            serversendstringmessage("A vote has been cast against you. You can not vote.", i, 255,
                                    msgtype_pub);
            return;
        }

        // check if he already voted
        if (votehasvoted[i])
            return;

        // check if the vote target is actually the target
        if (votetarget != inttostr(votekickmsg->num))
            return;

#ifdef SCRIPT
        scrptdispatcher.onvotekick(i, votekickmsg.num);
#endif
        countvote(i);
    }
    else
    {
        if (votecooldown[i] < 0)
        {
            // only allow valid votes
            if ((votekickmsg->num < 1) || (votekickmsg->num > max_players))
                return;
            if (sprite[i].player->muted == 1)
            {
                writeconsole(i, "You are muted. You can't cast a vote kick.", server_message_color);
                return;
            }

#ifdef SCRIPT
            if (scrptdispatcher.onvotekickstart(i, votekickmsg.num, string(votekickmsg.reason)))
                return;
#endif

            startvote(i, vote_kick, inttostr(votekickmsg->num), votekickmsg->reason.data());
            serversendvoteon(votetype, i, inttostr(votekickmsg->num), votekickmsg->reason.data());
            // Show started votekick in admin console
            GetServerMainConsole().console(
                sprite[i].player->name + " started votekick against " +
                    sprite[votekickmsg->num].player->name +
                    " - Reason:" + std::string(votekickmsg->reason.data()),
                vote_message_color);
        }
    }
}

void serverhandlevotemap(SteamNetworkingMessage_t *netmessage)
{
    tmsg_votemap *votemapmsg;
    tmsg_votemapreply votemapreplymsg;
    tplayer *player;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_votemap), netmessage->m_cbSize, msgid_votemap))
        return;

    votemapmsg = pmsg_votemap(netmessage->m_pData);
    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    i = player->spritenum;

    if (votemapmsg->mapid > mapslist.size() - 1)
        return;

    votemapreplymsg.header.id = msgid_votemapreply;
    votemapreplymsg.count = mapslist.size();
    strcpy(votemapreplymsg.mapname.data(), mapslist[votemapmsg->mapid].data());

    GetNetwork()->senddata(&votemapreplymsg, sizeof(votemapreplymsg), sprite[i].player->peer,
                  k_nSteamNetworkingSend_Reliable);
}

void serverhandlechangeteam(SteamNetworkingMessage_t *netmessage)
{
    tmsg_changeteam *changeteammsg;
    tplayer *player;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_changeteam), netmessage->m_cbSize, msgid_changeteam))
        return;
    changeteammsg = pmsg_changeteam(netmessage->m_pData);
    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    i = player->spritenum;
    sprite[i].changeteam(changeteammsg->team);
}

void serversyncmsg(std::int32_t tonum)
{
    tmsg_serversyncmsg syncmsg;
    std::int32_t i;

    syncmsg.header.id = msgid_serversyncmsg;
    syncmsg.time = timelimitcounter;
    if (mapchangecounter == 999999999)
        syncmsg.pause = 1;
    else
        syncmsg.pause = 0;

    for (i = 1; i <= max_players; i++)
        if ((tonum == 0) || (i == tonum))
            if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
                GetNetwork()->senddata(&syncmsg, sizeof(syncmsg), sprite[i].player->peer,
                              k_nSteamNetworkingSend_Reliable);
}

#ifdef STEAM
void serverhandlevoicedata(SteamNetworkingMessage_t *netmessage)
{
    pmsg_voicedata voicemsg;
    std::uint8_t i;
    tplayer player;

    if (!CVar::sv_voicechat)
        return;

    if (!verifypacketlargerorequal(sizeof(voicemsg), netmessage->m_cbSize, msgid_voicedata))
        return;

    player = tplayer(netmessage->m_nConnUserData);

    if (player.muted == 1)
        return;

    voicemsg = pmsg_voicedata(netmessage->m_pData);

    voicemsg.speaker = player.spritenum;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player.controlmethod == human))
            if ((player.spritenum != i) && (uint64(sprite[i].player.steamid) > 0))
                if (CVar::sv_voicechat_alltalk or sprite[player.spritenum].isinsameteam(sprite[i]))
                    udp->senddata(&voicemsg, netmessage->m_cbSize, sprite[i].player.peer,
                                  k_nsteamnetworkingsend_nodelay);
}
#endif
