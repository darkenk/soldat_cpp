// automatically converted

#include "Demo.hpp"

#ifdef SERVER
#include "../server/Server.hpp"
#else
#include "../client/Client.hpp"
#include "../client/ClientGame.hpp"
#include "../client/GameStrings.hpp"
#include "../client/InterfaceGraphics.hpp"
#endif
#include "Cvar.hpp"
#include "MapFile.hpp"
#include "Util.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include "shared/Game.hpp"
#include "shared/Version.hpp"
#include <filesystem>

using string = std::string;

tdemorecorder demorecorder;
#ifndef SERVER
tdemoplayer demoplayer;
#endif
std::uint64_t rsize;
std::uint8_t freecam;
std::uint8_t notexts = 0;

std::string extractfilename(const std::string &filepath)
{
    return std::filesystem::path(filepath).stem();
}

template <std::size_t N>
void stringtoarray(std::array<char, N> &arr, const std::string &str)
{
    int i = 0;
    for (auto &c : str)
    {
        arr[i++] = c;
    }
}

bool tdemorecorder::startrecord(const string &filename)
{
    std::int32_t spriteid;

    bool result;
    result = false;
#ifndef SERVER
    if (demoplayer.active())
        return result;
#endif

    mainconsole.console(
#ifdef SERVER
        string("Recording demo:") +
#else
        ("Recording demo:") +
#endif
            ' ' + (extractfilename(filename)),
        info_message_color);

    fname = extractfilename(filename);
    factive = true;
    foldcam = 255;
    fticksnum = 0;

    fdemoheader.header = {'S', 'O', 'L', 'D', 'E', 'M'};
    fdemoheader.ticksnum = 0;
    NotImplemented(NITag::OTHER, "No time for demo");
#if 0
    fdemoheader.startdate = datetimetounix(now);
#endif
    fdemoheader.version = demo_version;

    fillchar(fdemoheader.mapname.data(), sizeof(fdemoheader.mapname), '\0');
    stringtoarray(fdemoheader.mapname, map.name);

    fdemofile.writebuffer(&fdemoheader, sizeof(fdemoheader));

    spriteid = createdemoplayer();

    if (spriteid == max_sprites)
        result = true;
    return result;
}

void tdemorecorder::stoprecord()
{
    if (!active())
        return;

    mainconsole.console(
#ifdef SERVER
        string("Demo stopped") +
#else
        _("Demo stopped") +
#endif
            " (" + (fname) + ')',
        info_message_color);

    sprite[max_sprites].kill();

    fdemofile.position = 0;

    fdemoheader.version = demo_version;
    fdemoheader.ticksnum = ticksnum();

    fillchar(fdemoheader.mapname.data(), sizeof(fdemoheader.mapname), '\0');
    stringtoarray(fdemoheader.mapname, map.name);

    fdemofile.writebuffer(&fdemoheader, sizeof(fdemoheader));

    //  try
    fdemofile.savetofile(userdirectory + "demos/" + fname);
    //  except
    //    on e: Exception do
    //      mainconsole.Console('Failed to save demo file: ' + WideString(E.Message),
    //      INFO_MESSAGE_COLOR);
    //  end;

    factive = false;
    fname = "";
    fdemofile.free();
}

