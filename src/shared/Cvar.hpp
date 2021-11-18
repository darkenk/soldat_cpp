#pragma once

#include "misc/FlagSet.hpp"
#include "misc/SoldatConfig.hpp"
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>

template <typename T, Config::Module M = Config::GetModule()>
bool FromString(const std::string_view &value, T &outValue) noexcept;

enum class CVarFlags : std::uint32_t
{
    // clang-format off
    NONE      = 0x0,
    IMMUTABLE = 0x1,   // can't be changed after set
    ARCHIVE   = 0x2,   // save cvar to cfg file
    SPONLY    = 0x4,   // only in singleplayer mode
    NOTIFY    = 0x8,   // notify players after change
    MODIFIED  = 0x10,  // this flag is set after cvar changed initial value
    CLIENT    = 0x20,  // client cvar
    SERVER_FLAG    = 0x40,  // server cvar
    SYNC      = 0x80,  // sync cvar to client cvar
    SCRIPT    = 0x100, // cvar set by script
    INITONLY  = 0x200  // cvar can be changed only at startup
    // clang-format on
};

template <>
struct EnableFlagSet<CVarFlags>
{
    static constexpr bool enable = true;
};

template <typename T, Config::Module M = Config::GetModule()>
class CVarBase
{
  public:
    CVarBase(const std::string_view &name, const std::string_view &description,
             FlagSet<CVarFlags> flags, const T &value, const std::uint8_t id = 255)
        : Value{value}, Name{name}, Description{description}, Flags{flags}, Id{id}
    {
        if (CVars.contains(Name))
        {
            throw std::runtime_error("CVar has been created already");
        }
        CVars.emplace(std::make_pair(Name, this));
    };

    CVarBase(const CVarBase &ref) = delete;

    bool operator==(const T &ref) const
    {
        return ref == Value;
    }

    operator T() const
    {
        return Value;
    }

    T &operator=(const T &v)
    {
        Value = v;
        return Value;
    }

    // TODO: does it make sense?
    CVarBase &operator=(const CVarBase &ref)
    {
        Value = ref.Value;
        return *this;
    }

    bool ParseAndSetValue(const std::string_view &value) noexcept
    {
        return FromString<T>(value, Value);
    }

    std::string ValueAsString() const noexcept
    {
        return std::to_string(Value);
    }

    static CVarBase &Find(const std::string &cvarName) noexcept
    {
        auto it = CVars.find(cvarName);
        if (it == std::end(CVars))
        {
            return InvalidCVar;
        }
        return *(it->second);
    }

    static CVarBase &Find(const std::uint8_t id) noexcept
    {
        auto it = std::find_if(CVars.begin(), CVars.end(),
                               [id](const auto &cv) { return cv.second->Id == id; });
        if (it == std::end(CVars))
        {
            return InvalidCVar;
        }
        return *(it->second);
    }

    bool IsValid() const
    {
        return this != &InvalidCVar;
    }

    bool IsSyncable() const
    {
        return Flags.Test(CVarFlags::SYNC);
    }

    class AllCVars
    {
      public:
        // member typedefs provided through inheriting from std::iterator
        class iterator : public std::iterator<std::input_iterator_tag, CVarBase>
        {
          public:
            using InternalIterType = typename std::map<const std::string, CVarBase *>::iterator;
            InternalIterType InternalIter;
            explicit iterator(InternalIterType iter) : InternalIter(iter)
            {
            }
            iterator &operator++()
            {
                InternalIter++;
                return *this;
            }
            iterator operator++(int)
            {
                iterator retval = *this;
                ++(*this);
                return retval;
            }
            bool operator==(iterator other) const
            {
                return InternalIter == other.InternalIter;
            }
            bool operator!=(iterator other) const
            {
                return !(*this == other);
            }
            CVarBase &operator*() const
            {
                return *(InternalIter->second);
            }
        };
        iterator begin()
        {
            return iterator(CVarBase::CVars.begin());
        }
        iterator end()
        {
            return iterator(CVarBase::CVars.end());
        }
    };

    static AllCVars GetAllCVars()
    {
        return AllCVars();
    }

