// automatically converted

#include "ServerCommands.hpp"
#include "BanSystem.hpp"
#include "Server.hpp"
#include "ServerHelper.hpp"
#include "shared/Command.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/network/NetworkServer.hpp"
#include "shared/network/NetworkServerConnection.hpp"
#include "shared/network/NetworkServerFunctions.hpp"
#include "shared/network/NetworkServerGame.hpp"
#include "shared/network/NetworkServerMessages.hpp"
#include "shared/network/NetworkUtils.hpp"

//clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

using string = std::string;

namespace
{

/*$PUSH*/
/*$WARN 5024 OFF : Parameter "$1" not used*/
void commandaddbot(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name, tempstr;
    std::int32_t teamset;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    if (playersnum == max_players)
        return;

    tempstr = args[0];
    teamset = strtointdef({tempstr[6], 1}, 0);
    addbotplayer(name, teamset);
}

void commandnextmap(std::vector<std::string> &args, std::uint8_t sender)
{
    nextmap();
}

void commandmap(std::vector<std::string> &args, std::uint8_t sender)
{
    tmapinfo status;

    if (length(args) == 1)
        return;

    if (length(args[1]) < 1)
        return;

    if (getmapinfo(args[1], userdirectory, status))
    {
        preparemapchange(args[1]);
    }
    else
    {
#ifdef STEAM
        if (status.workshopid > 0)
        {
            mapchangeitemid = status.workshopid;
            if (steamapi.ugc.downloaditem(mapchangeitemid, true))
                GetServerMainConsole().console(string("[Steam] Workshop map ") +
                                                   inttostr(mapchangeitemid) +
                                                   " not found in cache, downloading.",
                                               warning_message_color, sender);
            else
                GetServerMainConsole().console(string("[Steam] Workshop map ") +
                                                   inttostr(mapchangeitemid) + " is invalid",
                                               warning_message_color, sender);
        }
        else
#endif
            GetServerMainConsole().console(string("Map not found (") + args[1] + ')',
                                           warning_message_color, sender);
    }

    // if not MapExists(MapChangeName, userdirectory) then
    // begin
    //  GetServerMainConsole().Console('Map not found (' + MapChangeName + ')',
    //  WARNING_MESSAGE_COLOR, Sender); Exit;
    // end;
}

void commandpause(std::vector<std::string> &args, std::uint8_t sender)
{
    mapchangecounter = 999999999;
    serversyncmsg();
}

void commandunpause(std::vector<std::string> &args, std::uint8_t sender)
{
    mapchangecounter = -60;
    serversyncmsg();
}

void commandrestart(std::vector<std::string> &args, std::uint8_t sender)
{
    mapchangename = map.name;
    mapchangecounter = mapchangetime;
    servermapchange(all_players); // Inform clients of Map Change
#ifdef SCRIPT
    scrptdispatcher.onbeforemapchange(mapchangename);
#endif
}

void commandkick(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t i;
    tcommandtargets targets;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    targets = commandtarget(name, sender);
    for (i = 0; i <= high(targets); i++)
        kickplayer(targets[i], false, kick_console, 0);
}

void commandkicklast(std::vector<std::string> &args, std::uint8_t sender)
{
    if ((lastplayer > 0) && (lastplayer < max_sprites + 1))
        if (sprite[lastplayer].active)
            kickplayer(lastplayer, false, kick_console, 0);
}

void commandban(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name, tempstr;
    std::int32_t i;
    tcommandtargets targets;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    if (sender == 255)
        tempstr = "an admin";
    else
        tempstr = sprite[sender].player->name;

    targets = commandtarget(name, sender);
    for (i = 0; i <= high(targets); i++)
        kickplayer(targets[i], true, kick_console, (day * 30), std::string("Banned by ") + tempstr);
}

void commandbaniphw(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name, tempstr;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    if (sender == 255)
        tempstr = "an admin";
    else
        tempstr = sprite[sender].player->name;

    if (args[0] == "banhw")
    {
        addbannedhw(name, string("Banned by ") + tempstr, (day * 30));
        GetServerMainConsole().console(string("HWID ") + name + " banned", client_message_color,
                                       sender);
    }
    else
    {
        addbannedip(name, string("Banned by ") + tempstr, day * 30);
        GetServerMainConsole().console(string("IP number ") + name + " banned",
                                       client_message_color, sender);
    }

    savetxtlists();
}

void commandunban(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    if (delbannedip(name))
        GetServerMainConsole().console(string("IP number ") + name + " unbanned",
                                       client_message_color, sender);

    if (delbannedhw(name))
        GetServerMainConsole().console(string("HWID ") + name + " unbanned", client_message_color,
                                       sender);

    savetxtlists();
}

void commandunbanlast(std::vector<std::string> &args, std::uint8_t sender)
{
    if (delbannedip(lastban))
        GetServerMainConsole().console(string("IP number ") + lastban + " unbanned",
                                       client_message_color, sender);

    if (delbannedhw(lastbanhw))
        GetServerMainConsole().console(string("HWID ") + lastbanhw + " unbanned",
                                       client_message_color, sender);

    savetxtlists();
}

void commandadm(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t i;
    tcommandtargets targets;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    targets = commandtarget(name, sender);
    for (i = 0; i <= high(targets); i++)
        if (isremoteadminip(sprite[targets[i]].player->ip))
        {
            remoteips.add(sprite[targets[i]].player->ip);
            GetServerMainConsole().console(string("IP number ") + sprite[targets[i]].player->ip +
                                               " added to Remote Admins",
                                           client_message_color, sender);
            savetxtlists();
        }
}

void commandadmip(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    if (!isremoteadminip(name))
    {
        remoteips.add(name);
        GetServerMainConsole().console(string("IP number ") + name + " added to Remote Admins",
                                       client_message_color, sender);
        savetxtlists();
    }
}

void commandunadm(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t j;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    if (isremoteadminip(name))
    {
        NotImplemented(NITag::NETWORK);
#if 0
        j = remoteips.indexof(name);
        remoteips.delete_(j);
#endif
        GetServerMainConsole().console(string("IP number ") + name + " removed from Remote Admins",
                                       client_message_color, sender);
        savetxtlists();
    }
}

void commandsetteam(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name, tempstr;
    std::int32_t i;
    std::uint8_t teamset;
    tcommandtargets targets;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    tempstr = args[0];
    NotImplemented(NITag::OTHER);
#if 0
    teamset = strtointdef(std::string(tempstr.at(8)), 1);
#endif

    targets = commandtarget(name, sender);
    for (i = 0; i <= high(targets); i++)
    {
        sprite[targets[i]].changeteam(teamset, true);
    }
}

void commandsay(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    serversendstringmessage((name), all_players, 255, msgtype_pub);
}

void commandkill(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    tvector2 a;
    std::int32_t i;
    tcommandtargets targets;

    if (length(args) == 1)
        return;

    name = args[1];

    targets = commandtarget(name, sender);
    for (i = 0; i <= high(targets); i++)
    {
        sprite[targets[i]].vest = 0;
        sprite[targets[i]].healthhit(3430, targets[i], 1, -1, a);
        GetServerMainConsole().console(sprite[targets[i]].player->name + " killed by admin",
                                       client_message_color, sender);
    }
}

void commandloadwep(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t i;

    if (length(args) == 1)
    {
        if (CVar::sv_realisticmode)
            name = "weapons_floatistic";
        else
            name = "weapons";
    }
    else
        name = args[1];

    lastwepmod = name;
    loadweapons(name);

    for (i = 1; i <= max_players; i++)
        if (sprite[i].active)
            if (sprite[i].player->controlmethod == human)
                servervars(i);
}

void commandloadcon(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t i;

    if (length(args) == 1)
        return;

    name = args[1];

    if (CVar::sv_lockedmode)
    {
        GetServerMainConsole().console(
            std::string("Locked Mode is enabled. Settings can't be changed mid-game."),
            server_message_color, sender);
        return;
    }

    mapchangecounter = -60;
    serverdisconnect();
    for (i = 1; i <= max_bullets; i++)
        bullet[i].kill();
    for (i = 1; i <= max_things; i++)
        thing[i].kill();
    for (i = 1; i <= max_players; i++)
    {
        if (sprite[i].active)
        {
            sprite[i].player->team = fixteam(sprite[i].player->team);
            sprite[i].respawn();
            sprite[i].player->kills = 0;
            sprite[i].player->deaths = 0;
            sprite[i].player->flags = 0;

            sprite[i].player->tkwarnings = 0;
            sprite[i].player->chatwarnings = 0;
            sprite[i].player->knifewarnings = 0;

            sprite[i].player->scorespersecond = 0;
            sprite[i].player->grabspersecond = 0;
        }
    }

    loadconfig(name);
    GetServerMainConsole().console(string("Config reloaded ") + currentconf, client_message_color,
                                   sender);
    startserver();
}

void commandloadlist(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t i;

    if (length(args) == 0)
    {
        NotImplemented(NITag::OTHER, "missing stringreplace");
#if 0
        name = stringreplace(CVar::sv_maplist, ".txt", "", rfreplaceall);
#endif
    }
    else
        name = args[1];

    if (fileexists(userdirectory + name + ".txt"))
    {
        mapslist.loadfromfile(userdirectory + name + ".txt");
        i = 1;
        mapslist.erase(std::remove(mapslist.begin(), mapslist.end(), ""), mapslist.end());
        CVar::sv_maplist = name + ".txt";
        GetServerMainConsole().console(string("Mapslist loaded ") + name, client_message_color,
                                       sender);
    }
}

void commandpm(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string pmtoid, pmmessage;
    std::int32_t i;
    tcommandtargets targets;

    if (length(args) <= 2)
        return;

    pmtoid = args[1];

    targets = commandtarget(pmtoid, sender);
    for (i = 0; i <= high(targets); i++)
    {
        pmmessage = args[2];
        GetServerMainConsole().console(string("Private Message sent to ") + idtoname(targets[i]),
                                       server_message_color, sender);
        GetServerMainConsole().console(string("(PM) To: ") + idtoname(targets[i]) +
                                           " From: " + idtoname(sender) + " Message: " + pmmessage,
                                       server_message_color);
        serversendstringmessage(string("(PM) ") + (pmmessage), targets[i], 255, msgtype_pub);
    }
}

void commandgmute(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t i, j;
    tcommandtargets targets;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    targets = commandtarget(name, sender);
    for (i = 0; i <= high(targets); i++)
    {
        sprite[targets[i]].player->muted = 1;
        for (j = 1; j <= max_players; j++)
            if (trim(mutelist[j]) == "")
            {
                mutelist[j] = sprite[targets[i]].player->ip;
                mutename[j] = sprite[targets[i]].player->name;
                break;
            }
        GetServerMainConsole().console(sprite[targets[i]].player->name + " has been muted.",
                                       client_message_color, sender);
    }
}

void commandungmute(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t i, j;
    tcommandtargets targets;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    targets = commandtarget(name, sender);
    for (i = 0; i <= high(targets); i++)
    {
        sprite[targets[i]].player->muted = 0;
        for (j = 1; j <= max_players; j++)
            if (trim(mutelist[j]) == sprite[targets[i]].player->ip)
            {
                mutelist[j] = "";
                break;
            }
        GetServerMainConsole().console(sprite[targets[i]].player->name + " has been unmuted.",
                                       client_message_color, sender);
    }
}

void commandaddmap(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    // if not (MapExists(Name, userdirectory)) then
    // begin
    //  GetServerMainConsole().Console('Map not found (' + Name + ')',
    //    SERVER_MESSAGE_COLOR, Sender);
    //  Exit;
    // end;

    mapslist.add(name);
    GetServerMainConsole().console(name + " has been added to the map list.", server_message_color,
                                   sender);
    savemaplist();
}

void commanddelmap(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t tempint;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    for (tempint = 0; tempint <= (mapslist.size() - 1); tempint++)
    {
        NotImplemented(NITag::OTHER);
#if 0
        if (uppercase(mapslist[tempint]) == uppercase(name))
        {
            GetServerMainConsole().console(name + " has been removed from the map list.", server_message_color,
                                sender);
            mapslist.delete_(tempint);
            break;
        }
#endif
    }
    savemaplist();
}

void commandtempban(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name, tempstr;

    if (length(args) <= 2)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    tempstr = args[2];
    // *BAN*
    addbannedip(tempstr, "Temporary Ban by an Admin", strtointdef(name, 1) * minute);
    GetServerMainConsole().console(string("IP number ") + tempstr + " banned for " + name +
                                       " minutes.",
                                   client_message_color, sender);
    savetxtlists();
}

void commandweaponon(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t i, j;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
        {
            j = strtointdef(name, -1);
            if ((j > -1) && (j < 15))
                setweaponactive(i, j, true);
        }
}

void commandweaponoff(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;
    std::int32_t i, j;

    if (length(args) == 1)
        return;

    name = args[1];

    if (length(name) < 1)
        return;

    for (i = 1; i <= max_players; i++)
        if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
        {
            j = strtointdef(name, -1);
            if ((j > -1) && (j < 15))
                setweaponactive(i, j, false);
        }
}

void commandbanlist(std::vector<std::string> &args, std::uint8_t sender)
{
    NotImplemented(NITag::OTHER);
#if 0
    std::int32_t i;
    tdatetime banduration;
    std::string bandurationtext;

    GetServerMainConsole().console(format("%-15s | %-9s | %s", set::of("HWID", "Duration", "Reason", eos)),
                        server_message_color, sender);
    for (i = 1; i <= high(bannedhwlist); i++)
    {
        if (bannedhwlist[i].time == permanent)
            bandurationtext = "PERMANENT";
        else
        {
            banduration = incsecond(now, bannedhwlist[i].time / 60) - now;
            bandurationtext =
                format("%dd%s", set::of(trunc(banduration),
                                        formatdatetime("h\"h\"n\"m\"", banduration), eos));
        }
        GetServerMainConsole().console(format("%-15s | %-9s | %s", set::of(bannedhwlist[i].hw, bandurationtext,
                                                                bannedhwlist[i].reason, eos)),
                            server_message_color, sender);
    }

    GetServerMainConsole().console(format("%-15s | %-9s | %s", set::of("IP", "Duration", "Reason", eos)),
                        server_message_color, sender);
    for (i = 1; i <= high(bannediplist); i++)
    {
        if (bannediplist[i].time == permanent)
            bandurationtext = "PERMANENT";
        else
        {
            banduration = incsecond(now, bannediplist[i].time / 60) - now;
            bandurationtext =
                format("%dd%s", set::of(trunc(banduration),
                                        formatdatetime("h\"h\"n\"m\"", banduration), eos));
        }
        GetServerMainConsole().console(format("%-15s | %-9s | %s", set::of(bannediplist[i].ip, bandurationtext,
                                                                bannediplist[i].reason, eos)),
                            server_message_color, sender);
    }
#endif
}

void commandnetstats(std::vector<std::string> &args, std::uint8_t sender)
{
    std::array<char, 2048> statstext;
    tplayer dstplayer;

    if (GetServerNetwork()->NetworkingSocket().GetDetailedConnectionStatus(1, statstext.data(),
                                                                           2048) == 0)
    {
        NotImplemented(NITag::OTHER);
#if 0
        output << statstext << NL;
#endif
    }
    for (auto &dstplayer : players)
    {
        if (GetServerNetwork()->NetworkingSocket().GetDetailedConnectionStatus(
                dstplayer->peer, statstext.data(), 2048) == 0)
        {
            NotImplemented(NITag::OTHER);
#if 0
            output << "----------------" << NL;
            output << dstplayer.name << NL;
            output << dstplayer.peer << NL;
            output << statstext << NL;
            output << "----------------" << NL;
#endif
        }
    }
}

void commandplayercommand(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string command;
    tvector2 a;

    command = args[0];

    if (command == "tabac")
    {
        sprite[sender].idlerandom = 0;
        sprite[sender].idletime = 1;
    }
    else if (command == "smoke")
    {
        sprite[sender].idlerandom = 1;
        sprite[sender].idletime = 1;
    }
    else if (command == "takeoff")
    {
        sprite[sender].idlerandom = 4;
        sprite[sender].idletime = 1;
    }
    else if (command == "victory")
    {
        sprite[sender].idlerandom = 5;
        sprite[sender].idletime = 1;
    }
    else if (command == "piss")
    {
        sprite[sender].idlerandom = 6;
        sprite[sender].idletime = 1;
    }
    else if (command == "mercy")
    {
        sprite[sender].idlerandom = 7;
        sprite[sender].idletime = 1;
        if (sprite[sender].player->kills > 0)
            sprite[sender].player->kills -= 1;
    }
    else if (command == "pwn")
    {
        sprite[sender].idlerandom = 8;
        sprite[sender].idletime = 1;
    }
    else if (command == "kill")
    {
        sprite[sender].vest = 0;
        sprite[sender].healthhit(150, sender, 1, -1, a);
        if (sprite[sender].player->kills > 0)
            sprite[sender].player->kills -= 1;
    }
    else if (command == "brutalkill")
    {
        sprite[sender].vest = 0;
        sprite[sender].healthhit(3423, sender, 1, -1, a);
        if (sprite[sender].player->kills > 0)
            sprite[sender].player->kills -= 1;
    }
}

void commandinfo(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string gametype = "";
#ifdef SCRIPT
    std::int8_t i;
    tstringlist scriptlist;
#endif

    if (length((std::string)CVar::sv_greeting) > 0)
        serversendstringmessage((CVar::sv_greeting), sender, 255, msgtype_pub);
    if (length((std::string)CVar::sv_greeting2) > 0)
        serversendstringmessage((CVar::sv_greeting2), sender, 255, msgtype_pub);
    if (length((std::string)CVar::sv_greeting3) > 0)
        serversendstringmessage((CVar::sv_greeting3), sender, 255, msgtype_pub);

    serversendstringmessage(string("Server: ") + string(CVar::sv_hostname), sender, 255,
                            msgtype_pub);
    serversendstringmessage(string("Address: ") + (serverip) + ':' + (inttostr(serverport)), sender,
                            255, msgtype_pub);
    serversendstringmessage(string("Version: ") + dedversion, sender, 255, msgtype_pub);

    switch (CVar::sv_gamemode)
    {
    case gamestyle_deathmatch:
        gametype = "Deathmatch";
        break;
    case gamestyle_pointmatch:
        gametype = "Pointmatch";
        break;
    case gamestyle_teammatch:
        gametype = "Teammatch";
        break;
    case gamestyle_ctf:
        gametype = "Capture the Flag";
        break;
    case gamestyle_rambo:
        gametype = "Rambomatch";
        break;
    case gamestyle_inf:
        gametype = "Infiltration";
        break;
    case gamestyle_htf:
        gametype = "Hold the Flag";
        break;
    }

    serversendstringmessage(string("Gamemode: ") + gametype, sender, 255, msgtype_pub);
    serversendstringmessage(string("Timelimit: ") + (inttostr(CVar::sv_timelimit / 3600)), sender,
                            255, msgtype_pub);
    serversendstringmessage(string("Nextmap: ") + (checknextmap()), sender, 255, msgtype_pub);

#ifdef SCRIPT
    serversendstringmessage(string("Scripting: ") + iif(CVar::sc_enable, "Enabled", "Disabled"),
                            sender, 255, msgtype_pub);
    if (CVar::sc_enable)
    {
        gametype = "";
        scriptlist = scrptdispatcher.scriptlist;
        for (i = 0; i <= scriptlist.count - 1; i++)
            gametype = gametype + iif(i != 0, ", ", "") + scriptlist.strings[i];
        serversendstringmessage(string("Scripts: ") + gametype, sender, 255, msgtype_pub);
        scriptlist.free(0);
    }
#endif

    if (loadedwmchecksum != defaultwmchecksum)
        serversendstringmessage(string("Server uses weapon mod \"") + (wmname) + " v" +
                                    (wmversion) + "\" (checksum " + (inttostr(loadedwmchecksum)) +
                                    ')',
                                sender, 255, msgtype_pub);
}

void commandadminlog(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string adminlog;

    if (length(args) == 1)
        return;

    adminlog = args[1];
    if (length((std::string)CVar::sv_adminpassword) > 0)
    {
        if (adminlog == CVar::sv_adminpassword)
        {
            if (!isadminip(sprite[sender].player->ip))
                adminips.add(sprite[sender].player->ip);
            GetServerMainConsole().console(sprite[sender].player->name + " added to Game Admins",
                                           server_message_color, sender);
        }
        else
        {
            GetServerMainConsole().console(sprite[sender].player->name +
                                               " tried to login as Game Admin with bad password",
                                           server_message_color, sender);
        }
    }
}

void commandvotemap(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string mapname;

    if (length(args) == 1)
        return;

    mapname = args[1];

    if (voteactive)
    {
        // check if the vote target is actually the target
        if (votetarget != string(mapname))
            return;

        // check if he already voted
        if (votehasvoted[sender])
            return;

#ifdef SCRIPT
        scrptdispatcher.onvotemap(sender, mapname);
#endif
        countvote(sender);
    }
    else
    {
        if (std::find(mapslist.begin(), mapslist.end(), mapname) !=
            mapslist.end()) /*and (MapExists(MapName, userdirectory))*/
        {
            if (votecooldown[sender] < 0)
            {
                if (!voteactive)
                {
#ifdef SCRIPT
                    if (scrptdispatcher.onvotemapstart(sender, mapname))
                        return;
#endif
                    startvote(sender, vote_map, mapname, "---");
                    serversendvoteon(votetype, sender, mapname, "---");
                }
            }
            else
                serversendstringmessage(
                    "Can't vote for 2:00 minutes after joining game or last vote", sender, 255,
                    msgtype_pub);
        }
        else
            serversendstringmessage(string("Map not found (") + (mapname) + ')', sender, 255,
                                    msgtype_pub);
    }
}

#ifdef SCRIPT
void commandrecompile(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string name;

    if (length(args) == 1)
    {
        if (!CVar::sc_enable)
            GetServerMainConsole().console("Scripting is currently disabled.", client_message_color,
                                           sender);
        else
        {
            scrptdispatcher.prepare(0);
            scrptdispatcher.launch(0);
        }
    }
    else
    {
        name = args[1];
        if (!CVar::sc_enable)
            GetServerMainConsole().console("Scripting is currently disabled.", client_message_color,
                                           sender);
        else
            scrptdispatcher.launch(name);
    }
}
#endif

void commandrecord(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string str1;

    if (length(args) == 2)
        str1 = args[1];
    else
    {
        NotImplemented(NITag::OTHER);
#if 0
        str1 = formatdatetime("yyyy-mm-dd_hh-nn-ss_", now(0)) + map.name;
#endif
    }

    demorecorder.stoprecord();
    demorecorder.startrecord(userdirectory + "demos/" + str1 + ".sdm");
}

void commandstop(std::vector<std::string> &args, std::uint8_t sender)
{
    demorecorder.stoprecord();
}

} // namespace

