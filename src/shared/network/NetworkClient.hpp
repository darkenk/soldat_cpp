#pragma once

#include <functional>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include "Net.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "common/port_utils/SourceLocation.hpp"

class tplayer;
class NetworkClientImpl;

struct GlobalStateNetworkClient
{
  template <Config::Module M = Config::GetModule()>
  NetworkClientImpl *GetNetwork()
    requires(Config::IsClient(M));
  template <Config::Module M = Config::GetModule()>
  void DeinitClientNetwork()
    requires(Config::IsClient(M));
  template <Config::Module M = Config::GetModule()>
  void InitClientNetwork()
    requires(Config::IsClient(M));
  std::int32_t clienttickcount = {};
  std::int32_t lastheartbeatcounter = {};
  std::int32_t clientplayerreceivedcounter = {};
  bool clientplayerreceived = {};
  bool clientplayersent = {};
  bool clientvarsrecieved = {};
  bool requestinggame = {};
  std::int32_t noheartbeattime = 0;
  std::string votemapname = {};
  std::uint32_t votemapcount = {};
};

extern GlobalStateNetworkClient gGlobalStateNetworkClient;

struct NetworkContext
{
  NetworkContext(tplayer* player, tmsgheader* packet, std::int32_t size, NetworkClientImpl& nc):
    player{player}, packet{packet}, size{size}, networkClient{nc} {}

  tplayer* player;
  tmsgheader* packet;
  std::int32_t size;
  NetworkClientImpl& networkClient;
};

template<class NetworkImpl>
class NetworkBase
{
public:
  template <typename T>
  bool SendData(const T& data, const source_location &location = source_location::current())
  {
    return This()->SendData(reinterpret_cast<const std::byte *>(&data), data.GetSize(), T::sIsReliableMessage(), location);
  }

  template <typename T>
  bool SendData(const T *data, const std::int32_t size, const bool reliable,
                const source_location &location = source_location::current())
  {
    return This()->SendDataImpl(reinterpret_cast<const std::byte *>(data), size, reliable, location);
  }
  bool Connect(const std::string_view host, std::uint32_t port)
  {
    return This()->ConnectImpl(host, port);
  }
  bool Disconnect(bool now)
  {
    return This()->DisconnectImpl(now);
  }
  void ProcessLoop()
  {
    return This()->ProcessLoopImpl();
  }
private:
  NetworkImpl* This() { return static_cast<NetworkImpl*>(this); }
};

class NetworkClientImpl : public NetworkBase<NetworkClientImpl>, public TNetwork
{
public:
  using ConnectionCallback = std::function<void(NetworkClientImpl&)>;
  using DisconnectionCallback = std::function<void(const char*)>;

  NetworkClientImpl();
  ~NetworkClientImpl() override = default;
  bool IsConnected();
  bool IsDisconnected();
  void SetConnectionCallback(const ConnectionCallback& callback) { mConnectionCallback = callback; }
  void SetDisconnectionCallback(const DisconnectionCallback& callback) { mDisconnectionCallback = callback; }
  void ProcessLoopImpl();
  [[nodiscard]] HSoldatNetConnection Peer() const { return mPeer; }
  void FlushMsg();

protected:
  void ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo) override;

private:
  friend class NetworkBase<NetworkClientImpl>;
  void HandleMessages(PSteamNetworkingMessage_t IncomingMsg);
  bool SendDataImpl(const std::byte *data, std::int32_t size, bool reliable,
              const source_location &location = source_location::current());
  bool ConnectImpl(const std::string_view host, std::uint32_t port);
  bool DisconnectImpl(bool now);

  DisconnectionCallback mDisconnectionCallback;
  ConnectionCallback mConnectionCallback;
  HSoldatNetConnection mPeer;
};
