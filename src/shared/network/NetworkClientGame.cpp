// automatically converted
#include "NetworkClientGame.hpp"
#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/InterfaceGraphics.hpp"
#include "../../client/Sound.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../GameMenus.hpp"
#include "../GameStrings.hpp"
#include "../gfx.hpp"
#include "../mechanics/Sprites.hpp"
#include "../misc/PortUtilsSoldat.hpp"
#include "NetworkUtils.hpp"
#include <limits>

/*#include "Steam.h"*/
/*#include "SteamTypes.h"*/
/*#include "Client.h"*/
/*#include "NetworkUtils.h"*/
/*#include "Game.h"*/
/*#include "Demo.h"*/
/*#include "ClientGame.h"*/
/*#include "Sparks.h"*/
/*#include "GameMenus.h"*/
/*#include "InterfaceGraphics.h"*/
/*#include "Sha1.h"*/

void clienthandlenewplayer(SteamNetworkingMessage_t *netmessage)
{
    tmsg_newplayer *newplayermsg;
    tvector2 a, b;
    std::int32_t i, d;

    if (!verifypacket(sizeof(tmsg_newplayer), netmessage->m_cbSize, msgid_newplayer))
        return;

    newplayermsg = pmsg_newplayer(netmessage->m_pData);
    i = newplayermsg->num;
    if ((i < 1) || (i > max_sprites))
        return;

    b.x = 0;
    b.y = 0;
    auto &player = *sprite[i].player; // reuse object
    player.name = returnfixedplayername(newplayermsg->name.data());
    player.shirtcolor = newplayermsg->shirtcolor & 0xffffff;
    player.pantscolor = newplayermsg->pantscolor & 0xffffff;
    player.skincolor = newplayermsg->skincolor & 0xffffff;
    player.haircolor = newplayermsg->haircolor & 0xffffff;
    player.jetcolor = newplayermsg->jetcolor;
    player.team = newplayermsg->team;

#ifdef STEAM
    player.steamid = tsteamid(newplayermsg->steamid);
    player.steamfriend = steamapi.friends.hasfriend(player.steamid, ord(k_efriendflagimmediate));
#endif

    player.controlmethod = human;

    player.hairstyle = 0;
    if ((newplayermsg->look & B1) == B1)
        player.hairstyle = 1;
    if ((newplayermsg->look & B2) == B2)
        player.hairstyle = 2;
    if ((newplayermsg->look & B3) == B3)
        player.hairstyle = 3;
    if ((newplayermsg->look & B4) == B4)
        player.hairstyle = 4;

    player.headcap = 0;
    if ((newplayermsg->look & B5) == B5)
        player.headcap = GFX::GOSTEK_HELM;
    if ((newplayermsg->look & B6) == B6)
        player.headcap = GFX::GOSTEK_KAP;

    player.chain = 0;
    if ((newplayermsg->look & B7) == B7)
        player.chain = 1;
    if ((newplayermsg->look & B8) == B8)
        player.chain = 2;

    a = newplayermsg->pos;
    i = createsprite(a, b, 1, i, &player, false);
    d = 0;

    // The NewPlayer message doubles as confirmation that a player object was
    // allocated for the client. So far the following happened:
    // Client                        Server
    // Msg_RequestGame -->           (ServerHandleRequestGame) "Requesting game..." console message
    // ...                       <-- ... (ServerSyncCvars)
    // (ClientHandlePlayersList) <-- Msg_PlayersList (ServerSendPlayList)
    // Msg_PlayerInfo -->            (ServerHandlePlayerInfo) "...joining game..." console message
    //                               CreateSprite() -- note: also called when switching teams!
    // You are here!             <-- Msg_NewPlayer with AdoptSpriteID=1 (ServerSendNewPlayerInfo)
    if (newplayermsg->adoptspriteid == 1)
    {
        d = 1;
        mysprite = i;

        if (demoplayer.active())
            sprite[mysprite].player->demoplayer = true;

        // TODO wat?
        sprite[mysprite].bulletcount = Random(std::numeric_limits<std::uint16_t>::max());

        if (player.team == team_spectator)
        {
            camerafollowsprite = 0;
            camerafollowsprite = getcameratarget();
            gamemenushow(limbomenu, false);
        }
        else
            camerafollowsprite = mysprite;

        gamemenushow(teammenu, false);
        clientplayerreceived = true;
        clientplayerreceivedcounter = -1;
        badmapidcount = 2;
        heartbeattime = maintickcounter;
        heartbeattimewarnings = 0;

        CVar::r_zoom = 0.0; // Reset zoom

        if (mapchangecounter < 999999999)
            mapchangecounter = -60;
        fragsmenushow = false;
        statsmenushow = false;
    }

    spriteparts.oldpos[i] = newplayermsg->pos;
    spriteparts.pos[i] = newplayermsg->pos;

    sprite[i].respawn();

    if (d == 1)
    {
        sprite[i].weapon = guns[noweapon];
        sprite[i].secondaryweapon = guns[noweapon];

        if (mysprite > 0)
        {
            gamemenushow(limbomenu);
            newplayerweapon();
        }
    }

    if (newplayermsg->jointype != join_silent)
        switch (newplayermsg->team)
        {
        case team_none:
            GetMainConsole().console(wideformat(_("{} has joined the game"), (player.name)),
                                enter_message_color);
            break;
        case team_alpha:
            GetMainConsole().console(wideformat(_("{} has joined alpha team"), (player.name)),
                                alphaj_message_color);
            break;
        case team_bravo:
            GetMainConsole().console(wideformat(_("{} has joined bravo team"), (player.name)),
                                bravoj_message_color);
            break;
        case team_charlie:
            GetMainConsole().console(wideformat(_("{} has joined charlie team"), (player.name)),
                                charliej_message_color);
            break;
        case team_delta:
            GetMainConsole().console(wideformat(_("{} has joined delta team"), (player.name)),
                                deltaj_message_color);
            break;
        case team_spectator:
            GetMainConsole().console(wideformat(_("{} has joined as spectator"), (player.name)),
                                deltaj_message_color);
            break;
        }
}

