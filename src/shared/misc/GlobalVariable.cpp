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
    grav = 0.06;
    goalticks = default_goalticks;
    survivalendround = false;
    weaponscleaned = false;
    ceasefiretime = default_ceasefire_time;
    mapchangetime = default_mapchange_time;
    timelimitcounter = 3600;
    starthealth = 150;
    sinuscounter = 0;

    voteactive = false;
    votetype = 0; // VOTE_MAP or VOTE_KICK
    votetarget = "";
    votestarter = "";
    votereason = "";
    votetimeremaining = -1;
    votenumvotes = 0;
    votemaxvotes = 0;
    votekickreasontype = false;

    pingticksadd = 2;

    notexts = 0;
}

template void InitAllGlobalVariables<Config::GetModule()>();