std::int32_t tdemorecorder::createdemoplayer()
{
    std::int32_t p;
    tplayer player;
    tvector2 a;

    std::int32_t createdemoplayer_result = -1;

    if (sprite[max_sprites].active)
    {
        mainconsole.console(
            "Failed to create Demo Recorder player. Demos can be recorded with up to 31 players",
            info_message_color);
        stoprecord();
        return createdemoplayer_result;
    }

    player.demoplayer = true;
    player.name = "Demo Recorder";
    player.team = team_spectator;
    player.controlmethod = human;

#ifdef SERVER
    player.peer = std::numeric_limits<std::uint32_t>::max();
#endif

    a.x = min_sectorz * map.sectorsdivision * 0.7;
    a.y = min_sectorz * map.sectorsdivision * 0.7;

    p = createsprite(a, vector2(0, 0), 1, max_sprites, &player, true);
    if ((p > 0) && (p < max_sprites + 1))
    {
        NotImplemented(NITag::NETWORK);
#if 0
#ifdef SERVER
        serversynccvars(p, player.peer, true);
        serversendplaylist(player.peer);
#else
        serversynccvars(p, 0, true);
        serversendplaylist();
#endif
        servervars(p);
        serversendnewplayerinfo(p, join_normal);
        serverthingmustsnapshotonconnect(p);
        sprite[p].player.demoplayer = true;
        spriteparts.pos[p] = vector2(0, 0);
        createdemoplayer_result = p;
#endif
    }

    return createdemoplayer_result;
}

void tdemorecorder::saverecord(const void *r, std::int32_t size)
{
    if (size == 0)
        return;

    if (!active())
        return;
    fdemofile.write1(&size, sizeof(rsize));
    fdemofile.write1(&r, size);
}

#ifndef SERVER
void tdemorecorder::savecamera()
{
    NotImplemented(NITag::NETWORK);
#if 0
    tmsg_clientspritesnapshot_dead msg;

    msg.header.id = msgid_clientspritesnapshot_dead;
    msg.camerafocus = camerafollowsprite;
    saverecord(&msg, sizeof(msg));
#endif
}

void tdemorecorder::saveposition()
{
    NotImplemented(NITag::NETWORK);
#if 0
    tmsg_serverspritedelta_movement movementmsg;

    movementmsg.header.id = msgid_delta_movement;

    movementmsg.num = mysprite;
    movementmsg.velocity = spriteparts.velocity[mysprite];
    movementmsg.pos = spriteparts.pos[mysprite];
    movementmsg.servertick = maintickcounter;

    encodekeys(sprite[mysprite], movementmsg.keys16);

    movementmsg.mouseaimx = sprite[mysprite].control.mouseaimx;
    movementmsg.mouseaimy = sprite[mysprite].control.mouseaimy;

    saverecord(&movementmsg, sizeof(movementmsg));
#endif
}
#endif

void tdemorecorder::savenextframe()
{
    if (!factive)
        return;

    // save record type
    rsize = 1;

    fdemofile.write1(&rsize, sizeof(rsize));

    // save camera change
#ifndef SERVER
    if (foldcam != camerafollowsprite)
    {
        savecamera();
        foldcam = camerafollowsprite;
    }
#endif

    fticksnum += 1;
}

#ifndef SERVER
bool tdemoplayer::opendemo(const string &filename)
{
    bool opendemo_result = false;

    //  try
    fdemofile.loadfromfile(filename);
    //  except
    //    on e: Exception do
    //    begin
    //      mainconsole.Console(_('Failed to load demo file: ') + WideString(E.Message),
    //      INFO_MESSAGE_COLOR); Exit;
    //    end;
    //  end;

    fdemofile.readbuffer(&fdemoheader, sizeof(fdemoheader));
    if (fdemoheader.header != demo_magic)
    {
        mainconsole.console(_("The provided file is not valid: ") + ' ' + (fname),
                            info_message_color);
        fdemofile.free();
    }
    else if (fdemoheader.version != demo_version)
    {
        mainconsole.console(
            _(wideformat("Wrong demo version: %d - %d", demo_version, fdemoheader.version)),
            info_message_color);
        fdemofile.free();
    }
    else
    {
        fname = extractfilename(filename);
        mainconsole.console(_("Playing demo") + ' ' + (fname), info_message_color);
        spectator = 1;
        factive = true;
        opendemo_result = true;
    }
    return opendemo_result;
}

void tdemoplayer::stopdemo()
{
    if (!factive)
        return;

    mainconsole.console("Demo stopped", info_message_color);

    fdemofile.free();

    factive = false;
}

