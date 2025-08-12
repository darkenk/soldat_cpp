
/*#include "idcompilerdefines.inc"*/
// #define DEVELOPMENT

#include "Client.hpp"

#include <Tracy.hpp>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_messagebox.h>
#include <SDL3/SDL_video.h>
#include <math.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <thread>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <memory>
#include <new>
#include <ratio>
#include <utility>
#include <vector>

#include "ClientCommands.hpp"
#include "ClientGame.hpp"
#include "GameMenus.hpp"
#include "GameRendering.hpp"
#include "Gfx.hpp"
#include "Input.hpp"
#include "InterfaceGraphics.hpp"
#include "Sound.hpp"
#include "common/FileUtility.hpp"
#include "common/GameStrings.hpp"
#include "common/Logging.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/SHA1Helper.hpp"
#include "common/misc/TFileStream.hpp"
#include "common/misc/TIniFile.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/Console.hpp"
#include "common/LogFile.hpp"
#include "shared/Command.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClient.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include "common/PolyMap.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/misc/SafeType.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/AnimationSystem.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Sparks.hpp"
#include "shared/network/Net.hpp"

namespace
{
bool progready;


Console sBigConsole;
ConsoleMain sKillConsole;

auto InitBigConsole(FileUtility *filesystem, const std::int32_t newMessageWait,
                    const std::int32_t countMax, const std::int32_t scrollTickMax) -> Console &
{
  return *new (&sBigConsole) Console(filesystem, newMessageWait, countMax, scrollTickMax);
}

auto InitKillConsole(FileUtility *filesystem, const std::int32_t newMessageWait,
                     const std::int32_t countMax, const std::int32_t scrollTickMax) -> ConsoleMain &
{
  return *new (&sKillConsole) ConsoleMain(filesystem, newMessageWait, countMax, scrollTickMax);
}

} // namespace

auto GetBigConsole() -> Console & { return sBigConsole; }

auto GetKillConsole() -> ConsoleMain & { return sKillConsole; }

// Client.cpp variables
static bool gamelooprun;

GlobalStateClient gGlobalStateClient{
  .joinpassword{},
  .joinport = "23073",
  .joinip = "127.0.0.1",
  .basedirectory{},
  .moddir{},
  .usesservermod{},
  .gClientServerIP = "127.0.0.1",
  .gClientServerPort = 23073,
  .connection = INTERNET,
  .sniperline_client_hpp = 0,
  .trails = 1,
  .spectator = 0,
  .packetadjusting = 1,
  .limbolock{},
  .selteam{},
  .mysprite{},
  .wepstats{},
  .wepstatsnum = 0,
  .gundisplayname{},
  .gamethingtarget{},
  .grenadeeffecttimer = 0,
  .badmapidcount{},
  .abnormalterminate = false,
  .hwid{},
  .hitspraycounter{},
  .screentaken{},
  .targetmode = false,
  .muteall = false,
  .redirecttoserver = false,
  .redirectip{},
  .redirectport{},
  .redirectmsg{},
  .radiomenu{},
  .rmenustate{},
  .showradiomenu = false,
  .radiocooldown = 3,
  .cameraprev{},
  .camerax{},
  .cameray{},
  .camerafollowsprite{},
  .notexts = 0,
  .freecam = 0,
  .shotdistanceshow{},
  .shotdistance{},
  .shotlife{},
  .shotricochet{},
  .gClient{},
};

// bullet shot stats

enum class GameState
{
  Loading,
  Game,
  ConnectionTimedOut
};

static GameState gGameState{GameState::Loading};

// End Client.cpp variables

void restartgraph()
{
  dotextureloading(true);

  auto &map = GS::GetGame().GetMap();

  // Load Map
  map.loadmap(GS::GetFileSystem(), GS::GetGame().GetMapchange(), CVar::r_forcebg, CVar::r_forcebg_color1,
              CVar::r_forcebg_color2);

  if (!gGlobalStateGameMenus.escmenu->active)
  {
    gGlobalStateClientGame.mx = gGlobalStateGame.gamewidthhalf;
    gGlobalStateClientGame.my = gGlobalStateGame.gameheighthalf;
    gGlobalStateClientGame.mouseprev.x = gGlobalStateClientGame.mx;
    gGlobalStateClientGame.mouseprev.y = gGlobalStateClientGame.my;
  }

  GS::GetMainConsole().console(("Graphics restart"), debug_message_color);
}

