#include "Server.hpp"
#include "BanSystem.hpp"
#include "FileServer.hpp"
#include "ServerCommands.hpp"
#include "ServerHelper.hpp"
#include "ServerLoop.hpp"
#include "common/Logging.hpp"
#include "common/PhysFSExt.hpp"
#include "common/Util.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/misc/TFileStream.hpp"
#include "shared/Command.hpp"
#include "shared/Console.hpp"
#include "shared/Constants.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "shared/LogFile.hpp"
#include "shared/SharedConfig.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/mechanics/Things.hpp"
#include "shared/misc/SignalUtils.hpp"
#include "shared/network/NetworkServer.hpp"
#include "shared/network/NetworkServerConnection.hpp"
#include "shared/network/NetworkServerGame.hpp"
#include "shared/network/NetworkServerSprite.hpp"
#include "shared/network/NetworkUtils.hpp"
#include <array>
#include <steam/steamnetworkingsockets.h>
#include <thread>

//clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

constexpr auto PATH_MAX = 4095;

bool progready = false;
namespace
{
std::string basedirectory;
}

tstringlist killlog;
std::string killlogfilename;

#if 0
// Cvars
CVarInt log_level;
CVarInt log_filesupdate;
CVarBool log_timestamp;

CVarString fs_mod;
CVarBool fs_portable;
CVarString fs_basepath;
CVarString fs_userpath;

CVarBool demo_autorecord;

CVarInt sv_dm_limit;
CVarInt sv_pm_limit;
CVarInt sv_tm_limit;
CVarInt sv_rm_limit;

CVarInt sv_inf_limit;
CVarInt sv_inf_redaward;

CVarInt sv_htf_limit;

CVarInt sv_ctf_limit;

CVarBool sv_spectatorchat;
CVarString sv_info;
CVarString sv_pidfilename;
CVarString sv_lobbyurl;

CVarBool sv_steamonly;

#ifdef STEAM
CVarBool sv_voicechat;
CVarBool sv_voicechat_alltalk;
CVarString sv_setsteamaccount;
#endif

CVarBool sv_anticheatkick;
CVarBool sv_botbalance;
CVarBool sv_echokills;
CVarInt sv_healthcooldown;
CVarBool sv_teamcolors;

CVarInt net_port = 23073;
CVarString net_ip;
CVarString net_adminip;
CVarBool net_compression;
CVarBool net_allowdownload;
CVarInt net_maxconnections;
CVarInt net_maxadminconnections;

CVarBool fileserver_enable;
CVarInt fileserver_port;
CVarString fileserver_ip;

// syncable cvars
CVarInt sv_gamemode;
CVarBool sv_friendlyfire;
CVarInt sv_timelimit;
CVarInt sv_maxgrenades;
CVarBool sv_bullettime;
CVarBool sv_sniperline;
CVarBool sv_balanceteams;
CVarBool sv_survivalmode;
CVarBool sv_survivalmode_antispy;
CVarBool sv_survivalmode_clearweapons;
CVarBool sv_realisticmode;
CVarBool sv_advancemode;
CVarInt sv_advancemode_amount;
CVarBool sv_guns_collide;
CVarBool sv_kits_collide;
CVarBool sv_minimap;
CVarBool sv_advancedspectator;
CVarBool sv_radio;
CVarFloat sv_gravity;
CVarString sv_hostname;
CVarInt sv_killlimit;
CVarString sv_downloadurl;
CVarBool sv_pure;
CVarString sv_website;

#ifdef ENABLE_FAE
CVarBool ac_enable;
#endif
#endif

// config stuff
std::string serverip = "127.0.0.1";
std::int32_t serverport = 23073;
std::int32_t bonusfreq = 3600;

tstringlist mapslist;

std::int8_t lastplayer;

// Mute array
PascalArray<std::string, 1, max_players> mutelist;
PascalArray<std::string, 1, max_players> mutename;

// TK array
PascalArray<std::string, 1, max_players> tklist;      // IP
PascalArray<std::int8_t, 1, max_players> tklistkills; // TK Warnings

std::int32_t TCPBytesSent64;
std::int32_t TCPBytesReceived64;

tstringlist remoteips, adminips;

std::array<std::string, 1000> floodip;
std::array<std::int32_t, 1000> floodnum;

std::array<std::string, 4> lastreqip; // last 4 IP"s to request game
std::int8_t lastreqid = 0;
std::string dropip = "";

std::array<std::string, Constants::MAX_ADMIN_FLOOD_IPS> adminfloodip;
std::array<std::string, Constants::MAX_LAST_ADMIN_IPS> lastadminips;
std::int32_t AdminIPCounter = 0;

std::int32_t waverespawntime, waverespawncounter;

std::array<std::int8_t, max_sprites> bulletwarningcount;
std::array<std::int8_t, max_sprites> cheattag;

#ifdef RCON
TAdminServer AdminServer; // TIdTCPServer;
#endif

std::int32_t htftime = Constants::HTF_SEC_POINT;

std::string currentconf = "soldat.ini";

std::string wmname, wmversion;
std::string lastwepmod;

float grav = 0.06;

std::string ModDir = "";

// DK_TODO replace tservernetwork with tservernetowrk<SERVER>
#if 0
tservernetwork<Config::GetModule()> *UDP;
#endif
tservernetwork *GetServerNetwork();
#if 0
TLobbyThread LobbyThread;
#endif

#ifdef STEAM
// TSteamCallbacks SteamCallbacks;
TSteamGS SteamAPI;
#endif

static void WriteLn(const std::string &msg)
{
    LogDebugG("{}", msg);
}

static std::string ParamStr(std::int32_t v)
{
    NotImplemented(NITag::OTHER);
    return "./";
}

static std::string ExtractFilePath(const std::string &path)
{
    NotImplemented(NITag::OTHER);
    return "./";
}

