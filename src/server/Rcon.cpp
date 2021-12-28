// automatically converted

#define __rcon_implementation__

#include "soldat_cpp/soldat/server/Rcon.cpp.h"

/*#include "SteamTypes.h"*/
/*#include "ScriptCore3.h"*/
/*#include "ScriptDispatcher.h"*/
/*#include "Sprites.h"*/
/*#include "Server.h"*/
/*#include "Util.h"*/
/*#include "TraceLog.h"*/
/*#include "Command.h"*/
/*#include "Game.h"*/
/*#include "ServerHelper.h"*/

tadminserver *tadminserver::create(string pass, std::string connectmessage = '')
{
    tidsockethandle binding1;

    inherited create(nullptr);
    fpassword = pass;
    fadmins = tthreadlist.create;
    fconnectmessage = connectmessage;
    fbytessent = 0;
    fbytesrecieved = 0;
    fmessagequeue = tqueue.create;
    fqueuelock = tcriticalsection.create;
    onconnect = this->handleconnect();
    ondisconnect = this->handledisconnect;
    onexecute = this->handleexecute;
    defaultport = CVar::net_port;
    maxconnections = CVar::net_maxadminconnections;
    bindings.clear;
    binding1 = bindings.add;
    binding1.port = CVar::net_port;
    if (CVar::net_adminip != "")
        binding1.ip = CVar::net_adminip;
    else
        binding1.ip = CVar::net_ip;
    active = true;
    return this;
}

tadminserver *tadminserver::destroy()
{
    active = false;
    freeandnullptr(fadmins);
    fmessagequeue.destroy();
    fqueuelock.destroy();
    inherited destroy();
    return this;
}

void tadminserver::sendtoall(string message)
{
    std::int32_t i;

    //  try
    //    try
    {
        void &with = fadmins.locklist;
        for (i = 0; i <= count - 1; i++)
        {
            void &with = tidcontext(items[i]).connection;
            if (connected)
            {
                iohandler.writeln1(message, indytextencoding_ascii(0), indytextencoding_ascii(0));
                fbytessent += length(message);
            }
        }
    }
    //    finally
    //      FAdmins.UnlockList
    //    end;
    //  except
    //  end;
}

void tadminserver::sendtoip(string ip, std::string message)
{
    std::int32_t i;

    //  try
    //    try
    {
        void &with = fadmins.locklist;
        for (i = 0; i <= count - 1; i++)
        {
            void &with = tidcontext(items[i]).connection;
            if (connected & (tidiohandlersocket(iohandler).binding.peerip == ip))
            {
                iohandler.writeln1(message, indytextencoding_ascii(0), indytextencoding_ascii(0));
                fbytessent += length(message);
            }
        }
    }
    //    finally
    //      FAdmins.UnlockList
    //    end;
    //  except
    //  end;
}