void Client::loadweaponnames(FileUtility& fs, GunArray& gunDisplayName, const std::string& modDir)
{
  SoldatAssert(gunDisplayName.size() == double_weapons);
  std::int32_t i;

  const std::string weaponNamesFile = modDir + "txt/weaponnames.txt";

  //GS::GetMainConsole().console(std::string("Loading Weapon Names from ") + weaponNamesFile, debug_message_color);
  NotImplemented("console");
  if (!fs.Exists((weaponNamesFile)))
  {
    return;
  }
  std::unique_ptr<std::byte[]> buff;
  std::size_t fileSize = 0;
  {
    auto *f = fs.Open(weaponNamesFile, FileUtility::FileMode::Read);
    fileSize = FileUtility::Size(f);
    buff = std::make_unique<std::byte[]>(fileSize);
    FileUtility::Read(f, buff.get(), fileSize);
    FileUtility::Close(f);
  }
#if __EMSCRIPTEN__
  std::istringstream sd(std::string(reinterpret_cast<char*>(buff.get()), fileSize));
#else
  std::istringstream sd;
  sd.rdbuf()->pubsetbuf(reinterpret_cast<char*>(buff.get()), fileSize);
#endif
  for (i = 0; i < double_weapons; i++)
  {
    SoldatAssert(sd.good());
    std::getline(sd, gunDisplayName[weaponnumexternaltointernal(i)]);
  }
}

void redirectdialog()
{
  std::array<SDL_MessageBoxButtonData, 2> buttons;
  SDL_MessageBoxData data;
  std::int32_t response;

  rendergameinfo("Server Redirect");
  buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
  buttons[0].buttonID = 0;
  buttons[0].text = "Yes";
  buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
  buttons[1].buttonID = 1;
  buttons[1].text = "No";

  data.flags = 0;
  data.window = gGlobalStateInput.gamewindow;
  data.title = "Server Redirect";
  auto msg =
    (gGlobalStateClient.redirectmsg + "\r\n\r\nRedirect to server " +
     gGlobalStateClient.redirectip + ":" + inttostr(gGlobalStateClient.redirectport) + "?");
  data.message = msg.c_str();
  data.numbuttons = 2;
  data.buttons = &buttons.at(0);
  data.colorScheme = nullptr;

  if (SDL_ShowMessageBox(&data, &response) != 0)
  {
    return;
  }

  gGlobalStateClient.redirecttoserver = false;

  if (response == 0)
  {
    gGlobalStateClient.joinip = gGlobalStateClient.redirectip;
    gGlobalStateClient.joinport = inttostr(gGlobalStateClient.redirectport);
    gGlobalStateClient.gClient.joinserver();
  }
  else
  {
    gGlobalStateClient.redirectip = "";
    gGlobalStateClient.redirectport = 0;
    gGlobalStateClient.redirectmsg = "";
    gGlobalStateClient.gClient.exittomenu();
  }
}

void Client::exittomenu()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;

  GS::GetGame().ResetGoalTicks();

  // Reset network state and show the status std::string (if any)
  // ShouldRenderFrames := False;
  // NetEncActive := False;

  // resetsynccvars;

  if (GS::GetDemoRecorder().active())
  {
    GS::GetDemoRecorder().stoprecord();
  }

  if (gGlobalStateDemo.demoplayer.active())
  {
    gGlobalStateDemo.demoplayer.stopdemo();
  }

  if (sprite_system.IsPlayerSpriteValid())
  {
    clientdisconnect(*GetNetwork());
  }
  if (GetNetwork() != nullptr)
  {
    GetNetwork()->Disconnect(true);
  }

  stopsound(channel_weather);

  auto &map = GS::GetGame().GetMap();

  map.name = "";

  if (gGlobalStateGameMenus.escmenu != nullptr)
  {
    gamemenushow(gGlobalStateGameMenus.escmenu, false);
  }

  map.filename = ""; // force reloading next time
  GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
  // WindowReady := False;

  auto &activeSprites = sprite_system.GetActiveSprites();

  std::for_each(std::begin(activeSprites), std::end(activeSprites),
                [](auto &sprite) { sprite.kill(); });
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
  for (i = 1; i < max_sprites; i++)
  {
    gGlobalStateInterfaceGraphics.chatdelay[i] = 0;
    gGlobalStateInterfaceGraphics.chatmessage[i] = "";
    gGlobalStateInterfaceGraphics.chatteam[i] = false;
  }

  gGlobalStateClient.mysprite = 0;
  gGlobalStateClient.camerafollowsprite = 0;
  gGlobalStateClient.gamethingtarget = 0;

  if (gGlobalStateClient.redirecttoserver)
  {
    redirectdialog();
  }
}