#ifndef MSWINDOWS
// from lazdaemon package
//  BUG"Old instance (the parent) writes it"s consolelog
// no idea how to avoid it. Perhaps somebody who knows this code could fix it.
void DaemonizeProgram()
{
    NotImplemented(NITag::NETWORK);
#if 0
    var pid, sid TPid;
    pid = FpFork;
    if ((pid < 0))
        raise Exception.Create("Failed to fork daemon process.");
    if (pid > 0)
    {
        // We are now in the main program, which has to terminate
        FpExit(0);
        end else
        {
            // Here we are in the daemonized proces
            sid = FpSetsid;
            if (sid < 0)
                raise Exception.Create("Failed to fork daemon process.");
            // Reset the file-mask
            FpUmask(0);
            // commented, it breaks script detection, and seems to work anyway (no locks)
            // Change the current directory, to avoid locking the current directory
            // chdir("/");
            AssignFile(devnull, "/dev/null");
            Rewrite(devnull);
            Input = devnull;
            Output = devnull;
            ErrOutput = devnull;
        }
    }
#endif // 0
}
#endif // MSWINDOWS

#ifdef STEAM
#ifdef STEAMSTATS
procedure StoreStats(TSteamID Player);
{
    // SteamCallResultDispatcher.Create(1801, @GSStatsStored, SizeOf(GSStatsStored_t),
    // SteamAPI.GameServerStats.StoreUserStats(Player), k_ECallbackFlagsGameServer);
}

procedure GSStatsStored(PGSStatsStored Callback_t);
{
    Debug(Format("[Steam] GSStatsReceived result %d result steamid %s",
                 [ Ord(Callback.m_eResult), Callback.m_steamIDUser.GetAsString ]));
}

procedure GSStatsReceived(PGSStatsReceived Callback_t);
var Byte j;
{
    Debug(Format("[Steam] GSStatsReceived result %d result steamid %s",
                 [ Ord(Callback.m_eResult), Callback.m_steamIDUser.GetAsString ]));
    for (j = 1; j < max_sprites; j++)
        if ((sprite[j].Active) and
            (Uint64(sprite[j].Player.SteamID) = Uint64(Callback.m_steamIDUser)))
        {
            if (Callback.m_eResult = k_EResultOK)
                sprite[j].Player.SteamStats = true else sprite[j].Player.SteamStats = false;
        }
}

procedure RequestUserStats(TSteamID Player);
{
    SteamAPI.GameServerStats.RequestUserStats(Player);
}
#endif

procedure GetCollectionDetails(PSteamUGCQueryCompleted Callback_t);
var array CollectionItems of PublishedFileId_t;
SteamUGCDetails CollectionDetails_t;
uint ItemState32;
i, std::int32_t j;
{
    Debug(Format("[Steam] SteamUGCQueryCompleted id %d result %d numresults %d totalresults %d "
                 "cached %s",
                 [
                     Callback.m_handle, Ord(Callback.m_eResult), Callback.m_unNumResultsReturned,
                     Callback.m_unTotalMatchingResults, Callback.m_bCachedData.toString
                 ]));

  for
      j = 0 to Callback.m_unNumResultsReturned - 1 do
      {
          if SteamAPI
              .UGC.GetQueryUGCResult(Callback.m_handle, j, @CollectionDetails) then
              {
                  SetLength(CollectionItems, CollectionDetails.m_unNumChildren);
                  if SteamAPI
                      .UGC.GetQueryUGCChildren(Callback.m_handle, 0, @CollectionItems[0],
                                               Length(CollectionItems)) then
                      {
        for
            i = 0 to CollectionDetails.m_unNumChildren - 1 do
            {
                ItemState = SteamAPI.UGC.GetItemState(CollectionItems[i]);
                Debug(Format("[Steam] GetItemState id %d state %d",
                             [ CollectionItems[i], ItemState ]));
                if (ItemState = 0)
                    or (Ord(k_EItemStateNeedsUpdate) and ItemState != 0) then
                    {
                        Debug("[Steam] Dowloading workshop item id:" +
                              IntToStr(CollectionItems[i]));
                        SteamAPI.UGC.DownloadItem(CollectionItems[i], true);
                    }
                else if (ItemState = Ord(k_EItemStateInstalled))
                {
                    MapsList.Add("workshop/" + IntToStr(CollectionItems[i]));
                }
            }
                      }
                  else
                      Debug("[Steam] GetQueryUGCChildren has failed");
              }
          else
              Debug("[Steam] GetQueryUGCResult has failed");
          SetLength(CollectionItems, 0);
      }
  SteamAPI.UGC.ReleaseQueryUGCRequest(Callback.m_handle);
}

procedure DownloadWorkshopCollection(Uint ItemID64);
var UGCQueryHandle QueryHandle_t;
Array Items[0..1] of PublishedFileId_t;
{
    Items[0] = ItemID;
    QueryHandle = SteamAPI.UGC.CreateQueryUGCDetailsRequest(@Items, 1);
    SteamAPI.UGC.SetReturnChildren(QueryHandle, true);
    SteamAPI.UGC.SetReturnOnlyIDs(QueryHandle, true);
    // ApiCall = SteamAPI.UGC.SendQueryUGCRequest(QueryHandle);
    // SteamCallResultDispatcher.Create(3401, @GetCollectionDetails,
    // SizeOf(SteamUGCQueryCompleted_t), ApiCall, k_ECallbackFlagsGameServer);
    // ParseMapName("workshop/1916893159");
}

function GetWorkshopItemDir(PublishedFileId ItemID_t) AnsiString;
var uint FileSizeOnDisk64 = 0;
uint DirSizeOnDisk32 = 0;
array Path[0..PATH_MAX] of Char;
Cardinal TimeStamp = 0;
{
    Result = "";
    if (MapChangeItemID = ItemID)
    {
        PrepareMapChange("workshop/" + IntToStr(ItemID));
    }
    else if SteamAPI
        .UGC.GetItemInstallInfo(ItemID, @FileSizeOnDisk, @Path, PATH_MAX, @TimeStamp) then
        {
            Result = Path;
        }
    else
        Result = "";
}

