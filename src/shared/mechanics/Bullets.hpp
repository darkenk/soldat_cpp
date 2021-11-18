#pragma once

#include "../Constants.hpp"
#include "../MapFile.hpp"
#include "../Parts.hpp"
#include "../PolyMap.hpp"
#include "../Weapons.hpp"
#include "Sprites.hpp"
#include "Things.hpp"
#include <vector>
/*#include "Parts.h"*/
/*#include "MapFile.h"*/
/*#include "PolyMap.h"*/
/*#include "Net.h"*/
/*#include "Weapons.h"*/
/*#include "Constants.h"*/
/*#include "Vector.h"*/
/*#include "Sprites.h"*/
/*#include "Things.h"*/

template <Config::Module M = Config::GetModule()>
class Bullet
{
  public:
    bool active;
#ifndef SERVER
    bool hashit;
#endif
    std::uint8_t style;
    std::int16_t num;
    std::uint8_t owner;
    std::uint8_t ownerweapon;
    float timeoutfloat;
    std::int16_t timeout, timeoutprev;
    float hitmultiply, hitmultiplyprev;
    tvector2 velocityprev;
    bool whizzed;
    std::uint8_t ownerpingtick;
    std::uint8_t hitbody;
    tvector2 hitspot;
    std::uint8_t tracking;
    std::uint8_t imagestyle;
    tvector2 initial;
    std::int32_t startuptime, ricochetcount, degradecount;
    std::uint16_t seed;
    std::vector<tthingcollision> thingcollisions;
    std::array<std::int32_t, 32> spritecollisions;
#ifdef SERVER
    bool dontcheat;
#else
    std::int16_t pingadd, pingaddstart;
#endif
    //  public
    void update();
#ifndef SERVER
    void render(double timeelapsed);
#endif
    void kill();
    tvector2 checkmapcollision(float x, float y);
    tvector2 checkspritecollision(float lasthitdist);
    tvector2 checkthingcollision(float lasthitdist);
    tvector2 checkcollidercollision(float lasthitdist);
    void hit(std::int32_t t, std::int32_t spritehit = 0, std::int32_t where = 0);
    void explosionhit(std::int32_t typ, std::int32_t spritehit, std::int32_t where);
    void checkoutofbounds();
    std::int32_t filterspritesbydistance(tspriteindexes &spriteindexes);
    bool targetablesprite(std::int32_t i);
    float getcomparablespritedistance(std::int32_t i);
    tvector2 getspritecollisionpoint(std::int32_t i);
    std::uint8_t getweaponindex();
};

using tbullet = Bullet<Config::GetModule()>;

constexpr std::int32_t hit_type_wall = 1;
constexpr std::int32_t hit_type_blood = 2;
constexpr std::int32_t hit_type_explode = 3;
constexpr std::int32_t hit_type_fragnade = 4;
constexpr std::int32_t hit_type_thing = 5;
constexpr std::int32_t hit_type_clusternade = 6;
constexpr std::int32_t hit_type_cluster = 7;
constexpr std::int32_t hit_type_flak = 8;
constexpr std::int32_t hit_type_bodyhit = 9;
constexpr std::int32_t hit_type_ricochet = 10;

template <Config::Module M = Config::GetModule()>
std::int32_t createbullet(tvector2 spos, tvector2 svelocity, std::uint8_t snum, std::int32_t sowner,
                          std::uint8_t n, float hitm, bool net, bool mustcreate,
                          std::uint16_t seed = -1); // Seed -1
#ifdef SERVER
std::int32_t servercreatebullet(tvector2 spos, tvector2 svelocity, std::uint8_t snum,
                                std::int32_t sowner, std::uint8_t n, float hitm, bool net);
#else
bool canhitspray(std::int32_t victim, std::int32_t attacker);
void calculaterecoil(float px, float py, float &cx, float &cy, float da);
void hitspray();
#endif
template <Config::Module M = Config::GetModule()>
bool bulletcansend(float x, float y, std::int32_t i, float vx);
