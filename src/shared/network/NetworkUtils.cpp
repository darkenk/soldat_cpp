#include "NetworkUtils.hpp"
#include "../Console.hpp"
#include "../Cvar.hpp"
#include "../Game.hpp"
#include "../Logging.hpp"
#include "../Version.hpp"
#include "../Weapons.hpp"
#include "../mechanics/Sprites.hpp"
#include "../misc/PortUtilsSoldat.hpp"
#include <algorithm>
#include <cstring>
#include <string>

#ifdef SERVER
#include "../../server/BanSystem.hpp"
#include "../../server/Server.hpp"
#else
#include "../../client/Client.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/Sound.hpp"
#include "shared/Parts.hpp"
#endif

void encodekeys(tsprite &SpriteC, uint16_t &Keys16);
void decodekeys(tsprite &SpriteC, std::uint32_t Keys16);

std::string ArrayToString(const char *c);

#ifndef SERVER
void playradiosound(std::uint8_t RadioID)
{
    if ((radiocooldown > 0) or (CVar::sv_radio))
        return;
    if ((CVar::sv_gamemode != Constants::GAMESTYLE_CTF) and
        (CVar::sv_gamemode != Constants::GAMESTYLE_HTF) and
        (CVar::sv_gamemode != Constants::GAMESTYLE_INF))
        return;

    radiocooldown = 3;
    switch (RadioID)
    {
    case 11:
        playsound(Constants::SFX_RADIO_EFCUP, spriteparts.pos[mysprite]);
        break;
    case 12:
        playsound(Constants::SFX_RADIO_EFCMID, spriteparts.pos[mysprite]);
        break;
    case 13:
        playsound(Constants::SFX_RADIO_EFCDOWN, spriteparts.pos[mysprite]);
        break;
    case 21:
        playsound(Constants::SFX_RADIO_FFCUP, spriteparts.pos[mysprite]);
        break;
    case 22:
        playsound(Constants::SFX_RADIO_FFCMID, spriteparts.pos[mysprite]);
        break;
    case 23:
        playsound(Constants::SFX_RADIO_FFCDOWN, spriteparts.pos[mysprite]);
        break;
    case 31:
        playsound(Constants::SFX_RADIO_ESUP, spriteparts.pos[mysprite]);
        break;
    case 32:
        playsound(Constants::SFX_RADIO_ESMID, spriteparts.pos[mysprite]);
        break;
    case 33:
        playsound(Constants::SFX_RADIO_ESDOWN, spriteparts.pos[mysprite]);
        break;
    }
}
#endif

void encodekeys(tsprite &SpriteC, std::uint16_t &Keys16)
{
    auto &Controls = SpriteC.control;

    Keys16 = 0;
    if (Controls.left)
        Keys16 = Keys16 | B1;
    if (Controls.right)
        Keys16 = Keys16 | B2;
    if (Controls.up)
        Keys16 = Keys16 | B3;
    if (Controls.down)
        Keys16 = Keys16 | B4;
    if (Controls.fire)
        Keys16 = Keys16 | B5;
    if (Controls.jetpack)
        Keys16 = Keys16 | B6;
    if (Controls.thrownade)
        Keys16 = Keys16 | B7;
    if (Controls.changeweapon)
        Keys16 = Keys16 | B8;
    if (Controls.throwweapon)
        Keys16 = Keys16 | B9;
    if (Controls.reload)
        Keys16 = Keys16 | B10;
    if (Controls.flagthrow)
        Keys16 = Keys16 | B11;

    if (SpriteC.bodyanimation.id == change.id)
        Keys16 = Keys16 | B8;
    if (SpriteC.bodyanimation.id == throwweapon.id)
        Keys16 = Keys16 | B9;
}

void decodekeys(tsprite &SpriteC, uint16_t Keys16)
{
    auto &Controls = SpriteC.control;

    Controls.left = (Keys16 & B1) == B1;
    Controls.right = (Keys16 & B2) == B2;
    Controls.up = (Keys16 & B3) == B3;
    Controls.down = (Keys16 & B4) == B4;
    Controls.fire = (Keys16 & B5) == B5;
    Controls.jetpack = (Keys16 & B6) == B6;
    Controls.thrownade = (Keys16 & B7) == B7;
    Controls.changeweapon = (Keys16 & B8) == B8;
    Controls.throwweapon = (Keys16 & B9) == B9;
    Controls.reload = (Keys16 & B10) == B10;
    Controls.flagthrow = (Keys16 & B11) == B11;
}