procedure DownloadItemResult(PDownloadItemResult Callback_t);
cdecl;
{
    if (Callback.m_unAppID = STEAM_APPID)
    {
        if (Callback.m_eResult = k_EResultOK)
        {
        }
        else
            WriteLn("[Steam] Failed to download workshop item, id:" +
                    IntToStr(Callback.m_nPublishedFileId) + "  error" +
                    IntToStr(Ord(Callback.m_eResult)));
    }
}

procedure OnSteamServersConnected();
cdecl;
{
    WriteLn("[Steam] Successfully connected to the Steam.");
    // DownloadItems;
}

procedure OnSteamServerConnectFailure(PSteamServerConnectFailure Callback_t);
cdecl;
{
    WriteLn("[Steam] Connection to the Steam has  failed" + IntToStr(Ord(Callback.m_eResult)));
}

procedure OnSteamServersDisconnected(PSteamServersDisconnected Callback_t);
cdecl;
{
    WriteLn("[Steam] Lost connection to the Steam  servers" + IntToStr(Ord(Callback.m_eResult)));
}

procedure SteamNetConnectionStatusChangedCallback(
    PSteamNetConnectionStatusChangedCallback Callback_t);
cdecl;
{
    if (Assigned(UDP))
        UDP.ProcessEvents(Callback);
}

procedure RunManualCallbacks;
var CallbackMsg callback_t;
SteamAPICallCompleted pCallCompleted_t;
bool bFailed;
Pointer Data;
{
    SteamAPI_ManualDispatch_RunFrame(SteamAPI.SteamPipeHandle);
    while
        SteamAPI_ManualDispatch_GetNextCallback(SteamAPI.SteamPipeHandle, @callback) do
        {
            if (callback.m_iCallback = 703)
            {
                pCallCompleted = PSteamAPICallCompleted_t(callback.m_pubParam) ^ ;
                GetMem(Data, pCallCompleted.m_cubParam);
                if SteamAPI_ManualDispatch_GetAPICallResult (SteamAPI.SteamPipeHandle,
                                                             pCallCompleted.m_hAsyncCall, Data,
                                                             pCallCompleted.m_cubParam,
                                                             pCallCompleted.m_iCallback, @bFailed)
                    then
                    {
                    }
                FreeMem(Data);
                end else if (callback.m_iCallback = 101)
                    OnSteamServersConnected() else if (callback.m_iCallback = 102)
                        OnSteamServerConnectFailure(
                            callback.m_pubParam) else if (callback.m_iCallback = 103)
                            OnSteamServersDisconnected(
                                callback.m_pubParam) else if (callback.m_iCallback = 1221)
                                SteamNetConnectionStatusChangedCallback(callback.m_pubParam);

                SteamAPI_ManualDispatch_FreeLastCallback(SteamAPI.SteamPipeHandle);
            }
        }
}
#endif // STEAM

