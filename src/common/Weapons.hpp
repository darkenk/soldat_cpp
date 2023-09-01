#pragma once

#include "misc/PortUtilsSoldat.hpp"
#include <array>
#include <cstdint>
#include <string>

struct tgun
{
  tgun();
  tgun(const std::uint8_t _num);
  tgun(const tgun &) = default;
  // Scrambled 1.6, to piss off gamehackers.
  std::uint8_t ammo = 0;
  std::uint8_t ammocount = 0;
  const std::uint8_t num;
  float movementacc = 0.f;
  std::int16_t bink = 0;
  std::uint32_t recoil = 0;
  std::uint32_t fireinterval = 0;
  std::uint32_t fireintervalprev = 0;
  std::uint32_t fireintervalcount = 0;
  float fireintervalfloat = 0.f;
  std::uint32_t startuptime = 0;
  std::uint32_t startuptimecount = 0;
  std::uint32_t reloadtime = 0;
  std::uint32_t reloadtimeprev = 0;
  std::uint32_t reloadtimecount = 0;
  float reloadtimefloat = 0.f;
  std::uint8_t texturenum = 0;
  std::uint8_t cliptexturenum = 0;
  bool clipreload = false;
  std::uint32_t clipintime = 0;
  std::uint32_t clipouttime = 0;
  std::string name;
  std::string ininame;
  float speed = 0.f;
  float hitmultiply = 0.f;
  float bulletspread = 0.f;
  float push = 0.f;
  float inheritedvelocity = 0.f;
  float modifierlegs = 0.f;
  float modifierchest = 0.f;
  float modifierhead = 0.f;
  std::uint8_t nocollision = 0;
  std::uint8_t firemode = 0;
  std::uint32_t timeout = 0;
  std::uint8_t bulletstyle = 0;
  std::uint8_t firestyle = 0;
  std::uint8_t bulletimagestyle = 0;
};

constexpr std::int32_t eagle = 1;
constexpr std::int32_t mp5 = 2;
constexpr std::int32_t ak74 = 3;
constexpr std::int32_t steyraug = 4;
constexpr std::int32_t spas12 = 5;
constexpr std::int32_t ruger77 = 6;
constexpr std::int32_t m79 = 7;
constexpr std::int32_t barrett = 8;
constexpr std::int32_t m249 = 9;
constexpr std::int32_t minigun = 10;
constexpr std::int32_t colt = 11;
constexpr std::int32_t knife = 12;
constexpr std::int32_t chainsaw = 13;
constexpr std::int32_t law = 14;
constexpr std::int32_t bow2 = 15;
constexpr std::int32_t bow = 16;
constexpr std::int32_t flamer = 17;
constexpr std::int32_t m2 = 18;
constexpr std::int32_t noweapon = 19;
constexpr std::int32_t fraggrenade = 20;
constexpr std::int32_t clustergrenade = 21;
constexpr std::int32_t cluster = 22;
constexpr std::int32_t thrownknife = 23;

constexpr std::int32_t primary_weapons = 10;
constexpr std::int32_t secondary_weapons = 4;
constexpr std::int32_t bonus_weapons = 3;
constexpr std::int32_t main_weapons = primary_weapons + secondary_weapons;
constexpr std::int32_t double_weapons = main_weapons + bonus_weapons;

constexpr std::int32_t original_weapons = 20;
constexpr std::int32_t total_weapons = 23;

// FIXME(skoskav): Normalize weapons' num with their index
constexpr std::int32_t eagle_num = 1;
constexpr std::int32_t mp5_num = 2;
constexpr std::int32_t ak74_num = 3;
constexpr std::int32_t steyraug_num = 4;
constexpr std::int32_t spas12_num = 5;
constexpr std::int32_t ruger77_num = 6;
constexpr std::int32_t m79_num = 7;
constexpr std::int32_t barrett_num = 8;
constexpr std::int32_t m249_num = 9;
constexpr std::int32_t minigun_num = 10;
constexpr std::int32_t colt_num = 0;
constexpr std::int32_t knife_num = 11;
constexpr std::int32_t chainsaw_num = 12;
constexpr std::int32_t law_num = 13;
constexpr std::int32_t bow2_num = 16;
constexpr std::int32_t bow_num = 15;
constexpr std::int32_t flamer_num = 14;
constexpr std::int32_t m2_num = 30;
constexpr std::int32_t noweapon_num = 255;
constexpr std::int32_t fraggrenade_num = 50;
constexpr std::int32_t clustergrenade_num = 51;
constexpr std::int32_t cluster_num = 52;
constexpr std::int32_t thrownknife_num = 53;

// BulletStyle types
constexpr std::int32_t bullet_style_plain = 1;
constexpr std::int32_t bullet_style_fragnade = 2;
constexpr std::int32_t bullet_style_shotgun = 3;
constexpr std::int32_t bullet_style_m79 = 4;
constexpr std::int32_t bullet_style_flame = 5;
constexpr std::int32_t bullet_style_punch = 6;
constexpr std::int32_t bullet_style_arrow = 7;
constexpr std::int32_t bullet_style_flamearrow = 8;
constexpr std::int32_t bullet_style_clusternade = 9;
constexpr std::int32_t bullet_style_cluster = 10;
constexpr std::int32_t bullet_style_knife = 11;
constexpr std::int32_t bullet_style_law = 12;
constexpr std::int32_t bullet_style_thrownknife = 13;
constexpr std::int32_t bullet_style_m2 = 14;

// Used for NoCollision attribute
constexpr std::int32_t weapon_nocollision_enemy = 1 << 0;
constexpr std::int32_t weapon_nocollision_team = 1 << 1;
constexpr std::int32_t weapon_nocollision_self = 1 << 2;
constexpr std::int32_t weapon_nocollision_exp_enemy = 1 << 3;
constexpr std::int32_t weapon_nocollision_exp_team = 1 << 4;
constexpr std::int32_t weapon_nocollision_exp_self = 1 << 5;

using GunsDescription = PascalArray<tgun, 1, total_weapons>;

void createweapons(bool floatisticmode, GunsDescription &guns, GunsDescription &defaultguns);
void createdefaultweapons(bool floatisticmode, GunsDescription &guns, GunsDescription &defaultguns);
void createweaponsbase(GunsDescription &guns);
void createnormalweapons(GunsDescription &guns);
void createfloatisticweapons(GunsDescription &guns);
void buildweapons(GunsDescription &guns);
std::uint32_t createwmchecksum(GunsDescription &guns);
std::int32_t weaponnumtoindex(std::uint8_t num, GunsDescription &guns);
std::int32_t weaponnametonum(const std::string &name, GunsDescription &guns);
std::string weaponnumtoname(std::int32_t num, GunsDescription &guns);
std::string weaponnamebynum(std::int32_t num, GunsDescription &guns);
bool ismainweaponindex(std::int16_t weaponindex);
bool issecondaryweaponindex(std::int16_t weaponindex);
bool isdoubleweaponindex(std::int16_t weaponindex);
std::uint32_t calculatebink(std::uint32_t accumulated, std::uint32_t bink);
std::uint8_t weaponnuminternaltoexternal(std::uint8_t num);
std::uint8_t weaponnumexternaltointernal(std::uint8_t num);