static void CreateDirectoryStructure(FileUtility &fs)
{
  SoldatEnsure(fs.MkDir("/user/configs"));
  SoldatEnsure(fs.MkDir("/user/screens"));
  SoldatEnsure(fs.MkDir("/user/demos"));
  SoldatEnsure(fs.MkDir("/user/logs"));
  SoldatEnsure(fs.MkDir("/user/logs/kills"));
  SoldatEnsure(fs.MkDir("/user/maps"));
  SoldatEnsure(fs.MkDir("/user/mods"));
}

static auto MountAssets(FileUtility &fu, const std::string &userdirectory,
                        const std::string &basedirectory, tsha1digest &outGameModChecksum,
                        tsha1digest &outCustomModChecksum) -> bool
{
  LogDebugG("[FS] Mounting game archive");
  if (CVar::fs_localmount)
  {
    if (!fu.Mount(userdirectory, "/"))
    {
      showmessage(("Could not load base game archive (game directory)."));
      return false;
    }
  }
  else
  {
    if (!fu.Mount(basedirectory + "/soldat.smod", "/"))
    {
      showmessage(("Could not load base game archive (soldat.smod). Try to reinstall the game."));
      return false;
    }

    outGameModChecksum = sha1file(basedirectory + "/soldat.smod");
  }
  gGlobalStateClient.moddir = "";
  if (CVar::fs_mod != "")
  {
    LogDebugG("[FS] Mounting mods/{}.smod", lowercase(CVar::fs_mod));
    if (!fu.Mount((userdirectory + "mods/" + lowercase(CVar::fs_mod) + ".smod"),
                  (std::string("mods/") + lowercase(CVar::fs_mod) + "/")))
    {
      showmessage((std::string("Could not load mod archive (") + std::string(CVar::fs_mod) + ")."));
      return false;
    }
    gGlobalStateClient.moddir = std::string("/mods/") + lowercase(CVar::fs_mod) + '/';
    outCustomModChecksum = sha1file(userdirectory + "mods/" + lowercase(CVar::fs_mod) + ".smod");
  }
  return true;
}

// TODO: throw away test variable
static void InitConsoles(bool test = false)
{
  // Create Consoles
  auto console = std::make_unique<ConsoleMain>(
    &GS::GetFileSystem(), 150,
    round(CVar::ui_console_length * gGlobalStateInterfaceGraphics._rscala.y), 150);
  GS::SetMainConsole(std::move(console));

  auto countMax = floor((0.85 * gGlobalStateClientGame.renderheight) /
                        (CVar::font_consolelineheight * fontstylesize(font_small)));
  if (test)
  {
    countMax = 20;
  }

  InitBigConsole(&GS::GetFileSystem(),0, countMax, 1500000);
  GS::GetMainConsole().SetBigConsole(&GetBigConsole());

  InitKillConsole(&GS::GetFileSystem(), 70,
                  round(CVar::ui_killconsole_length * gGlobalStateInterfaceGraphics._rscala.y),
                  240);
}

