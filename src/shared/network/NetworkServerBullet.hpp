#pragma once

/*#include "delphi.h"*/
/*#include "and.h"*/
/*#include "system.h"*/
/*#include "units.h"*/
/*#include "SysUtils.h"*/
/*#include "Classes.h"*/
/*#include "helper.h"*/
/*#include "units.h"*/
/*#include "Vector.h"*/
/*#include "Math.h"*/
/*#include "soldat.h"*/
/*#include "units.h"*/
/*#include "PolyMap.h"*/
/*#include "Calc.h"*/
/*#include "GameNetworkingSockets.h"*/
/*#include "Net.h"*/
/*#include "Sprites.h"*/
/*#include "Weapons.h"*/
/*#include "Constants.h"*/
/*#include "Demo.h"*/

#include <cstdint>
#include <steam/isteamnetworkingmessages.h>

void serverbulletsnapshot(std::uint8_t i, std::uint8_t tonum, bool forced);
#ifdef SERVER
void serverhandlebulletsnapshot(SteamNetworkingMessage_t *netmessage);
#endif
