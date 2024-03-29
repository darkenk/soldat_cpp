#pragma once

#include "../AnimationSystem.hpp"
#include "../Constants.hpp"
#include "../network/Net.hpp"
#include "BackgroundState.hpp"
#include "common/Parts.hpp"
#include "common/PolyMap.hpp"
#include "common/Vector.hpp"
#include "common/Weapons.hpp"
#include <array>
#include <cstdint>

constexpr std::int32_t max_sprites = max_players;
constexpr std::int32_t max_bullets = 254;
constexpr std::int32_t max_sparks = 558;
constexpr std::int32_t max_things = 90;

constexpr float surfacecoefx = 0.970;
constexpr float surfacecoefy = 0.970;
constexpr float crouchmovesurfacecoefx = 0.850;
constexpr float crouchmovesurfacecoefy = 0.970;
constexpr float standsurfacecoefx = 0.000;
constexpr float standsurfacecoefy = 0.000;
constexpr float grenade_surfacecoef = 0.880;
constexpr float spark_surfacecoef = 0.700;

constexpr std::int32_t part_radius = 7;
constexpr std::int32_t flag_part_radius = 10;
constexpr std::int32_t sprite_radius = 16;
constexpr std::int32_t m79grenade_explosion_radius = 64;
constexpr std::int32_t fraggrenade_explosion_radius = 85;
constexpr std::int32_t after_explosion_radius = 50;
constexpr std::int32_t clustergrenade_explosion_radius = 35;
constexpr std::int32_t base_radius = 75;
constexpr std::int32_t touchdown_radius = 28;
constexpr std::int32_t sprite_col_radius = 3;

constexpr std::int32_t flag_holding_forceup = -14;
constexpr std::int32_t flag_stand_forceup = -16;

constexpr std::int32_t bulletalpha = 110;
constexpr std::int32_t maxpathcount = 50;

constexpr float slidelimit = 0.2;
constexpr std::int32_t max_velocity = 11;
constexpr std::int32_t bullettime_mindistance = 320;
constexpr float flagthrow_power = 4.225;

constexpr std::int32_t normal_death = 1;
constexpr std::int32_t brutal_death = 2;
constexpr std::int32_t headchop_death = 3;

constexpr std::int32_t pos_stand = 1;
constexpr std::int32_t pos_crouch = 2;
constexpr std::int32_t pos_prone = 4;

typedef PascalArray<std::int32_t, 1, max_sprites> tspriteindexes;
typedef PascalArray<float, 1, max_sprites> tspritedistances;

struct tcontrol
{
  bool left, right, up, down, fire, jetpack, thrownade, changeweapon, throwweapon, reload, prone,
    flagthrow;
  std::int16_t mouseaimx, mouseaimy, mousedist;
};

struct tbotdata
{
  std::int32_t favweapon;
  std::string friend_;
  std::int32_t accuracy, grenadefreq;
  std::uint8_t deadkill;
  std::int32_t waypointtimeoutcounter, waypointtimeout;
  std::int32_t chatfreq;
  std::string chatkill, chatdead, chatlowhealth, chatseeenemy, chatwinning;
  std::uint8_t pissedoff;
  std::uint8_t pathnum;
  std::uint8_t targetnum;
  bool gothing;
  std::int32_t currentwaypoint, nextwaypoint, oldwaypoint, waypointtime, lastwaypoint;
  std::uint8_t use;
  std::int32_t oneplacecount;
  std::uint8_t camper;
  std::int32_t camptimer;
  std::uint8_t fallsave;
};

