// automatically converted

#include "Demo.hpp"

#ifdef SERVER
#include "../server/Server.hpp"
#else
#include "../client/Client.hpp"
#include "../client/ClientGame.hpp"
#include "../client/InterfaceGraphics.hpp"
#endif
#include "Cvar.hpp"
#include "common/GameStrings.hpp"
#include "common/MapFile.hpp"
#include "common/Util.hpp"
#include "common/Console.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Game.hpp"
#include "shared/Version.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <filesystem>

using string = std::string;

#ifndef SERVER
GlobalStateDemo gGlobalStateDemo{
  .demoplayer{},
};

#endif

namespace
{

auto extractfilename(const std::string &filepath) -> std::string
{
  return std::filesystem::path(filepath).stem();
}

std::uint64_t rsize;

} // namespace

template <std::size_t N>
void stringtoarray(std::array<char, N> &arr, const std::string &str)
{
  int i = 0;
  for (const auto &c : str)
  {
    arr[i++] = c;
  }
}
template <Config::Module M>
auto tdemorecorder<M>::startrecord(const string &filename) -> bool
{
  std::int32_t spriteid;

  bool result;
  result = false;
#ifndef SERVER
  if (gGlobalStateDemo.demoplayer.active())
  {
    return result;
  }
#endif

  GS::GetMainConsole().console(("Recording demo: ") + (extractfilename(filename)),
                               info_message_color);

  fname = extractfilename(filename);
  factive = true;
  foldcam = 255;
  fticksnum = 0;

  fdemoheader.header = {'S', 'O', 'L', 'D', 'E', 'M'};
  fdemoheader.ticksnum = 0;
  NotImplemented("No time for demo");
#if 0
    fdemoheader.startdate = datetimetounix(now);
#endif
  fdemoheader.version = demo_version;

  fillchar(fdemoheader.mapname.data(), sizeof(fdemoheader.mapname), '\0');
  stringtoarray(fdemoheader.mapname, GS::GetGame().GetMap().name);

  fdemofile.writebuffer(&fdemoheader, sizeof(fdemoheader));

  spriteid = createdemoplayer();

  if (spriteid == max_sprites)
  {
    result = true;
  }
  return result;
}

template <Config::Module M>
void tdemorecorder<M>::stoprecord()
{
  auto &sprite_system = SpriteSystem::Get();
  if (!active())
  {
    return;
  }

  GS::GetMainConsole().console(
#ifdef SERVER
    string("Demo stopped") +
#else
    _("Demo stopped") +
#endif
      " (" + (fname) + ')',
    info_message_color);

  sprite_system.GetSprite(max_sprites).kill();

  fdemofile.position = 0;

  fdemoheader.version = demo_version;
  fdemoheader.ticksnum = ticksnum();

  fillchar(fdemoheader.mapname.data(), sizeof(fdemoheader.mapname), '\0');
  stringtoarray(fdemoheader.mapname, GS::GetGame().GetMap().name);

  fdemofile.writebuffer(&fdemoheader, sizeof(fdemoheader));

  //  try
  fdemofile.savetofile(GS::GetGame().GetUserDirectory() + "demos/" + fname);
  //  except
  //    on e: Exception do
  //      GS::GetMainConsole().Console('Failed to save demo file: ' + WideString(E.Message),
  //      INFO_MESSAGE_COLOR);
  //  end;

  factive = false;
  fname = "";
  fdemofile.free();
}

template <Config::Module M>
auto tdemorecorder<M>::createdemoplayer() -> std::int32_t
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t p;
#if SERVER
  auto player = std::make_shared<TServerPlayer>();
#else
  auto player = std::make_shared<tplayer>();
#endif
  tvector2 a;

  std::int32_t createdemoplayer_result = -1;

  if (sprite_system.GetSprite(max_sprites).IsActive())
  {
    GS::GetMainConsole().console(
      "Failed to create Demo Recorder player. Demos can be recorded with up to 31 players",
      info_message_color);
    stoprecord();
    return createdemoplayer_result;
  }

  player->demoplayer = true;
  player->name = "Demo Recorder";
  player->team = team_spectator;
  player->controlmethod = human;

