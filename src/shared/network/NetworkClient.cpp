#include "NetworkClient.hpp"
#include "../../client/Client.hpp"
#include "../../client/GameRendering.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "NetworkClientBullet.hpp"
#include "NetworkClientConnection.hpp"
#include "NetworkClientFunctions.hpp"
#include "NetworkClientGame.hpp"
#include "NetworkClientHeartbeat.hpp"
#include "NetworkClientMessages.hpp"
#include "NetworkClientSprite.hpp"
#include "NetworkClientThing.hpp"
#include "common/Logging.hpp"
#include "shared/misc/GlobalSystems.hpp"

static std::string_view NETMSG = "net_msg";
auto constexpr LOG_NET = "network";

std::int32_t clienttickcount, lastheartbeatcounter;
std::int32_t clientplayerreceivedcounter;
bool clientplayerreceived, clientplayersent;
bool clientvarsrecieved;
bool requestinggame;
std::int32_t noheartbeattime = 0;
std::string votemapname;
std::uint32_t votemapcount;

namespace
{
void ProcessEventsCallback(PSteamNetConnectionStatusChangedCallback_t pInfo)
{
  GetNetwork()->ProcessEvents(pInfo);
}
} // namespace

void tclientnetwork::processloop()
{
  std::int32_t NumMsgs;
  PSteamNetworkingMessage_t IncomingMsg;
  RunCallbacks();
  if (FPeer == k_HSteamNetConnection_Invalid)
  {
    return;
  }

  while ((NumMsgs = NetworkingSockets->ReceiveMessagesOnConnection(FPeer, &IncomingMsg, 1)) > 0)
  {
    handlemessages(IncomingMsg);
  }

  if (NumMsgs < 0)
  {
    LogWarn(LOG_NET, "Failed to poll messages");
    return;
  }
}

void tclientnetwork::ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo)
{
  if (pInfo->m_hConn == k_HSteamNetConnection_Invalid)
  {
    LogWarn(LOG_NET, "Invalid connection handle");
    return;
  }
#ifdef DEVELOPMENT
  Debug("[NET] Received SteamNetConnectionStatusChangedCallback_t ",
        ToStr(pInfo ^, TypeInfo(SteamNetConnectionStatusChangedCallback_t)));
#endif
  // Make sure it"s for us
  if (pInfo->m_hConn == FPeer)
  {
    switch (pInfo->m_info.m_eState)
    {
    case k_ESteamNetworkingConnectionState_None: {
      FPeer = k_HSteamNetConnection_Invalid;
    }
    break;
    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
      if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting)
      {
        LogInfo(LOG_NET, "[NET] Connection error #1 {}", pInfo->m_info.m_szEndDebug);
      }
      else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
      {
        LogInfo(LOG_NET, "[NET] Connection error #2 {}", pInfo->m_info.m_szEndDebug);
      }
      else
      {
        LogInfo(LOG_NET, "[NET] Connection error #3 {}", pInfo->m_info.m_szEndDebug);
      }

      rendergameinfo(std::string("Network  error ") + pInfo->m_info.m_szEndDebug);
      NetworkingSockets->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
      break;
    }
    case k_ESteamNetworkingConnectionState_Connecting: {
      LogInfo(LOG_NET, "[NET] Connection request from {}", pInfo->m_info.m_szConnectionDescription);
      break;
    }
    case k_ESteamNetworkingConnectionState_Connected: {
      LogInfo(LOG_NET, "[NET] Connected to the server");
      clientrequestgame(*this);
      break;
    }
    default:
      break;
    }
  }
  else
  {
    // break;
  }
}

tclientnetwork::tclientnetwork()
{
}

bool tclientnetwork::connect(std::string Host, std::uint32_t Port)
{
  SteamNetworkingIPAddr ServerAddress;
  SteamNetworkingConfigValue_t InitSettings[2];

  LogInfo(LOG_NET, "Connecting to {}:{}", Host, Port);

  auto Result = true;

  ServerAddress.Clear();
  ServerAddress.ParseString(pchar(Host + ":" + inttostr(Port)));

  NetworkingSockets->InitAuthentication();

  InitSettings[0].m_eValue = k_ESteamNetworkingConfig_IP_AllowWithoutAuth;
  InitSettings[0].m_eDataType = k_ESteamNetworkingConfig_Int32;
  InitSettings[0].m_val.m_int32 = 1;
  InitSettings[1].SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
                         (void *)&ProcessEventsCallback);

  {
    std::lock_guard m(TNetwork::sNetworksMutex);
    FPeer = NetworkingSockets->ConnectByIPAddress(ServerAddress, 1, InitSettings);
  }

  if (FPeer == k_HSteamNetConnection_Invalid)
  {
    GS::GetMainConsole().console("[NET] Failed to connect to  server" +
                               GetNetwork()->GetStringAddress(&ServerAddress, true),
                             warning_message_color);
    Result = false;
    return Result;
  }

  FAddress = ServerAddress;
  return Result;
}