void ActivateServer(int argc, const char *argv[])
{
    std::int32_t j, i;
    NotImplemented(NITag::NETWORK, "Rewrite message");
#if 0
  WriteLn("");
  WriteLn("             -= Soldat Dedicated Server " + SOLDAT_VERSION + " - " +
    DEDVERSION + " (build " + SOLDAT_VERSION_LONG + ") =-");
  WriteLn("");
  WriteLn("----------------------------------------------------------------");
  WriteLn("         Soldat Dedicated Server initializing...");
  WriteLn("----------------------------------------------------------------");
  WriteLn("");
  WriteLn("   Need help running your server?");
  WriteLn("   https Discord://discord.gg/soldat");
  WriteLn("");
  WriteLn("   ---> https://forums.soldat.pl/");
  WriteLn("");
  WriteLn("   Additional parameters:");
  WriteLn("   ./soldatserver -net_port PORT -sv_maxplayers MAXPLAYERS -sv_password PASSWORD");
  WriteLn("    Example./soldatserver -net_port 23073 -sv_maxplayers 16 -sv_password "my pass"");
  WriteLn("");
  WriteLn("");

  WriteLn(" Compiled with FreePascal " + {$I %FPCVERSION%});
  WriteLn("");
#endif

#if 0
    DefaultFormatSettings.DecimalSeparator = ".";
    DefaultFormatSettings.DateSeparator = "-";
#endif

    servertickcounter = 0;
    maintickcounter = 0;

    // Initialize player dummy objects (cf. DummyPlayer definition for documentation)
    for (auto &s : sprite)
    {
        s.player = new tplayer();
    }

    // Create Consoles
    GetServerMainConsole().countmax = 7;
    GetServerMainConsole().scrolltickmax = 150;
    GetServerMainConsole().newmessagewait = 150;
    GetServerMainConsole().alphacount = 255;

    NotImplemented(NITag::OTHER, "Who cares about colors?");
#if 0
    if GetEnvironmentVariable ("COLORTERM")
        != "" then GetServerMainConsole().TerminalColors = true;
#endif

    NotImplemented(NITag::OTHER, "No cvarinit");
#if 0
    cvarinit();
#endif
    initservercommands();
    parsecommandline(argc, argv);

    if (CVar::fs_basepath == "")
    {
        basedirectory = ExtractFilePath(ParamStr(0));
    }
    if (CVar::fs_userpath == "")
    {
        userdirectory = ExtractFilePath(ParamStr(0));
    }

    LogDebugG("[FS]  userdirectory {}", userdirectory);
    LogDebugG("[FS]  basedirectory {}", basedirectory);

    NotImplemented(NITag::OTHER, "No set current dir");
#if 0
    SetCurrentDir(userdirectory);
#endif

    if (PHYSFS_isInit() == 0 && not PHYSFS_init(pchar(ParamStr(0))))
    {
        WriteLn("Could not initialize PhysFS.");
        progready = false;
        CVar::sc_enable = false;
        return;
    }
    if (not PHYSFS_mount(pchar(basedirectory + "/soldat.smod"), "/", false))
    {
        WriteLn("Could not load base game archive (soldat.smod).");
        progready = false;
        CVar::sc_enable = false;
        return;
    }

    gamemodchecksum = sha1file(basedirectory + "/soldat.smod");

    ModDir = "";

    if (CVar::fs_mod != "")
    {
        if (not PHYSFS_mount(pchar(userdirectory + "mods/" + lowercase(CVar::fs_mod) + ".smod"),
                             pchar("mods/" + lowercase(CVar::fs_mod) + "/"), false))
        {
            WriteLn("Could not load mod archive (" + std::string(CVar::fs_mod) + ").");
            progready = false;
            CVar::sc_enable = false;
            return;
        }
        ModDir = "mods/" + lowercase(CVar::fs_mod) + "/";
        custommodchecksum = sha1file(userdirectory + "mods/" + lowercase(CVar::fs_mod) + ".smod");
    }

    // Create the basic folder structure
    createdirifmissing(userdirectory + "/configs");
    createdirifmissing(userdirectory + "/demos");
    createdirifmissing(userdirectory + "/logs");
    createdirifmissing(userdirectory + "/logs/kills");
    createdirifmissing(userdirectory + "/maps");
    createdirifmissing(userdirectory + "/mods");

    // Copy default configs if they are missing
    PhysFS_CopyFileFromArchive("configs/server.cfg", userdirectory + "/configs/server.cfg");
    PhysFS_CopyFileFromArchive("configs/weapons.ini", userdirectory + "/configs/weapons.ini");
    PhysFS_CopyFileFromArchive("configs/weapons_realistic.ini",
                               userdirectory + "/configs/weapons_realistic.ini");

    loadconfig("server.cfg");

#if 0
    CvarsInitialized = true;
#endif

    newlogfiles();

    LogDebugG("ActivateServer");

#ifdef SCRIPT
    ScrptDispatcher.SafeMode = sc_safemode;
    ScrptDispatcher.MaxScripts = sc_maxscripts;
#endif

    if (CVar::net_ip == "")
    {
        CVar::net_ip = "0.0.0.0";
    }

#ifdef STEAM
    SteamAPI = TSteamGS.Init(
        0,             // The IP address you are going to bind to
        net_port,      // The port that clients will connect to for gameplay.
        net_port + 20, // The port that will manage server browser related duties and
                       // info pings from clients.
        eServerModeAuthenticationAndSecure, // Sets the authentication method of the server.
        pchar(SOLDAT_VERSION));

    SteamAPI_ManualDispatch_Init();

    if (SteamAPI = nil)
    {
        WriteLn("[Steam] Failed to initialize Steam instance.");
        ShutDown;
    }
    else
    {
        SteamAPI.Utils.SetWarningMessageHook(@SteamWarning);

        SteamAPI.GameServer.SetModDir(pchar("Soldat"));
        SteamAPI.GameServer.SetProduct(pchar("Soldat"));
        SteamAPI.GameServer.SetGameDescription(pchar("Soldat"));

        // SteamCallbackDispatcher.Create(101, @OnSteamServersConnected,
        // SizeOf(SteamServersConnected_t), k_ECallbackFlagsGameServer);
        // SteamCallbackDispatcher.Create(102, @OnSteamServerConnectFailure,
        // SizeOf(SteamServerConnectFailure_t), k_ECallbackFlagsGameServer);
        // SteamCallbackDispatcher.Create(103, @OnSteamServersConnected,
        // SizeOf(SteamServersDisconnected_t), k_ECallbackFlagsGameServer);
        // SteamCallbackDispatcher.Create(1221,
        // @SteamNetConnectionStatusChangedCallback,
        // SizeOf(SteamNetConnectionStatusChangedCallback_t),
        // k_ECallbackFlagsGameServer); SteamCallbackDispatcher.Create(3406,
        // @DownloadItemResult, SizeOf(DownloadItemResult_t),
        // k_ECallbackFlagsGameServer);

        if sv_setsteamaccount
            != "" then SteamAPI.GameServer.LogOn(pchar(
                   sv_setsteamaccount)) else SteamAPI.GameServer.LogOnAnonymous();

        if SteamAPI
            .UGC.BInitWorkshopForGameServer(638490, pchar(userdirectory + "/workshop"))
                then WriteLn("[Steam] Initialized Workshop.") else WriteLn(
                    "[Steam] Failed to initialize Workshop.");
    }
#endif // STEAM

    progready = true;

#ifndef SCRIPT
    CVar::sc_enable = 0;
#endif

    // Implement TODO enabled weapons in cvar system
    for (i = 1; i < main_weapons; i++)
        weaponactive[i] = 1;

    loadanimobjects("");
    if (length(ModDir) > 0)
        loadanimobjects(ModDir);

    // greet!
    WriteLn(" Hit CTRL+C to Exit");
    WriteLn(" Please command the server using the Soldat Admin program");

    mapchangecounter = -60;

    sinuscounter = 0;

    addlinetologfile(gamelog, "Loading Maps List", consolelogfilename);

    if (fileexists(userdirectory + "configs/" + std::string(CVar::sv_maplist)))
    {
        mapslist.loadfromfile(userdirectory + "configs/" + std::string(CVar::sv_maplist));
        auto it = std::remove(mapslist.begin(), mapslist.end(), "");
        mapslist.erase(it, mapslist.end());
    }

    if (mapslist.size() == 0)
    {
        WriteLn("");
        WriteLn("  No maps list found (adding default). Please add maps in configs/mapslist.txt");
        WriteLn("");
        if (not isteamgame())
            mapslist.add("Arena");
        else
            mapslist.add("ctf_Ash");
    }
#if 0
        for (i = 1; i < max_sprites; i++)
            for (j = 1; j < max_sprites; j++)
                OldHelmetMsg[i, j].WearHelmet = 1;
#endif

    // Banned IPs text file
    if (not createfileifmissing(userdirectory + "configs/banned.txt"))
    {
        NotImplemented(NITag::OTHER, "Failed to create configs/banned.txt");
    }

    if (not createfileifmissing(userdirectory + "configs/bannedhw.txt"))
    {
        NotImplemented(NITag::OTHER, "Failed to create configs/bannedhw.txt");
    }

    loadbannedlist(userdirectory + "configs/banned.txt");
    loadbannedlisthw(userdirectory + "configs/bannedhw.txt");

    if (fileexists(userdirectory + "configs/remote.txt"))
    {
        NotImplemented(NITag::OTHER);
#if 0
            RemoteIPs.LoadFromFile(userdirectory + "configs/remote.txt");
#endif
    }

    adminips = remoteips;
    adminips.add("127.0.0.1");

    // Flood IP stuff
    for (i = 1; i < max_floodips; i++)
        floodip[i] = " ";
    for (i = 1; i < max_floodips; i++)
        floodnum[i] = 0;

    weaponsingame = 0;
    for (j = 1; j < main_weapons; j++)
        if (weaponactive[j] == 1)
            weaponsingame++;

#ifdef RCON
    if sv_adminpassword
        != "" then AdminServer = TAdminServer.Create(sv_adminpassword, "Welcome") else
        {
            WriteLn("");
            WriteLn(" The server must be started with an Admin Password parameter" +
                    " to run Admin");
            WriteLn("   edit server.cfg and set sv_adminpassword variable");
            WriteLn("");
        }
#endif

    WriteLn(" Server  name" + std::string(CVar::sv_hostname));
    updategamestats();
    writelogfile(&GetKillLog(), GetKillLogFilename());
    writelogfile(gamelog, consolelogfilename);

    NotImplemented(NITag::OTHER, "mixing commands between server and client");
    // rundeferredcommands();
}

