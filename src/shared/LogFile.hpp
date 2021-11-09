#pragma once

/*#include "SyncObjs.h"*/
/*#include "Classes.h"*/

#include <list>
#include <mutex>
#include <string>

#include "misc/PortUtilsSoldat.hpp"
#include "misc/SoldatConfig.hpp"

void newlogfile(tstringlist *f, const std::string &name);
void writelogfile(tstringlist *f, const std::string &name);
void addlinetologfile(tstringlist *f, const std::string &s, const std::string &name,
                      bool withdate = true);
void newlogfiles();

extern tstringlist *gamelog;
extern std::string consolelogfilename;

constexpr tstringlist &GetKillLog() requires(Config::IsServer())
{
    extern tstringlist killlog;
    return killlog;
}

constexpr std::string &GetKillLogFilename() requires(Config::IsServer())
{
    extern std::string killlogfilename;
    return killlogfilename;
}
extern std::mutex loglock;