void tadminserver::processcommands()
{
    tadminmessage *msg;
    tidiohandlersocket iohandlersocket;
    tmsg_refreshx refreshmsgx;
    std::int32_t i, j;
    std::int32_t n, c;
    std::string ip, ips;
    std::array<string, 4> s;

    fqueuelock.acquire;
    if (fmessagequeue.count == 0)
    {
        fqueuelock.release;
        return;
    }
    msg = fmessagequeue.pop;
    fqueuelock.release;
    //  try
    {
        void &with = fadmins.locklist;
        for (i = 0; i <= count - 1; i++)
        {
            void &with = tidcontext(items[i]).connection;
            if (connected & (tidiohandlersocket(iohandler).binding.peerip == msg->ip) &&
                (tidiohandlersocket(iohandler).binding.peerport == msg->port))
            {
                iohandlersocket = tidiohandlersocket(iohandler);
            }
        }
    }
    //  finally
    //    FAdmins.UnlockList
    //  end;
    if (iohandlersocket == nullptr)
    {
        freemem(msg);
        return;
    }
    if (msg->msg == "SHUTDOWN")
    {
        GetMainConsole().console(string("SHUTDOWN (") + msg->ip + ").", game_message_color);
        progready = false;
        freemem(msg);
        return;
    }
    else if (msg->msg == "REFRESHX")
    {
        refreshmsgx = default_(tmsg_refreshx);
        for (i = 1; i <= max_players; i++)
            if (sortedplayers[i].playernum > 0)
            {
                refreshmsgx.name[i] = SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.name;
#ifdef STEAM
                refreshmsgx.hwid[i] = inttostr(
                    tsteamid(SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.steamid).getaccountid);
#else
                refreshmsgx.hwid[i] = SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.hwid;
#endif
                refreshmsgx.kills[i] = SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.kills;
                refreshmsgx.caps[i] = SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.flags;
                refreshmsgx.deaths[i] = SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.deaths;
                refreshmsgx.ping[i] = SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.pingtime;
                refreshmsgx.team[i] = SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.team;
                refreshmsgx.number[i] = SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).num;
                refreshmsgx.x[i] = iif(SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).deadmeat, 0,
                                       SpriteSystem::Get().GetSprite(sortedplayers[i].playernum].skeleton.pos[1).x);
                refreshmsgx.y[i] = iif(SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).deadmeat, 0,
                                       SpriteSystem::Get().GetSprite(sortedplayers[i].playernum].skeleton.pos[1).y);

                if (SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.controlmethod == human)
                {
                    n = 1;
                    c = 0;
                    ips = SpriteSystem::Get().GetSprite(sortedplayers[i].playernum).player.ip;
                    for (j = 1; j <= length(ips); j++)
                    {
                        c += 1;
                        if (ips[j] == '.')
                        {
                            c = 0;
                            n += 1;
                        }
                        else
                        {
                            setlength(s[n], c);
                            s[n][c] = ips[j];
                        }
                    }

                    for (j = 1; j <= 4; j++)
                        refreshmsgx.ip[i][j] = strtoint(s[j]);
                }
                else
                    for (j = 1; j <= 4; j++)
                        refreshmsgx.ip[i][j] = 0;
            }
            else
                refreshmsgx.team[i] = 255;

        refreshmsgx.header = string("REFRESHX") + "\r\n";
        refreshmsgx.teamscore[team_alpha] = teamscore[team_alpha];
        refreshmsgx.teamscore[team_bravo] = teamscore[team_bravo];
        refreshmsgx.teamscore[team_charlie] = teamscore[team_charlie];
        refreshmsgx.teamscore[team_delta] = teamscore[team_delta];

        if (teamflag[team_alpha] > 0)
        {
            refreshmsgx.redflagx = things[teamflag[team_alpha]].skeleton.pos[1].x;
            refreshmsgx.redflagy = things[teamflag[team_alpha]].skeleton.pos[1].y;
        }

        if (teamflag[team_bravo] > 0)
        {
            refreshmsgx.blueflagx = things[teamflag[team_bravo]].skeleton.pos[1].x;
            refreshmsgx.blueflagy = things[teamflag[team_bravo]].skeleton.pos[1].y;
        }

        refreshmsgx.mapname = map.name;
        refreshmsgx.timelimit = CVar::sv_timelimit;
        refreshmsgx.currenttime = timelimitcounter;
        refreshmsgx.killlimit = CVar::sv_killlimit;
        refreshmsgx.gamestyle = CVar::sv_gamemode;
        refreshmsgx.maxplayers = CVar::sv_maxplayers;
        refreshmsgx.maxspectators = CVar::sv_maxspectators;
        refreshmsgx.passworded = iif(CVar::sv_password != "", 1, 0);
        refreshmsgx.nextmap = checknextmap;

        iohandlersocket.write1(rawtobytes(refreshmsgx, sizeof(refreshmsgx)));
        freemem(msg);
        return;
    }
    else
    {
        ip = msg->ip;
        GetMainConsole().console(msg->msg + " (" + ip + ')', game_message_color);
#ifdef SCRIPT
        scrptdispatcher.onadminmessage(msg->ip, msg->port, msg->msg);
#endif
        if (msg->msg[1] == '/')
#ifdef SCRIPT
            if (!scrptdispatcher.onconsolecommand(msg->ip, msg->port, msg->msg))
#endif
            {
                Delete(msg->msg, 1, 1);
                parseinput(msg->msg, 255);
            }
        freemem(msg);
    }
}

void findadminfloodid(string srcip)
{
    std::int32_t i;
    const std::int32_t flood_id_not_found = 0;

    void findadminfloodid_result;
    result = flood_id_not_found;
    for (i = 1; i <= max_admin_flood_ips; i++)
        if (adminfloodip[i] == srcip)
        {
            result = i;
            break;
        }
    return findadminfloodid_result;
}

void addadminfloodip(string srcip)
{
    std::uint32_t i;
    const std::int32_t flood_id_not_found = 0;

    void addadminfloodip_result;
    result = flood_id_not_found;
    for (i = 1; i <= max_admin_flood_ips; i++)
        if (adminfloodip[i] == "")
        {
            adminfloodip[i] = srcip;
            result = i;
            break;
        }
    return addadminfloodip_result;
}

void updateantiadminflood(string ip)
{
    std::uint32_t i;

    // update last requested admin ip array
    void updateantiadminflood_result;
    lastadminips[adminipcounter] = ip;
    adminipcounter = (adminipcounter + 1) % max_last_admin_ips + 1;

    // check for flood
    for (i = 0; i <= max_last_admin_ips; i++)
    {
        if (lastadminips[i] != lastadminips[(i + 1) % max_last_admin_ips + 1])
            break;
    }

    // if is flood then ban
    if (i == max_last_admin_ips + 1)
    {
        i = addadminfloodip(ip);
        if (i == 0)
            output << "failed adding ip to banlst" << NL;
        result = 1;
    }
    else
    {
        result = 0;
    }
    return updateantiadminflood_result;
}

