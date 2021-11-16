#pragma once

#include "mechanics/Sprites.hpp"
#include "misc/SoldatConfig.hpp"
#include "misc/TIniFile.hpp"

bool loadbotconfig(TIniFile &iniFile, tsprite &spritec);

bool loadweaponsconfig(TIniFile &iniFile, std::string &modname, std::string &modversion,
                       GunsDescription &gunDesc);
