#pragma once
#include "SoldatConfig.hpp"
#include "PortUtils.hpp"

template <typename T, PortUtils::StringLiteral VariableName, Config::Module M = Config::GetModule()>
T &InitGlobalVariable();