void Client::startgame(int argc, char *argv[])
{
  initclientcommands();
  commandinit();

  parsecommandline(argc, argv);
  if (argc == 1)
  {
    parseinput("join 127.0.0.1 23073");
  }

  auto &fs = GS::GetFileSystem();
  const auto userDirectory = FileUtility::GetPrefPath("client");
  const auto baseDirectory = FileUtility::GetBasePath();

  LogDebugG("[FS] userDirectory: {}", userDirectory);
  LogDebugG("[FS] baseDirectory: {}", baseDirectory);

  fs.Mount(userDirectory, "/user");

  // Create the basic folder structure
  CreateDirectoryStructure(fs);

  {
    tsha1digest gameSha1;
    tsha1digest modSha1;

    if (!MountAssets(fs, userDirectory, baseDirectory, gameSha1, modSha1))
    {
      SoldatAssert(false);
      return;
    }
    GS::GetGame().SetCustomModChecksum(modSha1);
    GS::GetGame().SetGameModChecksum(gameSha1);
  }

  GS::GetConsoleLogFile().Enable(CVar::log_enable);
  GS::GetConsoleLogFile().SetLogLevel(CVar::log_level);
  GS::GetConsoleLogFile().Init("/user/logs/consolelog");

  std::string systemlang = "en_US";
  // todo this variable is needed when code is refactored
  const std::string systemfallbacklang = "en_US"; // NOLINT

  // TODO remove HWIDs, replace by Fae auth tickets
  gGlobalStateClient.hwid = "00000000000";

  LogDebugG("[FS] Initializing system");

  loadinterfacearchives(userDirectory + "/custom-interfaces/");

  fs.Copy("/configs/bindings.cfg", "/user/configs/bindings.cfg");
  fs.Copy("/configs/client.cfg", "/user/configs/client.cfg");
  fs.Copy("/configs/controls.cfg", "/user/configs/controls.cfg");
  fs.Copy("/configs/game.cfg", "/user/configs/game.cfg");
  fs.Copy("/configs/graphics.cfg", "/user/configs/graphics.cfg");
  fs.Copy("/configs/player.cfg", "/user/configs/player.cfg");
  fs.Copy("/configs/sound.cfg", "/user/configs/sound.cfg");

  loadconfig("client.cfg", fs);

  // these might change so keep a backup to avoid changing the settings file
  gGlobalStateClientGame.screenwidth = CVar::r_screenwidth;
  gGlobalStateClientGame.screenheight = CVar::r_screenheight;
  gGlobalStateClientGame.renderheight = CVar::r_renderheight;
  gGlobalStateClientGame.renderwidth = CVar::r_renderwidth;

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  SDL_DisplayID display = SDL_GetPrimaryDisplay();
  const SDL_DisplayMode* currentdisplay = SDL_GetCurrentDisplayMode(display);

  if ((gGlobalStateClientGame.screenwidth == 0) || (gGlobalStateClientGame.screenheight == 0))
  {
    gGlobalStateClientGame.screenwidth = currentdisplay->w;
    gGlobalStateClientGame.screenheight = currentdisplay->h;
  }

  if ((gGlobalStateClientGame.renderwidth == 0) || (gGlobalStateClientGame.renderheight == 0))
  {
    gGlobalStateClientGame.renderwidth = gGlobalStateClientGame.screenwidth;
    gGlobalStateClientGame.renderheight = gGlobalStateClientGame.screenheight;
  }

  // Calculcate FOV to check for too high/low vision
  float fov = (float)(gGlobalStateClientGame.renderwidth) / gGlobalStateClientGame.renderheight;
  if (fov > max_fov)
  {
    gGlobalStateClientGame.renderwidth = ceil(gGlobalStateClientGame.renderheight * max_fov);
    fov = max_fov;
  }
  else if (fov < min_fov)
  {
    gGlobalStateClientGame.renderheight =
      ceil((float)(gGlobalStateClientGame.renderwidth) / min_fov);
    fov = min_fov;
  }

  // Calulcate internal game width based on the fov and internal height
  gGlobalStateGame.gamewidth = round(fov * gGlobalStateGame.gameheight);
  gGlobalStateGame.gamewidthhalf = (float)(gGlobalStateGame.gamewidth) / 2;
  gGlobalStateGame.gameheighthalf = (float)(gGlobalStateGame.gameheight) / 2;

  if (CVar::r_fullscreen == 0)
  {
    // avoid black bars in windowed mode
    if (((float)(gGlobalStateClientGame.screenwidth) / gGlobalStateClientGame.screenheight) >=
        ((float)(gGlobalStateClientGame.renderwidth) / gGlobalStateClientGame.renderheight))
    {
      gGlobalStateClientGame.screenwidth =
        round(gGlobalStateClientGame.screenheight *
              ((float)(gGlobalStateClientGame.renderwidth) / gGlobalStateClientGame.renderheight));
    }
    else
    {
      gGlobalStateClientGame.screenheight =
        round(gGlobalStateClientGame.screenwidth *
              ((float)(gGlobalStateClientGame.renderheight) / gGlobalStateClientGame.renderwidth));
    }
  }

  // window size equals "screen" size except in windowed fullscreen
  gGlobalStateClientGame.windowwidth = gGlobalStateClientGame.screenwidth;
  gGlobalStateClientGame.windowheight = gGlobalStateClientGame.screenheight;

  LogInfo("gfx", "Window size: {}x{}", gGlobalStateClientGame.windowwidth,
          gGlobalStateClientGame.windowheight);
  LogInfo("gfx", "Target resolution: {}x{}", gGlobalStateClientGame.screenwidth,
          gGlobalStateClientGame.screenheight);
  LogInfo("gfx", "Internal resolution: {}x{}", gGlobalStateClientGame.renderwidth,
          gGlobalStateClientGame.renderheight);

  // even windowed mode can behave as fullscreen with the right size
  // IsFullscreen := (WindowWidth = Screen.Width) and (WindowHeight = Screen.Height);

  // interface is hard-coded to work on 4:3 aspect ratio,
  // but luckily for us the interface rendering code
  // translates the points using _RScala scale factor
  // above, so all we floatly need to do to make interace
  // work for widescreens is translate those points to a wider
  // area, which we can do by using the 640/480 as scale factors
  // even in widescreen resolutions. The interface code does NOT
  // use the _RScala to scale the interface, so this won't make
  // it look distorted.
  if (CVar::r_scaleinterface)
  {
    gGlobalStateInterfaceGraphics._rscala.x = 1;
    gGlobalStateInterfaceGraphics._rscala.y = 1;

    gGlobalStateInterfaceGraphics._iscala.x = (float)(gGlobalStateGame.gamewidth) / default_width;
    gGlobalStateInterfaceGraphics._iscala.y = 1;

    gGlobalStateInterfaceGraphics.fragx = floor(gGlobalStateGame.gamewidthhalf - 300) - 25;
  }
  else
  {
    gGlobalStateInterfaceGraphics._rscala.x =
      (float)(gGlobalStateClientGame.renderwidth) / gGlobalStateGame.gamewidth;
    gGlobalStateInterfaceGraphics._rscala.y =
      (float)(gGlobalStateClientGame.renderheight) / gGlobalStateGame.gameheight;

    gGlobalStateInterfaceGraphics._iscala.x = (float)(gGlobalStateClientGame.renderwidth) / 640;
    gGlobalStateInterfaceGraphics._iscala.y = (float)(gGlobalStateClientGame.renderheight) / 480;

    gGlobalStateInterfaceGraphics.fragx =
      floor((float)(gGlobalStateClientGame.renderwidth) / 2 - 300) - 25;

    if (gGlobalStateClientGame.renderheight > gGlobalStateGame.gameheight)
    {
      gGlobalStateInterfaceGraphics.fragy = round(10 * gGlobalStateInterfaceGraphics._rscala.y);
    }
  }

  gGlobalStateGameRendering.gamerenderingparams.interfacename = CVar::ui_style;

  resetframetiming();

  gfxlog("Loading game graphics");

  if (!initgamegraphics())
  {
    showmessage(std::string("The required OpenGL functionality isn't supported. ") +
                "Please, update your video drivers and try again.");
    // ExitButtonClick(nullptr);
    return;
  }

  if (CVar::cl_lang != "")
  {
    systemlang = CVar::cl_lang;
  }
  else
  {
    NotImplemented("localization");
#if 0
        getlanguageids(systemlang, systemfallbacklang);
#endif
  }

  if (inittranslation(
        ReadAsFileStream(fs, gGlobalStateClient.moddir + "/txt/" + systemlang + ".mo").get()))
  {
    LogDebugG("Game captions loaded from {}/txt/{}", gGlobalStateClient.moddir, systemlang);
  }
  else
  {
    LogDebugG("Game captions not found");
  }

  GS::GetConsoleLogFile().Log("Initializing Sound Library.");
  // Init Sound Library
  if (!initsound())
  {
    GS::GetConsoleLogFile().Log("Failed to initialize Sound Library.");
    // Let the player know that he has no sound (no popup window)
  }

  loadsounds("");
  if (length(gGlobalStateClient.moddir) > 0)
  {
    loadsounds(gGlobalStateClient.moddir);
  }

  GS::GetConsoleLogFile().Log("Creating network interface.");

  InitConsoles();
  // Create static player objects
  for (auto &s : SpriteSystem::Get().GetSprites())
  {
    s.player = std::make_shared<tplayer>();
  }

  AnimationSystem::Get().LoadAnimObjects("");
  if (length(gGlobalStateClient.moddir) > 0)
  {
    AnimationSystem::Get().LoadAnimObjects(gGlobalStateClient.moddir);
  }

  // greet!
  // GS::GetMainConsole().console(("Welcome to Soldat ") + soldat_version, default_message_color);
  GS::GetMainConsole().console(("Welcome to Soldat "), default_message_color);

  // Load weapon display names
  loadweaponnames(fs, gGlobalStateClient.gundisplayname, gGlobalStateClient.moddir);
  createweaponsbase(GS::GetWeaponSystem().GetGuns());

  GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);

  gGlobalStateInterfaceGraphics.playernamesshow = true;

  gGlobalStateInterfaceGraphics.cursortext = "";

  initgamemenus();

  {
    TIniFile ini{ReadAsFileStream(fs, "txt/radiomenu-default.ini")};
    ini.ReadSectionValues("OPTIONS", gGlobalStateClient.radiomenu);
  }

  // Play demo
  gGlobalStateClient.freecam = 1;
  gGlobalStateClient.notexts = 0;
  gGlobalStateClient.shotdistanceshow = -1;

  if (CVar::r_compatibility)
  {
    CVar::cl_actionsnap = false;
  }

  GS::GetConsoleLogFile().WriteToFile();


  resetframetiming();
  initgamegraphics();
  dotextureloading(true);

  InitClientNetwork();
  GetNetwork()->SetDisconnectionCallback([](const char* msg){rendergameinfo(std::string("Network  error ") + msg);});
  GetNetwork()->SetConnectionCallback(
    [](NetworkClientImpl &nc) { clientrequestgame(nc, gGlobalStateClient.joinpassword); });

  gamelooprun = true;
  rundeferredcommands();
  // void startgameloop();
  // startgameloop();
}