void tadminserver::handleconnect(tidcontext athread)
{
    std::string msg;
    std::string ip;

    {
        void &with = athread.connection;
        //  try
        ip = tidiohandlersocket(iohandler).binding.peerip;
    }

    // if Admin IP is banned then drop
    if (findadminfloodid(ip) != 0)
    {
        athread.connection.disconnect;
        fadmins.remove(athread);
        return;
    }

    if (updateantiadminflood(ip) > 0)
        return;

    iohandler.writeln1("Soldat Admin Connection Established.");

    msg = iohandler.readln1(lf, 5000, -1);

    fbytesrecieved += length(msg);
    if (msg != fpassword)
    {
        if (msg == "")
            iohandler.writeln1("password request timed out.");
        else
            iohandler.writeln1("Invalid password.");

        GetMainConsole().console(string("Admin failed to connect (") + ip + ").", game_message_color);
        disconnect;
    }
    else
    {
        GetMainConsole().console(string("Admin connected (") + ip + ").", game_message_color);
        fadmins.add(athread);
        iohandler.writeln1("Welcome, you are in command of the server now.");
        iohandler.writeln1("List of commands available in the Soldat game Manual.");
        iohandler.writeln1(string("Server Version: ") + dedversion);

#ifdef SCRIPT
        scrptdispatcher.onadminconnect(tidiohandlersocket(iohandler).binding.peerip,
                                       tidiohandlersocket(iohandler).binding.peerport);
#endif
    }
    //  except
    //    on e: Exception do
    //    begin
    //      try
    //        IOHandler.WriteLn(e.Message);
    //      except
    //      end;
    //      Disconnect;
    //    end;
    //  end;
}

void tadminserver::handledisconnect(tidcontext athread)
{
    std::string ip;

    // TODO: test if still cause access violations
    if (athread == nullptr)
    {
        fadmins.remove(athread);
        return;
    }
    else if ((athread.connection == nullptr) || (athread.connection.iohandler == nullptr))
    {
        athread.connection.disconnect;
        fadmins.remove(athread);
        return;
    }

    // try
    ip = tidiohandlersocket(athread.connection.iohandler).binding.peerip;

    // if Admin IP is banned then drop
    if (findadminfloodid(ip) != 0)
    {
        athread.connection.disconnect;
        fadmins.remove(athread);
        return;
    }

    if (updateantiadminflood(ip) > 0)
        return;
    //  except
    //  end;
    //  try
    athread.connection.disconnect;
    //  except
    //  end;
    //  try
    fadmins.remove(athread);
    //  except
    //  end;
    //  try
    GetMainConsole().console(string("Admin disconnected (") + ip + ").", game_message_color);
    //  except
    //  end;
#ifdef SCRIPT
    scrptdispatcher.onadmindisconnect(
        tidiohandlersocket(athread.connection.iohandler).binding.peerip,
        tidiohandlersocket(athread.connection.iohandler).binding.peerport);
#endif
}

void tadminserver::handleexecute(tidcontext athread)
{
    std::string msg, ip;
    tidport port;
    tadminmessage *msgrecord;

    {
        void &with = athread.connection;
        //  try
        ip = tidiohandlersocket(iohandler).binding.peerip;
    }
    port = tidiohandlersocket(iohandler).binding.peerport;
    // if Admin IP is banned then drop
    if (findadminfloodid(ip) != 0)
    {
        athread.connection.disconnect;
        fadmins.remove(athread);
        return;
    }

    //    try
    if (fadmins.locklist.indexof(athread) == -1)
        return;
    //    finally
    //      FAdmins.UnlockList;
    //    end;

    msg = iohandler.readln1(lf, 1000, -1, indytextencoding_ascii(0), indytextencoding_ascii(0));
    if (msg != "")
    {
        fbytesrecieved += length(msg);
        msgrecord = new tadminmessage;
        msgrecord->msg = msg;
        msgrecord->ip = ip;
        msgrecord->port = port;

        fqueuelock.acquire;
        fmessagequeue.push(msgrecord);
        fqueuelock.release;
    }
    //  except
    //    on e: Exception do
    //    begin
    //      try
    //        IOHandler.WriteLn(e.Message);
    //      except
    //      end;
    //    end;
    //  end;
}

void broadcastmsg(string text)
{
    trace("BroadcastMsg");
    if (assigned(sv_adminpassword))
        if (CVar::sv_adminpassword == "")
            return;
    if (adminserver == nullptr)
        return;

    adminserver.sendtoall(text);
}

void sendmessagetoadmin(string toip, std::string text)
{
    if (CVar::sv_adminpassword == "")
        return;
    if (adminserver == nullptr)
        return;

    adminserver.sendtoip(toip, text);
}
