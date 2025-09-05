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

constexpr std::string_view NETMSG = "net_msg";
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

void NetworkClientImpl::RegisterMsgHandler(msgid id, msg_handler handler)
{
  mMessageHandlers[id] = handler;
}

void NetworkClientImpl::RegisterMsgHandler(msgid id, std::unique_ptr<INetMessageHandler> handler)
{
  mMessageHandlers_[id] = std::move(handler);
}

NetworkClientImpl::NetworkClientImpl(): mPeer(k_HSteamNetConnection_Invalid)
{
  RegisterMsgHandler(msgid_bulletsnapshot, std::make_unique<clienthandlebulletsnapshot>());
  RegisterMsgHandler(msgid_chatmessage, std::make_unique<clienthandlechatmessage>());
  RegisterMsgHandler(msgid_clientfreecam, std::make_unique<clienthandleclientfreecam>());
  RegisterMsgHandler(msgid_clientspritesnapshot_dead,
                     std::make_unique<clienthandleclientspritesnapshot_dead>());
  RegisterMsgHandler(msgid_delta_helmet, std::make_unique<clienthandledelta_helmet>());
  RegisterMsgHandler(msgid_delta_mouseaim, std::make_unique<clienthandledelta_mouseaim>());
  RegisterMsgHandler(msgid_delta_movement, std::make_unique<clienthandledelta_movement>());
  RegisterMsgHandler(msgid_delta_weapons, std::make_unique<clienthandledelta_weapons>());
  RegisterMsgHandler(msgid_flaginfo, std::make_unique<clienthandleflaginfo>());
  RegisterMsgHandler(msgid_forceposition, std::make_unique<clienthandleforceposition>());
  RegisterMsgHandler(msgid_forcevelocity, std::make_unique<clienthandleforcevelocity>());
  RegisterMsgHandler(msgid_forceweapon, std::make_unique<clienthandleforceweapon>());
  RegisterMsgHandler(msgid_heartbeat, std::make_unique<ClientHandleHeartbeat>());
  RegisterMsgHandler(msgid_idleanimation, std::make_unique<clienthandleidleanimation>());
  RegisterMsgHandler(msgid_joinserver, std::make_unique<clienthandlejoinserver>());
  RegisterMsgHandler(msgid_mapchange, std::make_unique<clienthandlemapchange>());
  RegisterMsgHandler(msgid_newplayer, std::make_unique<clienthandlenewplayer>());
  RegisterMsgHandler(msgid_ping, std::make_unique<clienthandleping>());
  RegisterMsgHandler(msgid_playerdisconnect, std::make_unique<clienthandleplayerdisconnect>());
  RegisterMsgHandler(msgid_playerslist, std::make_unique<clienthandleplayerslist>());
  RegisterMsgHandler(msgid_playsound, std::make_unique<clienthandleplaysound>());
  RegisterMsgHandler(msgid_serverdisconnect, std::make_unique<clienthandleserverdisconnect>());
  RegisterMsgHandler(msgid_serverskeletonsnapshot,
                     std::make_unique<clienthandleserverskeletonsnapshot>());
  RegisterMsgHandler(msgid_serverspritesnapshot,
                     std::make_unique<clienthandleserverspritesnapshot>());
  RegisterMsgHandler(msgid_serverspritesnapshot_major,
                     std::make_unique<clienthandleserverspritesnapshot_major>());
  RegisterMsgHandler(msgid_serversyncmsg, std::make_unique<clienthandleserversyncmsg>());
  RegisterMsgHandler(msgid_serverthingmustsnapshot,
                     std::make_unique<clienthandleserverthingmustsnapshot>());
  RegisterMsgHandler(msgid_serverthingsnapshot,
                     std::make_unique<clienthandleserverthingsnapshot>());
  RegisterMsgHandler(msgid_servervars, std::make_unique<clienthandleservervars>());
  RegisterMsgHandler(msgid_specialmessage, std::make_unique<clienthandlespecialmessage>());
  RegisterMsgHandler(msgid_spritedeath, std::make_unique<clienthandlespritedeath>());
  RegisterMsgHandler(msgid_synccvars, std::make_unique<clienthandlesynccvars>());
  RegisterMsgHandler(msgid_thingtaken, std::make_unique<clienthandlethingtaken>());
  RegisterMsgHandler(msgid_unaccepted, std::make_unique<clienthandleunaccepted>());
  RegisterMsgHandler(msgid_votemapreply, std::make_unique<clienthandlevoteresponse>());
  RegisterMsgHandler(msgid_voteoff, std::make_unique<clienthandlevoteoff>());
  RegisterMsgHandler(msgid_voteon, std::make_unique<clienthandlevoteon>());
  RegisterMsgHandler(msgid_weaponactivemessage,
                     std::make_unique<clienthandleweaponactivemessage>());
}

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

  if (auto handler = mMessageHandlers.find(PacketHeader->id); handler != mMessageHandlers.end())
  {
    handler->second(&nc);
  }
  else if (auto handler = mMessageHandlers_.find(PacketHeader->id); handler != mMessageHandlers_.end())
  {
    handler->second->Handle(&nc);
  }
  else
  {
    LogWarn(LOG_NET, "Unknown message {:x}", PacketHeader->id);
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