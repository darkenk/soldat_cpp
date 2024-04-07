#include "Net.hpp"
#include "../Constants.hpp"
#include "../Logging.hpp"
#include "../misc/PortUtilsSoldat.hpp"
#include <array>
#include <numeric>
#include <string>
#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

static_assert(sizeof(HSoldatNetConnection) == sizeof(HSteamNetConnection));
static_assert(std::is_same_v<HSoldatNetConnection, HSteamNetConnection> == true);

auto constexpr LOG_MSG = "net_msg";

void NetworksGlobalCallback(SteamNetConnectionStatusChangedCallback_t* pInfo)
{
  auto network = reinterpret_cast<TNetwork*>(pInfo->m_info.m_nUserData);
  network->EmplaceSteamNetConnectionStatusChangeMessage(pInfo);
}

static void sDebugNet(ESteamNetworkingSocketsDebugOutputType nType, const char *pszMsg)
{
  LogDebug("network", "{}", pszMsg);
}

static std::atomic<std::int64_t> sNetworkCount = 0;
static std::mutex sNetworkGNSInit;

TNetwork::TNetwork()
{
  if (std::lock_guard m(sNetworkGNSInit); ++sNetworkCount == 1)
  {
    SteamNetworkingErrMsg error; // NOLINT
    if (not GameNetworkingSockets_Init(nullptr, error))
    {
      LogCritical("network", "Game networking sockets failed {}", error);
    }
  }

  mNetworkingSockets = SteamNetworkingSockets();
  SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, &sDebugNet);
  SteamNetworkingUtils()->SetGlobalCallback_SteamNetConnectionStatusChanged(NetworksGlobalCallback);
}

TNetwork::~TNetwork()
{
  if (std::lock_guard m(sNetworkGNSInit); --sNetworkCount == 0)
  {
    GameNetworkingSockets_Kill();
  }
#ifndef STEAM
  NotImplemented("network");
#if 0
    NetworkingSockets->Destroy;
#endif
#endif
}

std::string TNetwork::GetDetailedConnectionStatus(HSoldatNetConnection hConn) const
{
  std::array<char, 2048> buf; // NOLINT
  if (mNetworkingSockets->GetDetailedConnectionStatus(hConn, buf.data(), buf.size()) == 0)
  {
    return buf.data();
  }
  return std::string{};
}

void TNetwork::SetConnectionName(const HSoldatNetConnection hConn, const std::string_view name)
{
  mNetworkingSockets->SetConnectionName(hConn, name.data());
}

std::string TNetwork::GetStringAddress(bool withPort)
{
  if (!withPort)
  {
    return mIpAddress;
  }
  return mIpAddress + ":" + std::to_string(mPort);
}

void TNetwork::RunCallbacks()
{
  mNetworkingSockets->RunCallbacks();
  std::lock_guard m(mQueueMutex);
  while (!mQueuedCallbacks.empty())
  {
    ProcessEvents(&mQueuedCallbacks.front());
    mQueuedCallbacks.pop();
  }
}
void TNetwork::EmplaceSteamNetConnectionStatusChangeMessage(
  SteamNetConnectionStatusChangedCallback_t *pInfo)
{
  std::lock_guard m(mQueueMutex);
  mQueuedCallbacks.emplace(*pInfo);
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