template <Config::Module M = Config::GetModule()>
class Sprite
{
public:
  enum class Style
  {
    Default = 1,
    All
  };
  Sprite(const std::uint8_t _num, bool _active = false);
  Sprite(const Sprite &s) = default;
  bool active = false;
  bool deadmeat = false;
  bool dummy;
  Style style = Style::Default;
  const std::uint8_t num;
  std::uint8_t visible;
  bool onground, ongroundforlaw;
  bool ongroundlastframe;
  bool ongroundpermanent;
  std::int16_t direction, olddirection;
  std::uint8_t holdedthing = 0;
  std::int32_t flaggrabcooldown;
  float aimdistcoef = defaultaimdist;
  std::uint8_t fired;
  std::uint8_t alpha = 255;
  float jetscountfloat;
  std::int16_t jetscount, jetscountprev;
  std::uint8_t wearhelmet = 1;
  std::uint8_t hascigar = 0;
  bool canmercy;
  std::int16_t respawncounter = 0;
  std::int16_t ceasefirecounter;
  std::uint8_t selweapon = 0;
  std::int32_t bonusstyle = bonus_none;
  std::int32_t bonustime = 0;
  std::int32_t multikilltime = 0;
  std::int32_t multikills = 0;
  float vest = 0.0f;
  std::int32_t idletime = default_idletime;
  std::int8_t idlerandom = -1;
  std::uint8_t burstcount;
  std::uint8_t position = pos_stand;
  std::uint8_t onfire = 0;
  std::uint8_t colliderdistance;
  std::int32_t deadcollidecount = 0;
  std::int32_t deadtime;
  std::uint8_t para;
  std::uint8_t stat = 0;
  std::int16_t usetime;
  bool halfdead = false;
  float lastweaponhm, lastweaponspeed;
  std::uint8_t lastweaponstyle;
  std::uint64_t lastweaponfire, lastweaponreload;
  particlesystem skeleton;
  tanimation legsanimation, bodyanimation;
  tcontrol control;
  tgun weapon, secondaryweapon, tertiaryweapon;
  bool grenadecanthrow;
  tbotdata brain;
#ifdef SERVER
  std::shared_ptr<TServerPlayer> player;
#else
  std::shared_ptr<tplayer> player;
#endif
  bool typing;
  bool autoreloadwhencanfire;
  bool canautoreloadspas;
  tbackgroundstate bgstate;
#ifdef SERVER
  bool haspack;
  float targetx, targety;
#else
  std::uint32_t gattlingsoundchannel2, reloadsoundchannel, jetssoundchannel, gattlingsoundchannel;
  bool olddeadmeat = false;
  bool muted;
#endif
  bool dontdrop;
  PascalArray<tvector2, 0, max_pushtick> nextpush;
  std::uint16_t bulletcount;
#ifdef SERVER
  std::array<std::uint64_t, bulletcheckarraysize> bulletcheck;
  std::int32_t bulletcheckindex;
  std::int32_t bulletcheckamount;
#endif
  //  public
  void update();
  void kill();
  void die(std::int32_t how, std::int32_t who, std::int32_t where, std::int32_t what,
           tvector2 impact);
  std::int32_t dropweapon();
  void legsapplyanimation(const AnimationType anim, std::int32_t curr);
  void bodyapplyanimation(const AnimationType anim, std::int32_t curr);
  void moveskeleton(float x1, float y1, bool fromzero);
  bool checkmapcollision(float x, float y, std::int32_t area);
  bool checkradiusmapcollision(float x, float y, bool hascollided);
  bool checkmapverticescollision(float x, float y, float r, bool hascollided);
  bool checkskeletonmapcollision(std::int32_t i, float x, float y);
  void handlespecialpolytypes(std::int32_t polytype, const tvector2 &pos);
  void applyweaponbynum(std::uint8_t wnum, std::uint8_t gun, std::int32_t ammo = -1,
                        bool restoreprimarystate = false); // Ammo -1
  void healthhit(float amount, std::int32_t who, std::int32_t where, std::int32_t what,
                 const tvector2 &impact);
  void freecontrols();
  void checkoutofbounds();
  void checkskeletonoutofbounds();
  void respawn();
  void parachute(tvector2 &a);
#ifndef SERVER
  void changeteam(std::int32_t team);
#else
  void changeteam(std::int32_t team, bool adminchange = false, std::uint8_t jointype = join_normal);
#endif
  void fire();
  void throwflag();
  void throwgrenade();
  float getmoveacc();
  tvector2 getcursoraimdirection();
  tvector2 gethandsaimdirection();
  bool issolo();
  bool isnotsolo();
  bool isinteam();
  bool isspectator();
  bool isnotspectator()
  {
    return player->team != team_spectator;
  }
  bool isinsameteam(const Sprite &otherplayer);
  bool isnotinsameteam(const Sprite &otherplayer);
  bool canrespawn(bool deadmeatbeforerespawn);

  void SetHealth(float health);
  float GetHealth();

  bool IsActive() const
  {
    return active;
  }

  void SetFirstWeapon(const tgun &gun);
  void SetSecondWeapon(const tgun &gun);
  void SetThirdWeapon(const tgun &gun);

  void CopyOldSpritePos();
  tvector2 GetOldSpritePos(std::int32_t idx);

private:
  float Health;
  PascalArray<tvector2, 0, max_oldpos> oldspritepos = {};
};

using tsprite = Sprite<Config::GetModule()>;

template <Config::Module M = Config::GetModule()>
std::int32_t createsprite(tvector2 &spos, std::uint8_t n, std::shared_ptr<tplayer> player,
                          const tsprite::Style style = tsprite::Style::Default);

template <Config::Module M = Config::GetModule()>
bool teamcollides(const PolygonType poly, const std::int32_t team, const bool bullet);

#ifndef SERVER
extern bool wasreloading;
void selectdefaultweapons(std::uint8_t mysprite);
#endif
