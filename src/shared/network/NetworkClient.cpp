#include "NetworkClient.hpp"
#include "../Demo.hpp"
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
#include <steam/isteamnetworkingutils.h>
#include <steam/isteamnetworkingsockets.h>

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

void NetworkClient::ProcessLoop()
{
  std::int32_t numMsgs;
  PSteamNetworkingMessage_t IncomingMsg;
  RunCallbacks();
  if (mPeer == k_HSteamNetConnection_Invalid)
  {
    return;
  }

  while ((numMsgs = mNetworkingSockets->ReceiveMessagesOnConnection(mPeer, &IncomingMsg, 1)) > 0)
  {
    HandleMessages(IncomingMsg);
  }

  if (numMsgs < 0)
  {
    LogWarn(LOG_NET, "Failed to poll messages");
    return;
  }
}

void NetworkClient::ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo)
{
  if (pInfo->m_hConn == k_HSteamNetConnection_Invalid)
  {
    LogWarn(LOG_NET, "Invalid connection handle");
    return;
  }
  LogDebug(LOG_NET, "Client: Process Events {}", pInfo->m_info.m_eState);

  // Make sure it"s for us
  if (pInfo->m_hConn != mPeer)
  {
    return;
  }
  switch (pInfo->m_info.m_eState)
  {
  case k_ESteamNetworkingConnectionState_None: {
    mPeer = k_HSteamNetConnection_Invalid;
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
    mNetworkingSockets->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
    if (mDisconnectionCallback)
    {
      mDisconnectionCallback(pInfo->m_info.m_szEndDebug);
    }
    break;
  }
  case k_ESteamNetworkingConnectionState_Connecting: {
    LogInfo(LOG_NET, "[NET] Connection request from {}", pInfo->m_info.m_szConnectionDescription);
    break;
  }
  case k_ESteamNetworkingConnectionState_Connected: {
    LogInfo(LOG_NET, "[NET] Connected to the server");
    if (mConnectionCallback)
    {
      mConnectionCallback(*this);
    }
    break;
  }
  default:
    break;
  }
}

NetworkClient::NetworkClient(): mPeer(k_HSteamNetConnection_Invalid) {}

bool NetworkClient::Connect(const std::string_view host, std::uint32_t port)
{
  SteamNetworkingIPAddr address; // NOLINT
  std::array<SteamNetworkingConfigValue_t, 2> initSettings; // NOLINT

  LogInfo(LOG_NET, "Connecting to {}:{}", host, port);

  address.Clear();
  address.ParseString((std::string(host.data()) + ":" + std::to_string(port)).c_str());

  mNetworkingSockets->InitAuthentication();

  initSettings[0].SetInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
  initSettings[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData, reinterpret_cast<std::int64_t>(this));

  mPeer = mNetworkingSockets->ConnectByIPAddress(address, initSettings.size(), initSettings.data());
  mIpAddress = host;
  mPort = port;
  return mPeer != k_HSteamNetConnection_Invalid;
}

bool NetworkClient::IsConnected()
{
  if (mPeer == k_HSteamNetConnection_Invalid)
  {
    return false;
  }
  SteamNetworkingQuickConnectionStatus status; // NOLINT
  mNetworkingSockets->GetQuickConnectionStatus(mPeer, &status);
  return status.m_eState == k_ESteamNetworkingConnectionState_Connected;
}

bool NetworkClient::IsDisconnected()
{
  if (mPeer == k_HSteamNetConnection_Invalid)
  {
    return true;
  }
  SteamNetworkingQuickConnectionStatus status; // NOLINT
  mNetworkingSockets->GetQuickConnectionStatus(mPeer, &status);
  return status.m_eState != k_ESteamNetworkingConnectionState_Connected &&
         status.m_eState != k_ESteamNetworkingConnectionState_Connecting;
}

void NetworkClient::FlushMsg()
{
  if (mPeer == k_HSteamNetConnection_Invalid)
  {
    return;
  }
  mNetworkingSockets->FlushMessagesOnConnection(mPeer);
}
void NetworkClient::HandleMessages(PSteamNetworkingMessage_t IncomingMsg)
{
  if (IncomingMsg->m_cbSize < sizeof(tmsgheader))
  {
    IncomingMsg->Release();
    return; // truncated packet
  }

  auto PacketHeader = pmsgheader(IncomingMsg->m_pData);

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

bool NetworkClient::SendData(const std::byte *Data, std::int32_t Size, std::int32_t Flags,
                              const source_location &location)
{
  if (Size < sizeof(tmsgheader))
    return false; // truncated packet

  LogDebug(NETMSG, "Senddata {} from {}", reinterpret_cast<const tmsgheader *>(Data)->id,
           location.function_name());

  if (mPeer == k_HSteamNetConnection_Invalid)
    return false; // not connected

  auto ret = mNetworkingSockets->SendMessageToConnection(mPeer, Data, Size, Flags, nullptr);
  SoldatAssert(ret == EResult::k_EResultOK);
  if (ret != EResult::k_EResultOK)
  {
    LogWarn(LOG_NET, "Cannot send message: {}", ret);
    return false;
  }
  return true;
}

bool NetworkClient::Disconnect(bool now)
{
  mNetworkingSockets->CloseConnection(mPeer, 0, "", !now);
  mPeer = k_HSteamNetConnection_Invalid;
  return true;
}

namespace
{
NetworkClient *gUDP = nullptr;
}

template <Config::Module M>
void DeinitClientNetwork() requires(Config::IsClient())
{
  delete gUDP;
  gUDP = nullptr;
}

template <Config::Module M>
NetworkClient *GetNetwork() requires(Config::IsClient())
{
  return gUDP;
}

template <Config::Module M>
void InitClientNetwork() requires(Config::IsClient())
{
  gUDP = new NetworkClient();
}

template NetworkClient *GetNetwork<Config::GetModule()>();
#ifndef SERVER
template void InitClientNetwork<Config::GetModule()>();
template void DeinitClientNetwork<Config::GetModule()>();
#endif