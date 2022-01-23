#include "NetworkUtils.hpp"
#include "../Console.hpp"
#include "../Cvar.hpp"
#include "../Game.hpp"
#include "../Version.hpp"
#include "../mechanics/Sprites.hpp"
#include "common/Logging.hpp"
#include "common/Weapons.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/misc/GlobalSystems.hpp"
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
#include "common/Parts.hpp"
#endif
#include "shared/mechanics/SpriteSystem.hpp"

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

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
    const auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(mysprite);
    switch (RadioID)
    {
    case 11:
        playsound(SfxEffect::radio_efcup, spritePartsPos);
        break;
    case 12:
        playsound(SfxEffect::radio_efcmid, spritePartsPos);
        break;
    case 13:
        playsound(SfxEffect::radio_efcdown, spritePartsPos);
        break;
    case 21:
        playsound(SfxEffect::radio_ffcup, spritePartsPos);
        break;
    case 22:
        playsound(SfxEffect::radio_ffcmid, spritePartsPos);
        break;
    case 23:
        playsound(SfxEffect::radio_ffcdown, spritePartsPos);
        break;
    case 31:
        playsound(SfxEffect::radio_esup, spritePartsPos);
        break;
    case 32:
        playsound(SfxEffect::radio_esmid, spritePartsPos);
        break;
    case 33:
        playsound(SfxEffect::radio_esdown, spritePartsPos);
        break;
    }
}
#endif

template <Config::Module M>
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

    if (SpriteC.bodyanimation.id == AnimationType::Change)
        Keys16 = Keys16 | B8;
    if (SpriteC.bodyanimation.id == AnimationType::ThrowWeapon)
        Keys16 = Keys16 | B9;
}

template <Config::Module M>
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
template <Config::Module M>
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

template <Config::Module M>
bool verifypacket(std::int32_t ValidSize, std::int32_t ReceiveSize, std::int32_t PacketId,
                  const source_location &location)
{
    std::string Dropped = "";
    auto Result = true;
    SoldatAssert(ValidSize == ReceiveSize);
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

template <Config::Module M>
bool verifypacketlargerorequal(std::int32_t ValidSize, std::int32_t ReceiveSize,
                               std::int32_t PacketId, const source_location &location)
{
    std::string Dropped = "";
    auto Result = true;
    SoldatAssert(ValidSize <= ReceiveSize);
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
    return GS::GetGame().GetPlayersNum() >= max_players;
}

// Checks if allowed server slots are taken
// If MaxPlayers slots is lower than MAX_PLAYERS there are still slots for
// admins to join
bool isserverfull()
{
    return ((GS::GetGame().GetPlayersNum() - GS::GetGame().GetBotsNum()) >= CVar::sv_maxplayers) or (isservertotallyfull());
}
#endif

// Checks if the Requested and the current Soldat version are the same
template <Config::Module M>
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
    if (SpriteSystem::Get().GetSprite(mysprite).weapon.num == noweapon_num)
    {
        gamemenushow(limbomenu);
    }

    i = mysprite;

    SpriteSystem::Get().GetSprite(i).player->secwep = CVar::cl_player_secwep;

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
    auto &weaponSystem = GS::GetWeaponSystem();

    for (j = 1; j < main_weapons; j++)
    {
        if (weaponSystem.IsEnabled(j))
        {
            limbomenu->button[j - 1].active = bool(weaponsel[i][j]);
        }
    }

    SecWep = SpriteSystem::Get().GetSprite(i).player->secwep + 1;

    if ((SecWep >= 1) and (SecWep <= secondary_weapons) and
        (weaponSystem.IsEnabled(primary_weapons + SecWep)))
    {
        SpriteSystem::Get().GetSprite(i).SetSecondWeapon(guns[primary_weapons + SecWep]);
    }
    else
    {
        SpriteSystem::Get().GetSprite(i).SetSecondWeapon(guns[noweapon]);
    }
}
#endif
#ifdef SERVER
bool checkweaponnotallowed(std::uint8_t i)
{
    std::int32_t WeaponIndex;
    LogTraceG("CheckWeaponNotAllowed");

    auto Result = true;

    auto &weaponSystem = GS::GetWeaponSystem();
    WeaponIndex = weaponnumtoindex(SpriteSystem::Get().GetSprite(i).weapon.num, guns);
    if (ismainweaponindex(WeaponIndex) and (!weaponSystem.IsEnabled(WeaponIndex)))
    {
        return Result;
    }

    if (((SpriteSystem::Get().GetSprite(i).weapon.num == bow_num) and
         (CVar::sv_gamemode != gamestyle_rambo)) or
        ((SpriteSystem::Get().GetSprite(i).weapon.num == bow2_num) and
         (CVar::sv_gamemode != gamestyle_rambo)) or
        ((SpriteSystem::Get().GetSprite(i).weapon.num == flamer_num) and (CVar::sv_bonus_flamer)))
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

template <Config::Module M>
void stringtoarray(char *c, std::string s)
{
    std::strcpy(c, s.data());
}

template std::string fixplayername(const char *);
template void encodekeys(tsprite &SpriteC, std::uint16_t &Keys16);
template void decodekeys(tsprite &SpriteC, uint16_t Keys16);
template void stringtoarray(char *c, std::string s);
template bool iswronggameversion<Config::GetModule()>(std::string RequestVersion);
template bool verifypacket<Config::GetModule()>(
    std::int32_t ValidSize, std::int32_t ReceiveSize, std::int32_t PacketId,
    const source_location &location = source_location::current());
template bool verifypacketlargerorequal<Config::GetModule()>(
    std::int32_t ValidSize, std::int32_t ReceiveSize, std::int32_t PacketId,
    const source_location &location = source_location::current());