// Sets the player name to Major if it is invalid
std::string fixplayername(const char *Name)
{
    NotImplemented(NITag::OTHER);
    return Name;
#if 0
    if ((trim(Name) == "")
        or (*Name == 0xC) || (uppercase(Name) = "SERVER MESSAGE") or ansicontainsstr(Name, 0xA) or
            AnsiContainsStr(Name, 0x13) then Result = "Major" else Result = Name;
#endif
}

bool verifypacket(std::int32_t ValidSize, std::int32_t ReceiveSize, std::int32_t PacketId,
                  const source_location &location)
{
    std::string Dropped = "";
    auto Result = true;
    Assert(ValidSize == ReceiveSize);
    LogDebug("net_msg", "{}", location.function_name());
    if (ValidSize != ReceiveSize)
    {
        Dropped = " - DROPPED (wrong size != " + inttostr(ValidSize) + ")";
        Result = false;
    }
    if (CVar::log_level > 1)
    {
        GetMainConsole().console("[NET] Received Packet (" + inttostr(PacketId) +
                                ") Size:" + inttostr(ReceiveSize) + Dropped,
                            debug_message_color);
    }
    return Result;
}

bool verifypacketlargerorequal(std::int32_t ValidSize, std::int32_t ReceiveSize,
                               std::int32_t PacketId, const source_location &location)
{
    std::string Dropped = "";
    auto Result = true;
    Assert(ValidSize <= ReceiveSize);
    LogDebug("net_msg", "{}", location.function_name());
    if (ValidSize > ReceiveSize)
    {
        Dropped = " - DROPPED (wrong size, expected at least " + inttostr(ValidSize) + ")";
        Result = false;
    }
    if (CVar::log_level > 1)
    {
        GetMainConsole().console("[NET] Received Packet (" + inttostr(PacketId) +
                                ") Size:" + inttostr(ReceiveSize) + Dropped,
                            debug_message_color);
    }
    return Result;
}

#ifdef SERVER
// Checks if the IP std::string is inside the remote IPs list
bool isremoteadminip(const std::string &ip)
{
    return std::find(remoteips.begin(), remoteips.end(), ip) != remoteips.end();
}

// Checks if the IP std::string is inside the admin IPs list
bool isadminip(const std::string &ip)
{
    return std::find(adminips.begin(), adminips.end(), ip) != adminips.end();
}

// Retruns true if the password is not empty and equal to the Admin password
// Server passwords are not allowed to be empty else everyone could login
bool isadminpassword(std::string Password)
{
    return (CVar::sv_adminpassword != "") and (Password == CVar::sv_adminpassword);
}

// Checks if the given passwords match
// If the password is not set then this returns false
bool iswronggamepassword(std::string GamePassword)
{
    return (CVar::sv_password != "") and (GamePassword != CVar::sv_password);
}

// Checks if server has MAX_PLAYERS slots taken and adding even and admin
// wouldn"t work
bool isservertotallyfull()
{
    return playersnum >= max_players;
}

// Checks if allowed server slots are taken
// If MaxPlayers slots is lower than MAX_PLAYERS there are still slots for
// admins to join
bool isserverfull()
{
    return ((playersnum - botsnum) >= CVar::sv_maxplayers) or (isservertotallyfull());
}
#endif

// Checks if the Requested and the current Soldat version are the same
bool iswronggameversion(std::string RequestVersion)
{
    NotImplemented(NITag::OTHER);
    return false;
#if 0
    return RequestVersion != soldat_version;
#endif
}

#ifndef SERVER
std::string returnfixedplayername(std::string name)
{
    std::int32_t i;
    std::string r;
    r = "";
    NotImplemented(NITag::OTHER);
    return name;
#if 0
  for
      i = 1 to Length(name) do r = r + name[i];

  if Length (r)
      > 24 then delete (r, 25, Length(r) - 24);

  result = r;
  return r;
#endif
}
#endif

#ifndef SERVER
void newplayerweapon()

{
    std::int32_t j, i;
    std::int32_t SecWep;
    if (sprite[mysprite].weapon.num == guns[noweapon].num)
    {
        gamemenushow(limbomenu);
    }

    i = mysprite;

    sprite[i].player->secwep = CVar::cl_player_secwep;

    for (j = 1; j < main_weapons; j++)
    {
        weaponsel[i][j] = 1;
    }

    if (CVar::sv_advancemode)
    {
        for (j = 1; j < primary_weapons; j++)
        {
            weaponsel[i][j] = 0;
        }
    }

    for (j = 1; j < main_weapons; j++)
    {
        if (weaponactive[j] == 1)
        {
            limbomenu->button[j - 1].active = bool(weaponsel[i][j]);
        }
    }

    SecWep = sprite[i].player->secwep + 1;

    if ((SecWep >= 1) and (SecWep <= secondary_weapons) and
        (weaponactive[primary_weapons + SecWep] == 1))
    {
        sprite[i].secondaryweapon = guns[primary_weapons + SecWep];
    }
    else
    {
        sprite[i].secondaryweapon = guns[noweapon];
    }
}
#endif
#ifdef SERVER
bool checkweaponnotallowed(std::uint8_t i)
{
    std::int32_t WeaponIndex;
    LogTraceG("CheckWeaponNotAllowed");

    auto Result = true;

    WeaponIndex = weaponnumtoindex(sprite[i].weapon.num);
    if (ismainweaponindex(WeaponIndex) and (weaponactive[WeaponIndex] == 0))
    {
        return Result;
    }

    if (((sprite[i].weapon.num == guns[bow].num) and (CVar::sv_gamemode != gamestyle_rambo)) or
        ((sprite[i].weapon.num == guns[bow2].num) and (CVar::sv_gamemode != gamestyle_rambo)) or
        ((sprite[i].weapon.num == guns[flamer].num) and (CVar::sv_bonus_flamer)))
    {
        return Result;
    }

    Result = false;
    return Result;
}

// Searches for the flood ip in the flood ips array
// Returns 0 when nothing was found
std::int32_t findfloodid(std::string SrcIP)
{
    std::int32_t i;
    auto Result = 0;
    for (i = 1; i < max_floodips; i++)
    {
        if (floodip[i] == SrcIP)
        {
            Result = i;
            break;
        }
    }
    return Result;
}

// Adds a flooding ip to the Flood ips array
// If the array is full the flood ip will not be added
std::int32_t addfloodip(std::string SrcIP)
{
    std::int32_t i;
    constexpr auto FLOOD_ID_NOT_FOUND = 0;
    auto Result = FLOOD_ID_NOT_FOUND;

    for (i = 1; i < max_floodips; i++)
    {
        if (floodip[i] == " ")
        {
            floodip[i] = SrcIP;
            Result = i;
            break;
        }
    }
    return Result;
}

std::int32_t updateantiflood(std::string SrcIP)
{
    std::int32_t FloodID;
    constexpr auto FLOOD_ID_NOT_FOUND = 0;

#ifdef SERVER
    lastreqip[lastreqid] = SrcIP;
    lastreqid = (lastreqid + 1) % 4;
#endif

    FloodID = findfloodid(SrcIP);

    if (FloodID == FLOOD_ID_NOT_FOUND)
    {
        FloodID = addfloodip(SrcIP);
    }
    else
    {
        floodnum[FloodID]++;

        if (floodnum[FloodID] > floodip_max)
        {
            addbannedip(SrcIP, "Flooding", Constants::TWENTY_MINUTES);
            GetMainConsole().console("IP number " + SrcIP + " banned for flooding",
                                client_message_color);
        }
    }
    return FloodID;
}

bool isfloodid(std::int32_t ID)
{
    constexpr auto FLOOD_ID_NOT_FOUND = 0;
    return (ID != FLOOD_ID_NOT_FOUND) and (floodnum[ID] > floodip_max);
}

bool addiptoremoteadmins(std::string SrcIP)
{
    auto Result = false;
    if (SrcIP == " ")
        return Result;

    if (not isadminip(SrcIP))
    {
        adminips.push_back(SrcIP);
        Result = true;
    }
    return Result;
}
#endif

#if 0
std::string ArrayToString(array c of Char)
{
     var std::int32_t i;
    Result = "";
    if Length (c)
        < 1 then Exit;
  for
      i = Low(c) to High(c) do if c[i] != #0 then
      {
          Result = Result + c[i];
      }
}
#endif

void stringtoarray(char *c, std::string s)
{
    std::strcpy(c, s.data());
}