    std::uint8_t GetId() const
    {
        return Id;
    }

    const std::string &GetName() const
    {
        return Name;
    }

  private:
    T Value;
    const std::string Name;
    const std::string Description;
    FlagSet<CVarFlags> Flags;
    std::uint8_t Id;

    using Map = std::map<const std::string, CVarBase *>;

    static CVarBase InvalidCVar;
    static Map CVars;
};

template <typename T, Config::Module M>
CVarBase<T, M> CVarBase<T, M>::InvalidCVar{"invalid", "", CVarFlags::NONE, {}};

template <typename T, Config::Module M>
typename CVarBase<T, M>::Map CVarBase<T, M>::CVars{};

using CVarBool = CVarBase<bool>;
using CVarString = CVarBase<std::string>;
using CVarInt = CVarBase<std::int32_t>;
using CVarFloat = CVarBase<float>;
using CVarColor = CVarBase<std::uint32_t>;

template <Config::Module M = Config::GetModule()>
struct CVarStruct
{
    static CVarBool log_enable;
    static CVarInt log_level;
    static CVarInt log_filesupdate;

    static CVarBool fs_localmount;
    static CVarString fs_mod;
    static CVarBool fs_portable;
    static CVarString fs_userpath;
    static CVarString fs_basepath;

    static CVarInt r_fullscreen;
    static CVarBool r_weathereffects;
    static CVarBool r_dithering;
    static CVarInt r_swapeffect;
    static CVarBool r_compatibility;
    static CVarInt r_texturefilter;
    static CVarBool r_optimizetextures;
    static CVarBool r_mipmapping;
    static CVarFloat r_mipmapbias;
    static CVarBool r_glfinish;
    static CVarBool r_smoothedges;
    static CVarBool r_scaleinterface;
    static CVarInt r_maxsparks;
    static CVarBool r_animations;
    static CVarBool r_renderbackground;
    static CVarInt r_maxfps;
    static CVarBool r_fpslimit;
    static CVarInt r_resizefilter;
    static CVarInt r_sleeptime;
    static CVarInt r_screenwidth;
    static CVarInt r_screenheight;
    static CVarInt r_renderwidth;
    static CVarInt r_renderheight;
    static CVarBool r_forcebg;
    static CVarColor r_forcebg_color1;
    static CVarColor r_forcebg_color2;
    static CVarBool r_renderui;
    static CVarFloat r_zoom;
    static CVarInt r_msaa;

    static CVarBool ui_playerindicator;
    static CVarInt ui_minimap_transparency;
    static CVarInt ui_minimap_posx;
    static CVarInt ui_minimap_posy;
    static CVarBool ui_bonuscolors;
    static CVarString ui_style;
    static CVarInt ui_status_transparency;
    static CVarBool ui_console;
    static CVarInt ui_console_length;
    static CVarBool ui_killconsole;
    static CVarInt ui_killconsole_length;
    static CVarBool ui_hidespectators;
    static CVarFloat cl_sensitivity;
    static CVarBool cl_endscreenshot;
    static CVarBool cl_actionsnap;
    static CVarBool cl_screenshake;
    static CVarBool cl_servermods;
    static CVarString cl_player_name;
    static CVarInt cl_player_team;
    static CVarColor cl_player_shirt;
    static CVarColor cl_player_pants;
    static CVarColor cl_player_hair;
    static CVarColor cl_player_jet;
    static CVarColor cl_player_skin;

    static CVarInt cl_player_hairstyle;
    static CVarInt cl_player_headstyle;
    static CVarInt cl_player_chainstyle;
    static CVarInt cl_player_secwep;
    static CVarInt cl_player_wep;

    static CVarInt cl_runs;
    static CVarString cl_lang;

    static CVarFloat demo_speed;
    static CVarInt demo_rate;
    static CVarBool demo_showcrosshair;
    static CVarBool demo_autorecord;

    static CVarInt snd_volume;
    static CVarBool snd_effects_battle;
    static CVarBool snd_effects_explosions;

