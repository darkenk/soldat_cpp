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
  std::int32_t bonusfreq;
  std::string serverip;
  std::int32_t serverport;
  std::string wmname;
  std::string wmversion;
  tstringlist mapslist;
  std::int8_t lastplayer;
  std::array<std::int8_t, max_players> cheattag;
  std::array<std::int8_t, max_players> bulletwarningcount;
  std::int32_t waverespawntime;
  std::int32_t waverespawncounter;
  tstringlist remoteips;
  tstringlist adminips;
  std::array<std::string, 1000> floodip;
  std::array<std::int32_t, 1000> floodnum;
  std::array<std::string, 4> lastreqip;
  std::int8_t lastreqid;
  std::string dropip;
  std::string currentconf;
  std::int32_t htftime;
  PascalArray<std::string, 1, max_players> mutelist;
  PascalArray<std::string, 1, max_players> mutename;
  PascalArray<std::string, 1, max_players> tklist;
  PascalArray<std::int8_t, 1, max_players> tklistkills;
  std::array<std::string, Constants::MAX_LAST_ADMIN_IPS> lastadminips;
  std::int32_t mapindex;
};

extern GlobalStateServer gGlobalStateServer;

// last 4 IP"s to request game

// Mute array

// TK array
// IP
// TK Warnings

void spawnthings(std::int8_t Style, std::int8_t Amount);
void nextmap();
bool kickplayer(std::int8_t num, bool Ban, std::int32_t why, std::int32_t time,
                std::string Reason = "");
bool preparemapchange(std::string Name);
std::int8_t addbotplayer(const std::string& name, std::int32_t team);
void loadweapons(const std::string &Filename);
void startserver();
void ActivateServer(int argc, char *argv[]);
void RunServer(int argc, char *argv[]);
void ShutdownServer();

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