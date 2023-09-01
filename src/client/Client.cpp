#include <cstdint>

/*#include "idcompilerdefines.inc"*/
// #define DEVELOPMENT

#include "Client.hpp"
#include "ClientCommands.hpp"
#include "ClientGame.hpp"
#include "ControlGame.hpp"
#include "GameMenus.hpp"
#include "GameRendering.hpp"
#include "GameStrings.hpp"
#include "Gfx.hpp"
#include "Input.hpp"
#include "InterfaceGraphics.hpp"
#include "Sound.hpp"
#include "common/FileUtility.hpp"
#include "common/Logging.hpp"
#include "common/PhysFSExt.hpp"
#include "common/Util.hpp"
#include "common/misc/Config.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/SHA1Helper.hpp"
#include "common/misc/TIniFile.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "shared/Command.hpp"
#include "shared/Constants.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/LogFile.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClient.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include <Tracy.hpp>
#include <memory>
#include <physfs.h>
#include <thread>


namespace
{
bool progready;
} // namespace

// Game.hpp
extern std::int32_t gamewidth;
extern std::int32_t gameheight;

extern float gamewidthhalf;
extern float gameheighthalf;

// ClientGame.hpp
extern bool mapchanged;

// Client.cpp variables
bool gamelooprun;

std::string joinpassword;         // server password
std::string joinport = "23073";   // join port to server
std::string joinip = "127.0.0.1"; // join ip to server

bool windowready = false;
std::uint8_t initing;
bool graphicsinitialized = false;

std::string basedirectory;

std::string moddir = "";
bool usesservermod;

std::string gClientServerIP = "127.0.0.1";
std::int32_t gClientServerPort = 23073;

std::uint8_t connection = INTERNET;

std::uint8_t sniperline_client_hpp = 0;

std::uint8_t trails = 1;
std::uint8_t spectator = 0; // TODO: Remove

std::uint8_t packetadjusting = 1;

bool limbolock;
std::uint8_t selteam;

std::uint8_t mysprite;

// Weapon Stats
PascalArray<tweaponstat, 1, 20> wepstats;
std::uint8_t wepstatsnum = 0;

// FIXME skipped item at index 0
GunArray gundisplayname;

std::uint8_t gamethingtarget;
std::int32_t grenadeeffecttimer = 0;

std::uint8_t badmapidcount;

bool abnormalterminate = false;

std::string hwid;

std::uint16_t hitspraycounter;
bool screentaken;

bool targetmode = false;

bool muteall = false;

bool redirecttoserver = false;
std::string redirectip;
std::int32_t redirectport;
std::string redirectmsg;

// Radio Menu
std::map<std::string, std::string> radiomenu;
std::array<char, 2> rmenustate;
//  RMenuState: array[0..1] of Char = ' ';
bool showradiomenu = false;
std::uint8_t radiocooldown = 3;

// screen
tvector2 cameraprev;
float camerax, cameray;          // camera x and y within world
std::uint8_t camerafollowsprite; // Tag number of object to follow

std::uint8_t notexts = 0;
std::uint8_t freecam = 0;

// bullet shot stats
std::int32_t shotdistanceshow;
float shotdistance;
float shotlife;
std::int32_t shotricochet;

// End Client.cpp variables

/*#include "IniFiles.h"*/
/*#include "TraceLog.h"*/
/*#include "ClientGame.h"*/
/*#include "ControlGame.h"*/
/*#include "InterfaceGraphics.h"*/
/*#include "Input.h"*/

void restartgraph()
{
  windowready = false;

  dotextureloading(true);

  auto &map = GS::GetGame().GetMap();

  // Load Map
  map.loadmap(GS::GetGame().GetMapchange(), CVar::r_forcebg, CVar::r_forcebg_color1,
              CVar::r_forcebg_color2);

  windowready = true;

  if (!escmenu->active)
  {
    mx = gamewidthhalf;
    my = gameheighthalf;
    mouseprev.x = mx;
    mouseprev.y = my;
  }

  GS::GetMainConsole().console(("Graphics restart"), debug_message_color);
}

