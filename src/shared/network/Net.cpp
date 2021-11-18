#include "Net.hpp"
#include "../Constants.hpp"
#include "../Logging.hpp"
#include "../misc/PortUtilsSoldat.hpp"
#include "../misc/SoldatConfig.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include <array>
#include <numeric>
#include <string>
#include <vector>
#ifdef SERVER
#include "../../server/Server.hpp"
#endif

auto constexpr LOG_MSG = "net_msg";

std::int32_t maintickcounter;
// Stores all network-generated TPlayer objects
TPlayers players;

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
#ifdef SERVER
    if (FHost != k_HSteamNetPollGroup_Invalid)
    {
        for (auto &DstPlayer : players)
        {
            if (FPeer != 0)
            {
                if (Now)
                    NetworkingSockets->CloseConnection(DstPlayer->peer, 0, "", false);
                else
                    NetworkingSockets->CloseConnection(DstPlayer->peer, 0, "", true);
            }
        }
        Result = true;
    }
#endif
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

bool TNetwork::SetGlobalConfigValueInt32(ESteamNetworkingConfigValue eValue, std::int32_t val)
{
    return NetworkingUtils->SetConfigValue(eValue, k_ESteamNetworkingConfig_Global, 0,
                                           k_ESteamNetworkingConfig_Int32, &val);
}

bool TNetwork::SetGlobalConfigValueFloat(ESteamNetworkingConfigValue eValue, float val)
{
    return NetworkingUtils->SetConfigValue(eValue, k_ESteamNetworkingConfig_Global, 0,
                                           k_ESteamNetworkingConfig_Float, &val);
}

bool TNetwork::SetGlobalConfigValueString(ESteamNetworkingConfigValue eValue, const char *val)
{
    return NetworkingUtils->SetConfigValue(eValue, k_ESteamNetworkingConfig_Connection, 0,
                                           k_ESteamNetworkingConfig_String, val);
}
bool TNetwork::SetConnectionConfigValueInt32(HSteamNetConnection hConn,
                                             ESteamNetworkingConfigValue eValue, std::int32_t val)
{
    return NetworkingUtils->SetConfigValue(eValue, k_ESteamNetworkingConfig_Connection, hConn,
                                           k_ESteamNetworkingConfig_String, &val);
}

bool TNetwork::SetConnectionConfigValueFloat(HSteamNetConnection hConn,
                                             ESteamNetworkingConfigValue eValue, std::int32_t val)
{
    return NetworkingUtils->SetConfigValue(eValue, k_ESteamNetworkingConfig_Connection, hConn,
                                           k_ESteamNetworkingConfig_String, &val);
}

bool TNetwork::SetConnectionConfigValueString(HSteamNetConnection hConn,
                                              ESteamNetworkingConfigValue eValue, std::int32_t val)
{
    return NetworkingUtils->SetConfigValue(eValue, k_ESteamNetworkingConfig_Connection, hConn,
                                           k_ESteamNetworkingConfig_String, &val);
}

void TNetwork::SetDebugLevel(ESteamNetworkingSocketsDebugOutputType Level)
{
    NetworkingUtils->SetDebugOutputFunction(Level, DebugNet);
}

tplayer tplayer::Clone()
{
    // NOTE that only fields used by TScriptNewPlayer really matter here, but we clone the whole
    // thing for consistency. Obviously don"t clone handles etc. unless they can be duplicated.
    NotImplemented(NITag::NETWORK);
#if 0
    Result = TPlayer->Create;

    Result.Name = Self.Name;
    Result.ShirtColor = Self.ShirtColor;
    Result.PantsColor = Self.PantsColor;
    Result.SkinColor = Self.SkinColor;
    Result.HairColor = Self.HairColor;
    Result.JetColor = Self.JetColor;
    Result.Kills = Self.Kills;
    Result.Deaths = Self.Deaths;
    Result.Flags = Self.Flags;
    Result.PingTicks = Self.PingTicks;
    Result.PingTicksB = Self.PingTicksB;
    Result.PingTime = Self.PingTime;
    Result.realping = Self.realping;
    Result.ConnectionQuality = Self.ConnectionQuality;
    Result.Ping = Self.Ping;
    Result.Team = Self.Team;
    Result.ControlMethod = Self.ControlMethod;
    Result.Chain = Self.Chain;
    Result.HeadCap = Self.HeadCap;
    Result.HairStyle = Self.HairStyle;
    Result.SecWep = Self.SecWep;
    Result.Camera = Self.Camera;
    Result.Muted = Self.Muted;
    Result.spritenum = Self.spritenum;
    Result.DemoPlayer = Self.DemoPlayer;
#ifdef STEAM
    Result.SteamID = Self.SteamID;
    Result.SteamStats = Self.SteamStats;
#endif

#ifdef SERVER
    Result.IP = Self.IP;
    Result.Port = Self.Port;
#ifdef ENABLE_FAE
    Result.FaeResponsePending = Self.FaeResponsePending;
    Result.FaeKicked = Self.FaeKicked;
    Result.FaeTicks = Self.FaeTicks;
    Result.FaeSecret = Self.FaeSecret;
#endif
    Result.hwid = Self.hwid;
    Result.PlayTime = Self.PlayTime;
    Result.GameRequested = Self.GameRequested;
    Result.ChatWarnings = Self.ChatWarnings;
    Result.TKWarnings = Self.TKWarnings;
    Result.ScoresPerSecond = Self.ScoresPerSecond;
    Result.GrabsPerSecond = Self.GrabsPerSecond;
    Result.GrabbedInBase = Self.GrabbedInBase;
    Result.StandingPolyType = Self.StandingPolyType;
    Result.KnifeWarnings = Self.KnifeWarnings;
#endif
#endif
}

void tplayer::applyshirtcolorfromteam()
{
#ifdef SERVER
    if ((CVar::sv_teamcolors) and isteamgame())
    {
        switch (team)
        {
        case 1:
            shirtcolor = 0xFFD20F05;
            break;
        case 2:
            shirtcolor = 0xFF151FD9;
            break;
        case 3:
            shirtcolor = 0xFFD2D205;
            break;
        case 4:
            shirtcolor = 0xFF05D205;
            break;
        }
    }
#endif
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
