#pragma once

#include "common/misc/TIniFile.hpp"
#include "misc/SoldatConfig.hpp"

template <Config::Module M>
class Sprite;

bool loadbotconfig(TIniFile &iniFile, Sprite<Config::GetModule()> &spritec, GunsDescription &guns);

bool loadweaponsconfig(TIniFile &iniFile, std::string &modname, std::string &modversion,
                       GunsDescription &gunDesc);
