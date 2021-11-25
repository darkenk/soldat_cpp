#pragma once

#include "common/Anims.hpp"
#include "misc/SoldatConfig.hpp"
#include <string>

template <Config::Module M = Config::GetModule()>
void loadanimobjects(const std::string &moddir);
