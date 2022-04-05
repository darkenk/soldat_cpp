#include "Net.hpp"
#include "../Constants.hpp"
#include "../Logging.hpp"
#include "../misc/PortUtilsSoldat.hpp"
#include <array>
#include <numeric>
#include <string>
#include <vector>

auto constexpr LOG_MSG = "net_msg";

std::mutex TNetwork::sNetworksMutex;
std::vector<TNetwork *> TNetwork::sNetworks;

void NetworksGlobalCallback(PSteamNetConnectionStatusChangedCallback_t pInfo)
{
  std::lock_guard m(TNetwork::sNetworksMutex);
  auto s = TNetwork::sNetworks.size();
  auto n =
    std::find_if(std::begin(TNetwork::sNetworks), std::end(TNetwork::sNetworks), [pInfo](auto &v) {
      if (v->FPeer != k_HSteamNetConnection_Invalid && pInfo->m_hConn == v->FPeer)
      {
        return true;
      }
      if (v->FHost != k_HSteamListenSocket_Invalid && pInfo->m_info.m_hListenSocket == v->FHost)
      {
        return true;
      }
      return false;
    });
  SoldatAssert(n != std::end(TNetwork::sNetworks));

  std::lock_guard m2((*n)->QueueMutex);
  (*n)->QuedCallbacks.emplace(*pInfo);
}

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
  NetworkingUtils->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, &DebugNet);

  std::lock_guard m(sNetworksMutex);
  NetworkingUtils->SetGlobalCallback_SteamNetConnectionStatusChanged(NetworksGlobalCallback);
  sNetworks.emplace_back(this);
}

TNetwork::~TNetwork()
{
  {
    std::lock_guard m(sNetworksMutex);
    auto n = std::find(std::begin(sNetworks), std::end(sNetworks), this);
    if (n != std::end(sNetworks))
    {
      sNetworks.erase(n);
    }
  }
  disconnect(true);
#ifndef STEAM
  NotImplemented("network");
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

void TNetwork::RunCallbacks()
{
  NetworkingSockets->RunCallbacks();
  std::lock_guard m(QueueMutex);
  while (!QuedCallbacks.empty())
  {
    ProcessEvents(&QuedCallbacks.front());
    QuedCallbacks.pop();
  }
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