void ShutDown()
{
    LogDebugG("ShutDown");
    progready = false;

    GetServerMainConsole().console("Shutting down server...", game_message_color);
    NotImplemented(NITag::OTHER, "Missing delete file");
#if 0
    SysUtils.DeleteFile(userdirectory + "logs/" + sv_pidfilename);
#endif

    if (GetServerNetwork() != nullptr)
    {
        serverdisconnect();

        GetServerMainConsole().console("Shutting down game networking.", game_message_color);

        DeinitServerNetwork();
    }

#ifdef RCON
    if (sv_adminpassword != "")
    {
        try GetServerMainConsole().console("Shutting down admin server...", GAME_MESSAGE_COLOR);
        if (AdminServer != nil)
        {
            AdminServer.Active = false;
            AdminServer.Bindings.Clear;
            sv_adminpassword = "";
            FreeAndNil(AdminServer);
        }
        except on Exception e do WriteLn("Error on  SHUTDOWN" + e.Message);
    }
#endif

    StopFileServer();

#ifdef SCRIPT
    ScrptDispatcher.Free;
#endif

#ifdef STEAM
    Debug("[Steam] Shutdown");
    SteamAPI.GameServer.Shutdown;
#endif

#ifndef STEAM
    GameNetworkingSockets_Kill();
#endif

    addlinetologfile(gamelog, "   End of Log.", consolelogfilename);
    LogDebugG("Updating gamestats");
    updategamestats();
    LogDebugG("Saving killlog");
    writelogfile(&GetKillLog(), GetKillLogFilename());
    LogDebugG("Saving gamelog");
    writelogfile(gamelog, consolelogfilename);
    LogDebugG("Freeing gamelog");
    delete gamelog;
    gamelog = nullptr;
#if 0
    LogDebugG("Freeing killlog");
    delete killlog;
    killlog = nullptr;
#endif
}

void loadweapons(std::string Filename)
{
    bool IsRealistic;
    LogDebugG("LoadWeapons");

    IsRealistic = CVar::sv_realisticmode = true;
    createweapons(IsRealistic, guns, defaultguns);
    // FIXME (falcon) while the above instruction has to be done every time,
    // because you never know if WM provides all the values,
    // this could be done only once per mode (realistic/non-realistic)
    NotImplemented(NITag::OTHER, "no checksum");
#if 0
    DefaultWMChecksum = CreateWMChecksum();
#endif
    {
        TIniFile ini{ReadAsFileStream(userdirectory + "configs/" + Filename + ".ini")};
        if (loadweaponsconfig(ini, wmname, wmversion, guns))
        {
            buildweapons(guns);
        }
        else
        {
            WriteLn("Using default weapons mod");
            createweapons(IsRealistic, guns, defaultguns);
        }
    }
    NotImplemented(NITag::OTHER, "no checksum");
#if 0
    LoadedWMChecksum = CreateWMChecksum();

    if LoadedWMChecksum
        != DefaultWMChecksum then
        {
            GetServerMainConsole().console("Loaded weapons mod "
                                " + WMName + " v " + WMVersion + "
                                "",
                                SERVER_MESSAGE_COLOR);
        }
#endif

    for (auto &s : sprite)
    {
        if (s.active)
        {
            s.applyweaponbynum(s.weapon.num, 1, s.weapon.ammocount);
        }
    }
}

