#include "NetworkServer.hpp"

#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <steam/steamclientpublic.h>
#include <steam/steamnetworkingtypes.h>

#include "../Demo.hpp"
#include "NetworkServerBullet.hpp"
#include "NetworkServerConnection.hpp"
#include "NetworkServerFunctions.hpp"
#include "NetworkServerGame.hpp"
#include "NetworkServerMessages.hpp"
#include "NetworkServerSprite.hpp"
#include "NetworkServerThing.hpp"
#include "common/Logging.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/port_utils/NotImplemented.hpp"
#include "common/port_utils/Utilities.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/network/Net.hpp"

GlobalStateNetworkServer gGlobalStateNetworkServer{

};

auto LOG_NET = "network";

NetworkServer::NetworkServer(const std::string_view host, std::uint32_t port)
{
  static_assert(std::is_same_v<HSoldatListenSocket, HSteamListenSocket>);
  static_assert(std::is_same_v<HSoldatNetPollGroup, HSteamNetPollGroup>);

  SteamNetworkingIPAddr serverAddress; // NOLINT
  std::array<SteamNetworkingConfigValue_t, 2> initSettings; // NOLINT

  serverAddress.Clear();
  serverAddress.ParseString((std::string(host) + ":" + std::to_string(port)).c_str());
  initSettings[0].SetInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
  initSettings[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData, reinterpret_cast<std::int64_t>(this));
  mHost = mNetworkingSockets->CreateListenSocketIP(serverAddress, initSettings.size(), initSettings.data());

  if (mHost == k_HSteamListenSocket_Invalid)
  {
    LogWarn(LOG_NET, "Cannot listen {}:{}", host, port);
    return;
  }

  FPollGroup = mNetworkingSockets->CreatePollGroup();
  if (FPollGroup == k_HSteamNetPollGroup_Invalid)
  {
    LogWarn(LOG_NET, "Failed to create poll group");
    return;
  }
  SteamNetworkingIPAddr mAddress; // NOLINT
  mNetworkingSockets->GetListenSocketAddress(mHost, &mAddress);
  SetActive(mHost != k_HSteamNetPollGroup_Invalid);
  mPort = mAddress.m_port;
  mIpAddress = host;
}

NetworkServer::~NetworkServer()
{
  Disconnect(true);
  if (mHost != k_HSteamNetConnection_Invalid)
  {
    mNetworkingSockets->CloseListenSocket(mHost);
  }

  if (FPollGroup != k_HSteamNetPollGroup_Invalid)
  {
    mNetworkingSockets->DestroyPollGroup(FPollGroup);
  }
}

void NetworkServer::ProcessLoop()
{
  PSteamNetworkingMessage_t IncomingMsg;
  RunCallbacks();

  auto numMsgs = mNetworkingSockets->ReceiveMessagesOnPollGroup(FPollGroup, &IncomingMsg, 1);

  if (numMsgs == 0)
  {
    return;
  }
  if (numMsgs < 0)
  {
    LogWarn(LOG_NET, "Failed to poll messages");
    return;
  }
  HandleMessages(IncomingMsg);
  IncomingMsg->Release();
}