    static CVarString font_1_name;
    static CVarString font_1_filename;
    static CVarInt font_1_scale;
    static CVarString font_2_name;
    static CVarString font_2_filename;
    static CVarInt font_2_scale;
    static CVarInt font_menusize;
    static CVarInt font_consolesize;
    static CVarInt font_consolesmallsize;
    static CVarFloat font_consolelineheight;
    static CVarInt font_bigsize;
    static CVarInt font_weaponmenusize;
    static CVarInt font_killconsolenamespace;

    // Matchhmaking cvars
    static CVarBool mm_ranked;

    static CVarInt sv_respawntime;   // TODO: Remove
    static CVarInt sv_inf_redaward;  // TODO: Remove
    static CVarInt net_contype;      // TODO: Remove
    static CVarBool net_compression; // TODO: Remove
    static CVarBool net_allowdownload;
    static CVarInt sv_gamemode;
    static CVarBool sv_friendlyfire;
    static CVarInt sv_timelimit;
    static CVarInt sv_maxgrenades;
    static CVarBool sv_bullettime;
    static CVarBool sv_sniperline;
    static CVarBool sv_balanceteams;
    static CVarBool sv_guns_collide;
    static CVarBool sv_kits_collide;
    static CVarBool sv_survivalmode;
    static CVarBool sv_survivalmode_antispy;
    static CVarBool sv_survivalmode_clearweapons;
    static CVarBool sv_realisticmode;
    static CVarBool sv_advancemode;
    static CVarInt sv_advancemode_amount;
    static CVarBool sv_minimap;
    static CVarBool sv_advancedspectator;
    static CVarBool sv_radio;
    static CVarString sv_info;
    static CVarFloat sv_gravity;
    static CVarString sv_hostname;
    static CVarInt sv_killlimit;
    static CVarString sv_downloadurl;
    static CVarBool sv_pure;
    static CVarString sv_website;

    // SERVER
    static CVarInt bots_difficulty;

#ifdef SERVER
    static CVarBool bots_chat;
    static CVarString sv_maplist;
    static CVarInt sv_votepercent;
    static CVarBool sv_stationaryguns;
    static CVarInt sv_healthcooldown;
    static CVarInt sv_warnings_tk;
    static CVarBool sv_punishtk;
    static CVarInt sv_maxspectators;
    static CVarBool sv_echokills;
    static CVarInt sv_respawntime_minwave;
    static CVarInt sv_respawntime_maxwave;
    static CVarBool sv_botbalance;
    static CVarString sv_adminpassword;
    static CVarString sv_password;
    static CVarInt sv_maxplayers;
    static CVarBool sv_bonus_flamer;
    static CVarInt sv_warnings_knifecheat;
    static CVarString sv_greeting;
    static CVarString sv_greeting2;
    static CVarString sv_greeting3;
    static CVarBool sv_lockedmode;
    static CVarInt net_lan;
    static CVarInt net_t1_snapshot;
    static CVarInt net_t1_majorsnapshot;
    static CVarInt net_t1_deadsnapshot;
    static CVarInt net_t1_heartbeat;
    static CVarInt net_t1_delta;
    static CVarInt net_t1_ping;
    static CVarInt net_t1_thingsnapshot;
    static CVarInt sv_bonus_frequency;
    static CVarBool sv_bonus_predator;
    static CVarBool sv_bonus_berserker;
    static CVarBool sv_bonus_vest;
    static CVarBool sv_bonus_cluster;
    static CVarInt sv_minping;
    static CVarInt sv_maxping;
    static CVarInt sv_warnings_flood;
    static CVarInt sv_warnings_ping;
    static CVarInt sv_warnings_votecheat;
    static CVarInt net_floodingpacketslan;
    static CVarInt net_floodingpacketsinternet;
    static CVarBool sv_antimassflag;
    static CVarBool sv_lobby;
    static CVarInt sv_inf_bluelimit;
    static CVarInt sv_htf_pointstime;
    static CVarInt bots_random_noteam;
    static CVarInt bots_random_alpha;
    static CVarInt bots_random_bravo;
    static CVarInt bots_random_charlie;
    static CVarInt bots_random_delta;
    static CVarBool sc_enable;
    static CVarString sc_onscriptcrash;
    static CVarInt sc_maxscripts;
    static CVarBool sc_safemode;
    static CVarString net_ip;
    static CVarString net_adminip;
    static CVarBool fileserver_enable;
    static CVarInt fileserver_port;
    static CVarString fileserver_ip;
    static CVarInt sv_dm_limit;
    static CVarInt sv_pm_limit;
    static CVarInt sv_tm_limit;
    static CVarInt sv_rm_limit;
    static CVarInt sv_inf_limit;
    static CVarInt sv_htf_limit;
    static CVarInt sv_ctf_limit;
    static CVarInt net_port;
    static CVarBool sv_anticheatkick;
    static CVarBool sv_teamcolors;
#endif
}; // namespace CVar