#ifdef SERVER
  player->peer = std::numeric_limits<std::uint32_t>::max();
#endif
  auto &map = GS::GetGame().GetMap();

  a.x = min_sectorz * map.GetSectorsDivision() * 0.7;
  a.y = min_sectorz * map.GetSectorsDivision() * 0.7;

  p = createsprite(a, max_sprites, player);
  if ((p > 0) && (p < max_sprites + 1))
  {
    NotImplemented("network");
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
        SpriteSystem::Get().GetSprite(p).player.demoplayer = true;
        auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(p);
        spritePartsPos = vector2(0, 0);
        createdemoplayer_result = p;
#endif
  }

  return createdemoplayer_result;
}

template <Config::Module M>
void tdemorecorder<M>::saverecord(const void *r, std::int32_t size)
{
  if (size == 0)
  {
    return;
  }

  if (!active())
  {
    return;
  }
  fdemofile.write1(&size, sizeof(rsize));
  fdemofile.write1(&r, size);
}

#ifndef SERVER
template <Config::Module M>
void tdemorecorder<M>::savecamera()
{
  NotImplemented("network");
#if 0
    tmsg_clientspritesnapshot_dead msg;

    msg.header.id = msgid_clientspritesnapshot_dead;
    msg.camerafocus = camerafollowsprite;
    saverecord(&msg, sizeof(msg));
#endif
}

template <Config::Module M>
void tdemorecorder<M>::saveposition()
{
  NotImplemented("network");
#if 0
    tmsg_serverspritedelta_movement movementmsg;

    movementmsg.header.id = msgid_delta_movement;

    auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(mysprite);
    auto &spriteVelocity = SpriteSystem::Get().GetVelocity(mysprite);
    movementmsg.num = mysprite;
    movementmsg.velocity = spriteVelocity;
    movementmsg.pos = spritePartsPos;
    movementmsg.servertick = GS::GetGame().GetMainTickCounter();

    encodekeys(SpriteSystem::Get().GetPlayerSprite(), movementmsg.keys16);

    movementmsg.mouseaimx = SpriteSystem::Get().GetPlayerSprite().control.mouseaimx;
    movementmsg.mouseaimy = SpriteSystem::Get().GetPlayerSprite().control.mouseaimy;

    saverecord(&movementmsg, sizeof(movementmsg));
#endif
}
#endif

template <Config::Module M>
void tdemorecorder<M>::savenextframe()
{
  if (!factive)
  {
    return;
  }

  // save record type
  rsize = 1;

  fdemofile.write1(&rsize, sizeof(rsize));

  // save camera change
#ifndef SERVER
  if (foldcam != gGlobalStateClient.camerafollowsprite)
  {
    savecamera();
    foldcam = gGlobalStateClient.camerafollowsprite;
  }
#endif

  fticksnum += 1;
}

#ifndef SERVER
auto tdemoplayer::opendemo(const string &filename) -> bool
{
  bool opendemo_result = false;

  //  try
  fdemofile.loadfromfile(filename);
  //  except
  //    on e: Exception do
  //    begin
  //      GS::GetMainConsole().Console(_('Failed to load demo file: ') + WideString(E.Message),
  //      INFO_MESSAGE_COLOR); Exit;
  //    end;
  //  end;

  fdemofile.readbuffer(&fdemoheader, sizeof(fdemoheader));
  if (fdemoheader.header != demo_magic)
  {
    GS::GetMainConsole().console(_("The provided file is not valid: ") + ' ' + (fname),
                                 info_message_color);
    fdemofile.free();
  }
  else if (fdemoheader.version != demo_version)
  {
    GS::GetMainConsole().console(
      _(wideformat("Wrong demo version: %d - %d", demo_version, fdemoheader.version)),
      info_message_color);
    fdemofile.free();
  }
  else
  {
    fname = extractfilename(filename);
    GS::GetMainConsole().console(_("Playing demo") + ' ' + (fname), info_message_color);
    gGlobalStateClient.spectator = 1;
    factive = true;
    opendemo_result = true;
  }
  return opendemo_result;
}

