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

class tclientnetwork : public TNetwork
{
public:
  void ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo) override;
  tclientnetwork();
  virtual ~tclientnetwork()
  {
  }
  bool connect(const std::string& Host, std::uint32_t Port);
  bool IsConnected();
  bool IsDisconnected();
  void SetDisconnectionCallback(const std::function<void(const char*)>& callback) { mDisconnectionCallback = callback;}
  void processloop();
  void handlemessages(PSteamNetworkingMessage_t IncomingMsg);
  template <typename T>
  bool senddata(const T *Data, std::int32_t Size, std::int32_t Flags,
                const source_location &location = source_location::current())
  {
    return senddata(reinterpret_cast<const std::byte *>(Data), Size, Flags, location);
  }
  bool senddata(const std::byte *Data, std::int32_t Size, std::int32_t Flags,
                const source_location &location = source_location::current());
private:
  std::function<void(const char*)> mDisconnectionCallback;
};

template <Config::Module M = Config::GetModule()>
void InitClientNetwork() requires(Config::IsClient());
template <Config::Module M = Config::GetModule()>
tclientnetwork *GetNetwork() requires(Config::IsClient());
template <Config::Module M = Config::GetModule()>
void DeinitClientNetwork() requires(Config::IsClient());