std::int8_t addbotplayer(std::string name, std::int32_t team)
{
    tvector2 a, b;
    std::int32_t p;
    tplayer *NewPlayer;
    std::string TempStr = "";
    LogDebugG("AddBotPlayer");
    std::int8_t Result = 0;

    if (playersnum == max_players)
    {
        GetServerMainConsole().console("Bot cannot be added because server is full",
                                       warning_message_color);
        return Result;
    }

    NewPlayer = new tplayer();
    NewPlayer->team = team;
    NewPlayer->applyshirtcolorfromteam();

    randomizestart(a, team);
    p = createsprite(a, b, 1, 255, NewPlayer, true);
    Result = p;

    {
        TIniFile ini(ReadAsFileStream(userdirectory + "configs/bots/" + name + ".bot"));

        if (not loadbotconfig(ini, sprite[p]))
        {
            GetServerMainConsole().console("Bot file " + name + " not found",
                                           warning_message_color);
            sprite[p].kill();
            return Result;
        }
    }

    sprite[p].respawn();
    sprite[p].player->controlmethod = bot;
    sprite[p].player->chatwarnings = 0;
    sprite[p].player->grabspersecond = 0;

    serversendnewplayerinfo(p, join_normal);

    switch (team)
    {
    case 0:
        TempStr = "the game";
        break;
    case 1:
        TempStr = "alpha team";
        break;
    case 2:
        TempStr = "bravo team";
        break;
    case 3:
        TempStr = "charlie team";
        break;
    case 4:
        TempStr = "delta team";
        break;
    case 5:
        TempStr = "as spectator";
        break;
    }
    GetServerMainConsole().console(sprite[p].player->name + " " + "has joined " + TempStr + ".",
                                   enter_message_color);

#ifdef SCRIPT
    ScrptDispatcher.OnJoinTeam(p, sprite[p].Player.Team, sprite[p].Player.Team, true);
    ScrptDispatcher.OnWeaponChange(p, sprite[p].Weapon.Num, sprite[p].SecondaryWeapon.Num,
                                   sprite[p].Weapon.AmmoCount, sprite[p].SecondaryWeapon.AmmoCount);
#endif

    sortplayers();
    return Result;
}

void startserver()
{
    tvector2 a;
    std::int32_t i, k, j;
    tmapinfo StartMap;
    LogDebugG("StartServer");
#ifdef SCRIPT
    if (sc_enable)
        ScrptDispatcher.Launch();
#endif

    if (not isteamgame())
    {
        k = CVar::bots_random_noteam;
    }
    else if (CVar::sv_gamemode == gamestyle_teammatch)
    {
        k = CVar::bots_random_alpha + CVar::bots_random_bravo + CVar::bots_random_charlie +
            CVar::bots_random_delta;
    }
    else
    {
        k = CVar::bots_random_alpha + CVar::bots_random_bravo;
    }

    // if (k > (max_sprites - 1)) then
    //  return;

    Randomize();

    for (i = 1; i < max_sprites; i++)
    {
        noclientupdatetime[i] = 0;
        time_spritesnapshot[i] = 0;
        time_spritesnapshot_mov[i] = 0;
    }

    for (i = 0; i < 5; i++)
        teamscore[i] = 0;

    addlinetologfile(gamelog, "Loading Map.", consolelogfilename);

    // playing over internet - optimize
    if (CVar::net_lan == LAN)
    {
        CVar::sv_guns_collide = true;
        CVar::sv_kits_collide = true;
    }
    if (CVar::net_lan == INTERNET)
    {
        CVar::sv_guns_collide = false;
        CVar::sv_kits_collide = false;
    }

    mapindex = 0;
    // StartMap = MapsList[MapIndex];

    // Load Map
    NotImplemented(NITag::NETWORK, "Is it really commented in reference version");
    /*
        if (not map.loadmap(StartMap))
        {
            GetServerMainConsole().console("Could Error not load map maps/" + StartMap.mapname +
".smap", debug_message_color); if (not map.loadmap("Arena"))
            {
                GetServerMainConsole().console("Could Error not load map maps/ Arena.smap",
                                    debug_message_color);
                return;
            }
            else
            {
                map.name = "Arena";
            }
        }
        else
            map.name = StartMap.name;
        NotImplemented(NITag::OTHER, "No checksum");
#if 0
        MapCheckSum = GetMapChecksum(MapChangeName, userdirectory);
#endif
    */

    if (getmapinfo(mapslist[mapindex], userdirectory, StartMap))
    {
        if (not map.loadmap(StartMap))
        {
            GetServerMainConsole().console("Could Error not load map " + StartMap.name,
                                           debug_message_color);
            Abort();
            return;
        }
    }
    NotImplemented(NITag::OTHER, "No checksum");
#if 0
    MapCheckSum = GetMapChecksum(StartMap);
#endif

#ifdef SCRIPT
    ScrptDispatcher.OnAfterMapChange(Map.Name);
#endif

    // Create Weapons
    addlinetologfile(gamelog, "Creating Weapons.", consolelogfilename);

    if (CVar::sv_realisticmode)
    {
        GetServerMainConsole().console("Realistic Mode ON", mode_message_color);
        starthealth = Constants::REALISTIC_HEALTH;
        loadweapons("weapons_realistic");
        lastwepmod = "weapons_realistic";
    }
    else
    {
        starthealth = Constants::DEFAULT_HEALTH;
        loadweapons("weapons");
        lastwepmod = "weapons";
    }

    // Weapons
    weaponsingame = 0;
    for (i = 1; i < 15; i++)
    {
        if (weaponactive[i] == 1)
            weaponsingame++;
    }

    for (j = 1; j < max_sprites; j++)
        for (i = 1; i < 11; i++)
            weaponsel[j][i] = weaponactive[i];

    if (CVar::sv_advancemode)
    {
        for (j = 1; j < max_sprites; j++)
            for (i = 1; i < 11; i++)
                weaponsel[j][i] = 1;

        GetServerMainConsole().console("Advance Mode ON", mode_message_color);
    }

    if (CVar::sv_gamemode == Constants::GAMESTYLE_DEATHMATCH)
    {
    }

    // add yellow flag
    if (CVar::sv_gamemode == Constants::GAMESTYLE_POINTMATCH)
    {
        randomizestart(a, 14);
        teamflag[1] = creatething(a, 255, Constants::OBJECT_POINTMATCH_FLAG, 255);
    }

    // add yellow flag
    if (CVar::sv_gamemode == Constants::GAMESTYLE_HTF)
    {
        randomizestart(a, 14);
        teamflag[1] = creatething(a, 255, Constants::OBJECT_POINTMATCH_FLAG, 255);
    }

    if (CVar::sv_gamemode == Constants::GAMESTYLE_CTF)
    {
        // red flag
        if (randomizestart(a, 5))
            teamflag[1] = creatething(a, 255, Constants::OBJECT_ALPHA_FLAG, 255);

        // blue flag
        if (randomizestart(a, 6))
            teamflag[2] = creatething(a, 255, Constants::OBJECT_BRAVO_FLAG, 255);
    }

    if (CVar::sv_gamemode == Constants::GAMESTYLE_RAMBO)
    {
        randomizestart(a, 15);
        creatething(a, 255, Constants::OBJECT_RAMBO_BOW, 255);
    }

    if (CVar::sv_gamemode == Constants::GAMESTYLE_INF)
    {
        // red flag
        if (randomizestart(a, 5))
            teamflag[1] = creatething(a, 255, Constants::OBJECT_ALPHA_FLAG, 255);

        // blue flag
        if (randomizestart(a, 6))
            teamflag[2] = creatething(a, 255, Constants::OBJECT_BRAVO_FLAG, 255);
    }

    if (not CVar::sv_survivalmode)
    {
        // spawn medikits
        spawnthings(Constants::OBJECT_MEDICAL_KIT, map.medikits);

        // spawn grenadekits
        if (CVar::sv_maxgrenades > 0)
            spawnthings(Constants::OBJECT_GRENADE_KIT, map.grenades);
    }
    else
    {
        GetServerMainConsole().console("Survival Mode ON", mode_message_color);
    }

    // stat gun
    if (CVar::sv_stationaryguns)
    {
        for (i = 1; i < max_spawnpoints; i++)
        {
            if (map.spawnpoints[i].active)
            {
                if (map.spawnpoints[i].team == 16)
                {
                    a.x = map.spawnpoints[i].x;
                    a.y = map.spawnpoints[i].y;
                    creatething(a, 255, Constants::OBJECT_STATIONARY_GUN, 255);
                }
            }
        }
    }

    switch (CVar::sv_gamemode)
    {
    case Constants::GAMESTYLE_DEATHMATCH:
        CVar::sv_killlimit = CVar::sv_dm_limit;
        break;
    case Constants::GAMESTYLE_POINTMATCH:
        CVar::sv_killlimit = CVar::sv_pm_limit;
        break;
    case Constants::GAMESTYLE_RAMBO:
        CVar::sv_killlimit = CVar::sv_rm_limit;
        break;
    case Constants::GAMESTYLE_TEAMMATCH:
        CVar::sv_killlimit = CVar::sv_tm_limit;
        break;
    case Constants::GAMESTYLE_CTF:
        CVar::sv_killlimit = CVar::sv_ctf_limit;
        break;
    case Constants::GAMESTYLE_INF:
        CVar::sv_killlimit = CVar::sv_inf_limit;
        break;
    case Constants::GAMESTYLE_HTF:
        CVar::sv_killlimit = CVar::sv_htf_limit;
        break;
    }

    // sort the players frag list
    sortplayers();

    mapchangecounter = -60;

    lastplayer = 0;

    timelimitcounter = CVar::sv_timelimit;

    // Wave respawn time
    updatewaverespawntime();
    waverespawncounter = waverespawntime;

    addlinetologfile(gamelog, "Starting Game Server.", consolelogfilename);

    InitNetworkServer(CVar::net_ip, CVar::net_port);

    if (GetServerNetwork()->Active() == true)
    {
        WriteLn("[NET] Game networking initialized.");
        auto addr = GetServerNetwork()->Address();
        WriteLn("[NET] Server is listening on " +
                GetServerNetwork()->GetStringAddress(&addr, true));
    }
    else
    {
        WriteLn("[NET] Failed to bind to " + std::string(CVar::net_ip) + ":" +
                inttostr(CVar::net_port));
        ShutDown();
        return;
    }

    serverport = GetServerNetwork()->Port();

    if (CVar::fileserver_enable)
    {
        NotImplemented(NITag::NETWORK, "No start file server");
#if 0
        StartFileServer();
#endif
    }
#ifdef ENABLE_FAE
    if (ac_enable)
    {
        WriteLn("[AC] Anti-Cheat enabled");
    }
#endif

    if (CVar::sv_lobby)
    {
        NotImplemented(NITag::NETWORK, "no lobby");
#if 0
        if (LobbyThread == nullptr)
        {
            LobbyThread = TLobbyThread.Create();
        }
#endif
    }

    for (k = 0; k < CVar::bots_random_alpha; k++)
        addbotplayer(randombot(), 1);
    for (k = 0; k < CVar::bots_random_bravo; k++)
        addbotplayer(randombot(), 2);
    for (k = 0; k < CVar::bots_random_charlie; k++)
        addbotplayer(randombot(), 3);
    for (k = 0; k < CVar::bots_random_delta; k++)
        addbotplayer(randombot(), 4);

    updategamestats();
}

