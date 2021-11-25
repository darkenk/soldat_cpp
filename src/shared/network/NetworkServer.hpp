#pragma once

#include "Net.hpp"

// We're assigning a dummy player class to all sprites that are currently not being controlled
// by a player. This avoids nasty surprises with older code that reads .Player despite .Active
// being false. A player object is swapped in by CreateSprite as needed. For bots we simply leave
// the bot object and free it when it is replaced.
// Albeit this approach is very robust I'd prefer if we get rid of this and fix all .Active
// checks (if any) later. Alternatively we could move a good bit if info from Player to Sprite.
extern tplayer dummyplayer;

extern std::int32_t servertickcounter;
extern PascalArray<std::int32_t, 1, max_players> noclientupdatetime;
extern PascalArray<std::int32_t, 1, max_players> messagesasecnum;
extern PascalArray<std::uint8_t, 1, max_players> floodwarnings;
extern PascalArray<std::uint8_t, 1, max_players> pingwarnings;
extern PascalArray<std::int32_t, 1, max_players> bullettime;
extern PascalArray<std::int32_t, 1, max_players> grenadetime;
extern PascalArray<bool, 1, max_players> knifecan;

class tservernetwork : public TNetwork
{
  public:
    void ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo) override;
    tservernetwork(std::string Host, std::uint32_t Port);
    ~tservernetwork();
    void ProcessLoop();
    void HandleMessages(PSteamNetworkingMessage_t IncomingMsg);
    template <typename T>
    bool senddata(const T *Data, std::int32_t Size, HSteamNetConnection peer, std::int32_t Flags)
    {
        return senddata(reinterpret_cast<const std::byte *>(Data), Size, peer, Flags);
    }
    bool senddata(const std::byte *Data, std::int32_t Size, HSteamNetConnection peer,
                  std::int32_t Flags);
    void UpdateNetworkStats(std::uint8_t Player);

    bool disconnect(bool now) override;

  private:
    HSteamNetPollGroup FPollGroup;
};

bool InitNetworkServer(const std::string &Host, std::uint32_t Port) requires(Config::IsServer());
tservernetwork *GetServerNetwork();
bool DeinitServerNetwork() requires(Config::IsServer());