void NetworkServer::ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo)
{
  LogDebug(LOG_NET, "Server: Process Events {}", pInfo->m_info.m_eState);

  switch (pInfo->m_info.m_eState)
  {
  case k_ESteamNetworkingConnectionState_None: {
    LogInfo(LOG_NET, "Destroying peer handle");
    break;
  }
  case k_ESteamNetworkingConnectionState_ClosedByPeer:
  case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
    // NOTE that this is not called for ordinary disconnects, where we use enet"s
    // disconnect_now, which does not generate additional events. Cleanup of Player is still
    // performed explicitly.
    auto it = mConnectionMap.find(pInfo->m_hConn);
    SoldatAssert(it != mConnectionMap.end());
    auto *Player = it->second.get();

    if (Player == nullptr)
    {
      // darkenk: hm?
      mNetworkingSockets->CloseConnection(pInfo->m_hConn, 0, "", false);
      return;
    }

    if (mDisconnectionCallback)
    {
      mDisconnectionCallback(it->second);
    }

    LogWarn(LOG_NET, "Connection lost {} {}", pInfo->m_info.m_eEndReason,
            pInfo->m_info.m_szConnectionDescription);

    // call destructor; this releases any additional resources managed for the connection, such
    // as anti-cheat handles etc.
    std::erase_if(mPlayers, [&Player](const auto &v) { return Player == v.get(); });
    mConnectionMap.erase(it);

    mNetworkingSockets->CloseConnection(pInfo->m_hConn, 0, "", false);
    break;
  }
  case k_ESteamNetworkingConnectionState_Connecting: {
    SoldatAssert(pInfo->m_info.m_hListenSocket != 0);
    LogWarn(LOG_NET, "Connection request from {}", pInfo->m_info.m_szConnectionDescription);

    //  A new connection arrives on a listen socket. m_info.m_hListenSocket will be set,
    //  m_eOldState = k_ESteamNetworkingConnectionState_None,
    // and m_info.m_eState = k_ESteamNetworkingConnectionState_Connecting. See
    // AcceptConnection.
    //  and (pInfo->m_info.m_eState = k_ESteamNetworkingConnectionState_Connecting)
    if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_None)
    {
      if (not mNetworkingSockets->SetConnectionPollGroup(pInfo->m_hConn, FPollGroup))
      {
        LogWarn(LOG_NET, "Failed to set poll group for user");
        mNetworkingSockets->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        return;
      }
      mNetworkingSockets->AcceptConnection(pInfo->m_hConn);
    }
    break;
  }
  case k_ESteamNetworkingConnectionState_Connected: {
    {
      std::array<char, 128> tmp; // NOLINT
      auto player = std::make_shared<TServerPlayer>();
      player->peer = pInfo->m_hConn;
      pInfo->m_info.m_addrRemote.ToString(tmp.data(), tmp.size(), false);
      player->ip = tmp.data();
      player->port = pInfo->m_info.m_addrRemote.m_port;
      LogInfo(LOG_NET, "Connection  accepted {}", pInfo->m_info.m_szConnectionDescription);
      mPlayers.push_back(player);
      SoldatAssert(!mConnectionMap.contains(pInfo->m_hConn));
      mConnectionMap[pInfo->m_hConn] = player;
    }
    break;
  }
  default:
    break;
  }
}