void Client::shutdown()
{
  exittomenu();

  if (gGlobalStateClient.abnormalterminate)
  {
    return;
  }

  GS::GetConsoleLogFile().Log("Freeing sprites.");

  // Free GFX
  destroygamegraphics();

  for (auto &s : SpriteSystem::Get().GetSprites())
  {
    s.player = nullptr;
  }

  deinittranslation();

  GS::GetConsoleLogFile().Log("UDP closing.");

  DeinitClientNetwork();

  GS::GetConsoleLogFile().Log("Sound closing.");

  closesound();
  SDL_Quit();

  GS::GetConsoleLogFile().Log("FS closing.");

  commanddeinit();

  GS::GetConsoleLogFile().Log("   End of Log.");

  GS::GetConsoleLogFile().WriteToFile();

  gamelooprun = false;
}

bool Client::mainloop()
{
  if (!gamelooprun)
  {
    return gamelooprun;
  }
  auto begin = std::chrono::system_clock::now();
  GetNetwork()->ProcessLoop();
  //gameinput();
  switch (gGameState)
  {
    case GameState::Loading:
      rendergameinfo(("Loading"));
      break;
    case GameState::Game:
      if (progready)
      {
        gameloop();
      }
      break;
    case GameState::ConnectionTimedOut:
      rendergameinfo(("Connection timed out."));
      break;
  }
  auto end = std::chrono::system_clock::now();
  constexpr auto frameTime = std::chrono::seconds(1) / 60.f;
  {
    ZoneScopedN("WaitingForNextFrame");
    std::this_thread::sleep_for(frameTime - (end - begin));
  }
  FrameMarkNamed("ClientFrame");
  return gamelooprun;
}