void clientvotekick(std::uint8_t num, bool ban, std::string reason)
{
    tmsg_votekick votemsg;

    votemsg.header.id = msgid_votekick;
    votemsg.ban = (std::uint8_t)(ban);
    votemsg.num = num;
    stringtoarray(votemsg.reason.data(), reason);
    udp->senddata(&votemsg, sizeof(votemsg), k_nSteamNetworkingSend_Reliable);
}

void clientvotemap(std::uint32_t mapid)
{
    tmsg_votemap votemsg;

    votemsg.header.id = msgid_votemap;
    votemsg.mapid = mapid;
    udp->senddata(&votemsg, sizeof(votemsg), k_nSteamNetworkingSend_Reliable);
}

void clienthandlevoteresponse(SteamNetworkingMessage_t *netmessage)
{
    tmsg_votemapreply *votemsgreply;

    if (!verifypacket(sizeof(tmsg_votemapreply), netmessage->m_cbSize, msgid_votemapreply))
        return;

    votemsgreply = pmsg_votemapreply(netmessage->m_pData);
    votemapname = votemsgreply->mapname.data();
    votemapcount = votemsgreply->count;
}

void clientfreecamtarget()
{
    tmsg_clientfreecam freecammsg;

    freecammsg.header.id = msgid_clientfreecam;
    freecammsg.freecamon = 0;
    freecammsg.targetpos.x = camerax;
    freecammsg.targetpos.y = cameray;

    udp->senddata(&freecammsg, sizeof(freecammsg), k_nSteamNetworkingSend_Reliable);
}