static void loadweaponnames(FileUtility& fs, GunArray& gunDisplayName = gundisplayname, const std::string& modDir = moddir)
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
    auto f = fs.Open(weaponNamesFile, FileUtility::FileMode::Read);
    fileSize = fs.Size(f);
    buff = std::make_unique<std::byte[]>(fileSize);
    fs.Read(f, buff.get(), fileSize);
    fs.Close(f);
  }
  std::istringstream sd;
  sd.rdbuf()->pubsetbuf(reinterpret_cast<char*>(buff.get()), fileSize);
  for (i = 0; i < double_weapons; i++)
  {
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
  buttons[0].buttonid = 0;
  buttons[0].text = "Yes";
  buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
  buttons[1].buttonid = 1;
  buttons[1].text = "No";

  data.flags = 0;
  data.window = gamewindow;
  data.title = "Server Redirect";
  auto msg =
    (redirectmsg + "\r\n\r\nRedirect to server " + redirectip + ":" + inttostr(redirectport) + "?");
  data.message = msg.c_str();
  data.numbuttons = 2;
  data.buttons = &buttons.at(0);
  data.colorScheme = nullptr;

  if (SDL_ShowMessageBox(&data, &response) != 0)
    return;

  redirecttoserver = false;

  if (response == 0)
  {
    joinip = redirectip;
    joinport = inttostr(redirectport);
    joinserver();
  }
  else
  {
    redirectip = "";
    redirectport = 0;
    redirectmsg = "";
    exittomenu();
  }
}