using CVar = CVarStruct<Config::GetModule()>;

#if 0

// clang-format off
template<> CVarInt CVarStruct<Config::GetModule()>::log_level;
template<> CVarBool CVarStruct<Config::GetModule()>:: log_enable;
template<> CVarInt CVarStruct<Config::GetModule()>::log_filesupdate;
#ifdef SERVER
//template<> CVarBool CVarStruct<Config::GetModule()>:: log_timestamp{ "log_timestamp", "Enables/Disables timestamps in console", CVarFlags::SERVER_FLAG, false};
#endif
#ifdef SERVER
#ifdef ENABLE_FAE
template<> CVarBool CVarStruct<Config::GetModule()>:: ac_enable{
                                  "ac_enable", "Enables/Disables anti-cheat checks via Fae", true, true, CVarFlags::SERVER_FLAG,
                                  nullptr};
#endif
#endif

template<> CVarBool CVarStruct<Config::GetModule()>:: fs_localmount;
template<> CVarString CVarStruct<Config::GetModule()>:: fs_mod;
template<> CVarBool CVarStruct<Config::GetModule()>:: fs_portable;
template<> CVarString CVarStruct<Config::GetModule()>:: fs_basepath;
template<> CVarString CVarStruct<Config::GetModule()>:: fs_userpath;

template<> CVarBool CVarStruct<Config::GetModule()>:: demo_autorecord;

template<> CVarInt CVarStruct<Config::GetModule()>::sv_inf_redaward;

#ifndef SERVER // Render Cvars
template<> CVarInt CVarStruct<Config::GetModule()>::r_fullscreen{ "r_fullscreen", "Set mode of fullscreen", CVarFlags::CLIENT, 0};
template<> CVarBool CVarStruct<Config::GetModule()>:: r_weathereffects;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_dithering;
template<> CVarInt CVarStruct<Config::GetModule()>::r_swapeffect;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_compatibility;
template<> CVarInt CVarStruct<Config::GetModule()>::r_texturefilter;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_optimizetextures;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_mipmapping;
template<> CVarFloat CVarStruct<Config::GetModule()>:: r_mipmapbias;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_glfinish;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_smoothedges;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_scaleinterface;
template<> CVarInt CVarStruct<Config::GetModule()>::r_maxsparks;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_animations;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_renderbackground;
template<> CVarInt CVarStruct<Config::GetModule()>::r_maxfps;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_fpslimit;
template<> CVarInt CVarStruct<Config::GetModule()>::r_resizefilter;
template<> CVarInt CVarStruct<Config::GetModule()>::r_sleeptime;
template<> CVarInt CVarStruct<Config::GetModule()>::r_screenwidth;
template<> CVarInt CVarStruct<Config::GetModule()>::r_screenheight;
template<> CVarInt CVarStruct<Config::GetModule()>::r_renderwidth;
template<> CVarInt CVarStruct<Config::GetModule()>::r_renderheight;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_forcebg;
template<> CVarColor CVarStruct<Config::GetModule()>:: r_forcebg_color1;
template<> CVarColor CVarStruct<Config::GetModule()>:: r_forcebg_color2;
template<> CVarBool CVarStruct<Config::GetModule()>:: r_renderui;
template<> CVarFloat CVarStruct<Config::GetModule()>:: r_zoom;
template<> CVarInt CVarStruct<Config::GetModule()>::r_msaa;