void clienthandleplayerdisconnect(SteamNetworkingMessage_t *netmessage)
{
    tmsg_playerdisconnect *playermsg;

    if (!verifypacket(sizeof(tmsg_playerdisconnect), netmessage->m_cbSize, msgid_playerdisconnect))
        return;

    playermsg = pmsg_playerdisconnect(netmessage->m_pData);
    if ((playermsg->num < 1) || (playermsg->num > max_sprites))
        return;

    if ((playermsg->why == kick_unknown) || (playermsg->why >= _kick_end) ||
        (playermsg->why == kick_leftgame))
        switch (sprite[playermsg->num].player->team)
        {
        case 0:
            GetMainConsole().console(
                wideformat(_("{} has left the game"), (sprite[playermsg->num].player->name)),
                enter_message_color);
            break;
        case 1:
            GetMainConsole().console(
                wideformat(_("{} has left alpha team"), (sprite[playermsg->num].player->name)),
                alphaj_message_color);
            break;
        case 2:
            GetMainConsole().console(
                wideformat(_("{} has left bravo team"), (sprite[playermsg->num].player->name)),
                bravoj_message_color);
            break;
        case 3:
            GetMainConsole().console(
                wideformat(_("{} has left charlie team"), (sprite[playermsg->num].player->name)),
                charliej_message_color);
            break;
        case 4:
            GetMainConsole().console(
                wideformat(_("{} has left delta team"), (sprite[playermsg->num].player->name)),
                deltaj_message_color);
            break;
        case 5:
            GetMainConsole().console(
                wideformat(_("{} has left spectators"), (sprite[playermsg->num].player->name)),
                deltaj_message_color);
            break;
        }

    switch (playermsg->why)
    {
    case kick_noresponse:
        GetMainConsole().console(
            wideformat(_("{} has disconnected"), (sprite[playermsg->num].player->name)),
            client_message_color);
        break;
    case kick_nocheatresponse:
        GetMainConsole().console(
            wideformat(_("{} has been disconnected"), (sprite[playermsg->num].player->name)),
            client_message_color);
        break;
    case kick_changeteam:
        GetMainConsole().console(
            wideformat(_("{} is changing teams"), (sprite[playermsg->num].player->name)),
            client_message_color);
        break;
    case kick_ping:
        GetMainConsole().console(wideformat(_("{} has been ping kicked (for 15 minutes)"),
                                       (sprite[playermsg->num].player->name)),
                            client_message_color);
        break;
    case kick_flooding:
        GetMainConsole().console(wideformat(_("{} has been flood kicked (for 5 minutes)"),
                                       (sprite[playermsg->num].player->name)),
                            client_message_color);
        break;
    case kick_console:
        GetMainConsole().console(
            wideformat(_("{} has been kicked from console"), (sprite[playermsg->num].player->name)),
            client_message_color);
        break;
    case kick_connectcheat:
        GetMainConsole().console(wideformat(_("{} has been 'connect cheat' kicked"),
                                       (sprite[playermsg->num].player->name)),
                            client_message_color);
        break;
    case kick_cheat:
        GetMainConsole().console(wideformat(_("{} has been kicked for possible cheat"),
                                       (sprite[playermsg->num].player->name)),
                            client_message_color);
        break;
    case kick_voted:
        GetMainConsole().console(wideformat(_("{} has been voted to leave the game"),
                                       (sprite[playermsg->num].player->name)),
                            client_message_color);
        break;
    case kick_ac:
        GetMainConsole().console(wideformat(_("{} has been kicked for Anti-Cheat violation"),
                                       (sprite[playermsg->num].player->name)),
                            client_message_color);
        break;
    case kick_steamticket:
        GetMainConsole().console(wideformat(_("{} has been kicked for invalid Steam ticket"),
                                       (sprite[playermsg->num].player->name)),
                            client_message_color);
        break;
    }
    if (voteactive)
        switch (playermsg->why)
        {
        case kick_noresponse:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        case kick_nocheatresponse:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        case kick_ping:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        case kick_flooding:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        case kick_console:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        case kick_connectcheat:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        case kick_cheat:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        case kick_voted:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        case kick_ac:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        case kick_silent:
            if (votetarget == inttostr(playermsg->num))
                stopvote();
            break;
        }

    if (playermsg->why != kick_changeteam)
        sprite[playermsg->num].kill();

    sortplayers();

    if ((playermsg->num == mysprite) && (mapchangecounter < 1))
    {
        showmapchangescoreboard(0);
        gamemenushow(teammenu, false);
    }

    if ((playermsg->why != kick_changeteam) && (playermsg->why != kick_leftgame))
    {
        fragsmenushow = false;
        statsmenushow = false;
    }
}

