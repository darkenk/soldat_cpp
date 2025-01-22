#pragma once

#include "common/misc/SoldatConfig.hpp"
template <Config::Module M>
class Sprite;

void controlsprite(Sprite<Config::GetModule()> &spritec);
