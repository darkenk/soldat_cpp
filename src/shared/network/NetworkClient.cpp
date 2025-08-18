#include "NetworkClient.hpp"

#include <steam/isteamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <steam/steamclientpublic.h>
#include <steam/steamnetworkingtypes.h>
#include <array>
#include <utility>

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
#include "common/misc/PortUtils.hpp"

static std::string_view NETMSG = "net_msg";
auto constexpr LOG_NET = "network";

GlobalStateNetworkClient gGlobalStateNetworkClient{

};

void NetworkClientImpl::ProcessLoop()
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
    IncomingMsg->Release();
  }

  if (numMsgs < 0)
  {
    LogWarn(LOG_NET, "Failed to poll messages");
    return;
  }
}

void NetworkClientImpl::ProcessEvents(PSteamNetConnectionStatusChangedCallback_t pInfo)
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

NetworkClientImpl::NetworkClientImpl(): mPeer(k_HSteamNetConnection_Invalid) {}

auto NetworkClientImpl::Connect(const std::string_view host, std::uint32_t port) -> bool
{
  SteamNetworkingIPAddr address; // NOLINT
  std::array<SteamNetworkingConfigValue_t, 2> initSettings; // NOLINT

  LogInfo(LOG_NET, "Connecting to {}:{}", host, port);

  address.Clear();
  address.ParseString((std::string(host.data()) + ":" + std::to_string(port)).c_str());

  mNetworkingSockets->InitAuthentication();

  initSettings[0].SetInt32(k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
  auto _this = dynamic_cast<TNetwork*>(this); // required for NetworksGlobalCallback, FIXME
  initSettings[1].SetInt64(k_ESteamNetworkingConfig_ConnectionUserData, reinterpret_cast<std::int64_t>(_this));

  mPeer = mNetworkingSockets->ConnectByIPAddress(address, initSettings.size(), initSettings.data());
  mIpAddress = host;
  mPort = port;
  return mPeer != k_HSteamNetConnection_Invalid;
}

auto NetworkClientImpl::IsConnected() -> bool
{
  if (mPeer == k_HSteamNetConnection_Invalid)
  {
    return false;
  }
  SteamNetworkingQuickConnectionStatus status; // NOLINT
  mNetworkingSockets->GetQuickConnectionStatus(mPeer, &status);
  return status.m_eState == k_ESteamNetworkingConnectionState_Connected;
}

auto NetworkClientImpl::IsDisconnected() -> bool
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

void NetworkClientImpl::FlushMsg()
{
  if (mPeer == k_HSteamNetConnection_Invalid)
  {
    return;
  }
  mNetworkingSockets->FlushMessagesOnConnection(mPeer);
}

void NetworkClientImpl::HandleMessages(PSteamNetworkingMessage_t IncomingMsg)
{
  if (IncomingMsg->m_cbSize < sizeof(tmsgheader))
  {
    return; // truncated packet
  }

  auto *const PacketHeader = static_cast<pmsgheader>(IncomingMsg->m_pData);

  if (GS::GetDemoRecorder().active())
  {
    GS::GetDemoRecorder().saverecord(IncomingMsg->m_pData, IncomingMsg->m_cbSize);
  }

  NetworkContext nc{nullptr, PacketHeader, IncomingMsg->m_cbSize, *this};

  switch (PacketHeader->id)
  {
  case msgid_playerslist:
    clienthandleplayerslist(&nc);
    break;

  case msgid_unaccepted:
    clienthandleunaccepted(&nc);
    break;

  case msgid_newplayer:
    clienthandlenewplayer(&nc);
    break;

#ifdef ENABLE_FAE
  MsgID_FaeData:
    ClientHandleFaeChallenge(IncomingMsg);
#endif

    // PLAYING GAME MESSAGES

  case msgid_serverspritesnapshot:
    clienthandleserverspritesnapshot(&nc);
    break;

  case msgid_serverspritesnapshot_major:
    clienthandleserverspritesnapshot_major(&nc);
    break;

  case msgid_serverskeletonsnapshot:
    clienthandleserverskeletonsnapshot(&nc);
    break;

  case msgid_bulletsnapshot:
    clienthandlebulletsnapshot(&nc);
    break;

  case msgid_heartbeat:
    clienthandleheartbeat(&nc);
    break;

  case msgid_serverthingsnapshot:
    clienthandleserverthingsnapshot(&nc);
    break;

  case msgid_serverthingmustsnapshot:
    clienthandleserverthingmustsnapshot(&nc);
    break;

  case msgid_thingtaken:
    clienthandlethingtaken(&nc);
    break;

  case msgid_spritedeath:
    clienthandlespritedeath(&nc);
    break;

  case msgid_serverdisconnect:
    clienthandleserverdisconnect(&nc);
    break;

  case msgid_playerdisconnect:
    clienthandleplayerdisconnect(&nc);
    break;

  case msgid_delta_movement:
    clienthandledelta_movement(&nc);
    break;

  case msgid_delta_mouseaim:
    clienthandledelta_mouseaim(&nc);
    break;

  case msgid_delta_weapons:
    clienthandledelta_weapons(&nc);
    break;

  case msgid_delta_helmet:
    clienthandledelta_helmet(&nc);
    break;

  case msgid_chatmessage:
    clienthandlechatmessage(&nc);
    break;

  case msgid_ping:
    clienthandleping(&nc);
    break;

  case msgid_mapchange:
    clienthandlemapchange(&nc);
    break;

  case msgid_flaginfo:
    clienthandleflaginfo(&nc);
    break;

  case msgid_idleanimation:
    clienthandleidleanimation(&nc);
    break;

  case msgid_voteon:
    clienthandlevoteon(&nc);
    break;

  case msgid_clientspritesnapshot_dead:
    clienthandleclientspritesnapshot_dead(&nc);
    break;

  case msgid_servervars:
    clienthandleservervars(&nc);
    break;

  case msgid_serversyncmsg:
    clienthandleserversyncmsg(&nc);
    break;

  case msgid_forceposition:
    clienthandleforceposition(&nc);
    break;

  case msgid_forcevelocity:
    clienthandleforcevelocity(&nc);
    break;

  case msgid_forceweapon:
    clienthandleforceweapon(&nc);
    break;

  case msgid_specialmessage:
    clienthandlespecialmessage(&nc);
    break;

  case msgid_weaponactivemessage:
    clienthandleweaponactivemessage(&nc);
    break;

  case msgid_clientfreecam:
    clienthandleclientfreecam(&nc);
    break;

  case msgid_voteoff:
    clienthandlevoteoff();
    break;

  case msgid_votemapreply:
    clienthandlevoteresponse(&nc);
    break;

  case msgid_joinserver:
    clienthandlejoinserver(&nc);
    break;

  case msgid_playsound:
    clienthandleplaysound(&nc);
    break;

  case msgid_synccvars:
    clienthandlesynccvars(&nc);
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

  }
}

auto NetworkClientImpl::SendDataImpl(const std::byte *data, std::int32_t size, bool reliable,
                                     const source_location &location) -> bool
{
  if (size < sizeof(tmsgheader))
  {
    return false; // truncated packet
  }

  LogDebug(NETMSG, "Senddata {} from {}", reinterpret_cast<const tmsgheader *>(data)->id,
           location.function_name());

  if (mPeer == k_HSteamNetConnection_Invalid)
  {
    return false; // not connected
  }

  auto flags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
  auto ret = mNetworkingSockets->SendMessageToConnection(mPeer, data, size, flags, nullptr);
  SoldatAssert(ret == EResult::k_EResultOK);
  if (ret != EResult::k_EResultOK)
  {
    LogWarn(LOG_NET, "Cannot send message: {}", ret);
    return false;
  }
  return true;
}

auto NetworkClientImpl::Disconnect(bool now) -> bool
{
  mNetworkingSockets->CloseConnection(mPeer, 0, "", !now);
  mPeer = k_HSteamNetConnection_Invalid;
  return true;
}

template <Config::Module M>
void GlobalStateNetworkClient::DeinitClientNetwork()
  requires(Config::IsClient(M))
{
  mUdp.release();
}

template <Config::Module M>
auto GlobalStateNetworkClient::GetNetwork()
  -> NetworkClientImpl *requires(Config::IsClient(M)) { return mUdp.get(); }

template <Config::Module M>
void GlobalStateNetworkClient::InitClientNetwork()
  requires(Config::IsClient(M))
{
  mUdp = std::make_unique<NetworkClientImpl>();
}

template NetworkClientImpl *GlobalStateNetworkClient::GetNetwork<Config::GetModule()>();
#ifndef SERVER
template void GlobalStateNetworkClient::InitClientNetwork<Config::GetModule()>();
template void GlobalStateNetworkClient::DeinitClientNetwork<Config::GetModule()>();
#endif