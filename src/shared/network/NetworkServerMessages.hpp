#pragma once

/*#include "delphi.h"*/
/*#include "and.h"*/
/*#include "system.h"*/
/*#include "units.h"*/
/*#include "SysUtils.h"*/
/*#include "Classes.h"*/
/*#include "helper.h"*/
/*#include "units.h"*/
/*#include "Util.h"*/
/*#include "ScriptDispatcher.h"*/
/*#include "soldat.h"*/
/*#include "units.h"*/
/*#include "Net.h"*/
/*#include "GameNetworkingSockets.h"*/
/*#include "Sprites.h"*/
/*#include "Command.h"*/
/*#include "Constants.h"*/
#include <steam/isteamnetworkingmessages.h>
#include <string>

void serversendstringmessage(const std::string &text, std::uint8_t tonum, std::uint8_t from,
                             std::uint8_t msgtype);

void serversendstringmessage(const std::string &text, std::uint8_t tonum, std::uint8_t from,
                             std::uint8_t msgtype);
void serverhandlechatmessage(SteamNetworkingMessage_t *netmessage);
void serversendspecialmessage(std::string text, std::uint8_t msgtype, std::uint8_t layerid,
                              std::int32_t delay, float scale, uint32 color, float x, float y,
                              std::uint8_t tonum);
