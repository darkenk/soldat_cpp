#include "NetworkServer.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
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

std::int32_t servertickcounter;
PascalArray<std::int32_t, 1, max_players> noclientupdatetime;
PascalArray<std::int32_t, 1, max_players> messagesasecnum;
PascalArray<std::uint8_t, 1, max_players> floodwarnings;
PascalArray<std::uint8_t, 1, max_players> pingwarnings;
PascalArray<std::int32_t, 1, max_players> bullettime;
PascalArray<std::int32_t, 1, max_players> grenadetime;
PascalArray<bool, 1, max_players> knifecan;

auto LOG_NET = "network";

namespace
{
void ProcessEventsCallback(PSteamNetConnectionStatusChangedCallback_t pInfo)
{
  GetServerNetwork()->ProcessEvents(pInfo);
}
} // namespace

ServerNetwork::ServerNetwork(const std::string_view host, std::uint32_t port)
{
  SteamNetworkingIPAddr ServerAddress; // NOLINT
  std::array<SteamNetworkingConfigValue_t, 2> InitSettings; // NOLINT

  ServerAddress.Clear();
  ServerAddress.ParseString((std::string(host) + ":" + std::to_string(port)).c_str());
  InitSettings[0].SetInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
  InitSettings[1].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                         reinterpret_cast<void*>(&ProcessEventsCallback));
  FHost = NetworkingSockets->CreateListenSocketIP(ServerAddress, 1, InitSettings.data());

  if (FHost == k_HSteamListenSocket_Invalid)
  {
    LogWarn(LOG_NET, "Cannot listen {}:{}", host, port);
    return;
  }

  FPollGroup = NetworkingSockets->CreatePollGroup();
  if (FPollGroup == k_HSteamNetPollGroup_Invalid)
  {
    LogWarn(LOG_NET, "Failed to create poll group");
    return;
  }
  NetworkingSockets->GetListenSocketAddress(FHost, &FAddress);

  if (FHost != k_HSteamNetPollGroup_Invalid)
  {
    SetActive(true);
  }
}

ServerNetwork::~ServerNetwork()
{
  ServerNetwork::disconnect(true);
  if (FHost != k_HSteamNetConnection_Invalid)
  {
    NetworkingSockets->CloseListenSocket(FHost);
  }

  if (FPollGroup != k_HSteamNetPollGroup_Invalid)
  {
    NetworkingSockets->DestroyPollGroup(FPollGroup);
  }
}

void ServerNetwork::ProcessLoop()
{
  PSteamNetworkingMessage_t IncomingMsg;
  RunCallbacks();

  auto NumMsgs = NetworkingSockets->ReceiveMessagesOnPollGroup(FPollGroup, &IncomingMsg, 1);

  if (NumMsgs == 0)
  {
    return;
  }
  if (NumMsgs < 0)
  {
    LogWarn(LOG_NET, "Failed to poll messages");
    return;
  }
  HandleMessages(IncomingMsg);
}

void ServerNetwork::ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo)
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
    auto Player = it->second.get();

    if (Player == nullptr)
    {
      // DK: hm?
      NetworkingSockets->CloseConnection(pInfo->m_hConn, 0, "", false);
      return;
    }

    // the sprite may be zero if we"re still in the setup phase
    if (Player->spritenum != 0)
    {
      GS::GetMainConsole().console(Player->name + " could not respond", warning_message_color);
      serverplayerdisconnect(Player->spritenum, kick_noresponse);
#ifdef SCRIPT
      ScrptDispatcher.OnLeaveGame(Player->spritenum, false);
#endif
      SpriteSystem::Get().GetSprite(Player->spritenum).kill();
      SpriteSystem::Get().GetSprite(Player->spritenum).player = std::make_shared<TServerPlayer>();
    }

    LogWarn(LOG_NET, "Connection lost {} {}", pInfo->m_info.m_eEndReason,
            pInfo->m_info.m_szConnectionDescription);

    // call destructor; this releases any additional resources managed for the connection, such
    // as anti-cheat handles etc.
    mPlayers.erase(std::remove_if(mPlayers.begin(), mPlayers.end(),
                                 [&Player](const auto &v) { return Player == v.get(); }),
                  mPlayers.end());
    delete Player;
    mConnectionMap.erase(it);

    NetworkingSockets->CloseConnection(pInfo->m_hConn, 0, "", false);
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
      if (not NetworkingSockets->SetConnectionPollGroup(pInfo->m_hConn, FPollGroup))
      {
        LogWarn(LOG_NET, "Failed to set poll group for user");
        NetworkingSockets->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        return;
      }
      NetworkingSockets->AcceptConnection(pInfo->m_hConn);
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
      NetworkingSockets->SetConnectionUserData(pInfo->m_hConn, reinterpret_cast<std::int64_t>(player.get()));
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

