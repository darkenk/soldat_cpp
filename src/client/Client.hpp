#pragma once

#include <array>
#include <map>
#include <string>
#include <cstdint>

#include "common/Vector.hpp"
#include "common/Weapons.hpp"
#include "common/misc/PortUtilsSoldat.hpp"

class Console;
class FileUtility;
class ConsoleMain;

struct tweaponstat
{
  std::string name;
  std::uint32_t textureid;
  std::uint32_t shots, hits, kills, headshots;
  std::uint8_t accuracy;
};

using GunArray = std::array<std::string, 17>;

struct GlobalStateClient
{
  Console &GetBigConsole();
  ConsoleMain &GetKillConsole();
  void joinserver();
  void startgame(int argc, char *argv[]);
  void shutdown();
  void exittomenu();
  void restartgraph();
  bool mainloop();
  void loadweaponnames(FileUtility& fs, GunArray& gunDisplayName, const std::string& modDir);
  void showmessage(const std::string &messagetext);
  std::string joinpassword;
  std::string joinport;
  std::string joinip;
  std::string basedirectory;
  std::string moddir;
  bool usesservermod;
  std::string gClientServerIP;
  std::int32_t gClientServerPort;
  std::uint8_t connection;
  std::uint8_t sniperline_client_hpp;
  std::uint8_t trails;
  std::uint8_t spectator;
  std::uint8_t packetadjusting;
  bool limbolock;
  std::uint8_t selteam;
  std::uint8_t mysprite;
  PascalArray<tweaponstat, 1, 20> wepstats;
  std::uint8_t wepstatsnum;
  GunArray gundisplayname;
  std::uint8_t gamethingtarget;
  std::int32_t grenadeeffecttimer;
  std::uint8_t badmapidcount;
  bool abnormalterminate;
  std::string hwid;
  std::uint16_t hitspraycounter;
  bool screentaken;
  bool targetmode;
  bool muteall;
  bool redirecttoserver;
  std::string redirectip;
  std::int32_t redirectport;
  std::string redirectmsg;
  std::map<std::string, std::string> radiomenu;
  std::array<char, 2> rmenustate;
  bool showradiomenu;
  std::uint8_t radiocooldown;
  tvector2 cameraprev;
  float camerax;
  float cameray;
  std::uint8_t camerafollowsprite;
  std::uint8_t notexts;
  std::uint8_t freecam;
  std::int32_t shotdistanceshow;
  float shotdistance;
  float shotlife;
  std::int32_t shotricochet;
};

extern GlobalStateClient gGlobalStateClient;