void exittomenu()
{
  std::int32_t i;

  GS::GetGame().ResetGoalTicks();

  // Reset network state and show the status std::string (if any)
  // ShouldRenderFrames := False;
  // NetEncActive := False;

  // resetsynccvars;

  if (GS::GetDemoRecorder().active())
    GS::GetDemoRecorder().stoprecord();

  if (demoplayer.active())
    demoplayer.stopdemo();

  if (mysprite > 0)
  {
    clientdisconnect();
  }
  if (GetNetwork())
  {
    GetNetwork()->disconnect(true);
  }

  stopsound(channel_weather);

  auto &map = GS::GetGame().GetMap();

  map.name = "";

  if (escmenu != nullptr)
  {
    gamemenushow(escmenu, false);
  }

  map.filename = ""; // force reloading next time
  GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
  // WindowReady := False;

  auto &activeSprites = SpriteSystem::Get().GetActiveSprites();

  std::for_each(std::begin(activeSprites), std::end(activeSprites),
                [](auto &sprite) { sprite.kill(); });
  GS::GetBulletSystem().KillAll();
  for (i = 1; i <= max_sparks; i++)
    spark[i].kill();
  GS::GetThingSystem().KillAll();

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
  for (i = 1; i < max_sprites; i++)
  {
    chatdelay[i] = 0;
    chatmessage[i] = "";
    chatteam[i] = false;
  }

  mysprite = 0;
  camerafollowsprite = 0;
  gamethingtarget = 0;

  if (redirecttoserver)
    redirectdialog();
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

static bool MountAssets(FileUtility &fu, const std::string &userdirectory,
                        const std::string &basedirectory, tsha1digest& outGameModChecksum, tsha1digest& outCustomModChecksum)
{
  LogDebugG("[PhysFS] Mounting game archive");
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
  moddir = "";
  if (CVar::fs_mod != "")
  {
    LogDebugG("[PhysFS] Mounting mods/{}.smod", lowercase(CVar::fs_mod));
    if (!fu.Mount((userdirectory + "mods/" + lowercase(CVar::fs_mod) + ".smod").c_str(),
                  (std::string("mods/") + lowercase(CVar::fs_mod) + "/").c_str()))
    {
      showmessage((std::string("Could not load mod archive (") + std::string(CVar::fs_mod) + ")."));
      return false;
    }
    moddir = std::string("mods/") + lowercase(CVar::fs_mod) + '/';
    outCustomModChecksum = sha1file(userdirectory + "mods/" + lowercase(CVar::fs_mod) + ".smod");
  }
  return true;
}

void startgame(int argc, const char *argv[])
{
  initclientcommands();
  commandinit();

  parsecommandline(argc, argv);
  if (argc == 1)
  {
    parseinput("join 127.0.0.1 23073");
  }

  auto &fs = GS::GetFileSystem();
  const auto userDirectory = fs.GetPrefPath("client");
  const auto baseDirectory = fs.GetBasePath();

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

  newlogfiles(userDirectory);

  GS::GetMainConsole().countmax = round(15 * _rscala.y);
  GS::GetMainConsole().scrolltickmax = 150;
  GS::GetMainConsole().newmessagewait = 150;
  GS::GetMainConsole().alphacount = 255;
  GS::GetMainConsole().count = 0;
  GS::GetMainConsole().countmax = round(CVar::ui_console_length * _rscala.y);

  if (GS::GetMainConsole().countmax > 254)
  {
    GS::GetMainConsole().countmax = 254;
  }

  float fov;
  SDL_DisplayMode currentdisplay;
  std::string systemlang = "en_US";
  std::string systemfallbacklang = "en_US";

  // TODO remove HWIDs, replace by Fae auth tickets
  hwid = "00000000000";

  initing = 0;

  LogDebugG("[PhysFS] Initializing system");

  loadinterfacearchives(userDirectory + "custom-interfaces/");

  fs.Copy("/configs/bindings.cfg", "/user/configs/bindings.cfg");
  fs.Copy("/configs/client.cfg", "/user/configs/client.cfg");
  fs.Copy("/configs/controls.cfg", "/user/configs/controls.cfg");
  fs.Copy("/configs/game.cfg", "/user/configs/game.cfg");
  fs.Copy("/configs/graphics.cfg", "/user/configs/graphics.cfg");
  fs.Copy("/configs/player.cfg", "/user/configs/player.cfg");
  fs.Copy("/configs/sound.cfg", "/user/configs/sound.cfg");

  loadconfig("client.cfg", fs);

  // these might change so keep a backup to avoid changing the settings file
  screenwidth = CVar::r_screenwidth;
  screenheight = CVar::r_screenheight;
  renderheight = CVar::r_renderheight;
  renderwidth = CVar::r_renderwidth;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_GetCurrentDisplayMode(0, &currentdisplay);

  if ((screenwidth == 0) || (screenheight == 0))
  {
    screenwidth = currentdisplay.w;
    screenheight = currentdisplay.h;
  }

  if ((renderwidth == 0) || (renderheight == 0))
  {
    renderwidth = screenwidth;
    renderheight = screenheight;
  }

  // Calculcate FOV to check for too high/low vision
  fov = (float)(renderwidth) / renderheight;
  if (fov > max_fov)
  {
    renderwidth = ceil(renderheight * max_fov);
    fov = max_fov;
  }
  else if (fov < min_fov)
  {
    renderheight = ceil((float)(renderwidth) / min_fov);
    fov = min_fov;
  }

  // Calulcate internal game width based on the fov and internal height
  gamewidth = round(fov * gameheight);
  gamewidthhalf = (float)(gamewidth) / 2;
  gameheighthalf = (float)(gameheight) / 2;

  if (CVar::r_fullscreen == 0)
  {
    // avoid black bars in windowed mode
    if (((float)(screenwidth) / screenheight) >= ((float)(renderwidth) / renderheight))
      screenwidth = round(screenheight * ((float)(renderwidth) / renderheight));
    else
      screenheight = round(screenwidth * ((float)(renderheight) / renderwidth));
  }

  // window size equals "screen" size except in windowed fullscreen
  windowwidth = screenwidth;
  windowheight = screenheight;

  if (CVar::r_fullscreen == 2)
  {
    ;
    //  WindowWidth := Screen.Width;
    //  WindowHeight := Screen.Height;
  }

  LogInfo("gfx", "Window size: {}x{}", windowwidth, windowheight);
  LogInfo("gfx", "Target resolution: {}x{}", screenwidth, screenheight);
  LogInfo("gfx", "Internal resolution: {}x{}", renderwidth, renderheight);

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
    _rscala.x = 1;
    _rscala.y = 1;

    _iscala.x = (float)(gamewidth) / default_width;
    _iscala.y = 1;

    fragx = floor(gamewidthhalf - 300) - 25;
  }
  else
  {
    _rscala.x = (float)(renderwidth) / gamewidth;
    _rscala.y = (float)(renderheight) / gameheight;

    _iscala.x = (float)(renderwidth) / 640;
    _iscala.y = (float)(renderheight) / 480;

    fragx = floor((float)(renderwidth) / 2 - 300) - 25;

    if (renderheight > gameheight)
      fragy = round(10 * _rscala.y);
  }

  gamerenderingparams.interfacename = CVar::ui_style;

  resetframetiming();

  gfxlog("Loading game graphics");

  if (!initgamegraphics())
  {
    showmessage(std::string("The required OpenGL functionality isn't supported. ") +
                "Please, update your video drivers and try again.");
    // ExitButtonClick(nullptr);
    return;
  }

  windowready = true;

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

  if (inittranslation(moddir + "/txt/" + systemlang + ".mo"))
    LogDebugG("Game captions loaded from {}/txt/{}", moddir, systemlang);
  else
    LogDebugG("Game captions not found");

  addlinetologfile(GetGameLog(), "Initializing Sound Library.", GetGameLogFilename());
  // Init Sound Library
  if (!initsound())
  {
    addlinetologfile(GetGameLog(), "Failed to initialize Sound Library.", GetGameLogFilename());
    // Let the player know that he has no sound (no popup window)
  }

  loadsounds("");
  if (length(moddir) > 0)
    loadsounds(moddir.c_str());

  addlinetologfile(GetGameLog(), "Creating network interface.", GetGameLogFilename());

  // Create Consoles
  GS::GetMainConsole().countmax = round(CVar::ui_console_length * _rscala.y);
  GS::GetMainConsole().scrolltickmax = 150;
  GS::GetMainConsole().newmessagewait = 150;
  GS::GetMainConsole().alphacount = 255;
  GS::GetMainConsole().count = 0;
  if (GS::GetMainConsole().countmax > 254)
    GS::GetMainConsole().countmax = 254;

  GetBigConsole().countmax =
    floor((0.85 * renderheight) / (CVar::font_consolelineheight * fontstylesize(font_small)));
  GetBigConsole().scrolltickmax = 1500000;
  GetBigConsole().newmessagewait = 0;
  GetBigConsole().alphacount = 255;
  GetBigConsole().count = 0;
  if (GetBigConsole().countmax > 254)
    GetBigConsole().countmax = 254;

  GetKillConsole().countmax = round(CVar::ui_killconsole_length * _rscala.y);
  GetKillConsole().scrolltickmax = 240;
  GetKillConsole().newmessagewait = 70;

  // Create static player objects
  for (auto &s : SpriteSystem::Get().GetSprites())
  {
    s.player = std::make_shared<tplayer>();
  }

  AnimationSystem::Get().LoadAnimObjects("");
  if (length(moddir) > 0)
    AnimationSystem::Get().LoadAnimObjects(moddir);

  // greet!
  // GS::GetMainConsole().console(("Welcome to Soldat ") + soldat_version, default_message_color);
  GS::GetMainConsole().console(("Welcome to Soldat "), default_message_color);

  // Load weapon display names
  loadweaponnames(fs);
  createweaponsbase(GS::GetWeaponSystem().GetGuns());

  GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);

  playernamesshow = true;

  cursortext = "";

  initgamemenus();

  {
    TIniFile ini{PhysFS_ReadAsStream("txt/radiomenu-default.ini")};
    ini.ReadSectionValues("OPTIONS", radiomenu);
  }

  // Play demo
  freecam = 1;
  notexts = 0;
  shotdistanceshow = -1;

  if (CVar::r_compatibility)
    CVar::cl_actionsnap = false;

  writelogfile(GetGameLog(), GetGameLogFilename());
  rundeferredcommands();
}