void ServerNetwork::HandleMessages(PSteamNetworkingMessage_t IncomingMsg)
{
  if (IncomingMsg->m_cbSize < sizeof(tmsgheader))
  {
    IncomingMsg->Release();
    return; // truncated packet
  }

  auto Player = GetPlayer(IncomingMsg);
  auto PacketHeader = pmsgheader(IncomingMsg->m_pData);

  switch (PacketHeader->id)
  {
  case msgid_requestgame:
    // only allowed if the player has not yet joined the game
    if ((Player->spritenum == 0) and (not Player->gamerequested))
      serverhandlerequestgame(IncomingMsg);
    break;

  case msgid_playerinfo:
    // allowed once after RequestGame was received, sets spritenum
    if ((Player->spritenum == 0) and Player->gamerequested)
      serverhandleplayerinfo(IncomingMsg);
    break;

#ifdef ENABLE_FAE
  MsgID_FaeData:
    ServerHandleFaeResponse(IncomingMsg);
#endif
  }

  // all the following commands can only be issued after the player has joined the game.
  if ((Player->spritenum == 0) or
      (SpriteSystem::Get().GetSprite(Player->spritenum).player.get() != Player))
  {
    IncomingMsg->Release();
    return;
  }

  switch (PacketHeader->id)
  {
  case msgid_clientspritesnapshot:
    serverhandleclientspritesnapshot(IncomingMsg);
    break;

  case msgid_clientspritesnapshot_mov:
    serverhandleclientspritesnapshot_mov(IncomingMsg);
    break;

  case msgid_clientspritesnapshot_dead:
    serverhandleclientspritesnapshot_dead(IncomingMsg);
    break;

  case msgid_playerdisconnect:
    serverhandleplayerdisconnect(IncomingMsg);
    break;

  case msgid_chatmessage:
    serverhandlechatmessage(IncomingMsg);
    break;

  case msgid_pong:
    serverhandlepong(IncomingMsg);
    break;

  case msgid_bulletsnapshot:
    serverhandlebulletsnapshot(IncomingMsg);
    break;

  case msgid_requestthing:
    serverhandlerequestthing(IncomingMsg);
    break;

  case msgid_votekick:
    serverhandlevotekick(IncomingMsg);
    break;

  case msgid_votemap:
    serverhandlevotemap(IncomingMsg);
    break;

  case msgid_changeteam:
    serverhandlechangeteam(IncomingMsg);
    break;

  case msgid_clientfreecam:
    serverhandleclientfreecam(IncomingMsg);
    break;

#ifdef STEAM
  MsgID_VoiceData:
    ServerHandleVoiceData(IncomingMsg);
#endif
  }

  IncomingMsg->Release();
}

bool ServerNetwork::senddata(const std::byte *Data, std::int32_t Size, HSteamNetConnection Peer,
                              std::int32_t Flags)
{
  if (Size < sizeof(tmsgheader))
    return false;

  if (FHost == k_HSteamNetConnection_Invalid)
    return false;

  if (GS::GetDemoRecorder().active())
  {
    NotImplemented("network", "check peer comparision");
    if (Peer == std::numeric_limits<std::uint32_t>::max())
      GS::GetDemoRecorder().saverecord(Data, Size);
  }

  auto ret = NetworkingSockets->SendMessageToConnection(Peer, Data, Size, Flags, nullptr);
  return ret == k_EResultOK;
}