void clienthandlemapchange(SteamNetworkingMessage_t *netmessage)
{
    tmsg_mapchange *mapchange;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_mapchange), netmessage->m_cbSize, msgid_mapchange))
        return;

    mapchange = pmsg_mapchange(netmessage->m_pData);

    mapchangename.resize(mapchange->mapnamelength, '0');
    std::copy(mapchange->mapname.begin(), mapchange->mapname.begin() + mapchange->mapnamelength,
              mapchangename.begin());
    mapchangecounter = mapchange->counter;
    mapchangechecksum = mapchange->mapchecksum;
    fragsmenushow = true;
    statsmenushow = false;
    gamemenushow(limbomenu, false);
    heartbeattime = maintickcounter;
    heartbeattimewarnings = 0;

    if (CVar::cl_endscreenshot)
        screentaken = true;

    for (i = 1; i <= max_players; i++)
        if (sprite[i].active)
        {
            stopsound(sprite[i].reloadsoundchannel);
            stopsound(sprite[i].jetssoundchannel);
            stopsound(sprite[i].gattlingsoundchannel);
            stopsound(sprite[i].gattlingsoundchannel2);
        }

    if (demoplayer.active())
    {
        showmapchangescoreboard(0);

        demoplayer.stopdemo();
        return;
    }

    GetMainConsole().console(_("Next map:") + ' ' + (mapchangename), game_message_color);

    if (!CVar::sv_survivalmode)
        if ((CVar::sv_gamemode == gamestyle_deathmatch) ||
            (CVar::sv_gamemode == gamestyle_pointmatch) || (CVar::sv_gamemode == gamestyle_rambo))
        {
            if (sortedplayers[1].playernum > 0)
                camerafollowsprite = sortedplayers[1].playernum;
            if (!escmenu->active)
            {
                mx = gamewidthhalf;
                my = gameheighthalf;
            }
        }
}

void clienthandleflaginfo(SteamNetworkingMessage_t *netmessage)
{
    std::int32_t j;
    tvector2 a, b;

    if (!verifypacket(sizeof(tmsg_serverflaginfo), netmessage->m_cbSize, msgid_flaginfo))
        return;

    if ((pmsg_serverflaginfo(netmessage->m_pData)->who < 1) ||
        (pmsg_serverflaginfo(netmessage->m_pData)->who > max_sprites))
        return;

    if (pmsg_serverflaginfo(netmessage->m_pData)->style == returnred)
        if (CVar::sv_gamemode == gamestyle_ctf)
        {
            playsound(sfx_capture);
            bigmessage(_("Red Flag returned!"), capturemessagewait, alpha_message_color);

            GetMainConsole().console(
                wideformat(_("{} returned the Red Flag"),
                           (sprite[pmsg_serverflaginfo(netmessage->m_pData)->who].player->name)),
                alpha_message_color);
            if (teamflag[1] > 0)
                thing[teamflag[1]].respawn();
        }
    if (pmsg_serverflaginfo(netmessage->m_pData)->style == returnblue)
        if (CVar::sv_gamemode == gamestyle_ctf)
        {
            playsound(sfx_capture);
            bigmessage(_("Blue Flag returned!"), capturemessagewait, alpha_message_color);

            GetMainConsole().console(
                wideformat(_("{} returned the Blue Flag"),
                           (sprite[pmsg_serverflaginfo(netmessage->m_pData)->who].player->name)),
                bravo_message_color);
            if (teamflag[2] > 0)
                thing[teamflag[2]].respawn();
        }
    if (pmsg_serverflaginfo(netmessage->m_pData)->style == capturered)
    {
        bigmessage(_("Alpha Team Scores!"), capturectfmessagewait, alpha_message_color);
        GetMainConsole().console(
            wideformat(_("{} scores for Alpha Team"),
                       (sprite[pmsg_serverflaginfo(netmessage->m_pData)->who].player->name)),
            alpha_message_color);

        if (CVar::sv_gamemode == gamestyle_inf)
        {
            playsound(sfx_infiltmus);

            // flame it
            for (j = 1; j <= 10; j++)
            {
                a.x = thing[teamflag[1]].skeleton.pos[2].x - 10 + Random(20);
                a.y = thing[teamflag[1]].skeleton.pos[2].y - 10 + Random(20);
                b.x = 0;
                b.y = 0;
                createspark(a, b, 36, 0, 35);
                if (Random(2) == 0)
                    createspark(a, b, 37, 0, 75);
            }
        }
        else
            playsound(sfx_ctf);
        if (teamflag[2] > 0)
            thing[teamflag[2]].respawn();

        // cap spark
        createspark(thing[teamflag[1]].skeleton.pos[2], b, 61,
                    pmsg_serverflaginfo(netmessage->m_pData)->who, 18);

        if (CVar::sv_survivalmode)
            survivalendround = true;
    }
    if (pmsg_serverflaginfo(netmessage->m_pData)->style == captureblue)
    {
        bigmessage(_("Bravo Team Scores!"), capturectfmessagewait, bravo_message_color);
        GetMainConsole().console(
            wideformat(_("{} scores for Bravo Team"),
                       (sprite[pmsg_serverflaginfo(netmessage->m_pData)->who].player->name)),
            bravo_message_color);
        playsound(sfx_ctf);
        if (teamflag[1] > 0)
            thing[teamflag[1]].respawn();

        // cap spark
        createspark(thing[teamflag[2]].skeleton.pos[2], b, 61,
                    pmsg_serverflaginfo(netmessage->m_pData)->who, 18);

        if (CVar::sv_survivalmode)
            survivalendround = true;
    }
}

