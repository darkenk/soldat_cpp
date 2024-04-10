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

class NetworkClientImpl;

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

template <Config::Module M = Config::GetModule()>
void InitClientNetwork() requires(Config::IsClient());
template <Config::Module M = Config::GetModule()>
NetworkClientImpl *GetNetwork() requires(Config::IsClient());
template <Config::Module M = Config::GetModule()>
void DeinitClientNetwork() requires(Config::IsClient());