bool preparemapchange(std::string Name)
{
    tmapinfo Status;
    bool Result = false;
    if (getmapinfo(Name, userdirectory, Status))
    {
        mapchange = Status;
        mapchangecounter = mapchangetime;
        // s} to client that map changes
        servermapchange(all_players);
        GetServerMainConsole().console("Next  map" + Status.name, game_message_color);
#ifdef SCRIPT
        ScrptDispatcher.OnBeforeMapChange(Status.Name);
#endif
        Result = true;
    }
    return Result;
}

void nextmap()
{
    LogDebugG("NextMap");

    if (mapslist.size() < 1)
    {
        GetServerMainConsole().console("Can"
                                       "t load maps from mapslist",
                                       game_message_color);
    }
    else
    {
        mapindex = mapindex + 1;

        if (mapindex >= mapslist.size())
            mapindex = 0;
        preparemapchange(mapslist[mapindex]);
    }
}

void spawnthings(std::int8_t Style, std::int8_t Amount)
{
    std::int32_t i, k, l, team;
    tvector2 a;

    LogTraceG("SpawnThings");

    k = 0;
    switch (Style)
    {
    case Constants::OBJECT_MEDICAL_KIT:
        k = 8;
        break;
    case Constants::OBJECT_GRENADE_KIT:
        k = 7;
        break;
    case Constants::OBJECT_FLAMER_KIT:
        k = 11;
        break;
    case Constants::OBJECT_PREDATOR_KIT:
        k = 13;
        break;
    case Constants::OBJECT_VEST_KIT:
        k = 10;
        break;
    case Constants::OBJECT_BERSERK_KIT:
        k = 12;
        break;
    case Constants::OBJECT_CLUSTER_KIT:
        k = 9;
        break;
    }

    for (i = 0; i < Amount; i++)
    {
        team = 0;
        if (CVar::sv_gamemode == Constants::GAMESTYLE_CTF)
        {
            if ((Style == Constants::OBJECT_MEDICAL_KIT) or
                (Style == Constants::OBJECT_GRENADE_KIT))
            {
                if (i % 2 == 0)
                {
                    team = 1;
                }
                else
                {
                    team = 2;
                }
            }
        }

        thing[max_things - 1].team = team;

        if (team == 0)
        {
            if (not randomizestart(a, k))
                return;
        }
        else if (not spawnboxes(a, k, max_things - 1))
            if (not randomizestart(a, k))
                return;

        a.x = a.x - Constants::SPAWNRANDOMVELOCITY +
              (Random(round(2 * 100 * Constants::SPAWNRANDOMVELOCITY)) / 100);
        a.y = a.y - Constants::SPAWNRANDOMVELOCITY +
              (Random(round(2 * 100 * Constants::SPAWNRANDOMVELOCITY)) / 100);
        l = creatething(a, 255, Style, 255);

        if ((l > 0) and (l < max_things + 1))
        {
            thing[l].team = team;
        }
    }
}

