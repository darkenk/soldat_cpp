#pragma once
#include "SoldatConfig.hpp"
#include "common/misc/PortUtils.hpp"

template<Config::Module M = Config::GetModule()>
struct GlobalSystems
{
    static void Init();
    static void Deinit();
};