#if __EMSCRIPTEN__
#include <emscripten.h>
#endif

void startgameloop()
{
#if __EMSCRIPTEN__
  emscripten_set_main_loop(loop, 30, 1);
#else
  while (gamelooprun)
  {
    gGlobalStateClient.gClient.mainloop();
  }
#endif
}

void Client::joinserver()
{
  resetframetiming();

  gGlobalStateClient.gClientServerIP = trim(gGlobalStateClient.joinip);

  NotImplemented("No error checking");
#if 0
    if (!trystrtoint(trim(joinport), serverport))
        return;
#endif

  // DEMO
  if (gGlobalStateClient.joinport == "0")
  {
    gGlobalStateDemo.demoplayer.opendemo(GS::GetGame().GetUserDirectory() + "demos/" +
                                         gGlobalStateClient.joinip + ".sdm");
    tdemoplayer::processdemo();
    progready = true;
    gamelooprun = true;
    rendergameinfo(("Loading"));
    startgameloop();
  }
  else
  {
    rendergameinfo(("Connecting to " + gGlobalStateClient.gClientServerIP + ":" +
                    std::to_string(gGlobalStateClient.gClientServerPort)));

    if (GetNetwork()->Connect(gGlobalStateClient.gClientServerIP,
                              gGlobalStateClient.gClientServerPort))
    {
      progready = true;
      gamelooprun = true;
      rendergameinfo(("Loading"));
      clientrequestgame(*GetNetwork(), gGlobalStateClient.joinpassword);
      gGameState = GameState::Game;
    }
    else
    {
      GS::GetMainConsole().console("[NET] Failed to connect to  server" +
                                     GetNetwork()->GetStringAddress(true),
                                   warning_message_color);
      return;
    }
  }
}

