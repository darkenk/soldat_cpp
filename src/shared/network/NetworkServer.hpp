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

class NetworkServer : public TNetwork
{
public:
  NetworkServer(const std::string_view host, std::uint32_t port);
  ~NetworkServer() override;
  void ProcessLoop();
  template <typename T>
  inline bool senddata(const T *Data, std::int32_t Size, HSteamNetConnection peer, std::int32_t Flags)
  {
    return senddata(reinterpret_cast<const std::byte *>(Data), Size, peer, Flags);
  }
  bool senddata(const std::byte *Data, std::int32_t Size, HSteamNetConnection peer,
                std::int32_t Flags);
  void UpdateNetworkStats(std::uint8_t Player);

  bool Disconnect(bool now);

  inline TServerPlayer* GetPlayer(const SteamNetworkingMessage_t * msg)
  {
    if (const auto it = mConnectionMap.find(msg->GetConnection()); it != mConnectionMap.end())
    {
      return it->second.get();
    }
    return nullptr;
  }

  inline TPlayers& GetPlayers() { return mPlayers; }
  void FlushMsg();

protected:
  void ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo) override;
private:
  HSteamNetPollGroup FPollGroup;
  std::map<HSteamNetConnection, std::shared_ptr<TServerPlayer>> mConnectionMap;
  TPlayers mPlayers;

  void HandleMessages(PSteamNetworkingMessage_t IncomingMsg);

};


bool InitNetworkServer(const std::string &Host, std::uint32_t Port);
NetworkServer *GetServerNetwork();
bool DeinitServerNetwork();
