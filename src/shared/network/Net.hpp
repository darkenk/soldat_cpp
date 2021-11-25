#pragma once

#include "../misc/SoldatConfig.hpp"
#include "common/Util.hpp"
#include "common/Vector.hpp"
#include "common/Weapons.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "common/network/Net.hpp"
#include "shared/Version.hpp"
#include <array>
#include <cstdint>
#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>
#include <string>

#pragma pack(push, 1)

// Network Player Class
// Client:
//   Each sprite has a static instance of this class, allocated on startup.
// Server:
//   This class is attached to the GNS peer's data pointer. Its lifetime equals the lifetime of
//   the GNS connection. The player's Sprite member MAY point to a valid sprite if the player has
//   joined a game.
class tplayer
{
  public:
    // (!!!) When extending this class also extend its clone method, else ScriptCore breaks (maybe).

    // client/server shared stuff:
    // TODO stuff here that is relevant for the sprite (color, team, ...) should be moved to
    // a template object instead. When a sprite is created, then copy the template to it and apply
    // modifications depending on the game mode (eg. change the shirt color to match the team.)
    // That would allow switching game modes etc. without losing information about the player.
    std::string name;
    std::uint32_t shirtcolor, pantscolor, skincolor, haircolor, jetcolor;
    std::int32_t kills, deaths;
    std::uint8_t flags;
    std::int32_t pingticks, pingticksb, pingtime, ping;
    std::uint16_t realping;
    std::uint8_t connectionquality;
    std::uint8_t team;
    std::uint8_t controlmethod;
    std::uint8_t chain, headcap, hairstyle;
    std::uint8_t secwep;
    std::uint8_t camera;
    std::uint8_t muted;
    std::uint8_t spritenum = 0; // 0 if no sprite exists yet
    bool demoplayer;

// server only below this line:
// -----
#ifdef SERVER
    std::string ip;
    std::int32_t port;

// anti-cheat client handles and state
#ifdef ENABLE_FAE
    bool FaeResponsePending;
    bool FaeKicked;
    std::int32_t FaeTicks;
    TFaeSecret FaeSecret;
#endif

    HSteamNetConnection peer;
    std::string hwid;
    std::int32_t playtime;
    bool gamerequested = false;

    // counters for warnings:
    std::uint8_t chatwarnings;
    std::uint8_t tkwarnings;

    // anti mass flag counters:
    std::int32_t scorespersecond;
    std::int32_t grabspersecond;
    bool grabbedinbase;            // to prevent false accusations
    std::uint8_t standingpolytype; // testing
    std::uint8_t knifewarnings;

    ~tplayer()
    {
    }

    void applyshirtcolorfromteam(); // TODO remove, see comment before Name
#endif
};

using TPlayers = std::vector<tplayer *>;

#pragma pack(pop)

extern std::int32_t maintickcounter;
// Stores all network-generated TPlayer objects
#ifdef SERVER
extern TPlayers players;
#endif

extern std::int32_t playersnum, botsnum, spectatorsnum;
extern PascalArray<std::int32_t, 1, 4> playersteamnum;

#ifdef SCRIPT
bool ForceWeaponCalled;
#endif
