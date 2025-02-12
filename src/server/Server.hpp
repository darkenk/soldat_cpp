#pragma once

#include "shared/Constants.hpp"
#include "shared/Cvar.hpp"

#include <common/Console.hpp>
#include <common/network/Net.hpp>
// #include "shared/network/NetworkServer.hpp"

extern std::int32_t bonusfreq;

extern std::string serverip;
extern std::int32_t serverport;
extern std::string wmname, wmversion;

extern tstringlist mapslist;
extern std::int8_t lastplayer;
extern std::array<std::int8_t, max_players> cheattag;
extern std::array<std::int8_t, max_players> bulletwarningcount;
extern std::int32_t waverespawntime, waverespawncounter;
extern tstringlist remoteips, adminips;
extern std::array<std::string, 1000> floodip;
extern std::array<std::int32_t, 1000> floodnum;
extern std::array<std::string, 4> lastreqip; // last 4 IP"s to request game
extern std::int8_t lastreqid;
extern std::string dropip;
extern std::string currentconf;

extern std::int32_t htftime;

// Mute array
extern PascalArray<std::string, 1, max_players> mutelist;
extern PascalArray<std::string, 1, max_players> mutename;

// TK array
extern PascalArray<std::string, 1, max_players> tklist;      // IP
extern PascalArray<std::int8_t, 1, max_players> tklistkills; // TK Warnings

extern std::array<std::string, Constants::MAX_LAST_ADMIN_IPS> lastadminips;
extern std::int32_t mapindex;

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