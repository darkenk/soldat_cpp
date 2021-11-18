#include "Cvar.hpp"
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
    auto ret = std::find_if(std::begin(value), std::end(value), [](auto const &c) {
        return !(std::isdigit(c) || c == '.' || c == 'f');
    });
    if (ret != std::end(value))
    {
        return false;
    }
    outValue = std::atof(value.data());
    return true;
}

// clang-format off
template<Config::Module M> CVarInt CVarStruct<M>::log_level{"log_level", "Sets log level", CVarFlags::NONE, 0};
template<Config::Module M> CVarBool CVarStruct<M>:: log_enable{"log_enable", "Enables logging to file", CVarFlags::NONE, false};
template<Config::Module M> CVarInt CVarStruct<M>::log_filesupdate{"log_filesupdate", "How often the log files should be updated", CVarFlags::NONE, 3600};
#ifdef SERVER
//template<Config::Module M> CVarBool CVarStruct<M>:: log_timestamp{ "log_timestamp", "Enables/Disables timestamps in console", CVarFlags::SERVER_FLAG, false};
#endif
#ifdef SERVER
#ifdef ENABLE_FAE
template<Config::Module M> CVarBool CVarStruct<M>:: ac_enable{
    "ac_enable", "Enables/Disables anti-cheat checks via Fae", true, true, CVarFlags::SERVER_FLAG,
    nullptr};
#endif
#endif

template<Config::Module M> CVarBool CVarStruct<M>:: fs_localmount{"fs_localmount", "Mount game directory as game mod", CVarFlags::CLIENT | CVarFlags::INITONLY, false};
template<Config::Module M> CVarString CVarStruct<M>:: fs_mod{"fs_mod", "File name of mod placed in mods directory (without .smod extension)", CVarFlags::CLIENT | CVarFlags::INITONLY, ""};
template<Config::Module M> CVarBool CVarStruct<M>:: fs_portable{"fs_portable", "Enables portable mode", CVarFlags::CLIENT | CVarFlags::INITONLY, true};
template<Config::Module M> CVarString CVarStruct<M>:: fs_basepath{"fs_basepath", "Path to base game directory", CVarFlags::CLIENT | CVarFlags::INITONLY, ""};
template<Config::Module M> CVarString CVarStruct<M>:: fs_userpath{"fs_userpath", "Path to user game directory", CVarFlags::CLIENT | CVarFlags::INITONLY,""};

template<Config::Module M> CVarBool CVarStruct<M>:: demo_autorecord{"demo_autorecord", "Auto record demos", CVarFlags::CLIENT, false};

template<Config::Module M> CVarInt CVarStruct<M>::sv_inf_redaward{"sv_inf_redaward", "Infiltration: Points awarded for a flag capture", CVarFlags::SERVER_FLAG, 30};