void tdemoplayer::processdemo()
{
    NotImplemented(NITag::NETWORK);
#if 0
    std::array<char, 16384> readbuf;
    SteamNetworkingMessage_t* packet;

    do
    {
        if (!factive)
            return;

        if (fdemofile.position == fdemofile.size)
        {
            stopdemo();
            exittomenu();
            return;
        }

        //    try
        fdemofile.read1(&rsize, sizeof(rsize));
        //    except
        //      Exit;
        //    end;

        if ((fskipto > 0) && (maintickcounter >= fskipto))
        {
            fskipto = -1;
            shouldrenderframes = true;
            goalticks = round(CVar::demo_speed * default_goalticks);
        }

        if (rsize == 0)
        {
            fdemofile.position = fdemofile.position + 2;
            return;
        }

        if (rsize == 1) // next frame
            return;

        //    try
        fdemofile.read1(readbuf.data(), rsize);
        //    except
        //      Exit;
        //    end;

        packet = udp.networkingutil.allocatemessage(rsize);
        packet.m_pData = &readbuf;
        udp.handlemessages(packet);
        packet->m_pData = nullptr;
        packet.m_pfnrelease(packet);

    } while (!(rsize == 1));
#endif
}

void tdemoplayer::position(std::int32_t ticks)
{
    std::int32_t i;

    fskipto = ticks;
    shouldrenderframes = false;

    if (fskipto < maintickcounter)
    {
        fdemofile.seek(sizeof(fdemoheader), tmemorystream::sofrombeginning);

        maintickcounter = 0;

        for (i = 1; i <= max_sprites; i++)
            sprite[i].kill();
        for (i = 1; i <= max_bullets; i++)
            bullet[i].kill();
        for (i = 1; i <= max_sparks; i++)
            spark[i].kill();
        for (i = 1; i <= max_things; i++)
            thing[i].kill();

        // Reset World and Big Texts
        for (i = 0; i < max_big_messages; i++)
        {
            // Big Text
            bigtext[i] = "";
            bigdelay[i] = 0;
            bigscale[i] = 0;
            bigcolor[i] = 0;
            bigposx[i] = 0;
            bigposy[i] = 0;
            bigx[i] = 0;
            // World Text
            worldtext[i] = "";
            worlddelay[i] = 0;
            worldscale[i] = 0;
            worldcolor[i] = 0;
            worldposx[i] = 0;
            worldposy[i] = 0;
            worldx[i] = 0;
        }

        // Reset ABOVE CHAT MESSAGE
        for (i = 1; i <= max_sprites; i++)
        {
            chatdelay[i] = 0;
            chatmessage[i] = "";
            chatteam[i] = false;
        }

        mainconsole.count = 0;
        bigconsole.count = 0;
    }

    goalticks = default_goalticks * 20;
}
#endif

// initialization
// begin
//  DemoRecorder := TDemoRecorder.Create;
//  {$IFNDEF SERVER}
//  DemoPlayer := TDemoPlayer.Create;
//  {$ENDIF}
// end;

bool tmemorystream::writebuffer(const void *buff, int32_t size)
{
    NotImplemented(NITag::FILESYSTEM);
    return false;
}

bool tmemorystream::savetofile(const std::string &filename)
{
    NotImplemented(NITag::FILESYSTEM);
    return false;
}

void tmemorystream::write1(const void *buff, int32_t size)
{
    NotImplemented(NITag::FILESYSTEM);
}

void tmemorystream::read1(void *buff, int32_t size)
{
    NotImplemented(NITag::FILESYSTEM);
}

void tmemorystream::readbuffer(void *buff, int32_t size)
{
    NotImplemented(NITag::FILESYSTEM);
}

void tmemorystream::loadfromfile(const std::string &filename)
{
    NotImplemented(NITag::FILESYSTEM);
}

void tmemorystream::seek(int32_t bytes, tmemorystream::pos position)
{
    NotImplemented(NITag::FILESYSTEM);
}

void tmemorystream::free()
{
    NotImplemented(NITag::FILESYSTEM);
}
