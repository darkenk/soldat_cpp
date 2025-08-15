#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <string_view>

#include "common/Vector.hpp"

struct tvector2;

// Sound effects
enum class SfxEffect : std::uint16_t
{
  INVALID = 0,
  ak74_fire = 1,
  rocketz = 2,
  ak74_reload = 3,
  m249_fire = 5,
  ruger77_fire = 6,
  ruger77_reload = 7,
  m249_reload = 8,
  mp5_fire = 9,
  mp5_reload = 10,
  spas12_fire = 11,
  spas12_reload = 12,
  standup = 13,
  fall = 14,
  spawn = 15,
  m79_fire = 16,
  m79_explosion = 17,
  m79_reload = 18,
  grenade_throw = 19,
  grenade_explosion = 20,
  grenade_bounce = 21,
  bryzg = 22,
  infiltmus = 23,
  headchop = 24,
  explosion_erg = 25,
  water_step = 26,
  bulletby = 27,
  bodyfall = 28,
  deserteagle_fire = 29,
  deserteagle_reload = 30,
  steyraug_fire = 31,
  steyraug_reload = 32,
  barretm82_fire = 33,
  barretm82_reload = 34,
  minigun_fire = 35,
  minigun_reload = 36,
  minigun_start = 37,
  minigun_end = 38,
  pickupgun = 39,
  capture = 40,
  colt1911_fire = 41,
  colt1911_reload = 42,
  changeweapon = 43,
  shell = 44,
  shell2 = 45,
  dead_hit = 46,
  throwgun = 47,
  bow_fire = 48,
  takebow = 49,
  takemedikit = 50,
  wermusic = 51,
  ts = 52,
  ctf = 53,
  berserker = 54,
  godflame = 55,
  flamer = 56,
  predator = 57,
  killberserk = 58,
  vesthit = 59,
  burn = 60,
  vesttake = 61,
  clustergrenade = 62,
  cluster_explosion = 63,
  grenade_pullout = 64,
  spit = 65,
  stuff = 66,
  smoke = 67,
  match = 68,
  roar = 69,
  step = 70,
  step2 = 71,
  step3 = 72,
  step4 = 73,
  hum = 74,
  ric = 75,
  ric2 = 76,
  ric3 = 77,
  ric4 = 78,
  dist_m79 = 79,
  dist_grenade = 80,
  dist_gun1 = 81,
  dist_gun2 = 82,
  dist_gun3 = 83,
  dist_gun4 = 84,
  death = 85,
  death2 = 86,
  death3 = 87,
  crouch_move = 88,
  hit_arg = 89,
  hit_arg2 = 90,
  hit_arg3 = 91,
  goprone = 92,
  roll = 93,
  fall_hard = 94,
  onfire = 95,
  firecrack = 96,
  scope = 97,
  scopeback = 98,
  playerdeath = 99,
  changespin = 100,
  arg = 101,
  lava = 102,
  regenerate = 103,
  prone_move = 104,
  jump = 105,
  crouch = 106,
  crouch_movel = 107,
  step5 = 108,
  step6 = 109,
  step7 = 110,
  step8 = 111,
  stop = 112,
  bulletby2 = 113,
  bulletby3 = 114,
  bulletby4 = 115,
  bulletby5 = 116,
  weaponhit = 117,
  clipfall = 118,
  bonecrack = 119,
  gaugeshell = 120,
  colliderhit = 121,
  kit_fall = 122,
  kit_fall2 = 123,
  flag = 124,
  flag2 = 125,
  takegun = 126,
  infilt_point = 127,
  menuclick = 128,
  knife = 129,
  slash = 130,
  chainsaw_d = 131,
  chainsaw_m = 132,
  chainsaw_r = 133,
  piss = 134,
  law = 135,
  chainsaw_o = 136,
  m2fire = 137,
  m2explode = 138,
  m2overheat = 139,
  signal = 140,
  m2use = 141,
  scoperun = 142,
  mercy = 143,
  ric5 = 144,
  ric6 = 145,
  ric7 = 146,
  law_start = 147,
  law_end = 148,
  boomheadshot = 149,
  snapshot = 150,
  radio_efcup = 151,
  radio_efcmid = 152,
  radio_efcdown = 153,
  radio_ffcup = 154,
  radio_ffcmid = 155,
  radio_ffcdown = 156,
  radio_esup = 157,
  radio_esmid = 158,
  radio_esdown = 159,
  bounce = 160,
  rain = 161,
  snow = 162,
  wind = 163,

  COUNT
};

template <typename T, typename E>
constexpr T ConvertEnum(const E enumValue)
{
  return static_cast<T>(enumValue);
}

template <typename E>
constexpr std::uint32_t ToUint32(const E enumValue)
{
  return ConvertEnum<std::uint32_t>(enumValue);
}

inline SfxEffect operator+(const SfxEffect &c, const std::uint32_t &value)
{
  // using IntType = typename std::underlying_type<E>::type;
  return static_cast<SfxEffect>(static_cast<std::uint32_t>(c) + value);
}

struct tsoundsample
{
  bool loaded = false;
  std::uint32_t buffer = 0;
};

struct tscriptsound
{
  std::string name;
  tsoundsample samp;
};

constexpr std::int32_t max_sources = 256;
constexpr std::int32_t reserved_sources = 128;
constexpr std::int32_t channel_weather = 127;

struct GlobalStateSound
{
  bool initsound();
  bool setsoundpaused(std::int32_t channel, bool paused);
  bool setvolume(std::int32_t channel, float volume);
  bool stopsound(std::int32_t channel);
  float scalevolumesetting(const uint8_t volumesetting);
  std::int8_t soundnametoid(const std::string &name);
  tsoundsample loadsample(const std::string_view &name, const tsoundsample &samp);
  void closesound();
  void fplaysound(SfxEffect samplenum, float listenerx, float listenery, float emitterx,
                  float emittery, std::int32_t chan);
  void loadsounds(const std::string &moddir);
  void playsound(SfxEffect sample);
  void playsound(SfxEffect sample, const tvector2 &emitter);
  void playsound(SfxEffect sample, const tvector2 &emitter, int32_t channel);
  void playsound(SfxEffect sample, std::int32_t channel);
  std::vector<tscriptsound> scriptsamp = {};
  float volumeinternal = 0.5f;
  std::int32_t defaultchannel = -1;
};

extern GlobalStateSound gGlobalStateSound;
