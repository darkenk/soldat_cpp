#pragma once

#include "misc/FlagSet.hpp"
#include "misc/SoldatConfig.hpp"
#include <algorithm>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>

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
    : Value{value}, Name{name}, Description{description}, Id{id}, Flags{flags}
  {
    if (sGetCVars().contains(Name))
    {
      throw std::runtime_error("CVar has been created already");
    }
    sGetCVars().emplace(std::make_pair(Name, this));
  };

  CVarBase(const CVarBase &ref) = delete;

  ~CVarBase()
  {
    sGetCVars().erase(Name);
  }

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

  [[nodiscard]] std::string ValueAsString() const noexcept
  {
    if constexpr(std::is_same<T, std::string>())
    {
      return Value;
    }
    else
    {
      return std::to_string(Value);
    }
  }

  static CVarBase &Find(const std::string &cvarName) noexcept
  {
    auto &CVars = sGetCVars();
    auto it = CVars.find(cvarName);
    if (it == std::end(CVars))
    {
      return sGetInvalidCVar();
    }
    return *(it->second);
  }

  static CVarBase &Find(const std::uint8_t id) noexcept
  {
    auto &CVars = sGetCVars();
    auto pred = [id](const auto &cv) -> bool { return cv.second->Id == id; };
    auto it = std::find_if(std::begin(CVars), std::end(CVars), pred);
    if (it == std::end(CVars))
    {
      return sGetInvalidCVar();
    }
    return *(it->second);
  }

  [[nodiscard]] bool IsValid() const
  {
    return this != &sGetInvalidCVar();
  }

  [[nodiscard]] bool IsSyncable() const
  {
    return Flags.Test(CVarFlags::SYNC);
  }

  class AllCVars
  {
  public:
    // member typedefs provided through inheriting from std::iterator
    class iterator// : public std::iterator<std::input_iterator_tag, CVarBase>
    {
    public:
      using iterator_category = std::input_iterator_tag;
      using value_type = CVarBase;
      using difference_type = int;
      using pointer = CVarBase*;
      using reference = CVarBase&;

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
      return iterator(CVarBase::sGetCVars().begin());
    }
    iterator end()
    {
      return iterator(CVarBase::sGetCVars().end());
    }
  };

  static AllCVars GetAllCVars()
  {
    return AllCVars();
  }

  [[nodiscard]] std::uint8_t GetId() const
  {
    return Id;
  }

  [[nodiscard]] const std::string &GetName() const
  {
    return Name;
  }

private:
  T Value;
  const std::string Name;
  const std::string Description;
  std::uint8_t Id;
  FlagSet<CVarFlags> Flags;

  using Map = std::map<const std::string, CVarBase *>;

  static Map& sGetCVars()
  {
    static Map CVars;
    return CVars;
  }

  static CVarBase& sGetInvalidCVar()
  {
    static CVarBase InvalidCVar{"invalid", "", CVarFlags::NONE, {}};
    return InvalidCVar;
  }
};

using CVarBool = CVarBase<bool>;
using CVarString = CVarBase<std::string>;
using CVarInt = CVarBase<std::int32_t>;
using CVarFloat = CVarBase<float>;
using CVarColor = CVarBase<std::uint32_t>;

