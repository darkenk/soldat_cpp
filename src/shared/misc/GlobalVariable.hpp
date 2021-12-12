#pragma once
#include "SoldatConfig.hpp"
#include "common/misc/PortUtils.hpp"

template <typename T, PortUtils::StringLiteral VariableName, Config::Module M = Config::GetModule(),
          typename... Args>
T &InitGlobalVariable(Args &&...args);

// template <typename _Tp, typename... _Args>
// inline typename _MakeUniq<_Tp>::__single_object make_unique(_Args &&...__args)
//{
//    return unique_ptr<_Tp>(new _Tp(std::forward<_Args>(__args)...));
//}

template <Config::Module M>
void InitAllGlobalVariables();
