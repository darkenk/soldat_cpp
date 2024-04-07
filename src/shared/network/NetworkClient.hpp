#pragma once

#include "Net.hpp"
#include <functional>

extern std::int32_t clienttickcount, lastheartbeatcounter;
extern std::int32_t clientplayerreceivedcounter;
extern bool clientplayerreceived, clientplayersent;
extern bool clientvarsrecieved;
extern bool requestinggame;
extern std::int32_t noheartbeattime;
extern std::string votemapname;
extern std::uint32_t votemapcount;

class NetworkClient : public TNetwork
{
public:
  using ConnectionCallback = std::function<void(NetworkClient&)>;
  using DisconnectionCallback = std::function<void(const char*)>;

  NetworkClient() = default;
  ~NetworkClient() override = default;
  bool Connect(const std::string_view host, std::uint32_t port);
  bool Disconnect(bool now);
  bool IsConnected();
  bool IsDisconnected();
  void SetConnectionCallback(const ConnectionCallback& callback) { mConnectionCallback = callback; }
  void SetDisconnectionCallback(const DisconnectionCallback& callback) { mDisconnectionCallback = callback; }
  void ProcessLoop();
  template <typename T>
  bool SendData(const T *Data, std::int32_t Size, std::int32_t Flags,
                const source_location &location = source_location::current())
  {
    return SendData(reinterpret_cast<const std::byte *>(Data), Size, Flags, location);
  }
  bool SendData(const std::byte *Data, std::int32_t Size, std::int32_t Flags,
                const source_location &location = source_location::current());
  [[nodiscard]] HSteamNetConnection Peer() const { return mPeer; }
  void FlushMsg();

protected:
  void ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo) override;

private:
  void HandleMessages(PSteamNetworkingMessage_t IncomingMsg);
  DisconnectionCallback mDisconnectionCallback;
  ConnectionCallback mConnectionCallback;
  HSteamNetConnection mPeer = k_HSteamNetConnection_Invalid;
};

template <Config::Module M = Config::GetModule()>
void InitClientNetwork() requires(Config::IsClient());
template <Config::Module M = Config::GetModule()>
NetworkClient *GetNetwork() requires(Config::IsClient());
template <Config::Module M = Config::GetModule()>
void DeinitClientNetwork() requires(Config::IsClient());