#ifndef SERVER // Render Cvars
template<Config::Module M> CVarInt CVarStruct<M>::r_fullscreen{ "r_fullscreen", "Set mode of fullscreen", CVarFlags::CLIENT, 0};
template<Config::Module M> CVarBool CVarStruct<M>:: r_weathereffects{"r_weathereffects", "Weather effects", CVarFlags::CLIENT, true};
template<Config::Module M> CVarBool CVarStruct<M>:: r_dithering{"r_dithering", "Dithering", CVarFlags::CLIENT, false};
template<Config::Module M> CVarInt CVarStruct<M>::r_swapeffect{"r_swapeffect", "Swap interval, 0 for immediate updates, 1 for updates synchronized with the vertical retrace, -1 for late swap tearing", CVarFlags::CLIENT, 0};
template<Config::Module M> CVarBool CVarStruct<M>:: r_compatibility{"r_compatibility","OpenGL compatibility mode (use fixed pipeline)", CVarFlags::CLIENT, false};
template<Config::Module M> CVarInt CVarStruct<M>::r_texturefilter{"r_texturefilter", "Texture filter (1 = nearest, 2 = linear)", CVarFlags::CLIENT, 2};
template<Config::Module M> CVarBool CVarStruct<M>:: r_optimizetextures{"r_optimizetextures", "Optimize textures (for older graphics card)", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: r_mipmapping{"r_mipmapping", "", CVarFlags::CLIENT, true};
template<Config::Module M> CVarFloat CVarStruct<M>:: r_mipmapbias{"r_mipmapbias", "", CVarFlags::CLIENT, -0.5};
template<Config::Module M> CVarBool CVarStruct<M>:: r_glfinish{"r_glfinish", "", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: r_smoothedges{"r_smoothedges", "", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: r_scaleinterface{"r_scaleinterface", "", CVarFlags::CLIENT, true};
template<Config::Module M> CVarInt CVarStruct<M>::r_maxsparks{"r_maxsparks", "", CVarFlags::CLIENT, 557};
template<Config::Module M> CVarBool CVarStruct<M>:: r_animations{"r_animations", "", CVarFlags::CLIENT, true};
template<Config::Module M> CVarBool CVarStruct<M>:: r_renderbackground{"r_renderbackground", "", CVarFlags::CLIENT, true};
template<Config::Module M> CVarInt CVarStruct<M>::r_maxfps{"r_maxfps", "", CVarFlags::CLIENT, 60};
template<Config::Module M> CVarBool CVarStruct<M>:: r_fpslimit{"r_fpslimit", "", CVarFlags::CLIENT, true};
template<Config::Module M> CVarInt CVarStruct<M>::r_resizefilter{"r_resizefilter", "", CVarFlags::CLIENT, 2};
template<Config::Module M> CVarInt CVarStruct<M>::r_sleeptime{"r_sleeptime", "", CVarFlags::CLIENT, 1};
template<Config::Module M> CVarInt CVarStruct<M>::r_screenwidth{"r_screenwidth", "", CVarFlags::CLIENT, 640};
template<Config::Module M> CVarInt CVarStruct<M>::r_screenheight{"r_screenheight", "", CVarFlags::CLIENT, 480};
template<Config::Module M> CVarInt CVarStruct<M>::r_renderwidth{"r_renderwidth", "", CVarFlags::CLIENT | CVarFlags::INITONLY, 0};
template<Config::Module M> CVarInt CVarStruct<M>::r_renderheight{"r_renderheight", "", CVarFlags::CLIENT | CVarFlags::INITONLY, 0};
template<Config::Module M> CVarBool CVarStruct<M>:: r_forcebg{"r_forcebg", "", CVarFlags::CLIENT, false};
template<Config::Module M> CVarColor CVarStruct<M>:: r_forcebg_color1{"r_forcebg_color1", "Force bg first color", CVarFlags::CLIENT, 0x00FF0000};
template<Config::Module M> CVarColor CVarStruct<M>:: r_forcebg_color2{"r_forcebg_color2", "Force bg second color", CVarFlags::CLIENT, 0x00FF0000};
template<Config::Module M> CVarBool CVarStruct<M>:: r_renderui{"r_renderui", "Enables interface rendering", CVarFlags::CLIENT, true};
template<Config::Module M> CVarFloat CVarStruct<M>:: r_zoom{"r_zoom", "Sets rendering zoom (only for spectators)", CVarFlags::CLIENT, 0.0};
template<Config::Module M> CVarInt CVarStruct<M>::r_msaa{"r_msaa", "Sets the number of samples for anti-aliasing (MSAA).", CVarFlags::CLIENT | CVarFlags::INITONLY, 0};

// Ui Cvars
template<Config::Module M> CVarBool CVarStruct<M>:: ui_playerindicator{"ui_playerindicator", "Enables player indicator", CVarFlags::CLIENT, true};
template<Config::Module M> CVarInt CVarStruct<M>::ui_minimap_transparency{"ui_minimap_transparency", "Transparency of minimap", CVarFlags::CLIENT, 230};
template<Config::Module M> CVarInt CVarStruct<M>::ui_minimap_posx{"ui_minimap_posx", "Horizontal position of minimap", CVarFlags::CLIENT, 285};
template<Config::Module M> CVarInt CVarStruct<M>::ui_minimap_posy{"ui_minimap_posy", "Vertical position of minimap", CVarFlags::CLIENT, 5};
template<Config::Module M> CVarBool CVarStruct<M>:: ui_bonuscolors{"ui_bonuscolors", "", CVarFlags::CLIENT, true};
template<Config::Module M> CVarString CVarStruct<M>:: ui_style{"ui_style", "", CVarFlags::CLIENT, "Default"};
template<Config::Module M> CVarInt CVarStruct<M>::ui_status_transparency{"ui_status_transparency", "Transparency of ui", CVarFlags::CLIENT, 200};
template<Config::Module M> CVarBool CVarStruct<M>:: ui_console{"ui_console", "Enables chat", CVarFlags::CLIENT, true};
template<Config::Module M> CVarInt CVarStruct<M>::ui_console_length{"ui_console_length", "Sets length of main console", CVarFlags::CLIENT, 6};
template<Config::Module M> CVarBool CVarStruct<M>:: ui_killconsole{"ui_killconsole", "Enables kill console", CVarFlags::CLIENT, true};
template<Config::Module M> CVarInt CVarStruct<M>::ui_killconsole_length{"ui_killconsole_length", "Sets length of kill console", CVarFlags::CLIENT, 15};
template<Config::Module M> CVarBool CVarStruct<M>:: ui_hidespectators{"ui_hidespectators", "Hides spectators from the fragsmenu",  CVarFlags::CLIENT, false};
//template<Config::Module M> CVarBool CVarStruct<M>::ui_sniperline{"ui_sniperline", "Draws a line between the player and the cursor", CVarFlags::CLIENT, false};

// Client cvars
template<Config::Module M> CVarFloat CVarStruct<M>:: cl_sensitivity{"cl_sensitivity", "Mouse sensitivity", CVarFlags::CLIENT, 1.0f};
template<Config::Module M> CVarBool CVarStruct<M>:: cl_endscreenshot{"cl_endscreenshot", "Take screenshot when game ends", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: cl_actionsnap{"cl_actionsnap", "Enables action snap", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: cl_screenshake{"cl_screenshake", "Enables screen shake from enemy fire", CVarFlags::CLIENT, true};
template<Config::Module M> CVarBool CVarStruct<M>:: cl_servermods{"cl_servermods", "Enables server mods feature", CVarFlags::CLIENT, true};


// Player cvars
template<Config::Module M> CVarString CVarStruct<M>:: cl_player_name{"cl_player_name",  "Player nickname", CVarFlags::CLIENT, "Major"};
template<Config::Module M> CVarInt CVarStruct<M>::cl_player_team{"cl_player_team", "Player team ID", CVarFlags::CLIENT, 0};
template<Config::Module M> CVarColor CVarStruct<M>:: cl_player_shirt{"cl_player_shirt", "Player shirt color", CVarFlags::CLIENT, 0x00304289};
template<Config::Module M> CVarColor CVarStruct<M>:: cl_player_pants{"cl_player_pants", "Player pants color", CVarFlags::CLIENT, 0x000000FF};
template<Config::Module M> CVarColor CVarStruct<M>:: cl_player_hair{"cl_player_hair", "Player hair color",  CVarFlags::CLIENT, 0x00000000};
template<Config::Module M> CVarColor CVarStruct<M>:: cl_player_jet{"cl_player_jet", "Player jet color", CVarFlags::CLIENT, 0x0000008B};
template<Config::Module M> CVarColor CVarStruct<M>:: cl_player_skin{"cl_player_skin", "Player skin color", CVarFlags::CLIENT, 0x00E6B478};

template<Config::Module M> CVarInt CVarStruct<M>::cl_player_hairstyle{"cl_player_hairstyle", "Player hair style", CVarFlags::CLIENT, 0};
template<Config::Module M> CVarInt CVarStruct<M>::cl_player_headstyle{"cl_player_headstyle", "Player head style", CVarFlags::CLIENT, 0};
template<Config::Module M> CVarInt CVarStruct<M>::cl_player_chainstyle{"cl_player_chainstyle", "Player chain style", CVarFlags::CLIENT, 0};
template<Config::Module M> CVarInt CVarStruct<M>::cl_player_secwep{"cl_player_secwep", "Player secondary weapon", CVarFlags::CLIENT, 0};
template<Config::Module M> CVarInt CVarStruct<M>::cl_player_wep{"cl_player_wep", "Player primary weapon", CVarFlags::CLIENT, 0};

template<Config::Module M> CVarInt CVarStruct<M>::cl_runs{"cl_runs", "Game runs", CVarFlags::CLIENT, 0};
template<Config::Module M> CVarString CVarStruct<M>:: cl_lang{"cl_lang", "Game language", CVarFlags::CLIENT | CVarFlags::INITONLY, ""};

// Demo cvars
template<Config::Module M> CVarFloat CVarStruct<M>:: demo_speed{"demo_speed", "Demo speed", CVarFlags::CLIENT, 1.0f};
template<Config::Module M> CVarInt CVarStruct<M>::demo_rate{"demo_rate", "Rate of demo recording", CVarFlags::CLIENT, 1};
template<Config::Module M> CVarBool CVarStruct<M>:: demo_showcrosshair{"demo_showcrosshair", "Enables rendering crosshair in demos", CVarFlags::CLIENT, true};

// Sound cvars
template<Config::Module M> CVarInt CVarStruct<M>::snd_volume{"snd_volume", "Sets sound volume", CVarFlags::CLIENT, 50};
template<Config::Module M> CVarBool CVarStruct<M>:: snd_effects_battle{"snd_effects_battle", "Enables battle sound effects", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: snd_effects_explosions{"snd_effects_explosions", "Enables sound explosions effects", CVarFlags::CLIENT, false};

// Matchmaking cvars
template<Config::Module M> CVarBool CVarStruct<M>:: mm_ranked{"mm_ranked", "Disable certain menu items in ranked matchmaking", CVarFlags::CLIENT, false};

// TODO: Remove
template<Config::Module M> CVarInt CVarStruct<M>::sv_respawntime{"sv_respawntime", "Respawn time in ticks (60 ticks = 1 second)", CVarFlags::SERVER_FLAG, 60};
//template<Config::Module M> CVarInt CVarStruct<M>::sv_inf_redaward{"sv_inf_redaward", "Infiltration: Points awarded for a flag capture", CVarFlags::SERVER_FLAG, 30};
template<Config::Module M> CVarBool CVarStruct<M>:: net_compression{"net_compression", "Enables/Disables compression of packets", CVarFlags::SERVER_FLAG, true};
template<Config::Module M> CVarBool CVarStruct<M>:: net_allowdownload{"net_allowdownload", "Enables/Disables file transfers", CVarFlags::SERVER_FLAG, true};

template<Config::Module M> CVarString CVarStruct<M>:: font_1_name{"font_1_name", "First font name", CVarFlags::CLIENT, "Play"};
template<Config::Module M> CVarString CVarStruct<M>:: font_1_filename{"font_1_filename", "First font filename", CVarFlags::CLIENT, "play-regular.ttf"};
template<Config::Module M> CVarInt CVarStruct<M>::font_1_scale{"font_1_scalex", "First font scale", CVarFlags::CLIENT, 150};

template<Config::Module M> CVarString CVarStruct<M>:: font_2_name{"font_2_name", "Second font name",  CVarFlags::CLIENT, "Lucida Console"};
template<Config::Module M> CVarString CVarStruct<M>:: font_2_filename{"font_2_filename", "Second font filename",  CVarFlags::CLIENT, "play-regular.ttf"};
template<Config::Module M> CVarInt CVarStruct<M>::font_2_scale{"font_2_scalex", "Second font scale", CVarFlags::CLIENT, 125};

template<Config::Module M> CVarInt CVarStruct<M>::font_menusize{"font_menusize",   "Menu font size", CVarFlags::CLIENT, 12};

template<Config::Module M> CVarInt CVarStruct<M>::font_consolesize{"font_consolesize", "Console font size", CVarFlags::CLIENT, 9};
template<Config::Module M> CVarInt CVarStruct<M>::font_consolesmallsize{"font_consolesmallsize", "Console small font size", CVarFlags::CLIENT, 7};

template<Config::Module M> CVarFloat CVarStruct<M>:: font_consolelineheight{"font_consolelineheight", "Console line height", CVarFlags::CLIENT, 1.5f};
template<Config::Module M> CVarInt CVarStruct<M>::font_bigsize{"font_bigsize", "Big message font size", CVarFlags::CLIENT, 28};
template<Config::Module M> CVarInt CVarStruct<M>::font_weaponmenusize{"font_weaponmenusize", "Weapon menu font size", CVarFlags::CLIENT, 8};
template<Config::Module M> CVarInt CVarStruct<M>::font_killconsolenamespace{"font_killconsolenamespace", "Kill console namespace size", CVarFlags::CLIENT, 8};

#else // Server cvars
template<Config::Module M> CVarInt CVarStruct<M>::sv_respawntime{"sv_respawntime", "Respawn time in ticks (60 ticks = 1 second)", CVarFlags::SERVER_FLAG, 360};
template<Config::Module M> CVarInt CVarStruct<M>::sv_respawntime_minwave{"sv_respawntime_minwave", "Min wave respawn time in ticks (60 ticks = 1 second)", CVarFlags::SERVER_FLAG, 120};
template<Config::Module M> CVarInt CVarStruct<M>::sv_respawntime_maxwave{"sv_respawntime_maxwave", "Max wave respawn time in ticks (60 ticks = 1 second)", CVarFlags::SERVER_FLAG, 240};

template<Config::Module M> CVarInt CVarStruct<M>::sv_dm_limit{"sv_dm_limit", "Deathmatch point limit", CVarFlags::SERVER_FLAG, 30};
template<Config::Module M> CVarInt CVarStruct<M>::sv_pm_limit{"sv_pm_limit", "Pointmatch point limit", CVarFlags::SERVER_FLAG, 30};
template<Config::Module M> CVarInt CVarStruct<M>::sv_tm_limit{"sv_tm_limit", "Teammatch point limit", CVarFlags::SERVER_FLAG, 60};
template<Config::Module M> CVarInt CVarStruct<M>::sv_rm_limit{"sv_rm_limit", "Rambomatch point limit", CVarFlags::SERVER_FLAG, 30};

template<Config::Module M> CVarInt CVarStruct<M>::sv_inf_limit{"sv_inf_limit", "Infiltration point limit", CVarFlags::SERVER_FLAG, 90};
template<Config::Module M> CVarInt CVarStruct<M>::sv_inf_bluelimit{"sv_inf_bluelimit", "Infiltration: Time for blue team to get points in seconds", CVarFlags::SERVER_FLAG, 5};

template<Config::Module M> CVarInt CVarStruct<M>::sv_htf_limit{"sv_htf_limit", "Hold the Flag point limit", CVarFlags::SERVER_FLAG, 80};
template<Config::Module M> CVarInt CVarStruct<M>::sv_htf_pointstime{"sv_htf_pointstime", "Hold The Flag points time", CVarFlags::SERVER_FLAG, 5};

template<Config::Module M> CVarInt CVarStruct<M>::sv_ctf_limit{"sv_ctf_limit", "Capture the Flag point limit", CVarFlags::SERVER_FLAG, 10};

template<Config::Module M> CVarInt CVarStruct<M>::sv_bonus_frequency{"sv_bonus_frequency", "The interval of bonuses occurring ingame.", CVarFlags::CLIENT, 0};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_bonus_flamer{"sv_bonus_flamer", "Flamer bonus availability", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_bonus_predator{"sv_bonus_predator", "Predator bonus availability", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_bonus_berserker{"sv_bonus_berserker", "Berserker bonus availability", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_bonus_vest{"sv_bonus_vest", "Bulletproof Vest bonus availability", CVarFlags::CLIENT, false};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_bonus_cluster{"sv_bonus_cluster", "Cluster Grenades bonus availability", CVarFlags::CLIENT, false};

template<Config::Module M> CVarBool CVarStruct<M>:: sv_stationaryguns{"sv_stationaryguns", "Enables/disables Stationary Guns ingame.", CVarFlags::SERVER_FLAG, false};

template<Config::Module M> CVarString CVarStruct<M>:: sv_password{"sv_password", "Sets game password", CVarFlags::SERVER_FLAG, ""};
template<Config::Module M> CVarString CVarStruct<M>:: sv_adminpassword{"sv_adminpassword", "Sets admin password", CVarFlags::SERVER_FLAG, ""};
template<Config::Module M> CVarInt CVarStruct<M>::sv_maxplayers{"sv_maxplayers", "Max number of players that can play on server", CVarFlags::SERVER_FLAG, 24};
template<Config::Module M> CVarInt CVarStruct<M>::sv_maxspectators{"sv_maxspectators", "Sets the limit of spectators", CVarFlags::SERVER_FLAG, 10};
//template<Config::Module M> CVarBool CVarStruct<M>:: sv_spectatorchat{"sv_spectatorchat", "Enables/disables spectators chat", CVarFlags::SERVER_FLAG, true};
template<Config::Module M> CVarString CVarStruct<M>:: sv_greeting{"sv_greeting", "First greeting message", CVarFlags::SERVER_FLAG, "Welcome"};
template<Config::Module M> CVarString CVarStruct<M>:: sv_greeting2{"sv_greeting2", "Second greeting message", CVarFlags::SERVER_FLAG, ""};
template<Config::Module M> CVarString CVarStruct<M>:: sv_greeting3{"sv_greeting3", "Third greeting message", CVarFlags::SERVER_FLAG, ""};
template<Config::Module M> CVarInt CVarStruct<M>::sv_minping{"sv_minping", "The minimum ping a player must have to play in your server", CVarFlags::SERVER_FLAG, 0};
template<Config::Module M> CVarInt CVarStruct<M>::sv_maxping{"sv_maxping", "The maximum ping a player can have to play in your server", CVarFlags::SERVER_FLAG, 400};
template<Config::Module M> CVarInt CVarStruct<M>::sv_votepercent{"sv_votepercent", "Percentage of players in favor of a map/kick vote to let it pass", CVarFlags::SERVER_FLAG, 60};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_lockedmode{"sv_lockedmode", "When Locked Mode is enabled, admins will not be able to type /loadcon, /password or /maxplayers", CVarFlags::SERVER_FLAG, false};
//template<Config::Module M> CVarString CVarStruct<M>:: sv_pidfilename{"sv_pidfilename", "Sets the Process ID file name", CVarFlags::SERVER_FLAG, "soldatserver.pid"};
template<Config::Module M> CVarString CVarStruct<M>:: sv_maplist{"sv_maplist", "Sets the name of maplist file", CVarFlags::SERVER_FLAG, "mapslist.txt"};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_lobby{"sv_lobby", "Enables/Disables registering in lobby", CVarFlags::SERVER_FLAG, true};
//template<Config::Module M> CVarString CVarStruct<M>:: sv_lobbyurl{"sv_lobbyurl", "URL of the lobby server", CVarFlags::SERVER_FLAG, "http://api.soldat.pl:443"};

//template<Config::Module M> CVarBool CVarStruct<M>:: sv_steamonly{"sv_steamonly", "Enables/Disables steam only mode", CVarFlags::SERVER_FLAG, false};

#ifdef STEAM
template<Config::Module M> CVarBool CVarStruct<M>:: sv_voicechat{"sv_voicechat", "Enables voice chat",   true,
                                     true,           CVarFlags::SERVER_FLAG, nullptr};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_voicechat_alltalk{"sv_voicechat_alltalk",
                              "Enables voice chat from enemy team and spectators",
                              false,
                              false,
                              CVarFlags::SERVER_FLAG,
                              nullptr};
template<Config::Module M> CVarString CVarStruct<M>:: sv_setsteamaccount{
    "sv_setsteamaccount",
    "Set game server account token to use for logging in to a persistent game server account",
    "",
    "",
    CVarFlags::SERVER_FLAG,
    nullptr,
    32,
    32};
#endif

template<Config::Module M> CVarInt CVarStruct<M>::sv_warnings_flood{"sv_warnings_flood", "How many warnings someone who is flooding the server gets before getting kicked for 20 minutes", CVarFlags::SERVER_FLAG, 4};
template<Config::Module M> CVarInt CVarStruct<M>::sv_warnings_ping{"sv_warnings_ping", "How many warnings someone who has a ping outside the required values above gets before being kicked for 15 minutes", CVarFlags::SERVER_FLAG, 10};

template<Config::Module M> CVarInt CVarStruct<M>::sv_warnings_votecheat{"sv_warnings_votecheat", "How many warnings someone gets before determining that they are automatically vote kicked", CVarFlags::SERVER_FLAG, 8};
template<Config::Module M> CVarInt CVarStruct<M>::sv_warnings_knifecheat{"sv_warnings_knifecheat", "How many warnings someone gets before determining that they are using a knife cheat", CVarFlags::SERVER_FLAG, 14};
template<Config::Module M> CVarInt CVarStruct<M>::sv_warnings_tk{"sv_warnings_tk", "Number of teamkills that needs to be done before a temporary ban is handed out", CVarFlags::SERVER_FLAG, 5};

template<Config::Module M> CVarBool CVarStruct<M>:: sv_anticheatkick{"sv_anticheatkick", "Enables/Disables anti cheat kicks", CVarFlags::SERVER_FLAG, false};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_punishtk{"sv_punishtk", "Enables/disables the built-in TK punish feature", CVarFlags::SERVER_FLAG, false};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_botbalance{"sv_botbalance", "Whether or not bots should count as players in the team balance", CVarFlags::SERVER_FLAG, false};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_echokills{"sv_echokills", "Echoes kills done to the admin console", CVarFlags::SERVER_FLAG, false};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_antimassflag{"sv_antimassflag", "", CVarFlags::SERVER_FLAG, true};
template<Config::Module M> CVarInt CVarStruct<M>::sv_healthcooldown{"sv_healthcooldown", "Amount of time (in seconds) a player needs to wait before he is able to pick up a second medikit. Use 0 to disable", CVarFlags::SERVER_FLAG, 2};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_teamcolors{"sv_teamcolors", "Overwrites shirt color in team games", CVarFlags::SERVER_FLAG, true};

// Network cvars
template<Config::Module M> CVarInt CVarStruct<M>::net_port{"net_port", "The port your server runs on, and player have to connect to", CVarFlags::SERVER_FLAG, 23073};
template<Config::Module M> CVarString CVarStruct<M>:: net_ip{"net_ip", "Binds server ports to specific ip address", CVarFlags::SERVER_FLAG, "0.0.0.0"};
template<Config::Module M> CVarString CVarStruct<M>:: net_adminip{"net_adminip", "Binds admin port to specific ip address", CVarFlags::SERVER_FLAG, ""};
template<Config::Module M> CVarInt CVarStruct<M>::net_lan{"net_lan", "Set to 1 to set server to LAN mode", CVarFlags::SERVER_FLAG, 0};
//template<Config::Module M> CVarInt CVarStruct<M>::net_maxconnections{"net_maxconnections", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 10};
//template<Config::Module M> CVarInt CVarStruct<M>::net_maxadminconnections{"net_maxadminconnections", "Maximum number of admin connections", CVarFlags::SERVER_FLAG, 10};

template<Config::Module M> CVarInt CVarStruct<M>::net_floodingpacketslan{"net_floodingpacketslan", "When running on a LAN, controls how many packets should be considered flooding", CVarFlags::SERVER_FLAG, 80};
template<Config::Module M> CVarInt CVarStruct<M>::net_floodingpacketsinternet{"net_floodingpacketsinternet", "When running on the Internet, controls how many packets should be considered flooding", CVarFlags::SERVER_FLAG, 42};

template<Config::Module M> CVarInt CVarStruct<M>::net_t1_snapshot{"net_t1_snapshot", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 35};
template<Config::Module M> CVarInt CVarStruct<M>::net_t1_majorsnapshot{"net_t1_majorsnapshot", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 19};
template<Config::Module M> CVarInt CVarStruct<M>::net_t1_deadsnapshot{"net_t1_deadsnapshot", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 50};
template<Config::Module M> CVarInt CVarStruct<M>::net_t1_heartbeat{"net_t1_heartbeat", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 135};
template<Config::Module M> CVarInt CVarStruct<M>::net_t1_delta{"net_t1_delta", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 4};
template<Config::Module M> CVarInt CVarStruct<M>::net_t1_ping{"net_t1_ping", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 21};
template<Config::Module M> CVarInt CVarStruct<M>::net_t1_thingsnapshot{"net_t1_thingsnapshot", "Maximum number of simultaneous file transfer connections", CVarFlags::SERVER_FLAG, 31};

// Bots cvars
template<Config::Module M> CVarInt CVarStruct<M>::bots_random_noteam{"bots_random_noteam", "Number of bots in DM, PM and RM modes", CVarFlags::SERVER_FLAG, 0};
template<Config::Module M> CVarInt CVarStruct<M>::bots_random_alpha{"bots_random_alpha", "Number of bots on Alpha in INF, CTF, HTF, PM and TM", CVarFlags::SERVER_FLAG, 0};
template<Config::Module M> CVarInt CVarStruct<M>::bots_random_bravo{"bots_random_bravo", "Number of bots on Bravo in INF, CTF, HTF, PM and TM", CVarFlags::SERVER_FLAG, 0};
template<Config::Module M> CVarInt CVarStruct<M>::bots_random_charlie{"bots_random_charlie", "Number of bots on Charlie in INF, CTF, HTF, PM and TM.", CVarFlags::SERVER_FLAG, 0};
template<Config::Module M> CVarInt CVarStruct<M>::bots_random_delta{"bots_random_delta", "Number of bots on Delta in INF, CTF, HTF, PM and TM", CVarFlags::SERVER_FLAG, 0};
template<Config::Module M> CVarBool CVarStruct<M>:: bots_chat{"bots_chat", "Enables/disables bots chatting", CVarFlags::SERVER_FLAG, true};

// ScriptCore cvars
template<Config::Module M> CVarBool CVarStruct<M>:: sc_enable{"sc_enable", "Enables/Disables scripting", CVarFlags::SERVER_FLAG, true};
template<Config::Module M> CVarString CVarStruct<M>:: sc_onscriptcrash{"sc_onscriptcrash", "What action to take when a script crashes. Available parameters are recompile, shutdown, ignore and disable", CVarFlags::SERVER_FLAG, "ignore"};
template<Config::Module M> CVarInt CVarStruct<M>::sc_maxscripts{"sc_maxscripts", "Set the maximum number of scripts which can be loaded by this server.", CVarFlags::SERVER_FLAG, 255};
template<Config::Module M> CVarBool CVarStruct<M>:: sc_safemode{"sc_safemode", "Enables/Disables Safe Mode for Scripts", CVarFlags::SERVER_FLAG, false};

// Fileserver cvars
template<Config::Module M> CVarBool CVarStruct<M>:: fileserver_enable{"fileserver_enable", "Enables/Disables built-in fileserver", CVarFlags::SERVER_FLAG | CVarFlags::INITONLY, true};
template<Config::Module M> CVarInt CVarStruct<M>::fileserver_port{"fileserver_port", "Binds fileserver to specific port", CVarFlags::SERVER_FLAG | CVarFlags::INITONLY, 0};
template<Config::Module M> CVarString CVarStruct<M>:: fileserver_ip{"fileserver_ip", "Binds fileserver to specific ip address", CVarFlags::SERVER_FLAG | CVarFlags::INITONLY, "0.0.0.0"};
#endif

template<Config::Module M> CVarInt CVarStruct<M>::bots_difficulty{"bots_difficulty", "Sets the skill level of the bots: 300=stupid, 200=poor, 100=normal, 50=hard, 10=impossible", CVarFlags::SERVER_FLAG, 100};

// Sync vars (todo);

template<Config::Module M> CVarInt CVarStruct<M>::sv_gamemode{"sv_gamemode", "Sets the gamemode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 3, 0};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_friendlyfire{"sv_friendlyfire", "Enables friendly fire", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 1};
template<Config::Module M> CVarInt CVarStruct<M>::sv_timelimit{"sv_timelimit", "Time limit of map", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 36000, 2};
template<Config::Module M> CVarInt CVarStruct<M>::sv_maxgrenades{"sv_maxgrenades", "Sets the max number of grenades a player can carry", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 2, 3};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_bullettime{"sv_bullettime", "Enables/disables the Bullet Time effect on server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 4};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_sniperline{"sv_sniperline", "Enables/disables the Sniper Line on server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 5};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_balanceteams{"sv_balanceteams", "Enables/disables team balancing", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 6};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_guns_collide{"sv_guns_collide", "Enables colliding guns", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 7};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_kits_collide{"sv_kits_collide", "Enables colliding kits", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 8};

template<Config::Module M> CVarBool CVarStruct<M>:: sv_survivalmode{"sv_survivalmode", "Enables survival mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 9};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_survivalmode_antispy{"sv_survivalmode_antispy", "Enables anti spy chat in survival mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 10};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_survivalmode_clearweapons{"sv_survivalmode_clearweapons", "Cluster Grenades bonus availability", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 11};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_realisticmode{"sv_realisticmode", "Enables realistic mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 12};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_advancemode{"sv_advancemode", "Enables advance mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 13};
template<Config::Module M> CVarInt CVarStruct<M>::sv_advancemode_amount{"sv_advancemode_amount", "Number of kills required in Advance Mode to gain a weapon.", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 2, 14};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_minimap{"sv_minimap", "Enables/disables minimap", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 15};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_advancedspectator{"sv_advancedspectator", "Enables/disables advanced spectator mode", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, true, 16};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_radio{"sv_radio", "Enables/disables radio chat", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, false, 17};
template<Config::Module M> CVarString CVarStruct<M>:: sv_info{"sv_info", "A website or e-mail address, or any other short text describing your server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, "", 18};
template<Config::Module M> CVarFloat CVarStruct<M>:: sv_gravity{"sv_gravity", "Gravity", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, 0.06f, 19};
template<Config::Module M> CVarString CVarStruct<M>:: sv_hostname{"sv_hostname", "Name of the server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, "Soldat Server", 20};
template<Config::Module M> CVarString CVarStruct<M>:: sv_website{"sv_website", "Server website", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, "", 21};

template<Config::Module M> CVarInt CVarStruct<M>::sv_killlimit{"sv_killlimit", "Game point limit", CVarFlags::SYNC, 10, 22};
template<Config::Module M> CVarString CVarStruct<M>:: sv_downloadurl{"sv_downloadurl", "URL from which clients can download missing assets", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, "", 23};
template<Config::Module M> CVarBool CVarStruct<M>:: sv_pure{"sv_pure","Requires clients to use the same game files (.smod) as the server", CVarFlags::SERVER_FLAG | CVarFlags::SYNC, true, 24};
// clang-format-on

template struct CVarStruct<Config::GetModule()>;