void showmessage(const std::string &message)
{
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message.c_str(), nullptr);
};

// tests
#include <doctest/doctest.h>
#include <fstream>
#include <filesystem>

namespace
{

class ClientFixture
{
public:
  ClientFixture() = default;
  ~ClientFixture() = default;
  ClientFixture(const ClientFixture &) = delete;

protected:
};

TEST_SUITE("Client")
{

  TEST_CASE_FIXTURE(ClientFixture, "Mount memory and write file and later read it")
  {
    FileUtility fu;
    fu.Mount("tmpfs.memory", "/user");
    CreateDirectoryStructure(fu);
    auto *f = fu.Open("/user/logs/nice_log.txt", FileUtility::FileMode::Write);
    CHECK_NE(nullptr, f);
    FileUtility::Close(f);
  }

  TEST_CASE_FIXTURE(ClientFixture, "Mount soldat.smod test")
  {
    // test soldat.smod, generated with xxd --include soldat.smod
    // contains:
    // client_test.txt
    // server_test.txt
    // shared_test.txt
    // NOLINTBEGIN
    unsigned char soldat_smod[] = {
      0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0x24, 0x33,
      0x50, 0xf5, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x63, 0x6c,
      0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x2b, 0x49, 0x2d,
      0x2e, 0x89, 0x4f, 0xce, 0xc9, 0x4c, 0xcd, 0x2b, 0xe1, 0x02, 0x00, 0x50, 0x4b, 0x03, 0x04, 0x14,
      0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xa7, 0xe8, 0x12, 0xba, 0x0e, 0x00, 0x00,
      0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x5f,
      0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x2b, 0x49, 0x2d, 0x2e, 0x89, 0x2f, 0x4e, 0x2d,
      0x2a, 0x4b, 0x2d, 0xe2, 0x02, 0x00, 0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00,
      0x00, 0x00, 0x21, 0x00, 0xab, 0x34, 0x36, 0xb2, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00,
      0x0f, 0x00, 0x00, 0x00, 0x73, 0x68, 0x61, 0x72, 0x65, 0x64, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e,
      0x74, 0x78, 0x74, 0x2b, 0x49, 0x2d, 0x2e, 0x89, 0x2f, 0xce, 0x48, 0x2c, 0x4a, 0x4d, 0xe1, 0x02,
      0x00, 0x50, 0x4b, 0x01, 0x02, 0x14, 0x0a, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21,
      0x00, 0x24, 0x33, 0x50, 0xf5, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63,
      0x6c, 0x69, 0x65, 0x6e, 0x74, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x50, 0x4b,
      0x01, 0x02, 0x14, 0x0a, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xa7, 0xe8,
      0x12, 0xba, 0x0e, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x00, 0x00, 0x73, 0x65, 0x72, 0x76,
      0x65, 0x72, 0x5f, 0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x50, 0x4b, 0x01, 0x02, 0x14,
      0x0a, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x21, 0x00, 0xab, 0x34, 0x36, 0xb2, 0x0e,
      0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x00, 0x00, 0x00, 0x73, 0x68, 0x61, 0x72, 0x65, 0x64, 0x5f,
      0x74, 0x65, 0x73, 0x74, 0x2e, 0x74, 0x78, 0x74, 0x50, 0x4b, 0x05, 0x06, 0x00, 0x00, 0x00, 0x00,
      0x03, 0x00, 0x03, 0x00, 0xb7, 0x00, 0x00, 0x00, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned int soldat_smod_len = 382;
    // NOLINTEND

    FileUtility fu;
    auto testDir = FileUtility::GetPrefPath("mount_test", true);
    std::filesystem::remove_all(testDir);
    // recreate directory
    testDir = FileUtility::GetPrefPath("mount_test", true);
    {
      std::ofstream s(testDir + "/soldat.smod", std::ios_base::binary | std::ios_base::trunc);
      s.write((char*)soldat_smod, soldat_smod_len);
    }
    tsha1digest customMod;
    tsha1digest mod;
    CHECK_EQ(true, MountAssets(fu, "", testDir, mod, customMod));
    //std::filesystem::remove_all(testDir);
  }

  TEST_CASE_FIXTURE(ClientFixture, "loadweaponnamesRefactorToUseVirtualFileSystem")
  {
    FileUtility fs;
    GunArray ga;
    const auto userDirectory = FileUtility::GetPrefPath("client");
    const auto baseDirectory = FileUtility::GetBasePath();
    tsha1digest checksum1;
    tsha1digest checksum2;
    auto ret = MountAssets(fs, userDirectory, baseDirectory, checksum1, checksum2);
    CHECK_EQ(true, ret);
    gGlobalStateClient.gClient.loadweaponnames(fs, ga, gGlobalStateClient.moddir);
    CHECK_EQ("USSOCOM", ga[0]);
    CHECK_EQ("Desert Eagles", ga[1]);
    CHECK_EQ("HK MP5", ga[2]);
    CHECK_EQ("Ak-74", ga[3]);
    CHECK_EQ("Steyr AUG", ga[4]);
    CHECK_EQ("Spas-12", ga[5]);
    CHECK_EQ("Ruger 77", ga[6]);
    CHECK_EQ("M79", ga[7]);
    CHECK_EQ("Barrett M82A1", ga[8]);
    CHECK_EQ("FN Minimi", ga[9]);
    CHECK_EQ("XM214 Minigun", ga[10]);
    CHECK_EQ("Combat Knife", ga[11]);
    CHECK_EQ("Chainsaw", ga[12]);
    CHECK_EQ("M72 LAW", ga[13]);
    CHECK_EQ("Flamer", ga[14]);
    CHECK_EQ("Rambo Bow", ga[15]);
    CHECK_EQ("Flamed Arrows", ga[16]);
  }

  TEST_CASE_FIXTURE(ClientFixture, "Test console initialization")
  {
    GlobalSystems<Config::CLIENT_MODULE>::Init();
    auto prev_y = gGlobalStateInterfaceGraphics._rscala.y;
    gGlobalStateInterfaceGraphics._rscala.y = 1;
    InitConsoles(true);
    gGlobalStateInterfaceGraphics._rscala.y = prev_y;
    const auto& console = GS::GetMainConsole();
    //CHECK_EQ(0, console.countmax);
    //CHECK_EQ(150, console.scrolltickmax);
    CHECK_EQ(150, console.GetNewMessageWait());
    CHECK_EQ(0, console.GetCount());

    const auto& big = GetBigConsole();
    //CHECK_EQ(0, big.countmax); todo countmax in tests
    //CHECK_EQ(1500000, big.scrolltickmax);
    CHECK_EQ(0, big.GetNewMessageWait());
    CHECK_EQ(0, big.GetCount());

    const auto& kill = GetKillConsole();
    //CHECK_EQ(0, kill.countmax);
    //CHECK_EQ(240, kill.scrolltickmax);
    CHECK_EQ(70, kill.GetNewMessageWait());
    GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  }

  TEST_CASE_FIXTURE(ClientFixture, "Start and shutdown" * doctest::skip(false))
  {
      GlobalSystems<Config::CLIENT_MODULE>::Init();
      std::string game = {"SoldatGame"};
      std::array<char*, 1> argv = { game.data() };
      gGlobalStateClient.gClient.startgame(argv.size(), argv.data());
      gGlobalStateClient.gClient.shutdown();
      GlobalSystems<Config::CLIENT_MODULE>::Deinit();
  }

} // TEST_SUITE("Client")

} // namespace