void tclientnetwork::handlemessages(PSteamNetworkingMessage_t IncomingMsg)
{
  pmsgheader PacketHeader;

  if (IncomingMsg->m_cbSize < sizeof(tmsgheader))
  {
    IncomingMsg->Release();
    return; // truncated packet
  }

  PacketHeader = pmsgheader(IncomingMsg->m_pData);

  if (GS::GetDemoRecorder().active())
  {
    GS::GetDemoRecorder().saverecord(IncomingMsg->m_pData, IncomingMsg->m_cbSize);
  }

  switch (PacketHeader->id)
  {
  case msgid_playerslist:
    clienthandleplayerslist(IncomingMsg);
    break;

  case msgid_unaccepted:
    clienthandleunaccepted(IncomingMsg);
    break;

  case msgid_newplayer:
    clienthandlenewplayer(IncomingMsg);
    break;

#ifdef ENABLE_FAE
  MsgID_FaeData:
    ClientHandleFaeChallenge(IncomingMsg);
#endif

    // PLAYING GAME MESSAGES

  case msgid_serverspritesnapshot:
    clienthandleserverspritesnapshot(IncomingMsg);
    break;

  case msgid_serverspritesnapshot_major:
    clienthandleserverspritesnapshot_major(IncomingMsg);
    break;

  case msgid_serverskeletonsnapshot:
    clienthandleserverskeletonsnapshot(IncomingMsg);
    break;

  case msgid_bulletsnapshot:
    clienthandlebulletsnapshot(IncomingMsg);
    break;

  case msgid_heartbeat:
    clienthandleheartbeat(IncomingMsg);
    break;

  case msgid_serverthingsnapshot:
    clienthandleserverthingsnapshot(IncomingMsg);
    break;

  case msgid_serverthingmustsnapshot:
    clienthandleserverthingmustsnapshot(IncomingMsg);
    break;

  case msgid_thingtaken:
    clienthandlethingtaken(IncomingMsg);
    break;

  case msgid_spritedeath:
    clienthandlespritedeath(IncomingMsg);
    break;

  case msgid_serverdisconnect:
    clienthandleserverdisconnect(IncomingMsg);
    break;

  case msgid_playerdisconnect:
    clienthandleplayerdisconnect(IncomingMsg);
    break;

  case msgid_delta_movement:
    clienthandledelta_movement(IncomingMsg);
    break;

  case msgid_delta_mouseaim:
    clienthandledelta_mouseaim(IncomingMsg);
    break;

  case msgid_delta_weapons:
    clienthandledelta_weapons(IncomingMsg);
    break;

  case msgid_delta_helmet:
    clienthandledelta_helmet(IncomingMsg);
    break;

  case msgid_chatmessage:
    clienthandlechatmessage(IncomingMsg);
    break;

  case msgid_ping:
    clienthandleping(IncomingMsg);
    break;

  case msgid_mapchange:
    clienthandlemapchange(IncomingMsg);
    break;

  case msgid_flaginfo:
    clienthandleflaginfo(IncomingMsg);
    break;

  case msgid_idleanimation:
    clienthandleidleanimation(IncomingMsg);
    break;

  case msgid_voteon:
    clienthandlevoteon(IncomingMsg);
    break;

  case msgid_clientspritesnapshot_dead:
    clienthandleclientspritesnapshot_dead(IncomingMsg);
    break;

  case msgid_servervars:
    clienthandleservervars(IncomingMsg);
    break;

  case msgid_serversyncmsg:
    clienthandleserversyncmsg(IncomingMsg);
    break;

  case msgid_forceposition:
    clienthandleforceposition(IncomingMsg);
    break;

  case msgid_forcevelocity:
    clienthandleforcevelocity(IncomingMsg);
    break;

  case msgid_forceweapon:
    clienthandleforceweapon(IncomingMsg);
    break;

  case msgid_specialmessage:
    clienthandlespecialmessage(IncomingMsg);
    break;

  case msgid_weaponactivemessage:
    clienthandleweaponactivemessage(IncomingMsg);
    break;

  case msgid_clientfreecam:
    clienthandleclientfreecam(IncomingMsg);
    break;

  case msgid_voteoff:
    clienthandlevoteoff();
    break;

  case msgid_votemapreply:
    clienthandlevoteresponse(IncomingMsg);
    break;

  case msgid_joinserver:
    clienthandlejoinserver(IncomingMsg);
    break;

  case msgid_playsound:
    clienthandleplaysound(IncomingMsg);
    break;

  case msgid_synccvars:
    clienthandlesynccvars(IncomingMsg);
    break;
#if 0
    case 92: {
#pragma pack(push, 1)
        struct test
        {
            tmsgheader header;
            std::uint8_t pbyte;
            bool pboolean;
            std::uint32_t plongword;
            std::uint16_t pword;
            float psingle;
            std::int32_t plongint;
            std::int16_t psmallint;
            std::int32_t pinteger;
        };
#pragma pack(pop)
        auto varmsg = reinterpret_cast<test *>(IncomingMsg->m_pData);
        Debug("varmsg %d", varmsg->plongword);

        break;
    }
#endif

#ifdef STEAM
  msgid_voicedata:
    clienthandlevoicedata(IncomingMsg);
#endif
  }

  IncomingMsg->Release();
}

bool tclientnetwork::senddata(const std::byte *Data, std::int32_t Size, std::int32_t Flags,
                              const source_location &location)
{
  auto Result = false;

  if (Size < sizeof(tmsgheader))
    return Result; // truncated packet

  LogDebug(NETMSG, "Senddata {} from {}", reinterpret_cast<const tmsgheader *>(Data)->id,
           location.function_name());

  if (FPeer == k_HSteamNetConnection_Invalid)
    return Result; // not connected

  NetworkingSockets->SendMessageToConnection(FPeer, Data, Size, Flags, nullptr);
  Result = true;
  return Result;
}

namespace
{
tclientnetwork *gUDP = nullptr;
}

template <Config::Module M>
void DeinitClientNetwork() requires(Config::IsClient())
{
  delete gUDP;
  gUDP = nullptr;
}

template <Config::Module M>
tclientnetwork *GetNetwork() requires(Config::IsClient())
{
  return gUDP;
}

template <Config::Module M>
void InitClientNetwork() requires(Config::IsClient())
{
  gUDP = new tclientnetwork();
}

template tclientnetwork *GetNetwork<Config::GetModule()>();
#ifndef SERVER
template void InitClientNetwork<Config::GetModule()>();
template void DeinitClientNetwork<Config::GetModule()>();
#endif