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

tservernetwork::tservernetwork(std::string Host, std::uint32_t Port)
{
  SteamNetworkingIPAddr ServerAddress;
  SteamNetworkingConfigValue_t InitSettings[2];
  // std::array<Char, 128> TempIP;
  if (FInit)
  {
    ServerAddress.Clear();
    ServerAddress.ParseString(pchar(Host + ":" + inttostr(Port)));
    InitSettings[0].m_eValue = k_ESteamNetworkingConfig_IP_AllowWithoutAuth;
    InitSettings[0].m_eDataType = k_ESteamNetworkingConfig_Int32;
    InitSettings[0].m_val.m_int32 = 1;
    InitSettings[1].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                           (void *)&ProcessEventsCallback);
    //#ifdef STEAM
    // if sv_steamonly.Value then
    //  InitSettings.m_int32 = 0
    // else
    //#endif
    FHost = NetworkingSockets->CreateListenSocketIP(ServerAddress, 1, InitSettings);

    if (FHost == k_HSteamListenSocket_Invalid)
    {
      return;
    }

    FPollGroup = NetworkingSockets->CreatePollGroup();
    if (FPollGroup == k_HSteamNetPollGroup_Invalid)
    {
      LogWarn(LOG_NET, "Failed to create poll group");
      return;
    }
    else
    {
      NetworkingSockets->GetListenSocketAddress(FHost, &FAddress);
    }
  }

  if (FHost != k_HSteamNetPollGroup_Invalid)
  {
    SetActive(true);
  }
}

void tservernetwork::ProcessLoop()
{
  std::int32_t NumMsgs;
  PSteamNetworkingMessage_t IncomingMsg;
  RunCallbacks();

  NumMsgs = NetworkingSockets->ReceiveMessagesOnPollGroup(FPollGroup, &IncomingMsg, 1);

  if (NumMsgs == 0)
  {
    return;
  }
  else if (NumMsgs < 0)
  {
    LogWarn(LOG_NET, "Failed to poll messages");
    return;
  }
  else
  {
    HandleMessages(IncomingMsg);
  }
}

void tservernetwork::ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo)
{
  std::array<char, 128> info;
  TIPString TempIP;
#ifdef DEVELOPMENT
  Debug("[NET] Received SteamNetConnectionStatusChangedCallback_t ",
        ToStr(pInfo ^, TypeInfo(SteamNetConnectionStatusChangedCallback_t)));
#endif
  switch (pInfo->m_info.m_eState)
  {
  case k_ESteamNetworkingConnectionState_None: {
    FPeer = k_HSteamNetConnection_Invalid;
    LogInfo(LOG_NET, "Destroying peer handle");
    break;
  }
  case k_ESteamNetworkingConnectionState_ClosedByPeer:
  case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
    if (pInfo->m_info.m_nUserData == 0)
    {
      NetworkingSockets->CloseConnection(pInfo->m_hConn, 0, "", false);
      return;
    }
    // NOTE that this is not called for ordinary disconnects, where we use enet"s
    // disconnect_now, which does not generate additional events. Cleanup of Player is still
    // performed explicitly.
    auto Player = reinterpret_cast<TServerPlayer *>(pInfo->m_info.m_nUserData);

    if (Player == nullptr)
    {
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
    players.erase(std::remove_if(players.begin(), players.end(),
                                 [&Player](const auto &v) { return Player == v.get(); }),
                  players.end());
    delete Player;

    NetworkingSockets->CloseConnection(pInfo->m_hConn, 0, "", false);
    break;
  }
  case k_ESteamNetworkingConnectionState_Connecting: {
    if (pInfo->m_info.m_hListenSocket != 0)
    {
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
        pInfo->m_info.m_identityRemote.ToString(info.data(), 1024);
      }
    }
    break;
  }
  case k_ESteamNetworkingConnectionState_Connected: {
    // if pInfo->m_eOldState = k_ESteamNetworkingConnectionState_Connecting then
    {
      auto Player = std::make_shared<TServerPlayer>();
      Player->peer = pInfo->m_hConn;
      pInfo->m_info.m_addrRemote.ToString(TempIP.data(), 128, false);
      Player->ip = TempIP.data();
      Player->port = pInfo->m_info.m_addrRemote.m_port;
#ifdef STEAM
      Player->SteamID = TSteamID(pInfo->m_info.m_identityRemote.GetSteamID64);
#endif
      NotImplemented("network", "Pointer cast is probably wrong");
      NetworkingSockets->SetConnectionUserData(pInfo->m_hConn, (std::uint64_t)Player.get());
      LogInfo(LOG_NET, "Connection  accepted {}", pInfo->m_info.m_szConnectionDescription);
      players.push_back(Player);
    }
    break;
  }
  default:
    break;
  }
}

void tservernetwork::HandleMessages(PSteamNetworkingMessage_t IncomingMsg)
{
  TServerPlayer *Player;
  pmsgheader PacketHeader;
  if (IncomingMsg->m_cbSize < sizeof(tmsgheader))
  {
    IncomingMsg->Release();
    return; // truncated packet
  }

  if (IncomingMsg->m_nConnUserData == -1)
  {
    IncomingMsg->Release();
    return;
  }

  Player = reinterpret_cast<TServerPlayer *>(IncomingMsg->m_nConnUserData);
  PacketHeader = pmsgheader(IncomingMsg->m_pData);

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

tservernetwork::~tservernetwork()
{
  tservernetwork::disconnect(true);
  if (FHost != k_HSteamNetConnection_Invalid)
  {
    NetworkingSockets->CloseListenSocket(FHost);
  }

  if (FPollGroup != k_HSteamNetPollGroup_Invalid)
  {
    NetworkingSockets->DestroyPollGroup(FPollGroup);
  }

  players.clear();
}

bool tservernetwork::senddata(const std::byte *Data, std::int32_t Size, HSteamNetConnection Peer,
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

void tservernetwork::UpdateNetworkStats(std::uint8_t Player)
{
  SteamNetworkingQuickConnectionStatus Stats;
  Stats = GetQuickConnectionStatus(SpriteSystem::Get().GetSprite(Player).player->peer);
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

bool tservernetwork::disconnect(bool now)
{
  if (FHost != k_HSteamNetPollGroup_Invalid)
  {
    for (auto &DstPlayer : players)
    {
      if (FPeer != 0)
      {
        if (now)
          NetworkingSockets->CloseConnection(DstPlayer->peer, 0, "", false);
        else
          NetworkingSockets->CloseConnection(DstPlayer->peer, 0, "", true);
      }
    }
    return true;
  }
  return false;
}

namespace
{
tservernetwork *gUDP;
}

bool InitNetworkServer(const std::string &Host, uint32_t Port)
{
  gUDP = new tservernetwork(Host, Port);
  return gUDP != nullptr;
}

tservernetwork *GetServerNetwork()
{
  return gUDP;
}

bool DeinitServerNetwork()
{
  delete gUDP;
  gUDP = nullptr;
  return true;
}
