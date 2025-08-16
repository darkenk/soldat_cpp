#pragma once

#include <array>
#include <map>
#include <string>
#include <cstdint>

#include "common/Vector.hpp"
#include "common/Weapons.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/network/Net.hpp"

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
  bool mainloop();
  void loadweaponnames(FileUtility& fs, GunArray& gunDisplayName, const std::string& modDir);
  void showmessage(const std::string &messagetext);
  std::string joinpassword = {};
  std::string joinport = "23073";
  std::string joinip = "127.0.0.1";
  std::string basedirectory = {};
  std::string moddir = {};
  bool usesservermod = {};
  std::string gClientServerIP = "127.0.0.1";
  std::int32_t gClientServerPort = 23073;
  std::uint8_t connection = INTERNET;
  std::uint8_t sniperline_client_hpp = 0;
  std::uint8_t trails = 1;
  std::uint8_t spectator = 0;
  std::uint8_t packetadjusting = 1;
  bool limbolock = {};
  std::uint8_t selteam = {};
  std::uint8_t mysprite = {};
  PascalArray<tweaponstat, 1, 20> wepstats = {};
  std::uint8_t wepstatsnum = 0;
  GunArray gundisplayname = {};
  std::uint8_t gamethingtarget = {};
  std::int32_t grenadeeffecttimer = 0;
  std::uint8_t badmapidcount = {};
  bool abnormalterminate = false;
  std::string hwid = {};
  std::uint16_t hitspraycounter = {};
  bool screentaken = {};
  bool targetmode = false;
  bool muteall = false;
  bool redirecttoserver = false;
  std::string redirectip = {};
  std::int32_t redirectport = {};
  std::string redirectmsg = {};
  std::map<std::string, std::string> radiomenu = {};
  std::array<char, 2> rmenustate = {};
  bool showradiomenu = false;
  std::uint8_t radiocooldown = 3;
  tvector2 cameraprev{};
  float camerax = {};
  float cameray = {};
  std::uint8_t camerafollowsprite = {};
  std::uint8_t notexts = 0;
  std::uint8_t freecam = 0;
  std::int32_t shotdistanceshow = {};
  float shotdistance = {};
  float shotlife = {};
  std::int32_t shotricochet = {};

  // should be private, but there are tests written for those methods already
  auto MountAssets(FileUtility &fu, const std::string &userdirectory,
                          const std::string &basedirectory, tsha1digest &outGameModChecksum,
                          tsha1digest &outCustomModChecksum) -> bool;
  void CreateDirectoryStructure(FileUtility &fs);
  void InitConsoles(bool test = false);

private:
  friend class ClientFixture;
  auto InitBigConsole(FileUtility *filesystem, const std::int32_t newMessageWait,
                      const std::int32_t countMax, const std::int32_t scrollTickMax) -> Console &;
  auto InitKillConsole(FileUtility *filesystem, const std::int32_t newMessageWait,
                       const std::int32_t countMax, const std::int32_t scrollTickMax)
    -> ConsoleMain &;
  void redirectdialog();
  void restartgraph();
  void startgameloop();
};

extern GlobalStateClient gGlobalStateClient;