void NetworkServer::HandleMessages(SteamNetworkingMessage_t *msg)
{
  auto &sprite_system = SpriteSystem::Get();
  if (msg->m_cbSize < sizeof(tmsgheader))
  {
    return; // truncated packet
  }

  auto *player = GetPlayer(msg);
  auto *packet = pmsgheader(msg->m_pData);

  switch (packet->id)
  {
  case msgid_requestgame:
    // only allowed if the player has not yet joined the game
    if ((player->spritenum == 0) and (not player->gamerequested))
    {
      serverhandlerequestgame(packet, msg->m_cbSize, *this, player);
    }
    break;

  case msgid_playerinfo:
    // allowed once after RequestGame was received, sets spritenum
    if ((player->spritenum == 0) and player->gamerequested)
    {
      serverhandleplayerinfo(packet, msg->m_cbSize, *this, player);
    }
    break;

#ifdef ENABLE_FAE
  MsgID_FaeData:
    ServerHandleFaeResponse(IncomingMsg);
#endif
  }

  // all the following commands can only be issued after the player has joined the game.
  if ((player->spritenum == 0) or
      (sprite_system.GetSprite(player->spritenum).player.get() != player))
  {
    return;
  }

  switch (packet->id)
  {
  case msgid_clientspritesnapshot:
    gGlobalStateNetworkServerSprite.serverhandleclientspritesnapshot(packet, msg->m_cbSize, *this,
                                                                     player);
    break;

  case msgid_clientspritesnapshot_mov:
    gGlobalStateNetworkServerSprite.serverhandleclientspritesnapshot_mov(packet, msg->m_cbSize,
                                                                         *this, player);
    break;

  case msgid_clientspritesnapshot_dead:
    gGlobalStateNetworkServerSprite.serverhandleclientspritesnapshot_dead(packet, msg->m_cbSize,
                                                                          *this, player);
    break;

  case msgid_playerdisconnect:
    serverhandleplayerdisconnect(packet, msg->m_cbSize, *this, player);
    break;

  case msgid_chatmessage:
    serverhandlechatmessage(packet, msg->m_cbSize, *this, player);
    break;

  case msgid_pong:
    serverhandlepong(packet, msg->m_cbSize, *this, player);
    break;

  case msgid_bulletsnapshot:
    serverhandlebulletsnapshot(packet, msg->m_cbSize, *this, player);
    break;

  case msgid_requestthing:
    serverhandlerequestthing(packet, msg->m_cbSize, *this, player);
    break;

  case msgid_votekick:
    serverhandlevotekick(packet, msg->m_cbSize, *this, player);
    break;

  case msgid_votemap:
    serverhandlevotemap(packet, msg->m_cbSize, *this, player);
    break;

  case msgid_changeteam:
    serverhandlechangeteam(packet, msg->m_cbSize, *this, player);
    break;

  case msgid_clientfreecam:
    serverhandleclientfreecam(packet, msg->m_cbSize, *this, player);
    break;
  }
}

auto NetworkServer::SendData(const std::byte *data, std::int32_t size, HSoldatNetConnection Peer,
                             bool reliable) -> bool
{
  SoldatAssert(size >= sizeof(tmsgheader));
  if (size < sizeof(tmsgheader))
  {
    LogWarn(LOG_NET, "Packet is too small: {}", size);
    return false;
  }

  if (mHost == k_HSteamNetConnection_Invalid)
  {
    return false;
  }

  if (GS::GetDemoRecorder().active())
  {
    NotImplemented("network", "check peer comparision");
    if (Peer == std::numeric_limits<std::uint32_t>::max())
    {
      GS::GetDemoRecorder().saverecord(data, size);
    }
  }
  auto flags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
  auto ret = mNetworkingSockets->SendMessageToConnection(Peer, data, size, flags, nullptr);
  return ret == k_EResultOK;
}

void NetworkServer::UpdateNetworkStats(std::shared_ptr<TServerPlayer> &player) const
{
  SteamNetworkingQuickConnectionStatus status; // NOLINT
  mNetworkingSockets->GetQuickConnectionStatus(player->peer, &status);
  player->realping = status.m_nPing;
  player->connectionquality = 0;
  if (status.m_flConnectionQualityLocal > 0.0)
  {
    player->connectionquality = status.m_flConnectionQualityLocal * 100;
  }
}

auto NetworkServer::Disconnect(bool now) -> bool
{
  if (mHost == k_HSteamNetPollGroup_Invalid)
  {
    return false;
  }

  for (const auto &player : mPlayers)
  {
    mNetworkingSockets->CloseConnection(player->peer, 0, "", !now);
  }
  mPlayers.clear();
  return true;
}
void NetworkServer::CloseConnection(HSoldatNetConnection peer, bool now)
{
  mNetworkingSockets->CloseConnection(peer, 0, "", !now);
}

auto NetworkServer::GetPlayer(const SteamNetworkingMessage_t *msg) -> TServerPlayer *
{
  if (const auto it = mConnectionMap.find(msg->GetConnection()); it != mConnectionMap.end())
  {
    return it->second.get();
  }
  return nullptr;
}

void NetworkServer::FlushMsg()
{
  for (const auto& player  : mPlayers)
  {
    mNetworkingSockets->FlushMessagesOnConnection(player->peer);
  }
}