bool kickplayer(std::int8_t num, bool Ban, std::int32_t why, std::int32_t time, std::string Reason)
{
    std::int32_t i;
    std::string timestr;

    bool Result = false;
    LogDebugG("KickPlayer");

    // bound check
    if ((num > max_players) or (num < 1))
    {
        return Result;
    }

    i = num;

    if ((not sprite[i].active))
        return Result;

    if (((why == kick_cheat)) and (CVar::sv_anticheatkick))
    {
        return Result;
    }

    // check if admin should be kicked
    if (((why == kick_ping or why == kick_flooding or why == kick_voted) and
         (length(sprite[i].player->ip) > 5)))
    {
        if (isremoteadminip(sprite[i].player->ip) or isadminip(sprite[i].player->ip))
        {
            GetServerMainConsole().console(
                sprite[i].player->name + " is admin and cannot be kicked.", client_message_color);
            return Result;
        }
    }

    if (why == kick_leftgame)
    {
        switch (sprite[i].player->team)
        {
        case 0:
            GetServerMainConsole().console(sprite[i].player->name + " has left the game.",
                                           enter_message_color);
            break;
        case 1:
            GetServerMainConsole().console(sprite[i].player->name + " has left alpha team.",
                                           alphaj_message_color);
            break;
        case 2:
            GetServerMainConsole().console(sprite[i].player->name + " has left bravo team.",
                                           bravoj_message_color);
            break;
        case 3:
            GetServerMainConsole().console(sprite[i].player->name + " has left charlie team.",
                                           charliej_message_color);
            break;
        case 4:
            GetServerMainConsole().console(sprite[i].player->name + " has left delta team.",
                                           deltaj_message_color);
            break;
        case 5:
            GetServerMainConsole().console(sprite[i].player->name + " has left spectators",
                                           deltaj_message_color);
            break;
        }
    }

    if (not Ban and not(why == kick_leftgame) and not(why == kick_silent))
    {
        GetServerMainConsole().console(sprite[i].player->name + " has been kicked." +
                                           iif(sprite[i].player->controlmethod == bot,
                                               std::string(""), "(" + sprite[i].player->ip + ")"),
                                       client_message_color);
    }

    if (Ban)
    {
        addbannedip(sprite[i].player->ip, Reason, time);
#ifdef STEAM
        AddBannedHW(IntToStr(sprite[i].Player.SteamID.GetAccountID), Reason, time);
        {$ELSE} AddBannedHW(sprite[i].Player.HWid, Reason, time);
#endif
    }

    if (Ban)
    {
        if (time > 0)
        {
            NotImplemented(NITag::OTHER, "Msg ban");
#if 0
            TimeStr = iif((time + 1) div 3600 > 1439, IntToStr((time + 1) div 5184000) + " days",
                          IntToStr((time + 1) div 3600) + " minutes");
            GetServerMainConsole().console(sprite[i].player->name + " has been kicked and banned for " +
                                    TimeStr + " (" + Reason + ")",
                                client_message_color)
#endif
        }
        else
            GetServerMainConsole().console(
                sprite[i].player->name + " has been kicked and permanently banned (" + Reason + ")",
                client_message_color);
    }

    savetxtlists();

    if (not sprite[i].active)
        return Result;
#ifdef SCRIPT
    if (why in[KICK_AC, KICK_CHEAT, KICK_CONSOLE, KICK_PING, KICK_NORESPONSE, KICK_NOCHEATRESPONSE,
               KICK_FLOODING, KICK_VOTED, KICK_SILENT])
    {
        ScrptDispatcher.OnLeaveGame(i, true);
    }
#endif

    serverplayerdisconnect(i, why);

    if ((why != kick_ac) and (why != kick_cheat) and (why != kick_console) and (why != kick_voted))
    {
        sprite[i].dropweapon();
    }

    sprite[i].kill();

    Result = true;
    return Result;
}

#ifdef STEAM
initialization
// Mask exceptions on 32 and 64 bit fpc builds
{
    $IF defined(cpui386) or defined(cpux86_64)
}
// SetExceptionMask([exInvalidOp, exDenormalized, exZeroDivide, exOverflow, exUnderflow,
// exPrecision]);
#endif

void RunServer(int argc, const char *argv[])
{
    ActivateServer(argc, argv);
    writepid();

    if (progready)
    {
        startserver();
    }
    while (progready)
    {
        apponidle();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
