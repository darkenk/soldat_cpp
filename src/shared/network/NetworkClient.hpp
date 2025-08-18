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

private:
  std::unique_ptr<NetworkClientImpl> mUdp;
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

class INetwork
{
public:
  template <typename T>
  bool SendData(const T& data, const source_location &location = source_location::current())
  {
    return SendData(reinterpret_cast<const std::byte *>(&data), data.GetSize(), T::sIsReliableMessage(), location);
  }

  template <typename T>
  bool SendData(const T *data, const std::int32_t size, const bool reliable,
                const source_location &location = source_location::current())
  {
    return SendDataImpl(reinterpret_cast<const std::byte *>(data), size, reliable, location);
  }

  virtual bool Connect(const std::string_view host, std::uint32_t port) = 0;
  virtual bool Disconnect(bool now) = 0;
  virtual void ProcessLoop() = 0;
  virtual ~INetwork() = default;

protected:  
  virtual bool SendDataImpl(const std::byte *data, const std::int32_t size, const bool reliable,
              const source_location &location = source_location::current()) = 0;
};

class NetworkClientImpl : public INetwork, public TNetwork
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
  void ProcessLoop() override;
  [[nodiscard]] HSoldatNetConnection Peer() const { return mPeer; }
  void FlushMsg();
  bool Connect(const std::string_view host, std::uint32_t port) override;
  bool Disconnect(bool now) override;

protected:
  void ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo) override;

private:
  void HandleMessages(PSteamNetworkingMessage_t IncomingMsg);
  bool SendDataImpl(const std::byte *data, std::int32_t size, bool reliable,
              const source_location &location = source_location::current()) override;

  DisconnectionCallback mDisconnectionCallback;
  ConnectionCallback mConnectionCallback;
  HSoldatNetConnection mPeer;
};