#pragma once
#include "SoldatConfig.hpp"
#include "common/misc/PortUtils.hpp"

template <typename T, PortUtils::StringLiteral VariableName, Config::Module M = Config::GetModule(),
          typename... Args>
T &InitGlobalVariable(Args &&...args);

template <Config::Module M>
void InitAllGlobalVariables();