// Ui Cvars
template<> CVarBool CVarStruct<Config::GetModule()>:: ui_playerindicator;
template<> CVarInt CVarStruct<Config::GetModule()>::ui_minimap_transparency;
template<> CVarInt CVarStruct<Config::GetModule()>::ui_minimap_posx;
template<> CVarInt CVarStruct<Config::GetModule()>::ui_minimap_posy;
template<> CVarBool CVarStruct<Config::GetModule()>:: ui_bonuscolors;
template<> CVarString CVarStruct<Config::GetModule()>:: ui_style;
template<> CVarInt CVarStruct<Config::GetModule()>::ui_status_transparency;
template<> CVarBool CVarStruct<Config::GetModule()>:: ui_console;
template<> CVarInt CVarStruct<Config::GetModule()>::ui_console_length;
template<> CVarBool CVarStruct<Config::GetModule()>:: ui_killconsole;
template<> CVarInt CVarStruct<Config::GetModule()>::ui_killconsole_length;
template<> CVarBool CVarStruct<Config::GetModule()>:: ui_hidespectators;
//template<> CVarBool CVarStruct<Config::GetModule()>::ui_sniperline;

// Client cvars
template<> CVarFloat CVarStruct<Config::GetModule()>:: cl_sensitivity;
template<> CVarBool CVarStruct<Config::GetModule()>:: cl_endscreenshot;
template<> CVarBool CVarStruct<Config::GetModule()>:: cl_actionsnap;
template<> CVarBool CVarStruct<Config::GetModule()>:: cl_screenshake;
template<> CVarBool CVarStruct<Config::GetModule()>:: cl_servermods;


// Player cvars
template<> CVarString CVarStruct<Config::GetModule()>:: cl_player_name;
template<> CVarInt CVarStruct<Config::GetModule()>::cl_player_team;
template<> CVarColor CVarStruct<Config::GetModule()>:: cl_player_shirt;
template<> CVarColor CVarStruct<Config::GetModule()>:: cl_player_pants;
template<> CVarColor CVarStruct<Config::GetModule()>:: cl_player_hair;
template<> CVarColor CVarStruct<Config::GetModule()>:: cl_player_jet;
template<> CVarColor CVarStruct<Config::GetModule()>:: cl_player_skin;

template<> CVarInt CVarStruct<Config::GetModule()>::cl_player_hairstyle;
template<> CVarInt CVarStruct<Config::GetModule()>::cl_player_headstyle;
template<> CVarInt CVarStruct<Config::GetModule()>::cl_player_chainstyle;
template<> CVarInt CVarStruct<Config::GetModule()>::cl_player_secwep;
template<> CVarInt CVarStruct<Config::GetModule()>::cl_player_wep;

template<> CVarInt CVarStruct<Config::GetModule()>::cl_runs;
template<> CVarString CVarStruct<Config::GetModule()>:: cl_lang;

// Demo cvars
template<> CVarFloat CVarStruct<Config::GetModule()>:: demo_speed;
template<> CVarInt CVarStruct<Config::GetModule()>::demo_rate;
template<> CVarBool CVarStruct<Config::GetModule()>:: demo_showcrosshair;

// Sound cvars
template<> CVarInt CVarStruct<Config::GetModule()>::snd_volume;
template<> CVarBool CVarStruct<Config::GetModule()>:: snd_effects_battle;
template<> CVarBool CVarStruct<Config::GetModule()>:: snd_effects_explosions;

// Matchmaking cvars
template<> CVarBool CVarStruct<Config::GetModule()>:: mm_ranked;

// TODO: Remove
template<> CVarInt CVarStruct<Config::GetModule()>::sv_respawntime;
//template<> CVarInt CVarStruct<Config::GetModule()>::sv_inf_redaward;
template<> CVarBool CVarStruct<Config::GetModule()>:: net_compression;
template<> CVarBool CVarStruct<Config::GetModule()>:: net_allowdownload;

