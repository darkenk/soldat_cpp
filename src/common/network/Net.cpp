#include "Net.hpp"
#include "../Constants.hpp"
#include "../Logging.hpp"
#include "../misc/PortUtilsSoldat.hpp"
#include <array>
#include <numeric>
#include <string>
#include <vector>

auto constexpr LOG_MSG = "net_msg";

std::int32_t maintickcounter;

std::int32_t playersnum, botsnum, spectatorsnum;
PascalArray<std::int32_t, 1, 4> playersteamnum;

void DebugNet(ESteamNetworkingSocketsDebugOutputType nType, const char *pszMsg)
{
    LogDebug("network", "{}", pszMsg);
}

TNetwork::TNetwork()
{
    FInit = true;

    SteamNetworkingErrMsg error;
    if (not GameNetworkingSockets_Init(nullptr, error))
    {
        LogDebug("network", "Game networking sockets failed {}", error);
    }

    NetworkingSockets = SteamNetworkingSockets();
    NetworkingUtils = SteamNetworkingUtils();

    NetworkingUtils->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg,
                                            &DebugNet);
}

TNetwork::~TNetwork()
{
    disconnect(true);
#ifndef STEAM
    NotImplemented(NITag::NETWORK);
#if 0
    NetworkingSockets->Destroy;
#endif
#endif
}

bool TNetwork::disconnect(bool Now)
{
    auto Result = false;

    return Result;
}

void TNetwork::FlushMsg()
{
    if (FPeer != k_HSteamNetConnection_Invalid)
    {
        NetworkingSockets->FlushMessagesOnConnection(FPeer);
    }
}

std::string TNetwork::GetDetailedConnectionStatus(HSteamNetConnection hConn)
{
    TStatsString StatsText;
    std::string Result;
    if (NetworkingSockets->GetDetailedConnectionStatus(hConn, StatsText.data(), 2048) == 0)
    {
        Result = StatsText.data();
    }
    else
    {
        Result = "";
    }
    return Result;
}

SteamNetworkingQuickConnectionStatus TNetwork::GetQuickConnectionStatus(HSteamNetConnection hConn)
{
    SteamNetworkingQuickConnectionStatus Result;
    NetworkingSockets->GetQuickConnectionStatus(hConn, &Result);
    return Result;
}

void TNetwork::setconnectionname(HSteamNetConnection hConn, std::string Name)
{
    NetworkingSockets->SetConnectionName(hConn, pchar(Name));
}

std::string TNetwork::GetStringAddress(PSteamNetworkingIPAddr pAddress, bool Port)
{
    std::array<char, 128> TempIP;
    pAddress->ToString(TempIP.data(), 128, Port);
    return TempIP.data();
}

template <>
struct fmt::formatter<tvector2> : fmt::formatter<std::string_view>
{
    template <typename FormatContext>
    auto format(tvector2 c, FormatContext &ctx)
    {
        return fmt::formatter<std::string_view>::format(fmt::format("({:3},{:3})", c.x, c.y), ctx);
    }
};

void tmsg_bulletsnapshot::Dump()
{
    LogDebug(LOG_MSG,
             "Bulletsnapstot: \n"
             "  owner: {}\n"
             "  weaponnum: {}\n"
             "  pos: {}..\n"
             "  velocity: {}\n"
             "  seed: {}\n"
             "  forced: {}",
             owner, weaponnum, pos, velocity, seed, forced);
}
