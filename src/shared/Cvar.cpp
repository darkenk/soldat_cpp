#include "Cvar.hpp"
#include "ConfigModule.tweaks.hpp"
#include "shared/misc/SoldatConfig.hpp"
#include <cstring>
#include <map>

template <>
bool FromString<std::int32_t>(const std::string_view &value, std::int32_t &outValue) noexcept
{
  auto ret = std::find_if(std::begin(value), std::end(value),
                          [](auto const &c) { return !std::isdigit(c); });
  if (ret != std::end(value))
  {
    return false;
  }
  outValue = std::atoi(value.data());
  return true;
}

template <>
bool FromString<bool>(const std::string_view &value, bool &outValue) noexcept
{
  const std::map<const std::string_view, bool> conversionTable = {
    {"true", true}, {"True", true}, {"false", false}, {"False", false}};
  auto v = conversionTable.find(value);
  if (v == std::end(conversionTable))
  {
    return false;
  }
  outValue = v->second;
  return true;
}

template <>
bool FromString<std::string>(const std::string_view &value, std::string &outValue) noexcept
{
  outValue = value;
  return true;
}

template <>
bool FromString<float>(const std::string_view &value, float &outValue) noexcept
{
  auto ret = std::find_if(std::begin(value), std::end(value),
                          [](auto const &c) { return !(std::isdigit(c) || c == '.' || c == 'f'); });
  if (ret != std::end(value))
  {
    return false;
  }
  outValue = std::atof(value.data());
  return true;
}

//template <typename T, Config::Module M = Config::GetModule()>
//CVarBase<T, M> CVarBase<T, M>::InvalidCVar{"invalid", "", CVarFlags::NONE, {}};


template<>
CVarInt::Map CVarInt::CVars{};

template<>
CVarInt CVarInt::InvalidCVar{"invalid", "", CVarFlags::NONE, {}};

template<>
CVarBool::Map CVarBool::CVars{};

template<>
CVarBool CVarBool::InvalidCVar{"invalid", "", CVarFlags::NONE, {}};

template<>
CVarFloat::Map CVarFloat::CVars{};

template<>
CVarFloat CVarFloat::InvalidCVar{"invalid", "", CVarFlags::NONE, {}};

template<>
CVarColor::Map CVarColor::CVars{};

template<>
CVarColor CVarColor::InvalidCVar{"invalid", "", CVarFlags::NONE, {}};

template<>
CVarString::Map CVarString::CVars{};

template<>
CVarString CVarString::InvalidCVar{"invalid", "", CVarFlags::NONE, {}};


