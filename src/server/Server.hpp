#pragma once

#include <common/Console.hpp>
#include <common/network/Net.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "shared/Constants.hpp"
#include "shared/Cvar.hpp"
#include "common/Constants.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
// #include "shared/network/NetworkServer.hpp"

struct GlobalStateServer
{
  bool kickplayer(std::int8_t num, bool Ban, std::int32_t why, std::int32_t time,
                  std::string Reason = "");
  bool preparemapchange(std::string Name);
  std::int8_t addbotplayer(const std::string &name, std::int32_t team);
  void ActivateServer(int argc, char *argv[]);
  void RunServer(int argc, char *argv[]);
  void ShutdownServer();
  void loadweapons(const std::string &Filename);
  void nextmap();
  void spawnthings(std::int8_t Style, std::int8_t Amount);
  void startserver();
  std::int32_t bonusfreq = 3600;
  std::string serverip = "127.0.0.1";
  std::int32_t serverport = 23073;
  std::string wmname = {};
  std::string wmversion = {};
  tstringlist mapslist = {};
  std::int8_t lastplayer = {};
  std::array<std::int8_t, max_players> cheattag = {};
  std::array<std::int8_t, max_players> bulletwarningcount = {};
  std::int32_t waverespawntime = {};
  std::int32_t waverespawncounter = {};
  tstringlist remoteips = {};
  tstringlist adminips = {};
  std::array<std::string, 1000> floodip = {};
  std::array<std::int32_t, 1000> floodnum = {};
  std::array<std::string, 4> lastreqip = {};
  std::int8_t lastreqid = 0;
  std::string dropip = {};
  std::string currentconf = "soldat.ini";
  std::int32_t htftime = Constants::HTF_SEC_POINT;
  PascalArray<std::string, 1, max_players> mutelist = {};
  PascalArray<std::string, 1, max_players> mutename = {};
  PascalArray<std::string, 1, max_players> tklist = {};
  PascalArray<std::int8_t, 1, max_players> tklistkills = {};
  std::array<std::string, Constants::MAX_LAST_ADMIN_IPS> lastadminips = {};
  std::int32_t mapindex = {};

private:
  void CreateDirectoryStructure(FileUtility &fs);
  void WriteLn(const std::string &msg);
  void DaemonizeProgram();
  void ShutDown();
};

extern GlobalStateServer gGlobalStateServer;

class FileUtility;

class ConsoleServer : public ConsoleMain
{
public:
  explicit ConsoleServer(FileUtility* filesystem = nullptr, const std::int32_t newMessageWait = 0, const std::int32_t countMax = 254,
                       const std::int32_t scrollTickMax = 150, bool writeToFile = true)
    : ConsoleMain(filesystem, newMessageWait, countMax, scrollTickMax, writeToFile)
  {
  }
  void console(const std::string_view what, std::int32_t col, std::uint8_t sender = 255);
};