template<> CVarString CVarStruct<Config::GetModule()>:: font_1_name;
template<> CVarString CVarStruct<Config::GetModule()>:: font_1_filename;
template<> CVarInt CVarStruct<Config::GetModule()>::font_1_scale;

template<> CVarString CVarStruct<Config::GetModule()>:: font_2_name;
template<> CVarString CVarStruct<Config::GetModule()>:: font_2_filename;
template<> CVarInt CVarStruct<Config::GetModule()>::font_2_scale;

template<> CVarInt CVarStruct<Config::GetModule()>::font_menusize;

template<> CVarInt CVarStruct<Config::GetModule()>::font_consolesize;
template<> CVarInt CVarStruct<Config::GetModule()>::font_consolesmallsize;

template<> CVarFloat CVarStruct<Config::GetModule()>:: font_consolelineheight;
template<> CVarInt CVarStruct<Config::GetModule()>::font_bigsize;
template<> CVarInt CVarStruct<Config::GetModule()>::font_weaponmenusize;
template<> CVarInt CVarStruct<Config::GetModule()>::font_killconsolenamespace;

#else // Server cvars
template<> CVarInt CVarStruct<Config::GetModule()>::sv_respawntime;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_respawntime_minwave;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_respawntime_maxwave;

template<> CVarInt CVarStruct<Config::GetModule()>::sv_dm_limit;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_pm_limit;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_tm_limit;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_rm_limit;

template<> CVarInt CVarStruct<Config::GetModule()>::sv_inf_limit;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_inf_bluelimit;

template<> CVarInt CVarStruct<Config::GetModule()>::sv_htf_limit;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_htf_pointstime;

template<> CVarInt CVarStruct<Config::GetModule()>::sv_ctf_limit;

template<> CVarInt CVarStruct<Config::GetModule()>::sv_bonus_frequency;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_bonus_flamer;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_bonus_predator;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_bonus_berserker;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_bonus_vest;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_bonus_cluster;

template<> CVarBool CVarStruct<Config::GetModule()>:: sv_stationaryguns;

template<> CVarString CVarStruct<Config::GetModule()>:: sv_password;
template<> CVarString CVarStruct<Config::GetModule()>:: sv_adminpassword;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_maxplayers;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_maxspectators;
//template<> CVarBool CVarStruct<Config::GetModule()>:: sv_spectatorchat;
template<> CVarString CVarStruct<Config::GetModule()>:: sv_greeting;
template<> CVarString CVarStruct<Config::GetModule()>:: sv_greeting2;
template<> CVarString CVarStruct<Config::GetModule()>:: sv_greeting3;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_minping;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_maxping;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_votepercent;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_lockedmode;
//template<> CVarString CVarStruct<Config::GetModule()>:: sv_pidfilename;
template<> CVarString CVarStruct<Config::GetModule()>:: sv_maplist;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_lobby;
//template<> CVarString CVarStruct<Config::GetModule()>:: sv_lobbyurl;

//template<> CVarBool CVarStruct<Config::GetModule()>:: sv_steamonly;

#ifdef STEAM
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_voicechat{"sv_voicechat", "Enables voice chat",   true,
                                     true,           CVarFlags::SERVER_FLAG, nullptr};
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_voicechat_alltalk{"sv_voicechat_alltalk",
                                             "Enables voice chat from enemy team and spectators",
                                             false,
                                             false,
                                             CVarFlags::SERVER_FLAG,
                                             nullptr};
template<> CVarString CVarStruct<Config::GetModule()>:: sv_setsteamaccount{
                                             "sv_setsteamaccount",
                                             "Set game server account token to use for logging in to a persistent game server account",
                                             "",
                                             "",
                                             CVarFlags::SERVER_FLAG,
                                             nullptr,
                                             32,
                                             32};
#endif

template<> CVarInt CVarStruct<Config::GetModule()>::sv_warnings_flood;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_warnings_ping;

template<> CVarInt CVarStruct<Config::GetModule()>::sv_warnings_votecheat;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_warnings_knifecheat;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_warnings_tk;

