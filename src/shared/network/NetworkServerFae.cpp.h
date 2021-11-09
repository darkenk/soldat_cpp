#pragma once


/*#include "GameNetworkingSockets.h"*/


void serversendfaechallenge(HSteamNetConnection peer, bool inorder);
void serverhandlefaeresponse(SteamNetworkingMessage_t* netmessage);

