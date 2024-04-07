#pragma once

#include "Net.hpp"

// We're assigning a dummy player class to all sprites that are currently not being controlled
// by a player. This avoids nasty surprises with older code that reads .Player despite .Active
// being false. A player object is swapped in by CreateSprite as needed. For bots we simply leave
// the bot object and free it when it is replaced.
// Albeit this approach is very robust I'd prefer if we get rid of this and fix all .Active
// checks (if any) later. Alternatively we could move a good bit if info from Player to Sprite.
extern std::int32_t servertickcounter;
extern PascalArray<std::int32_t, 1, max_players> noclientupdatetime;
extern PascalArray<std::int32_t, 1, max_players> messagesasecnum;
extern PascalArray<std::uint8_t, 1, max_players> floodwarnings;
extern PascalArray<std::uint8_t, 1, max_players> pingwarnings;
extern PascalArray<std::int32_t, 1, max_players> bullettime;
extern PascalArray<std::int32_t, 1, max_players> grenadetime;
extern PascalArray<bool, 1, max_players> knifecan;

using HSoldatMessageId = std::uint32_t;

class NetworkServer : public TNetwork
{
public:
  using DisconnectionCallback = std::function<void(std::shared_ptr<TServerPlayer>)>;

  NetworkServer(const std::string_view host, std::uint32_t port);
  ~NetworkServer() override;
  void ProcessLoop();
  template <typename T>
  inline bool SendData(const T *data, std::int32_t size, HSoldatNetConnection peer,
                       bool reliable)
  {
    return SendData(reinterpret_cast<const std::byte *>(data), size, peer, reliable);
  }
  void UpdateNetworkStats(std::shared_ptr<TServerPlayer>& player) const;

  void SetDisconnectionCallback(const DisconnectionCallback& callback) { mDisconnectionCallback = callback; }
  bool Disconnect(bool now);
  // darkenk: should be changed to Disconnect?
  void CloseConnection(HSoldatNetConnection peer, bool now);
  void FlushMsg();

  TServerPlayer *GetPlayer(const SteamNetworkingMessage_t *msg);
  inline TPlayers& GetPlayers() { return mPlayers; }

protected:
  void ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo) override;

private:
  using HSoldatListenSocket = std::uint32_t;
  using HSoldatNetPollGroup = std::uint32_t;
  HSoldatListenSocket mHost;
  HSoldatNetPollGroup FPollGroup;
  std::map<HSoldatNetConnection, std::shared_ptr<TServerPlayer>> mConnectionMap;
  TPlayers mPlayers;
  DisconnectionCallback mDisconnectionCallback;

  void HandleMessages(SteamNetworkingMessage_t *msg);
  bool SendData(const std::byte *data, std::int32_t size, HSoldatNetConnection peer, bool reliable);
};


bool InitNetworkServer(const std::string_view &host, std::uint32_t port);
NetworkServer *GetServerNetwork();
bool DeinitServerNetwork();
