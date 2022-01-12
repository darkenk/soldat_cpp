#pragma once

/*#include "SyncObjs.h"*/
/*#include "Classes.h"*/

#include <list>
#include <mutex>
#include <string>

#include "common/misc/PortUtilsSoldat.hpp"
#include "misc/SoldatConfig.hpp"

template <Config::Module M = Config::GetModule()>
void newlogfile(tstringlist *f, const std::string &name);
template <Config::Module M = Config::GetModule()>
void writelogfile(tstringlist *f, const std::string &name);
template <Config::Module M = Config::GetModule()>
void addlinetologfile(tstringlist *f, const std::string &s, const std::string &name,
                      bool withdate = true);
template <Config::Module M = Config::GetModule()>
void newlogfiles(const std::string& userdirectory);

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

template <Config::Module M = Config::GetModule()>
tstringlist *&GetGameLog();

template <Config::Module M = Config::GetModule()>
std::string &GetGameLogFilename();