#ifdef SERVER
namespace CVarServer
#else
namespace CVarClient
#endif
{

// clang-format off
CVarInt log_level{"log_level", "Sets log level", CVarFlags::NONE, 0};
CVarBool log_enable{"log_enable", "Enables logging to file", CVarFlags::NONE, false};
CVarInt log_filesupdate{"log_filesupdate", "How often the log files should be updated", CVarFlags::NONE, 3600};
#ifdef SERVER
CVarBool log_timestamp{ "log_timestamp", "Enables/Disables timestamps in console", CVarFlags::SERVER_FLAG, false};
#endif
#ifdef SERVER
#ifdef ENABLE_FAE
CVarBool ac_enable{
    "ac_enable", "Enables/Disables anti-cheat checks via Fae", true, true, CVarFlags::SERVER_FLAG,
    nullptr};
#endif
#endif

CVarBool fs_localmount{"fs_localmount", "Mount game directory as game mod", CVarFlags::CLIENT | CVarFlags::INITONLY, false};
CVarString fs_mod{"fs_mod", "File name of mod placed in mods directory (without .smod extension)", CVarFlags::CLIENT | CVarFlags::INITONLY, ""};
CVarBool fs_portable{"fs_portable", "Enables portable mode", CVarFlags::CLIENT | CVarFlags::INITONLY, true};
CVarString fs_basepath{"fs_basepath", "Path to base game directory", CVarFlags::CLIENT | CVarFlags::INITONLY, ""};
CVarString fs_userpath{"fs_userpath", "Path to user game directory", CVarFlags::CLIENT | CVarFlags::INITONLY,""};

CVarBool demo_autorecord{"demo_autorecord", "Auto record demos", CVarFlags::CLIENT, false};

CVarInt sv_inf_redaward{"sv_inf_redaward", "Infiltration: Points awarded for a flag capture", CVarFlags::SERVER_FLAG, 30};

#ifndef SERVER // Render Cvars
CVarInt r_fullscreen{ "r_fullscreen", "Set mode of fullscreen", CVarFlags::CLIENT, 0};
CVarBool r_weathereffects{"r_weathereffects", "Weather effects", CVarFlags::CLIENT, true};
CVarBool r_dithering{"r_dithering", "Dithering", CVarFlags::CLIENT, false};
CVarInt r_swapeffect{"r_swapeffect", "Swap interval, 0 for immediate updates, 1 for updates synchronized with the vertical retrace, -1 for late swap tearing", CVarFlags::CLIENT, 0};
CVarBool r_compatibility{"r_compatibility","OpenGL compatibility mode (use fixed pipeline)", CVarFlags::CLIENT, false};
CVarInt r_texturefilter{"r_texturefilter", "Texture filter (1 = nearest, 2 = linear)", CVarFlags::CLIENT, 2};
CVarBool r_optimizetextures{"r_optimizetextures", "Optimize textures (for older graphics card)", CVarFlags::CLIENT, false};
CVarBool r_mipmapping{"r_mipmapping", "", CVarFlags::CLIENT, true};
CVarFloat r_mipmapbias{"r_mipmapbias", "", CVarFlags::CLIENT, -0.5};
CVarBool r_glfinish{"r_glfinish", "", CVarFlags::CLIENT, false};
CVarBool r_smoothedges{"r_smoothedges", "", CVarFlags::CLIENT, false};
CVarBool r_scaleinterface{"r_scaleinterface", "", CVarFlags::CLIENT, true};
CVarInt r_maxsparks{"r_maxsparks", "", CVarFlags::CLIENT, 557};
CVarBool r_animations{"r_animations", "", CVarFlags::CLIENT, true};
CVarBool r_renderbackground{"r_renderbackground", "", CVarFlags::CLIENT, true};
CVarInt r_maxfps{"r_maxfps", "", CVarFlags::CLIENT, 60};
CVarBool r_fpslimit{"r_fpslimit", "", CVarFlags::CLIENT, true};
CVarInt r_resizefilter{"r_resizefilter", "", CVarFlags::CLIENT, 2};
CVarInt r_sleeptime{"r_sleeptime", "", CVarFlags::CLIENT, 0};
CVarInt r_screenwidth{"r_screenwidth", "", CVarFlags::CLIENT, 640};
CVarInt r_screenheight{"r_screenheight", "", CVarFlags::CLIENT, 480};
CVarInt r_renderwidth{"r_renderwidth", "", CVarFlags::CLIENT | CVarFlags::INITONLY, 0};
CVarInt r_renderheight{"r_renderheight", "", CVarFlags::CLIENT | CVarFlags::INITONLY, 0};
CVarBool r_forcebg{"r_forcebg", "", CVarFlags::CLIENT, false};
CVarColor r_forcebg_color1{"r_forcebg_color1", "Force bg first color", CVarFlags::CLIENT, 0x00FF0000};
CVarColor r_forcebg_color2{"r_forcebg_color2", "Force bg second color", CVarFlags::CLIENT, 0x00FF0000};
CVarBool r_renderui{"r_renderui", "Enables interface rendering", CVarFlags::CLIENT, true};
CVarFloat r_zoom{"r_zoom", "Sets rendering zoom (only for spectators)", CVarFlags::CLIENT, 0.0};
CVarInt r_msaa{"r_msaa", "Sets the number of samples for anti-aliasing (MSAA).", CVarFlags::CLIENT | CVarFlags::INITONLY, 0};
// Ui Cvars
CVarBool ui_playerindicator{"ui_playerindicator", "Enables player indicator", CVarFlags::CLIENT, true};
CVarInt ui_minimap_transparency{"ui_minimap_transparency", "Transparency of minimap", CVarFlags::CLIENT, 230};
CVarInt ui_minimap_posx{"ui_minimap_posx", "Horizontal position of minimap", CVarFlags::CLIENT, 285};
CVarInt ui_minimap_posy{"ui_minimap_posy", "Vertical position of minimap", CVarFlags::CLIENT, 5};
CVarBool ui_bonuscolors{"ui_bonuscolors", "", CVarFlags::CLIENT, true};
CVarString ui_style{"ui_style", "", CVarFlags::CLIENT, "Default"};
CVarInt ui_status_transparency{"ui_status_transparency", "Transparency of ui", CVarFlags::CLIENT, 200};
CVarBool ui_console{"ui_console", "Enables chat", CVarFlags::CLIENT, true};
CVarInt ui_console_length{"ui_console_length", "Sets length of main console", CVarFlags::CLIENT, 6};
CVarBool ui_killconsole{"ui_killconsole", "Enables kill console", CVarFlags::CLIENT, true};
CVarInt ui_killconsole_length{"ui_killconsole_length", "Sets length of kill console", CVarFlags::CLIENT, 15};
CVarBool ui_hidespectators{"ui_hidespectators", "Hides spectators from the fragsmenu",  CVarFlags::CLIENT, false};
CVarBool ui_sniperline{"ui_sniperline", "Draws a line between the player and the cursor", CVarFlags::CLIENT, false};

// Client cvars
CVarFloat cl_sensitivity{"cl_sensitivity", "Mouse sensitivity", CVarFlags::CLIENT, 1.0f};
CVarBool cl_endscreenshot{"cl_endscreenshot", "Take screenshot when game ends", CVarFlags::CLIENT, false};
CVarBool cl_actionsnap{"cl_actionsnap", "Enables action snap", CVarFlags::CLIENT, false};
CVarBool cl_screenshake{"cl_screenshake", "Enables screen shake from enemy fire", CVarFlags::CLIENT, true};
CVarBool cl_servermods{"cl_servermods", "Enables server mods feature", CVarFlags::CLIENT, true};


// Player cvars
CVarString cl_player_name{"cl_player_name",  "Player nickname", CVarFlags::CLIENT, "Major"};
CVarInt cl_player_team{"cl_player_team", "Player team ID", CVarFlags::CLIENT, 0};
CVarColor cl_player_shirt{"cl_player_shirt", "Player shirt color", CVarFlags::CLIENT, 0x00304289};
CVarColor cl_player_pants{"cl_player_pants", "Player pants color", CVarFlags::CLIENT, 0x000000FF};
CVarColor cl_player_hair{"cl_player_hair", "Player hair color",  CVarFlags::CLIENT, 0x00000000};
CVarColor cl_player_jet{"cl_player_jet", "Player jet color", CVarFlags::CLIENT, 0x0000008B};
CVarColor cl_player_skin{"cl_player_skin", "Player skin color", CVarFlags::CLIENT, 0x00E6B478};

CVarInt cl_player_hairstyle{"cl_player_hairstyle", "Player hair style", CVarFlags::CLIENT, 0};
CVarInt cl_player_headstyle{"cl_player_headstyle", "Player head style", CVarFlags::CLIENT, 0};
CVarInt cl_player_chainstyle{"cl_player_chainstyle", "Player chain style", CVarFlags::CLIENT, 0};
CVarInt cl_player_secwep{"cl_player_secwep", "Player secondary weapon", CVarFlags::CLIENT, 0};
CVarInt cl_player_wep{"cl_player_wep", "Player primary weapon", CVarFlags::CLIENT, 0};

CVarInt cl_runs{"cl_runs", "Game runs", CVarFlags::CLIENT, 0};
CVarString cl_lang{"cl_lang", "Game language", CVarFlags::CLIENT | CVarFlags::INITONLY, ""};

// Demo cvars
CVarFloat demo_speed{"demo_speed", "Demo speed", CVarFlags::CLIENT, 1.0f};
CVarInt demo_rate{"demo_rate", "Rate of demo recording", CVarFlags::CLIENT, 1};
CVarBool demo_showcrosshair{"demo_showcrosshair", "Enables rendering crosshair in demos", CVarFlags::CLIENT, true};

// Sound cvars
CVarInt snd_volume{"snd_volume", "Sets sound volume", CVarFlags::CLIENT, 50};
CVarBool snd_effects_battle{"snd_effects_battle", "Enables battle sound effects", CVarFlags::CLIENT, false};
CVarBool snd_effects_explosions{"snd_effects_explosions", "Enables sound explosions effects", CVarFlags::CLIENT, false};

// Matchmaking cvars
CVarBool mm_ranked{"mm_ranked", "Disable certain menu items in ranked matchmaking", CVarFlags::CLIENT, false};

// TODO: Remove
CVarInt sv_respawntime{"sv_respawntime", "Respawn time in ticks (60 ticks = 1 second)", CVarFlags::SERVER_FLAG, 60};
//CVarInt sv_inf_redaward{"sv_inf_redaward", "Infiltration: Points awarded for a flag capture", CVarFlags::SERVER_FLAG, 30};
CVarBool net_compression{"net_compression", "Enables/Disables compression of packets", CVarFlags::SERVER_FLAG, true};
CVarBool net_allowdownload{"net_allowdownload", "Enables/Disables file transfers", CVarFlags::SERVER_FLAG, true};

CVarString font_1_name{"font_1_name", "First font name", CVarFlags::CLIENT, "Play"};
CVarString font_1_filename{"font_1_filename", "First font filename", CVarFlags::CLIENT, "play-regular.ttf"};
CVarInt font_1_scale{"font_1_scalex", "First font scale", CVarFlags::CLIENT, 150};

CVarString font_2_name{"font_2_name", "Second font name",  CVarFlags::CLIENT, "Lucida Console"};
CVarString font_2_filename{"font_2_filename", "Second font filename",  CVarFlags::CLIENT, "play-regular.ttf"};
CVarInt font_2_scale{"font_2_scalex", "Second font scale", CVarFlags::CLIENT, 125};

CVarInt font_menusize{"font_menusize",   "Menu font size", CVarFlags::CLIENT, 12};

CVarInt font_consolesize{"font_consolesize", "Console font size", CVarFlags::CLIENT, 9};
CVarInt font_consolesmallsize{"font_consolesmallsize", "Console small font size", CVarFlags::CLIENT, 7};

CVarFloat font_consolelineheight{"font_consolelineheight", "Console line height", CVarFlags::CLIENT, 1.5f};
CVarInt font_bigsize{"font_bigsize", "Big message font size", CVarFlags::CLIENT, 28};
CVarInt font_weaponmenusize{"font_weaponmenusize", "Weapon menu font size", CVarFlags::CLIENT, 8};
CVarInt font_killconsolenamespace{"font_killconsolenamespace", "Kill console namespace size", CVarFlags::CLIENT, 8};

#else // Server cvars
CVarInt sv_respawntime{"sv_respawntime", "Respawn time in ticks (60 ticks = 1 second)", CVarFlags::SERVER_FLAG, 360};
CVarInt sv_respawntime_minwave{"sv_respawntime_minwave", "Min wave respawn time in ticks (60 ticks = 1 second)", CVarFlags::SERVER_FLAG, 120};
CVarInt sv_respawntime_maxwave{"sv_respawntime_maxwave", "Max wave respawn time in ticks (60 ticks = 1 second)", CVarFlags::SERVER_FLAG, 240};

CVarInt sv_dm_limit{"sv_dm_limit", "Deathmatch point limit", CVarFlags::SERVER_FLAG, 30};
CVarInt sv_pm_limit{"sv_pm_limit", "Pointmatch point limit", CVarFlags::SERVER_FLAG, 30};
CVarInt sv_tm_limit{"sv_tm_limit", "Teammatch point limit", CVarFlags::SERVER_FLAG, 60};
CVarInt sv_rm_limit{"sv_rm_limit", "Rambomatch point limit", CVarFlags::SERVER_FLAG, 30};

CVarInt sv_inf_limit{"sv_inf_limit", "Infiltration point limit", CVarFlags::SERVER_FLAG, 90};
CVarInt sv_inf_bluelimit{"sv_inf_bluelimit", "Infiltration: Time for blue team to get points in seconds", CVarFlags::SERVER_FLAG, 5};

CVarInt sv_htf_limit{"sv_htf_limit", "Hold the Flag point limit", CVarFlags::SERVER_FLAG, 80};
CVarInt sv_htf_pointstime{"sv_htf_pointstime", "Hold The Flag points time", CVarFlags::SERVER_FLAG, 5};

CVarInt sv_ctf_limit{"sv_ctf_limit", "Capture the Flag point limit", CVarFlags::SERVER_FLAG, 10};

CVarInt sv_bonus_frequency{"sv_bonus_frequency", "The interval of bonuses occurring ingame.", CVarFlags::CLIENT, 0};
CVarBool sv_bonus_flamer{"sv_bonus_flamer", "Flamer bonus availability", CVarFlags::CLIENT, false};
CVarBool sv_bonus_predator{"sv_bonus_predator", "Predator bonus availability", CVarFlags::CLIENT, false};
CVarBool sv_bonus_berserker{"sv_bonus_berserker", "Berserker bonus availability", CVarFlags::CLIENT, false};
CVarBool sv_bonus_vest{"sv_bonus_vest", "Bulletproof Vest bonus availability", CVarFlags::CLIENT, false};
CVarBool sv_bonus_cluster{"sv_bonus_cluster", "Cluster Grenades bonus availability", CVarFlags::CLIENT, false};

CVarBool sv_stationaryguns{"sv_stationaryguns", "Enables/disables Stationary Guns ingame.", CVarFlags::SERVER_FLAG, false};

CVarString sv_password{"sv_password", "Sets game password", CVarFlags::SERVER_FLAG, ""};
CVarString sv_adminpassword{"sv_adminpassword", "Sets admin password", CVarFlags::SERVER_FLAG, ""};
CVarInt sv_maxplayers{"sv_maxplayers", "Max number of players that can play on server", CVarFlags::SERVER_FLAG, 24};
CVarInt sv_maxspectators{"sv_maxspectators", "Sets the limit of spectators", CVarFlags::SERVER_FLAG, 10};
CVarBool sv_spectatorchat{"sv_spectatorchat", "Enables/disables spectators chat", CVarFlags::SERVER_FLAG, true};
CVarString sv_greeting{"sv_greeting", "First greeting message", CVarFlags::SERVER_FLAG, "Welcome"};
CVarString sv_greeting2{"sv_greeting2", "Second greeting message", CVarFlags::SERVER_FLAG, ""};
CVarString sv_greeting3{"sv_greeting3", "Third greeting message", CVarFlags::SERVER_FLAG, ""};
CVarInt sv_minping{"sv_minping", "The minimum ping a player must have to play in your server", CVarFlags::SERVER_FLAG, 0};
CVarInt sv_maxping{"sv_maxping", "The maximum ping a player can have to play in your server", CVarFlags::SERVER_FLAG, 400};
CVarInt sv_votepercent{"sv_votepercent", "Percentage of players in favor of a map/kick vote to let it pass", CVarFlags::SERVER_FLAG, 60};
CVarBool sv_lockedmode{"sv_lockedmode", "When Locked Mode is enabled, admins will not be able to type /loadcon, /password or /maxplayers", CVarFlags::SERVER_FLAG, false};
CVarString sv_pidfilename{"sv_pidfilename", "Sets the Process ID file name", CVarFlags::SERVER_FLAG, "soldatserver.pid"};
CVarString sv_maplist{"sv_maplist", "Sets the name of maplist file", CVarFlags::SERVER_FLAG, "mapslist.txt"};
CVarBool sv_lobby{"sv_lobby", "Enables/Disables registering in lobby", CVarFlags::SERVER_FLAG, true};
CVarString sv_lobbyurl{"sv_lobbyurl", "URL of the lobby server", CVarFlags::SERVER_FLAG, "http://api.soldat.pl:443"};

CVarBool sv_steamonly{"sv_steamonly", "Enables/Disables steam only mode", CVarFlags::SERVER_FLAG, false};

#ifdef STEAM
CVarBool sv_voicechat{"sv_voicechat", "Enables voice chat",   true,
                                     true,           CVarFlags::SERVER_FLAG, nullptr};
CVarBool sv_voicechat_alltalk{"sv_voicechat_alltalk",
                              "Enables voice chat from enemy team and spectators",
                              false,
                              false,
                              CVarFlags::SERVER_FLAG,
                              nullptr};
CVarString sv_setsteamaccount{
    "sv_setsteamaccount",
    "Set game server account token to use for logging in to a persistent game server account",
    "",
    "",
    CVarFlags::SERVER_FLAG,
    nullptr,
    32,
    32};
#endif

CVarInt sv_warnings_flood{"sv_warnings_flood", "How many warnings someone who is flooding the server gets before getting kicked for 20 minutes", CVarFlags::SERVER_FLAG, 4};
CVarInt sv_warnings_ping{"sv_warnings_ping", "How many warnings someone who has a ping outside the required values above gets before being kicked for 15 minutes", CVarFlags::SERVER_FLAG, 10};

CVarInt sv_warnings_votecheat{"sv_warnings_votecheat", "How many warnings someone gets before determining that they are automatically vote kicked", CVarFlags::SERVER_FLAG, 8};
CVarInt sv_warnings_knifecheat{"sv_warnings_knifecheat", "How many warnings someone gets before determining that they are using a knife cheat", CVarFlags::SERVER_FLAG, 14};
CVarInt sv_warnings_tk{"sv_warnings_tk", "Number of teamkills that needs to be done before a temporary ban is handed out", CVarFlags::SERVER_FLAG, 5};

CVarBool sv_anticheatkick{"sv_anticheatkick", "Enables/Disables anti cheat kicks", CVarFlags::SERVER_FLAG, false};
CVarBool sv_punishtk{"sv_punishtk", "Enables/disables the built-in TK punish feature", CVarFlags::SERVER_FLAG, false};
CVarBool sv_botbalance{"sv_botbalance", "Whether or not bots should count as players in the team balance", CVarFlags::SERVER_FLAG, false};
CVarBool sv_echokills{"sv_echokills", "Echoes kills done to the admin console", CVarFlags::SERVER_FLAG, false};
CVarBool sv_antimassflag{"sv_antimassflag", "", CVarFlags::SERVER_FLAG, true};
CVarInt sv_healthcooldown{"sv_healthcooldown", "Amount of time (in seconds) a player needs to wait before he is able to pick up a second medikit. Use 0 to disable", CVarFlags::SERVER_FLAG, 2};
CVarBool sv_teamcolors{"sv_teamcolors", "Overwrites shirt color in team games", CVarFlags::SERVER_FLAG, true};

// Network cvars
CVarInt net_port{"net_port", "The port your server runs on, and player have to connect to", CVarFlags::SERVER_FLAG, 23073};
CVarString net_ip{"net_ip", "Binds server ports to specific ip address", CVarFlags::SERVER_FLAG, "0.0.0.0"};
CVarString net_adminip{"net_adminip", "Binds admin port to specific ip address", CVarFlags::SERVER_FLAG, ""};
CVarInt net_lan{"net_lan", "Set to 1 to set server to LAN mode", CVarFlags::SERVER_FLAG, 0};
CVarInt net_maxconnections{"net_maxconnections", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 10};
CVarInt net_maxadminconnections{"net_maxadminconnections", "Maximum number of admin connections", CVarFlags::SERVER_FLAG, 10};

CVarInt net_floodingpacketslan{"net_floodingpacketslan", "When running on a LAN, controls how many packets should be considered flooding", CVarFlags::SERVER_FLAG, 80};
CVarInt net_floodingpacketsinternet{"net_floodingpacketsinternet", "When running on the Internet, controls how many packets should be considered flooding", CVarFlags::SERVER_FLAG, 42};

CVarInt net_t1_snapshot{"net_t1_snapshot", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 35};
CVarInt net_t1_majorsnapshot{"net_t1_majorsnapshot", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 19};
CVarInt net_t1_deadsnapshot{"net_t1_deadsnapshot", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 50};
CVarInt net_t1_heartbeat{"net_t1_heartbeat", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 135};
CVarInt net_t1_delta{"net_t1_delta", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 4};
CVarInt net_t1_ping{"net_t1_ping", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 21};
CVarInt net_t1_thingsnapshot{"net_t1_thingsnapshot", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 31};

// Bots cvars
CVarInt bots_random_noteam{"bots_random_noteam", "Number of bots in DM, PM and RM modes", CVarFlags::SERVER_FLAG, 0};
CVarInt bots_random_alpha{"bots_random_alpha", "Number of bots on Alpha in INF, CTF, HTF, PM and TM", CVarFlags::SERVER_FLAG, 0};
CVarInt bots_random_bravo{"bots_random_bravo", "Number of bots on Bravo in INF, CTF, HTF, PM and TM", CVarFlags::SERVER_FLAG, 0};
CVarInt bots_random_charlie{"bots_random_charlie", "Number of bots on Charlie in INF, CTF, HTF, PM and TM.", CVarFlags::SERVER_FLAG, 0};
CVarInt bots_random_delta{"bots_random_delta", "Number of bots on Delta in INF, CTF, HTF, PM and TM", CVarFlags::SERVER_FLAG, 0};
CVarBool bots_chat{"bots_chat", "Enables/disables bots chatting", CVarFlags::SERVER_FLAG, true};

// ScriptCore cvars
CVarBool sc_enable{"sc_enable", "Enables/Disables scripting", CVarFlags::SERVER_FLAG, true};
CVarString sc_onscriptcrash{"sc_onscriptcrash", "What action to take when a script crashes. Available parameters are recompile, shutdown, ignore and disable", CVarFlags::SERVER_FLAG, "ignore"};
CVarInt sc_maxscripts{"sc_maxscripts", "Set the maximum number of scripts which can be loaded by this server.", CVarFlags::SERVER_FLAG, 255};
CVarBool sc_safemode{"sc_safemode", "Enables/Disables Safe Mode for Scripts", CVarFlags::SERVER_FLAG, false};

// Fileserver cvars
CVarBool fileserver_enable{"fileserver_enable", "Enables/Disables built-in fileserver", CVarFlags::SERVER_FLAG | CVarFlags::INITONLY, true};
CVarInt fileserver_port{"fileserver_port", "Binds fileserver to specific port", CVarFlags::SERVER_FLAG | CVarFlags::INITONLY, 0};
CVarString fileserver_ip{"fileserver_ip", "Binds fileserver to specific ip address", CVarFlags::SERVER_FLAG | CVarFlags::INITONLY, "0.0.0.0"};
#endif

CVarInt bots_difficulty{"bots_difficulty", "Sets the skill level of the bots: 300=stupid, 200=poor, 100=normal, 50=hard, 10=impossible", CVarFlags::SERVER_FLAG, 100};

// Sync vars (todo);

CVarInt sv_gamemode{"sv_gamemode", "Sets the gamemode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 3, 0};
CVarBool sv_friendlyfire{"sv_friendlyfire", "Enables friendly fire", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 1};
CVarInt sv_timelimit{"sv_timelimit", "Time limit of map", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 36000, 2};
CVarInt sv_maxgrenades{"sv_maxgrenades", "Sets the max number of grenades a player can carry", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 2, 3};
CVarBool sv_bullettime{"sv_bullettime", "Enables/disables the Bullet Time effect on server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 4};
CVarBool sv_sniperline{"sv_sniperline", "Enables/disables the Sniper Line on server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 5};
CVarBool sv_balanceteams{"sv_balanceteams", "Enables/disables team balancing", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 6};
CVarBool sv_guns_collide{"sv_guns_collide", "Enables colliding guns", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 7};
CVarBool sv_kits_collide{"sv_kits_collide", "Enables colliding kits", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 8};

CVarBool sv_survivalmode{"sv_survivalmode", "Enables survival mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 9};
CVarBool sv_survivalmode_antispy{"sv_survivalmode_antispy", "Enables anti spy chat in survival mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 10};
CVarBool sv_survivalmode_clearweapons{"sv_survivalmode_clearweapons", "Cluster Grenades bonus availability", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 11};
CVarBool sv_realisticmode{"sv_realisticmode", "Enables realistic mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 12};
CVarBool sv_advancemode{"sv_advancemode", "Enables advance mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 13};
CVarInt sv_advancemode_amount{"sv_advancemode_amount", "Number of kills required in Advance Mode to gain a weapon.", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 2, 14};
CVarBool sv_minimap{"sv_minimap", "Enables/disables minimap", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 15};
CVarBool sv_advancedspectator{"sv_advancedspectator", "Enables/disables advanced spectator mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, true, 16};
CVarBool sv_radio{"sv_radio", "Enables/disables radio chat", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 17};
CVarString sv_info{"sv_info", "A website or e-mail address, or any other short text describing your server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, "", 18};
CVarFloat sv_gravity{"sv_gravity", "Gravity", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 0.06f, 19};
CVarString sv_hostname{"sv_hostname", "Name of the server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, "Soldat Server", 20};
CVarString sv_website{"sv_website", "Server website", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, "", 21};

CVarInt sv_killlimit{"sv_killlimit", "Game point limit", CVarFlags::SYNC, 10, 22};
CVarString sv_downloadurl{"sv_downloadurl", "URL from which clients can download missing assets", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, "", 23};
CVarBool sv_pure{"sv_pure","Requires clients to use the same game files (.smod) as the server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, true, 24};
// clang-format-on
};
