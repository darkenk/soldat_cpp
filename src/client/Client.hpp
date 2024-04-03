#pragma once

#include <array>
#include <map>
#include <string>

#include "common/Vector.hpp"
#include "common/Weapons.hpp"
#include "shared/Console.hpp"
#include "shared/LogFile.hpp"
#include "shared/network/NetworkClient.hpp"

void joinserver();
void startgame(int argc, const char *argv[]);
void shutdown();
void exittomenu();
void restartgraph();
void showmessage(const std::string &messagetext);
void showmessage(const std::wstring &messagetext);

struct tweaponstat
{
  std::string name;
  std::uint32_t textureid;
  std::uint32_t shots, hits, kills, headshots;
  std::uint8_t accuracy;
};

extern bool gamelooprun;

extern std::string joinpassword; // server passsword
extern std::string joinport;     // join port to server
extern std::string joinip;       // join ip to server

extern std::uint8_t initing;

extern std::string basedirectory;

extern std::string moddir;
extern bool usesservermod;

extern std::string gClientServerIP;
extern std::int32_t gClientServerPort;

extern std::uint8_t connection;

extern std::uint8_t sniperline_client_hpp;

extern std::uint8_t trails;
extern std::uint8_t spectator; // TODO: Remove

extern std::uint8_t packetadjusting;

extern bool limbolock;
extern std::uint8_t selteam;

extern std::uint8_t mysprite;

// Weapon Stats
extern PascalArray<tweaponstat, 1, 20> wepstats;
extern std::uint8_t wepstatsnum;

// FIXME skipped item at index 0
using GunArray = std::array<std::string, 17>;
extern GunArray gundisplayname;

extern std::uint8_t gamethingtarget;
extern std::int32_t grenadeeffecttimer;

extern std::uint8_t badmapidcount;

extern bool abnormalterminate;

extern std::string hwid;

extern std::uint16_t hitspraycounter;
extern bool screentaken;

extern bool targetmode;

extern bool muteall;

extern bool redirecttoserver;
extern std::string redirectip;
extern std::int32_t redirectport;
extern std::string redirectmsg;

// Radio Menu
extern std::map<std::string, std::string> radiomenu;
extern std::array<char, 2> rmenustate;
//  RMenuState: array[0..1] of Char = ' ';
extern bool showradiomenu;
extern std::uint8_t radiocooldown;

// screen
extern tvector2 cameraprev;
extern float camerax, cameray;          // camera x and y within world
extern std::uint8_t camerafollowsprite; // Tag number of object to follow
extern std::uint8_t notexts;
extern std::uint8_t freecam;
extern std::int32_t shotdistanceshow;
extern float shotdistance;
extern float shotlife;
extern std::int32_t shotricochet;
