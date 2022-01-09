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
    goalticks = default_goalticks;
    survivalendround = false;
    weaponscleaned = false;
    ceasefiretime = default_ceasefire_time;
    mapchangetime = default_mapchange_time;
    timelimitcounter = 3600;
    starthealth = 150;
    sinuscounter = 0;
    notexts = 0;
}

template void InitAllGlobalVariables<Config::GetModule()>();
