#pragma once

#include "mechanics/Sprites.hpp"
#include "misc/SoldatConfig.hpp"

bool loadbotconfig(const std::string filepath, tsprite &spritec) requires(Config::IsServer());

bool loadweaponsconfig(const std::string &filepath);