void tdemoplayer::stopdemo()
{
  if (!factive)
  {
    return;
  }

  GS::GetMainConsole().console("Demo stopped", info_message_color);

  fdemofile.free();

  factive = false;
}

void tdemoplayer::processdemo()
{
  NotImplemented("network");
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

        if ((fskipto > 0) && (GS::GetGame().GetMainTickCounter() >= fskipto))
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
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;

  fskipto = ticks;
  gGlobalStateClientGame.shouldrenderframes = false;

  if (fskipto < GS::GetGame().GetMainTickCounter())
  {
    fdemofile.seek(sizeof(fdemoheader), tmemorystream<>::sofrombeginning);

    GS::GetGame().ResetMainTickCounter();

    for (i = 1; i <= max_sprites; i++)
    {
      sprite_system.GetSprite(i).kill();
    }
    GS::GetBulletSystem().KillAll();
    for (i = 1; i <= max_sparks; i++)
    {
      gGlobalStateGame.spark[i].kill();
    }
    GS::GetThingSystem().KillAll();

    // Reset World and Big Texts
    for (i = 0; i < max_big_messages; i++)
    {
      // Big Text
      gGlobalStateInterfaceGraphics.bigtext[i] = "";
      gGlobalStateInterfaceGraphics.bigdelay[i] = 0;
      gGlobalStateInterfaceGraphics.bigscale[i] = 0;
      gGlobalStateInterfaceGraphics.bigcolor[i] = 0;
      gGlobalStateInterfaceGraphics.bigposx[i] = 0;
      gGlobalStateInterfaceGraphics.bigposy[i] = 0;
      gGlobalStateInterfaceGraphics.bigx[i] = 0;
      // World Text
      gGlobalStateInterfaceGraphics.worldtext[i] = "";
      gGlobalStateInterfaceGraphics.worlddelay[i] = 0;
      gGlobalStateInterfaceGraphics.worldscale[i] = 0;
      gGlobalStateInterfaceGraphics.worldcolor[i] = 0;
      gGlobalStateInterfaceGraphics.worldposx[i] = 0;
      gGlobalStateInterfaceGraphics.worldposy[i] = 0;
      gGlobalStateInterfaceGraphics.worldx[i] = 0;
    }

    // Reset ABOVE CHAT MESSAGE
    for (i = 1; i <= max_sprites; i++)
    {
      gGlobalStateInterfaceGraphics.chatdelay[i] = 0;
      gGlobalStateInterfaceGraphics.chatmessage[i] = "";
      gGlobalStateInterfaceGraphics.chatteam[i] = false;
    }

    GS::GetMainConsole().ResetCount();
    GetBigConsole().ResetCount();
  }

  GS::GetGame().SetGoalTicks(default_goalticks * 20);
}
#endif

// initialization
// begin
//  DemoRecorder := TDemoRecorder.Create;
//  {$IFNDEF SERVER}
//  DemoPlayer := TDemoPlayer.Create;
//  {$ENDIF}
// end;

template <Config::Module M>
auto tmemorystream<M>::writebuffer(const void *buff, int32_t size) -> bool
{
  NotImplemented("filesystem");
  return false;
}

template <Config::Module M>
auto tmemorystream<M>::savetofile(const std::string &filename) -> bool
{
  NotImplemented("filesystem");
  return false;
}

template <Config::Module M>
void tmemorystream<M>::write1(const void *buff, int32_t size)
{
  NotImplemented("filesystem");
}

template <Config::Module M>
void tmemorystream<M>::read1(void *buff, int32_t size)
{
  NotImplemented("filesystem");
}

template <Config::Module M>
void tmemorystream<M>::readbuffer(void *buff, int32_t size)
{
  NotImplemented("filesystem");
}

template <Config::Module M>
void tmemorystream<M>::loadfromfile(const std::string &filename)
{
  NotImplemented("filesystem");
}

template <Config::Module M>
void tmemorystream<M>::seek(int32_t bytes, tmemorystream::pos position)
{
  NotImplemented("filesystem");
}

template <Config::Module M>
void tmemorystream<M>::free()
{
  NotImplemented("filesystem");
}

template struct tmemorystream<>;
template class tdemorecorder<>;
template class tdemo<>;
