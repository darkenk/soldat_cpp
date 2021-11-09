#pragma once


/*#include "GameNetworkingSockets.h"*/
/*#include "FaeBase.h"*/
/*#include "FaeClient.h"*/



#ifdef __networkclientfae_implementation__
#undef EXTERN
#define EXTERN
#endif

extern pointer faependingauth;
#undef EXTERN
#define extern extern


void clientsendfaeresponse(tfaeresponsebox response);
void clienthandlefaechallenge(SteamNetworkingMessage_t* netmessage);

