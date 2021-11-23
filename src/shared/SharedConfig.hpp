#pragma once

#include "common/misc/TIniFile.hpp"
#include "mechanics/Sprites.hpp"
#include "misc/SoldatConfig.hpp"

bool loadbotconfig(TIniFile &iniFile, tsprite &spritec);

bool loadweaponsconfig(TIniFile &iniFile, std::string &modname, std::string &modversion,
                       GunsDescription &gunDesc);
