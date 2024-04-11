#pragma once
#include <queue>

struct SteamNetConnectionStatusChangedCallback_t;
struct SteamNetworkingIPAddr;
struct SteamNetworkingMessage_t;
class ISteamNetworkingSockets;

class GNSBase {
protected:
  GNSBase();
  virtual ~GNSBase();

  void RunCallbacks();
  virtual void ProcessEvents(const SteamNetConnectionStatusChangedCallback_t* pInfo) = 0;

  friend void NetworksGlobalCallback(SteamNetConnectionStatusChangedCallback_t * pInfo);
  ISteamNetworkingSockets *mNetworkingSockets;
private:
  void EmplaceSteamNetConnectionStatusChangeMessage(
    const SteamNetConnectionStatusChangedCallback_t *pInfo);
  std::mutex mQueueMutex;
  std::queue<SteamNetConnectionStatusChangedCallback_t*> mQueuedCallbacks;
};