/*$POP*/

void initservercommands()
{
    commandadd("addbot", commandaddbot, "Add specific bot to game", cmd_adminonly);
    commandadd("addbot1", commandaddbot, "Add specific bot to alpha team", cmd_adminonly);
    commandadd("addbot2", commandaddbot, "Add specific bot to blue team", cmd_adminonly);
    commandadd("addbot3", commandaddbot, "Add specific bot to charlie team", cmd_adminonly);
    commandadd("addbot4", commandaddbot, "Add specific bot to delta team", cmd_adminonly);
    commandadd("addbot5", commandaddbot, "Add specific bot to spectators", cmd_adminonly);
    commandadd("nextmap", commandnextmap, "Change map to next in maplist", cmd_adminonly);
    commandadd("map", commandmap, "Change map to specified mapname", cmd_adminonly);
    commandadd("pause", commandpause, "Pause game", cmd_adminonly);
    commandadd("unpause", commandunpause, "Unpause game", cmd_adminonly);
    commandadd("restart", commandrestart, "Restart current map", cmd_adminonly);
    commandadd("kick", commandkick, "Kick player with specified nick or id", cmd_adminonly);
    commandadd("kicklast", commandkicklast, "Kick last connected player", cmd_adminonly);
    commandadd("ban", commandban, "Ban player with specified nick or id", cmd_adminonly);
    commandadd("banip", commandbaniphw, "Ban specified IP address", cmd_adminonly);
    commandadd("banhw", commandbaniphw, "Ban specified hwid", cmd_adminonly);
    commandadd("unban", commandunban, "Unban specified ip or hwid", cmd_adminonly);
    commandadd("unbanlast", commandunban, "Unban last player", cmd_adminonly);
    commandadd("adm", commandadm, "Give admin to specified nick or id", cmd_adminonly);
    commandadd("admip", commandadmip, "add the IP number to the Remote Admins list", cmd_adminonly);
    commandadd("unadm", commandunadm, "remove the IP number from the admins list", cmd_adminonly);
    commandadd("setteam1", commandsetteam, "move specified id or nick to alpha team",
               cmd_adminonly);
    commandadd("setteam2", commandsetteam, "move specified id or nick to bravo team",
               cmd_adminonly);
    commandadd("setteam3", commandsetteam, "move specified id or nick to charlie team",
               cmd_adminonly);
    commandadd("setteam4", commandsetteam, "move specified id or nick to delta team",
               cmd_adminonly);
    commandadd("setteam5", commandsetteam, "move specified id or nick to spectators",
               cmd_adminonly);
    commandadd("say", commandsay, "Send chat message", cmd_adminonly);
    commandadd("pkill", commandkill, "Kill specified id or nick", cmd_adminonly);
    commandadd("loadwep", commandloadwep, "Load weapons config", cmd_adminonly);
    commandadd("loadcon", commandloadcon, "Load server config", cmd_adminonly);
    commandadd("loadlist", commandloadlist, "Load maplist", cmd_adminonly);
    commandadd("pm", commandpm, "Send private message to other player", cmd_adminonly);
    commandadd("gmute", commandgmute, "Mute player on server", cmd_adminonly);
    commandadd("ungmute", commandungmute, "Unmute player on server", cmd_adminonly);
    commandadd("addmap", commandaddmap, "Add map to the maplist", cmd_adminonly);
    commandadd("delmap", commanddelmap, "Remove map from the maplist", cmd_adminonly);
    commandadd("weaponon", commandweaponon, "Enable specific weapon on the server", cmd_adminonly);
    commandadd("weaponoff", commandweaponoff, "Disable specific weapon on the server",
               cmd_adminonly);
    commandadd("banlist", commandbanlist, "Show banlist", cmd_adminonly);
    commandadd("net_stats", commandnetstats, "Show network stats", cmd_adminonly);

    commandadd("tabac", commandplayercommand, "tabac", cmd_playeronly);
    commandadd("smoke", commandplayercommand, "smoke", cmd_playeronly);
    commandadd("takeoff", commandplayercommand, "takeoff", cmd_playeronly);
    commandadd("victory", commandplayercommand, "victory", cmd_playeronly);
    commandadd("piss", commandplayercommand, "piss", cmd_playeronly);
    commandadd("mercy", commandplayercommand, "mercy", cmd_playeronly);
    commandadd("pwn", commandplayercommand, "pwn", cmd_playeronly);
    commandadd("kill", commandplayercommand, "kill", cmd_playeronly);
    commandadd("brutalkill", commandplayercommand, "brutalkill", cmd_playeronly);
    commandadd("info", commandinfo, "info", cmd_playeronly);
    commandadd("adminlog", commandadminlog, "adminlog", cmd_playeronly);
    commandadd("votemap", commandvotemap, "votemap", cmd_playeronly);
    commandadd("record", commandrecord, "record demo", cmd_adminonly);
    commandadd("stop", commandstop, "stop recording demo", cmd_adminonly);

#ifdef SCRIPT
    commandadd("recompile", commandrecompile(), "Recompile all or specific script", cmd_adminonly);
#endif
}