void ServerNetwork::UpdateNetworkStats(std::uint8_t Player)
{
  SteamNetworkingQuickConnectionStatus Stats = GetQuickConnectionStatus(SpriteSystem::Get().GetSprite(Player).player->peer);
  SpriteSystem::Get().GetSprite(Player).player->realping = Stats.m_nPing;
  if (Stats.m_flConnectionQualityLocal > 0.0)
  {
    SpriteSystem::Get().GetSprite(Player).player->connectionquality =
      Stats.m_flConnectionQualityLocal * 100;
  }
  else
  {
    SpriteSystem::Get().GetSprite(Player).player->connectionquality = 0;
  }
}

bool ServerNetwork::disconnect(bool now)
{
  if (FHost == k_HSteamNetPollGroup_Invalid)
  {
    return false;
  }

  for (const auto &DstPlayer : mPlayers)
  {
    NetworkingSockets->CloseConnection(DstPlayer->peer, 0, "", !now);
  }
  mPlayers.clear();
  return true;
}

namespace
{
ServerNetwork *gUDP;
}

bool InitNetworkServer(const std::string &Host, uint32_t Port)
{
  gUDP = new ServerNetwork(Host, Port);
  return gUDP != nullptr;
}

ServerNetwork *GetServerNetwork()
{
  return gUDP;
}

bool DeinitServerNetwork()
{
  delete gUDP;
  gUDP = nullptr;
  return true;
}

// tests
#include <doctest/doctest.h>
#include "NetworkClient.hpp"

namespace
{

class NetworkServerFixture
{
public:
  NetworkServerFixture() {}
  ~NetworkServerFixture() {}
  NetworkServerFixture(const NetworkServerFixture &) = delete;

protected:
};

//--exit -ts=NetworkServer*
TEST_SUITE("NetworkServer")
{
  TEST_CASE_FIXTURE(NetworkServerFixture, "Test recreation of server" * doctest::skip(true))
  {
    {
      auto server = std::make_unique<ServerNetwork>("0.0.0.0", 23073);
    }
    {
      auto server = std::make_unique<ServerNetwork>("0.0.0.0", 23073);
    }
  }

  TEST_CASE_FIXTURE(NetworkServerFixture, "First initial test" * doctest::skip(true))
  {
    auto server = std::make_unique<ServerNetwork>("0.0.0.0", 23073);
    auto client = std::make_unique<tclientnetwork>();
    client->connect("127.0.0.1", 23073);
    auto iterations = 0;
    while(!client->IsConnected())
    {
      client->FlushMsg();
      server->FlushMsg();
      client->processloop();
      server->ProcessLoop();
      iterations++;
    }
    LogDebug(LOG_NET, "Iters1 {}", iterations);
    client->FlushMsg();
    server->FlushMsg();
    client->processloop();
    server->ProcessLoop();
    iterations++;
    LogDebug(LOG_NET, "Iters2 {}", iterations);
    server->disconnect(true);
    while(!client->IsDisconnected())
    {
      client->FlushMsg();
      server->FlushMsg();
      server->ProcessLoop();
      client->processloop();
      iterations++;
    }
    LogDebug(LOG_NET, "Iters3 {}", iterations);
    client->FlushMsg();
    server->FlushMsg();
    server->ProcessLoop();
    client->processloop();
    iterations++;
    LogDebug(LOG_NET, "Iters4 {}", iterations);
    auto ret = client->GetQuickConnectionStatus(client->Peer());
    client->disconnect(true);
    CHECK_EQ(true, server->GetPlayers().empty());
  }
} // TEST_SUITE(NetworkServer)


} // namespace