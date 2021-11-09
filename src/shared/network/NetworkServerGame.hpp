#pragma once

/*#include "delphi.h"*/
/*#include "and.h"*/
/*#include "system.h"*/
/*#include "units.h"*/
/*#include "SysUtils.h"*/
/*#include "Classes.h"*/
/*#include "Script.h"*/
/*#include "ScriptDispatcher.h"*/
/*#include "soldat.h"*/
/*#include "units.h"*/
/*#include "GameNetworkingSockets.h"*/
/*#include "Net.h"*/
/*#include "Sprites.h"*/
/*#include "Constants.h"*/
#include <steam/isteamnetworkingmessages.h>
#include <string>

void serverhandleplayerdisconnect(SteamNetworkingMessage_t *netmessage);
void servermapchange(std::uint8_t id);
void serverflaginfo(std::uint8_t style, std::uint8_t who);
void serveridleanimation(std::uint8_t num, std::int16_t style);
void serversendvoteon(std::uint8_t votestyle, std::int32_t voter, std::string targetname,
                      std::string reason);
void serversendvoteoff();
void serverhandlevotekick(SteamNetworkingMessage_t *netmessage);
void serverhandlevotemap(SteamNetworkingMessage_t *netmessage);
void serverhandlechangeteam(SteamNetworkingMessage_t *netmessage);
void serversyncmsg(std::int32_t tonum = 0);
#ifdef STEAM
void serverhandlevoicedata(SteamNetworkingMessage_t *netmessage);
#endif