auto NetworkServer::GetDetailedConnectionStatus(HSoldatNetConnection hConn) const -> std::string
{
  std::array<char, 2048> buf; // NOLINT
  if (mNetworkingSockets->GetDetailedConnectionStatus(hConn, buf.data(), buf.size()) == 0)
  {
    return buf.data();
  }
  return std::string{};
}

void NetworkServer::SetConnectionName(const HSoldatNetConnection hConn, const std::string_view name)
{
  mNetworkingSockets->SetConnectionName(hConn, name.data());
}

namespace
{
NetworkServer *gUDP;
}

auto GlobalStateNetworkServer::InitNetworkServer(const std::string_view &host, uint32_t port)
  -> bool
{
  gUDP = new NetworkServer(host, port);
  return gUDP != nullptr;
}

auto GlobalStateNetworkServer::GetServerNetwork() -> NetworkServer * { return gUDP; }

auto GlobalStateNetworkServer::DeinitServerNetwork() -> bool
{
  delete gUDP;
  gUDP = nullptr;
  return true;
}

// tests
#include <doctest/doctest.h>
#include <array>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "NetworkClient.hpp"

namespace
{

class NetworkServerFixture
{
public:
  NetworkServerFixture() = default;
  ~NetworkServerFixture() = default;
  NetworkServerFixture(const NetworkServerFixture &) = delete;

protected:
};

inline void sHelperProcessMessages(std::unique_ptr<NetworkServer>& server, std::unique_ptr<NetworkClientImpl>& client)
{
  client->FlushMsg();
  server->FlushMsg();
  client->ProcessLoop();
  server->ProcessLoop();
}


//--exit -ts=NetworkServer*
TEST_SUITE("NetworkServer")
{
  TEST_CASE_FIXTURE(NetworkServerFixture, "Test recreation of server")
  {
    {
      auto server = std::make_unique<NetworkServer>("0.0.0.0", 23073);
    }
    {
      auto server = std::make_unique<NetworkServer>("0.0.0.0", 23073);
    }
  }

  TEST_CASE_FIXTURE(NetworkServerFixture, "First initial test")
  {
    auto server = std::make_unique<NetworkServer>("0.0.0.0", 23073);
    auto client = std::make_unique<NetworkClientImpl>();
    client->Connect("127.0.0.1", 23073);
    while(!client->IsConnected())
    {
      sHelperProcessMessages(server, client);
    }
    sHelperProcessMessages(server, client);
    server->Disconnect(true);
    while(!client->IsDisconnected())
    {
      sHelperProcessMessages(server, client);
    }
    sHelperProcessMessages(server, client);
    CHECK_EQ(true, server->GetPlayers().empty());
  }

  TEST_CASE_FIXTURE(NetworkServerFixture, "Client disconnects first")
  {
    auto server = std::make_unique<NetworkServer>("0.0.0.0", 23073);
    auto client = std::make_unique<NetworkClientImpl>();
    client->Connect("127.0.0.1", 23073);
    while(!client->IsConnected())
    {
      sHelperProcessMessages(server, client);
    }
    CHECK_EQ(false, server->GetPlayers().empty());
    sHelperProcessMessages(server, client);
    client->Disconnect(true);
    sHelperProcessMessages(server, client);
    while(!server->GetPlayers().empty())
    {
      sHelperProcessMessages(server, client);
    }
    CHECK_EQ(true, server->GetPlayers().empty());
  }

  TEST_CASE_FIXTURE(NetworkServerFixture, "Get string address")
  {
    auto server = std::make_unique<NetworkServer>("0.0.0.0", 23073);
    CHECK_EQ("0.0.0.0:23073", server->GetStringAddress(true));
    CHECK_EQ("0.0.0.0", server->GetStringAddress(false));
  }

} // TEST_SUITE(NetworkServer)

} // namespace