void shutdown()
{
  exittomenu();

  if (abnormalterminate)
    return;

  addlinetologfile(GetGameLog(), "Freeing sprites.", GetGameLogFilename());

  // Free GFX
  destroygamegraphics();

  for (auto &s : SpriteSystem::Get().GetSprites())
  {
    s.player = nullptr;
  }

  SDL_Quit();

  deinittranslation();

  addlinetologfile(GetGameLog(), "UDP closing.", GetGameLogFilename());

  NotImplemented("network");
#if 0
    freeandnullptr(udp);
#endif

  addlinetologfile(GetGameLog(), "Sound closing.", GetGameLogFilename());

  closesound();

  addlinetologfile(GetGameLog(), "PhysFS closing.", GetGameLogFilename());

  PHYSFS_deinit();

  commanddeinit();

  addlinetologfile(GetGameLog(), "   End of Log.", GetGameLogFilename());

  writelogfile(GetGameLog(), GetGameLogFilename());

  gamelooprun = false;
}

void startgameloop()
{
  while (gamelooprun)
  {
    auto begin = std::chrono::system_clock::now();
    GetNetwork()->processloop();
    gameinput();
    if (!gamelooprun)
    {
      break;
    }
    gameloop();
    auto end = std::chrono::system_clock::now();
    constexpr auto frameTime = std::chrono::seconds(1) / 60.f;
    {
      ZoneScopedN("WaitingForNextFrame");
      std::this_thread::sleep_for(frameTime - (end - begin));
    }
    FrameMarkNamed("ClientFrame");
  }
}