template<> CVarBool CVarStruct<Config::GetModule()>:: sv_anticheatkick;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_punishtk;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_botbalance;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_echokills;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_antimassflag;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_healthcooldown;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_teamcolors;

// Network cvars
template<> CVarInt CVarStruct<Config::GetModule()>::net_port;
template<> CVarString CVarStruct<Config::GetModule()>:: net_ip;
template<> CVarString CVarStruct<Config::GetModule()>:: net_adminip;
template<> CVarInt CVarStruct<Config::GetModule()>::net_lan;
//template<> CVarInt CVarStruct<Config::GetModule()>::net_maxconnections;
//template<> CVarInt CVarStruct<Config::GetModule()>::net_maxadminconnections;

template<> CVarInt CVarStruct<Config::GetModule()>::net_floodingpacketslan;
template<> CVarInt CVarStruct<Config::GetModule()>::net_floodingpacketsinternet;

template<> CVarInt CVarStruct<Config::GetModule()>::net_t1_snapshot;
template<> CVarInt CVarStruct<Config::GetModule()>::net_t1_majorsnapshot;
template<> CVarInt CVarStruct<Config::GetModule()>::net_t1_deadsnapshot;
template<> CVarInt CVarStruct<Config::GetModule()>::net_t1_heartbeat;
template<> CVarInt CVarStruct<Config::GetModule()>::net_t1_delta;
template<> CVarInt CVarStruct<Config::GetModule()>::net_t1_ping;
template<> CVarInt CVarStruct<Config::GetModule()>::net_t1_thingsnapshot;

// Bots cvars
template<> CVarInt CVarStruct<Config::GetModule()>::bots_random_noteam;
template<> CVarInt CVarStruct<Config::GetModule()>::bots_random_alpha;
template<> CVarInt CVarStruct<Config::GetModule()>::bots_random_bravo;
template<> CVarInt CVarStruct<Config::GetModule()>::bots_random_charlie;
template<> CVarInt CVarStruct<Config::GetModule()>::bots_random_delta;
template<> CVarBool CVarStruct<Config::GetModule()>:: bots_chat;

// ScriptCore cvars
template<> CVarBool CVarStruct<Config::GetModule()>:: sc_enable;
template<> CVarString CVarStruct<Config::GetModule()>:: sc_onscriptcrash;
template<> CVarInt CVarStruct<Config::GetModule()>::sc_maxscripts;
template<> CVarBool CVarStruct<Config::GetModule()>:: sc_safemode;

// Fileserver cvars
template<> CVarBool CVarStruct<Config::GetModule()>:: fileserver_enable;
template<> CVarInt CVarStruct<Config::GetModule()>::fileserver_port;
template<> CVarString CVarStruct<Config::GetModule()>:: fileserver_ip;
#endif

template<> CVarInt CVarStruct<Config::GetModule()>::bots_difficulty;

// Sync vars (todo);

template<> CVarInt CVarStruct<Config::GetModule()>::sv_gamemode;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_friendlyfire;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_timelimit;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_maxgrenades;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_bullettime;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_sniperline;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_balanceteams;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_guns_collide;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_kits_collide;

template<> CVarBool CVarStruct<Config::GetModule()>:: sv_survivalmode;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_survivalmode_antispy;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_survivalmode_clearweapons;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_realisticmode;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_advancemode;
template<> CVarInt CVarStruct<Config::GetModule()>::sv_advancemode_amount;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_minimap;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_advancedspectator;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_radio;
template<> CVarString CVarStruct<Config::GetModule()>:: sv_info;
template<> CVarFloat CVarStruct<Config::GetModule()>:: sv_gravity;
template<> CVarString CVarStruct<Config::GetModule()>:: sv_hostname;
template<> CVarString CVarStruct<Config::GetModule()>:: sv_website;

template<> CVarInt CVarStruct<Config::GetModule()>::sv_killlimit;
template<> CVarString CVarStruct<Config::GetModule()>:: sv_downloadurl;
template<> CVarBool CVarStruct<Config::GetModule()>:: sv_pure;
// clang-format-on
#endif

extern template struct CVarStruct<Config::GetModule()>;