#ifdef SERVER
namespace CVarServer
#else
namespace CVarClient
#endif
{
extern CVarBool log_enable;
extern CVarInt log_level;
extern CVarInt log_filesupdate;

extern CVarBool fs_localmount;
extern CVarString fs_mod;

extern CVarInt r_fullscreen;
extern CVarBool r_weathereffects;
extern CVarBool r_dithering;
extern CVarInt r_swapeffect;
extern CVarBool r_compatibility;
extern CVarInt r_texturefilter;
extern CVarBool r_optimizetextures;
extern CVarBool r_mipmapping;
extern CVarFloat r_mipmapbias;
extern CVarBool r_glfinish;
extern CVarBool r_smoothedges;
extern CVarBool r_scaleinterface;
extern CVarInt r_maxsparks;
extern CVarBool r_animations;
extern CVarBool r_renderbackground;
extern CVarInt r_maxfps;
extern CVarBool r_fpslimit;
extern CVarInt r_resizefilter;
extern CVarInt r_sleeptime;
extern CVarInt r_screenwidth;
extern CVarInt r_screenheight;
extern CVarInt r_renderwidth;
extern CVarInt r_renderheight;
extern CVarBool r_forcebg;
extern CVarColor r_forcebg_color1;
extern CVarColor r_forcebg_color2;
extern CVarBool r_renderui;
extern CVarFloat r_zoom;
extern CVarInt r_msaa;

extern CVarBool ui_playerindicator;
extern CVarInt ui_minimap_transparency;
extern CVarInt ui_minimap_posx;
extern CVarInt ui_minimap_posy;
extern CVarBool ui_bonuscolors;
extern CVarString ui_style;
extern CVarInt ui_status_transparency;
extern CVarBool ui_console;
extern CVarInt ui_console_length;
extern CVarBool ui_killconsole;
extern CVarInt ui_killconsole_length;
extern CVarBool ui_hidespectators;
extern CVarFloat cl_sensitivity;
extern CVarBool cl_endscreenshot;
extern CVarBool cl_actionsnap;
extern CVarBool cl_screenshake;
extern CVarBool cl_servermods;
extern CVarString cl_player_name;
extern CVarInt cl_player_team;
extern CVarColor cl_player_shirt;
extern CVarColor cl_player_pants;
extern CVarColor cl_player_hair;
extern CVarColor cl_player_jet;
extern CVarColor cl_player_skin;

extern CVarInt cl_player_hairstyle;
extern CVarInt cl_player_headstyle;
extern CVarInt cl_player_chainstyle;
extern CVarInt cl_player_secwep;

extern CVarInt cl_runs;
extern CVarString cl_lang;

extern CVarFloat demo_speed;
extern CVarInt demo_rate;
extern CVarBool demo_showcrosshair;
extern CVarBool demo_autorecord;

extern CVarInt snd_volume;
extern CVarBool snd_effects_battle;
extern CVarBool snd_effects_explosions;

extern CVarString font_1_filename;
extern CVarInt font_1_scale;
extern CVarString font_2_filename;
extern CVarInt font_2_scale;
extern CVarInt font_menusize;
extern CVarInt font_consolesize;
extern CVarInt font_consolesmallsize;
extern CVarFloat font_consolelineheight;
extern CVarInt font_bigsize;
extern CVarInt font_weaponmenusize;

extern CVarInt sv_respawntime;   // TODO: Remove
extern CVarInt sv_inf_redaward;  // TODO: Remove
extern CVarInt net_contype;      // TODO: Remove
// TODO: Remove
extern CVarInt sv_gamemode;
extern CVarBool sv_friendlyfire;
extern CVarInt sv_timelimit;
extern CVarInt sv_maxgrenades;
extern CVarBool sv_bullettime;
extern CVarBool sv_sniperline;
extern CVarBool sv_balanceteams;
extern CVarBool sv_guns_collide;
extern CVarBool sv_kits_collide;
extern CVarBool sv_survivalmode;
extern CVarBool sv_survivalmode_antispy;
extern CVarBool sv_survivalmode_clearweapons;
extern CVarBool sv_realisticmode;
extern CVarBool sv_advancemode;
extern CVarInt sv_advancemode_amount;
extern CVarBool sv_minimap;
extern CVarBool sv_advancedspectator;
extern CVarBool sv_radio;
extern CVarString sv_info;
extern CVarString sv_hostname;
extern CVarInt sv_killlimit;
extern CVarString sv_downloadurl;
extern CVarBool sv_pure;
extern CVarString sv_website;

// SERVER
extern CVarInt bots_difficulty;

#ifdef SERVER
extern CVarBool bots_chat;
extern CVarString sv_maplist;
extern CVarInt sv_votepercent;
extern CVarBool sv_stationaryguns;
extern CVarInt sv_healthcooldown;
extern CVarInt sv_warnings_tk;
extern CVarBool sv_punishtk;
extern CVarInt sv_maxspectators;
extern CVarBool sv_echokills;
extern CVarInt sv_respawntime_minwave;
extern CVarInt sv_respawntime_maxwave;
extern CVarBool sv_botbalance;
extern CVarString sv_adminpassword;
extern CVarString sv_password;
extern CVarInt sv_maxplayers;
extern CVarBool sv_bonus_flamer;
extern CVarInt sv_warnings_knifecheat;
extern CVarString sv_greeting;
extern CVarString sv_greeting2;
extern CVarString sv_greeting3;
extern CVarBool sv_lockedmode;
extern CVarInt net_lan;
extern CVarInt net_t1_snapshot;
extern CVarInt net_t1_majorsnapshot;
extern CVarInt net_t1_deadsnapshot;
extern CVarInt net_t1_heartbeat;
extern CVarInt net_t1_delta;
extern CVarInt net_t1_ping;
extern CVarInt net_t1_thingsnapshot;
extern CVarInt sv_bonus_frequency;
extern CVarBool sv_bonus_predator;
extern CVarBool sv_bonus_berserker;
extern CVarBool sv_bonus_vest;
extern CVarBool sv_bonus_cluster;
extern CVarInt sv_minping;
extern CVarInt sv_maxping;
extern CVarInt sv_warnings_flood;
extern CVarInt sv_warnings_ping;
extern CVarInt sv_warnings_votecheat;
extern CVarInt net_floodingpacketslan;
extern CVarInt net_floodingpacketsinternet;
extern CVarBool sv_antimassflag;
extern CVarBool sv_lobby;
extern CVarInt sv_inf_bluelimit;
extern CVarInt sv_htf_pointstime;
extern CVarInt bots_random_noteam;
extern CVarInt bots_random_alpha;
extern CVarInt bots_random_bravo;
extern CVarInt bots_random_charlie;
extern CVarInt bots_random_delta;
extern CVarInt bots_difficulty;
extern CVarBool sc_enable;
extern CVarString sc_onscriptcrash;
extern CVarInt sc_maxscripts;
extern CVarBool sc_safemode;
extern CVarString net_ip;
extern CVarString net_adminip;
extern CVarBool fileserver_enable;
extern CVarInt fileserver_port;
extern CVarString fileserver_ip;
extern CVarBool sv_realisticmode;
extern CVarInt sv_dm_limit;
extern CVarInt sv_pm_limit;
extern CVarInt sv_tm_limit;
extern CVarInt sv_rm_limit;
extern CVarInt sv_inf_limit;
extern CVarInt sv_htf_limit;
extern CVarInt sv_ctf_limit;
extern CVarInt net_port;
extern CVarBool sv_anticheatkick;
extern CVarBool sv_teamcolors;
#endif
}; // namespace CVar

#ifdef SERVER
namespace CVar = CVarServer;
#else
namespace CVar = CVarClient;
#endif
