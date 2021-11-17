#include "NetworkServer.hpp"
#include "../../server/Server.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../Logging.hpp"
#include "NetworkServerBullet.hpp"
#include "NetworkServerConnection.hpp"
#include "NetworkServerFunctions.hpp"
#include "NetworkServerGame.hpp"
#include "NetworkServerMessages.hpp"
#include "NetworkServerSprite.hpp"
#include "NetworkServerThing.hpp"

// We"re assigning a dummy player class to all sprites that are currently not being controlled
// by a player. This avoids nasty surprises with older code that reads .Player despite .Active
// being false. A player object is swapped in by CreateSprite as needed. For bots we simply leave
// the bot object and free it when it is replaced.
// Albeit this approach is very robust I"d prefer if we get rid of this and fix all .Active
// checks (if any) later. Alternatively we could move a good bit if info from Player to Sprite.
tplayer dummyplayer;

std::int32_t servertickcounter;
PascalArray<std::int32_t, 1, max_players> noclientupdatetime;
PascalArray<std::int32_t, 1, max_players> messagesasecnum;
PascalArray<std::uint8_t, 1, max_players> floodwarnings;
PascalArray<std::uint8_t, 1, max_players> pingwarnings;
PascalArray<std::int32_t, 1, max_players> bullettime;
PascalArray<std::int32_t, 1, max_players> grenadetime;
PascalArray<bool, 1, max_players> knifecan;

std::int32_t pingticksadd = 0;

auto LOG_NET = "network";

void ProcessEventsCallback(PSteamNetConnectionStatusChangedCallback_t pInfo)
{
    GetNetwork()->ProcessEvents(pInfo);
}

tservernetwork::tservernetwork(std::string Host, std::uint32_t Port)
{
    SteamNetworkingIPAddr ServerAddress;
    SteamNetworkingConfigValue_t InitSettings;
    // std::array<Char, 128> TempIP;
    if (FInit)
    {
        ServerAddress.Clear();
        ServerAddress.ParseString(pchar(Host + ":" + inttostr(Port)));
        InitSettings.m_eValue = k_ESteamNetworkingConfig_IP_AllowWithoutAuth;
        InitSettings.m_eDataType = k_ESteamNetworkingConfig_Int32;
        InitSettings.m_val.m_int32 = 1;
        //#ifdef STEAM
        // if sv_steamonly.Value then
        //  InitSettings.m_int32 = 0
        // else
        //#endif
        FHost = NetworkingSockets->CreateListenSocketIP(ServerAddress, 1, &InitSettings);

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
#ifndef STEAM
    NetworkingSockets->RunCallbacks();
#endif

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
    tplayer *Player = nullptr;
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
        Player = reinterpret_cast<tplayer *>(pInfo->m_info.m_nUserData);

        if (Player == nullptr)
        {
            NetworkingSockets->CloseConnection(pInfo->m_hConn, 0, "", false);
            return;
        }

        // the sprite may be zero if we"re still in the setup phase
        if (Player->spritenum != 0)
        {
            GetServerMainConsole().console(Player->name + " could not respond",
                                           warning_message_color);
            serverplayerdisconnect(Player->spritenum, kick_noresponse);
#ifdef SCRIPT
            ScrptDispatcher.OnLeaveGame(Player->spritenum, false);
#endif
            sprite[Player->spritenum].kill();
            sprite[Player->spritenum].player = &dummyplayer;
        }

        LogWarn(LOG_NET, "Connection lost {} {}", pInfo->m_info.m_eEndReason,
                pInfo->m_info.m_szConnectionDescription);

        // call destructor; this releases any additional resources managed for the connection, such
        // as anti-cheat handles etc.
        players.erase(std::remove(players.begin(), players.end(), Player), players.end());

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
            Player = new tplayer();
            Player->peer = pInfo->m_hConn;
            pInfo->m_info.m_addrRemote.ToString(TempIP.data(), 128, false);
            Player->ip = TempIP.data();
            Player->port = pInfo->m_info.m_addrRemote.m_port;
#ifdef STEAM
            Player->SteamID = TSteamID(pInfo->m_info.m_identityRemote.GetSteamID64);
#endif
            NotImplemented(NITag::NETWORK, "Pointer cast is probably wrong");
            NetworkingSockets->SetConnectionUserData(pInfo->m_hConn, (std::uint64_t)Player);
            LogInfo(LOG_NET, "Connection  accepted {}", pInfo->m_info.m_szConnectionDescription);
            players.push_back(Player);
        }
    }
    }
}

void tservernetwork::HandleMessages(PSteamNetworkingMessage_t IncomingMsg)
{
    tplayer *Player;
    pmsgheader PacketHeader;
    if (IncomingMsg->m_cbSize < sizeof(tmsgheader))
    {
        return; // truncated packet
    }

    if (IncomingMsg->m_nConnUserData == -1)
    {
        return;
    }

    Player = reinterpret_cast<tplayer *>(IncomingMsg->m_nConnUserData);
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
    if ((Player->spritenum == 0) or (sprite[Player->spritenum].player != Player))
    {
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

    IncomingMsg->m_pfnRelease(IncomingMsg);
}

tservernetwork::~tservernetwork()
{
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
    auto Result = false;

    if (Size < sizeof(tmsgheader))
        return Result;

    if (FHost == k_HSteamNetConnection_Invalid)
        return Result;

    if (demorecorder.active())
    {
        NotImplemented(NITag::NETWORK, "check peer comparision");
        if (Peer == std::numeric_limits<std::uint32_t>::max())
            demorecorder.saverecord(Data, Size);
    }

    NetworkingSockets->SendMessageToConnection(Peer, Data, Size, Flags, nullptr);

    Result = true;
    return Result;
}

void tservernetwork::UpdateNetworkStats(std::uint8_t Player)
{
    SteamNetworkingQuickConnectionStatus Stats;
    Stats = GetQuickConnectionStatus(sprite[Player].player->peer);
    sprite[Player].player->realping = Stats.m_nPing;
    if (Stats.m_flConnectionQualityLocal > 0.0)
    {
        sprite[Player].player->connectionquality = Stats.m_flConnectionQualityLocal * 100;
    }
    else
    {
        sprite[Player].player->connectionquality = 0;
    }
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

tservernetwork *GetNetwork()
{
    return gUDP;
}

bool DeinitServerNetwork()
{
    delete gUDP;
    gUDP = nullptr;
    return true;
}