void clienthandleidleanimation(SteamNetworkingMessage_t *netmessage)
{
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_idleanimation), netmessage->m_cbSize, msgid_idleanimation))
        return;

    i = pmsg_idleanimation(netmessage->m_pData)->num;

    if (!sprite[i].active)
        return;

    sprite[i].idletime = 1;
    sprite[i].idlerandom = pmsg_idleanimation(netmessage->m_pData)->idlerandom;
}

#ifdef STEAM
void clientsendvoicedata(pointer data, std::uint64_t datasize)
{
    pmsg_voicedata voicemsg;
    std::int32_t size;
    array of std::uint8_t sendbuffer;

    size = sizeof(tmsg_voicedata) + datasize;
    setlength(sendbuffer, size);

    voicemsg = pmsg_voicedata(sendbuffer);
    voicemsg.header.id = msgid_voicedata;

    move(data, voicemsg.data, datasize);

    sprite[mysprite].player->lastreceivevoicetime = maintickcounter + 10;

    udp->senddata(&voicemsg, size, k_nsteamnetworkingsend_nodelay);
}

void clienthandlevoicedata(SteamNetworkingMessage_t *netmessage)
{
    pmsg_voicedata voicemsg;
    std::uint64_t textlen;
    evoiceresult voiceresult;
    array of std::uint8_t audiodata;
    std::uint32_t audiolength;

    if (!CVar::cl_voicechat)
        return;

    if (!verifypacketlargerorequal(sizeof(voicemsg), netmessage->m_cbSize, msgid_voicedata))
        return;

    voicemsg = pmsg_voicedata(netmessage->m_pData);

    if ((voicemsg.speaker > 0) && (voicemsg.speaker < max_players))
    {
        if (!sprite[voicemsg.speaker].active)
            return;

        if (sprite[voicemsg.speaker].player->muted == 1)
            return;

        setlength(audiodata, 20480);

        textlen = netmessage->m_cbSize - sizeof(tmsg_voicedata);
        voiceresult = steamapi.user.decompressvoice(&voicemsg->data, textlen, audiodata,
                                                    length(audiodata), &audiolength, 44100);

        if (voiceresult == k_evoiceresultbuffertoosmall)
        {
            setlength(audiodata, audiolength);
            voiceresult = steamapi.user.decompressvoice(&voicemsg->data, textlen, audiodata,
                                                        length(audiodata), &audiolength, 44100);
        }

        if ((voiceresult == k_evoiceresultok) && (audiolength > 0))
        {
            sprite[voicemsg.speaker].player->lastreceivevoicetime = maintickcounter + 30;
            playvoicedata(&audiodata[0], audiolength, voicemsg.speaker);
        }
    }
}
#endif
