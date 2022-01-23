#include "GlobalVariable.hpp"
#include "../Constants.hpp"

template <typename T, PortUtils::StringLiteral VariableName, Config::Module M, typename... Args>
T &InitGlobalVariable(Args &&...args)
{
    static T global = {std::forward<Args>(args)...};
    return global;
}

#include "GlobalVariableStorage.cpp"

template <Config::Module M>
void InitAllGlobalVariables()
{
}

template void InitAllGlobalVariables<Config::GetModule()>();
