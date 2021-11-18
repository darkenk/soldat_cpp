#pragma once

/*#include "Parts.h"*/
/*#include "Anims.h"*/
/*#include "MapFile.h"*/
/*#include "PolyMap.h"*/
/*#include "Net.h"*/
/*#include "Weapons.h"*/
/*#include "Constants.h"*/
#include "../Anims.hpp"
#include "../Constants.hpp"
#include "../Parts.hpp"
#include "../Vector.hpp"
#include "../Weapons.hpp"
#include "../network/Net.hpp"
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
class BackgroundState
{
  public:
    std::uint8_t backgroundstatus;
    std::int16_t backgroundpoly;
    bool backgroundtestresult;
    bool backgroundtest(std::uint64_t poly);
    void backgroundtestbigpolycenter(tvector2 pos);
    std::int16_t backgroundfindcurrentpoly(tvector2 pos);
    void backgroundtestprepare();
    void backgroundtestreset();
};

using tbackgroundstate = BackgroundState<Config::GetModule()>;

template <Config::Module M = Config::GetModule()>
class Sprite
{
  public:
    bool active = false;
    bool deadmeat, dummy;
    std::uint8_t style;
    std::uint8_t num;
    std::uint8_t visible;
    bool onground, ongroundforlaw;
    bool ongroundlastframe;
    bool ongroundpermanent;
    std::int16_t direction, olddirection;
    float health;
    std::uint8_t holdedthing;
    std::int32_t flaggrabcooldown;
    float aimdistcoef;
    std::uint8_t fired;
    std::uint8_t alpha;
    float jetscountfloat;
    std::int16_t jetscount, jetscountprev;
    std::uint8_t wearhelmet;
    std::uint8_t hascigar;
    bool canmercy;
    std::int16_t respawncounter, ceasefirecounter;
    std::uint8_t selweapon;
    std::int32_t bonusstyle, bonustime;
    std::int32_t multikilltime, multikills;
    float vest;
    std::int32_t idletime;
    std::int8_t idlerandom;
    std::uint8_t burstcount;
    std::uint8_t position;
    std::uint8_t onfire;
    std::uint8_t colliderdistance;
    std::int32_t deadcollidecount;
    std::int32_t deadtime;
    std::uint8_t para, stat;
    std::int16_t usetime;
    bool halfdead;
    float lastweaponhm, lastweaponspeed;
    std::uint8_t lastweaponstyle;
    std::uint64_t lastweaponfire, lastweaponreload;
    particlesystem skeleton;
    tanimation legsanimation, bodyanimation;
    tcontrol control;
    tgun weapon, secondaryweapon, tertiaryweapon;
    bool grenadecanthrow;
    tbotdata brain;
    tplayer *player;
    bool isplayerobjectowner;
    bool typing;
    bool autoreloadwhencanfire;
    bool canautoreloadspas;
    tbackgroundstate bgstate;
#ifdef SERVER
    bool haspack;
    float targetx, targety;
#else
    std::uint32_t gattlingsoundchannel2, reloadsoundchannel, jetssoundchannel, gattlingsoundchannel;
    bool olddeadmeat;
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
    void legsapplyanimation(tanimation anim, std::int32_t curr);
    void bodyapplyanimation(tanimation anim, std::int32_t curr);
    void moveskeleton(float x1, float y1, bool fromzero);
    bool checkmapcollision(float x, float y, std::int32_t area);
    bool checkradiusmapcollision(float x, float y, bool hascollided);
    bool checkmapverticescollision(float x, float y, float r, bool hascollided);
    bool checkskeletonmapcollision(std::int32_t i, float x, float y);
    void handlespecialpolytypes(std::int32_t polytype, tvector2 pos);
    void applyweaponbynum(std::uint8_t wnum, std::uint8_t gun, std::int32_t ammo = -1,
                          bool restoreprimarystate = false); // Ammo -1
    void healthhit(float amount, std::int32_t who, std::int32_t where, std::int32_t what,
                   tvector2 impact);
    void freecontrols();
    void checkoutofbounds();
    void checkskeletonoutofbounds();
    void respawn();
#ifdef SERVER
    void resetspriteoldpos();
#endif
    void parachute(tvector2 a);
#ifndef SERVER
    void changeteam(std::int32_t team);
#else
    void changeteam(std::int32_t team, bool adminchange = false,
                    std::uint8_t jointype = join_normal);
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
    bool isnotspectator();
    bool isinsameteam(const Sprite &otherplayer);
    bool isnotinsameteam(const Sprite &otherplayer);
    bool canrespawn(bool deadmeatbeforerespawn);
};

using tsprite = Sprite<Config::GetModule()>;

template <Config::Module M = Config::GetModule()>
std::int32_t createsprite(tvector2 spos, tvector2 svelocity, std::uint8_t sstyle, std::uint8_t n,
                          tplayer *player, bool transferownership);

template <Config::Module M = Config::GetModule()>
bool teamcollides(std::int32_t poly, std::int32_t team, bool bullet);

#ifndef SERVER
extern bool wasreloading;
void selectdefaultweapons(std::uint8_t mysprite);
#endif