void joinserver()
{
  resetframetiming();

  initing += 1;
  if (initing > 10)
    initing = 10;

  gClientServerIP = trim(joinip);

  NotImplemented("No error checking");
#if 0
    if (!trystrtoint(trim(joinport), serverport))
        return;
#endif

  initgamegraphics();
  dotextureloading(true);

  InitClientNetwork();
  // DEMO
  if (joinport == "0")
  {
    demoplayer.opendemo(GS::GetGame().GetUserDirectory() + "demos/" + joinip + ".sdm");
    demoplayer.processdemo();
    progready = true;
    gamelooprun = true;
    rendergameinfo(("Loading"));
    startgameloop();
  }
  else
  {

    rendergameinfo(("Connecting to " + gClientServerIP + ":" + std::to_string(gClientServerPort)));

    if (GetNetwork()->connect(gClientServerIP, gClientServerPort))
    {
      progready = true;
      gamelooprun = true;
      rendergameinfo(("Loading"));
      clientrequestgame();
      startgameloop();
    }
    else
    {
      rendergameinfo(("Connection timed out."));
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
};

TEST_CASE_FIXTURE(ClientFixture, "Mount memory and write file and later read it")
{
  FileUtility fu;
  fu.Mount("tmpfs.memory", "/user");
  CreateDirectoryStructure(fu);
  auto f = fu.Open("/user/logs/nice_log.txt", FileUtility::FileMode::Write);
  CHECK_NE(nullptr, f);
  fu.Close(f);
}

TEST_CASE_FIXTURE(ClientFixture, "Mount soldat.smod test")
{
  // test soldat.smod, generated with xxd --include soldat.smod
  // contains:
  // client/client_test.txt
  // server/server_test.txt
  // shared/shared_test.txt
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

  FileUtility fu;
  auto testDir = fu.GetPrefPath("mount_test", true);
  std::filesystem::remove_all(testDir);
  // recreate directory
  testDir = fu.GetPrefPath("mount_test", true);
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
  const auto userDirectory = fs.GetPrefPath("client");
  const auto baseDirectory = fs.GetBasePath();
  tsha1digest checksum1;
  tsha1digest checksum2;
  auto ret = MountAssets(fs, userDirectory, baseDirectory, checksum1, checksum2);
  CHECK_EQ(true, ret);
  loadweaponnames(fs, ga);
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

} // namespace
