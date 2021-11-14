#pragma once

#include "mechanics/Sprites.hpp"
#include "misc/SoldatConfig.hpp"
#include "misc/TIniFile.hpp"

bool loadbotconfig(TIniFile &iniFile, tsprite &spritec);

bool loadweaponsconfig(const std::string &filepath);
