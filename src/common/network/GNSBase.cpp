#include "GNSBase.hpp"

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

auto constexpr LOG_MSG = "net_msg";

void NetworksGlobalCallback(SteamNetConnectionStatusChangedCallback_t * pInfo)
{
  auto network = reinterpret_cast<GNSBase*>(pInfo->m_info.m_nUserData);
  network->EmplaceSteamNetConnectionStatusChangeMessage(pInfo);
}

static void sDebugNet(ESteamNetworkingSocketsDebugOutputType nType, const char *pszMsg)
{
  LogDebug("network", "{}", pszMsg);
}

static std::atomic<std::int64_t> sNetworkCount = 0;
static std::mutex sNetworkGNSInit;

GNSBase::GNSBase()
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

GNSBase::~GNSBase()
{
  if (std::lock_guard m(sNetworkGNSInit); --sNetworkCount == 0)
  {
    GameNetworkingSockets_Kill();
  }
}

void GNSBase::RunCallbacks()
{
  mNetworkingSockets->RunCallbacks();
  auto GetCallback = [this]() -> const SteamNetConnectionStatusChangedCallback_t* {
    std::lock_guard m(mQueueMutex);
    if (mQueuedCallbacks.empty())
    {
      return nullptr;
    }
    const auto r = mQueuedCallbacks.front();
    mQueuedCallbacks.pop();
    return r;
  };

  while (const auto callback = GetCallback())
  {
    ProcessEvents(callback);
    delete callback;
  }
}

void GNSBase::EmplaceSteamNetConnectionStatusChangeMessage(
  const SteamNetConnectionStatusChangedCallback_t *pInfo)
{
  std::lock_guard m(mQueueMutex);
  static_assert(std::is_trivial_v<SteamNetConnectionInfo_t>);
  auto copy = new SteamNetConnectionStatusChangedCallback_t(*pInfo);
  mQueuedCallbacks.emplace(copy);
}
