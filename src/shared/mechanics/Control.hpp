#pragma once

#include "common/misc/SoldatConfig.hpp" // IWYU pragma: keep
template <Config::Module M>
class Sprite;

void controlsprite(Sprite<Config::GetModule()> &spritec);
