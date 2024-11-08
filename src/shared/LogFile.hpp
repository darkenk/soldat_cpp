#pragma once

/*#include "SyncObjs.h"*/
/*#include "Classes.h"*/

#include <list>
#include <mutex>
#include <string>

#include "common/FileUtility.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "misc/SoldatConfig.hpp"

template <Config::Module M = Config::GetModule()>
void newlogfile(FileUtility& fu, tstringlist *f, const std::string &name);
template <Config::Module M = Config::GetModule()>
void writelogfile(FileUtility& fu, tstringlist *f, const std::string &name);
template <Config::Module M = Config::GetModule()>
void addlinetologfile(FileUtility& fu, tstringlist *f, const std::string &s, const std::string &name,
                      bool withdate = true);
template <Config::Module M = Config::GetModule()>
void newlogfiles(FileUtility& fu);

template <Config::Module M = Config::GetModule()>
tstringlist &GetKillLog() requires(Config::IsServer(M));

template <Config::Module M = Config::GetModule()>
std::string &GetKillLogFilename() requires(Config::IsServer(M));

template <Config::Module M = Config::GetModule()>
tstringlist *GetGameLog();

template <Config::Module M = Config::GetModule()>
std::string &GetGameLogFilename();
