#pragma once

#include "..//Vector.hpp"
#include "../Util.hpp"
#include "../Weapons.hpp"
#include <array>
#include <cstdint>
#include <mutex>
#include <queue>
#include <string>

// Binary ops
auto constexpr B1 = 1;
auto constexpr B2 = 2;
auto constexpr B3 = 4;
auto constexpr B4 = 8;
auto constexpr B5 = 16;
auto constexpr B6 = 32;
auto constexpr B7 = 64;
auto constexpr B8 = 128;
auto constexpr B9 = 256;
auto constexpr B10 = 512;
auto constexpr B11 = 1024;
auto constexpr B12 = 2048;
auto constexpr B13 = 4096;
auto constexpr B14 = 8192;
auto constexpr B15 = 16384;
auto constexpr B16 = 32768;

// MESSAGE IDs
auto constexpr msgid_custom = 0;
auto constexpr msgid_heartbeat = msgid_custom + 2;
auto constexpr msgid_serverspritesnapshot = msgid_custom + 3;
auto constexpr msgid_clientspritesnapshot = msgid_custom + 4;
auto constexpr msgid_bulletsnapshot = msgid_custom + 5;
auto constexpr msgid_chatmessage = msgid_custom + 6;
auto constexpr msgid_serverskeletonsnapshot = msgid_custom + 7;
auto constexpr msgid_mapchange = msgid_custom + 8;
auto constexpr msgid_serverthingsnapshot = msgid_custom + 9;
auto constexpr msgid_thingtaken = msgid_custom + 12;
auto constexpr msgid_spritedeath = msgid_custom + 13;
auto constexpr msgid_playerinfo = msgid_custom + 15;
auto constexpr msgid_playerslist = msgid_custom + 16;
auto constexpr msgid_newplayer = msgid_custom + 17;
auto constexpr msgid_serverdisconnect = msgid_custom + 18;
auto constexpr msgid_playerdisconnect = msgid_custom + 19;
auto constexpr msgid_delta_movement = msgid_custom + 21;
auto constexpr msgid_delta_weapons = msgid_custom + 25;
auto constexpr msgid_delta_helmet = msgid_custom + 26;
auto constexpr msgid_delta_mouseaim = msgid_custom + 29;
auto constexpr msgid_ping = msgid_custom + 30;
auto constexpr msgid_pong = msgid_custom + 31;
auto constexpr msgid_flaginfo = msgid_custom + 32;
auto constexpr msgid_serverthingmustsnapshot = msgid_custom + 33;
auto constexpr msgid_idleanimation = msgid_custom + 37;
auto constexpr msgid_serverspritesnapshot_major = msgid_custom + 41;
auto constexpr msgid_clientspritesnapshot_mov = msgid_custom + 42;
auto constexpr msgid_clientspritesnapshot_dead = msgid_custom + 43;
auto constexpr msgid_unaccepted = msgid_custom + 44;
auto constexpr msgid_voteon = msgid_custom + 45;
auto constexpr msgid_votemap = msgid_custom + 46;
auto constexpr msgid_votemapreply = msgid_custom + 47;
auto constexpr msgid_votekick = msgid_custom + 48;
auto constexpr msgid_requestthing = msgid_custom + 51;
auto constexpr msgid_servervars = msgid_custom + 52;
auto constexpr msgid_serversyncmsg = msgid_custom + 54;
auto constexpr msgid_clientfreecam = msgid_custom + 55;
auto constexpr msgid_voteoff = msgid_custom + 56;
auto constexpr msgid_faedata = msgid_custom + 57;
auto constexpr msgid_requestgame = msgid_custom + 58;
auto constexpr msgid_forceposition = msgid_custom + 60;
auto constexpr msgid_forcevelocity = msgid_custom + 61;
auto constexpr msgid_forceweapon = msgid_custom + 62;
auto constexpr msgid_changeteam = msgid_custom + 63;
auto constexpr msgid_specialmessage = msgid_custom + 64;
auto constexpr msgid_weaponactivemessage = msgid_custom + 65;
auto constexpr msgid_joinserver = msgid_custom + 68;
auto constexpr msgid_playsound = msgid_custom + 70;
auto constexpr msgid_synccvars = msgid_custom + 71;
auto constexpr msgid_voicedata = msgid_custom + 72;

auto constexpr max_players = 32;

// ControlMethod
auto constexpr human = 1;
auto constexpr bot = 2;

// Request Reply States
auto constexpr ok = 1;
auto constexpr wrong_version = 2;
auto constexpr wrong_password = 3;
auto constexpr banned_ip = 4;
auto constexpr server_full = 5;
auto constexpr invalid_handshake = 8;
auto constexpr wrong_checksum = 9;
auto constexpr anticheat_required = 10;
auto constexpr anticheat_rejected = 11;
auto constexpr STEAM_ONLY = 12;

auto constexpr LAN = 1;
auto constexpr INTERNET = 0;

// FLAG INFO
auto constexpr returnred = 1;
auto constexpr returnblue = 2;
auto constexpr capturered = 3;
auto constexpr captureblue = 4;

// Kick/Ban Why's
auto constexpr kick_unknown = 0;
auto constexpr kick_noresponse = 1;
auto constexpr kick_nocheatresponse = 2; // tood remove?
auto constexpr kick_changeteam = 3;      // todo remove?
auto constexpr kick_ping = 4;
auto constexpr kick_flooding = 5;
auto constexpr kick_console = 6;
auto constexpr kick_connectcheat = 7; // todo remove?
auto constexpr kick_cheat = 8;
auto constexpr kick_leftgame = 9;
auto constexpr kick_voted = 10;
auto constexpr kick_ac = 11;
auto constexpr kick_silent = 12;
auto constexpr kick_steamticket = 13;
auto constexpr _kick_end = 14;

// Join types
auto constexpr join_normal = 0;
auto constexpr join_silent = 1;

// RECORD
auto constexpr netw = 0;
auto constexpr REC = 1;

auto constexpr clientplayerrecieved_time = 3 * 60;

auto constexpr floodip_max = 18;
auto constexpr max_floodips = 1000;
auto constexpr max_banips = 1000;

auto constexpr PLAYERNAME_CHARS = 24;
auto constexpr PLAYERHWID_CHARS = 11;
auto constexpr MAPNAME_CHARS = 64;
auto constexpr REASON_CHARS = 26;

auto constexpr actype_none = 0;
auto constexpr ACTYPE_FAE = 1;

auto constexpr msgtype_cmd = 0;
auto constexpr msgtype_pub = 1;
auto constexpr msgtype_team = 2;
auto constexpr msgtype_radio = 3;

struct SteamNetConnectionStatusChangedCallback_t;
struct SteamNetworkingIPAddr;
struct SteamNetworkingMessage_t;
class ISteamNetworkingSockets;

using PSteamNetConnectionStatusChangedCallback_t = SteamNetConnectionStatusChangedCallback_t *;
using PSteamNetworkingIPAddr = SteamNetworkingIPAddr *;
using PSteamNetworkingMessage_t = SteamNetworkingMessage_t *;

using HSoldatNetConnection = std::uint32_t;

class TNetwork
{
public:
  TNetwork();
  virtual ~TNetwork();

  [[nodiscard]] bool IsActive() const { return mActive; }
  void SetActive(bool active) { mActive = active; }
  [[nodiscard]] std::string GetStringAddress(bool withPort);
  [[nodiscard]] std::uint32_t Port() const { return mPort; }

protected:
  bool mActive;
  ISteamNetworkingSockets *mNetworkingSockets;
  std::string mIpAddress;
  std::uint32_t mPort;

  void RunCallbacks();
  virtual void ProcessEvents(SteamNetConnectionStatusChangedCallback_t* pInfo) = 0;

  friend void NetworksGlobalCallback(SteamNetConnectionStatusChangedCallback_t* pInfo);

private:
  void EmplaceSteamNetConnectionStatusChangeMessage(SteamNetConnectionStatusChangedCallback_t* pInfo);
  std::mutex mQueueMutex;
  std::queue<SteamNetConnectionStatusChangedCallback_t> mQueuedCallbacks;
};

enum class SoldatNetMessageType
{
  Reliable,
  Unreliable
};

#pragma pack(push, 1)
struct tmsgheader
{
  std::uint8_t id;
};

static_assert(sizeof(tmsgheader) == 1);
using pmsgheader = tmsgheader *;

template<typename T, std::uint32_t MsgId, SoldatNetMessageType Type>
struct SoldatConstSizeMessage
{
  static constexpr std::uint32_t GetSize() { return sizeof(T); }
  static consteval std::uint8_t sGetMsgId() { return MsgId; }
  static consteval bool sIsReliableMessage() { return Type == SoldatNetMessageType::Reliable; }
  SoldatConstSizeMessage(): header{MsgId} {}
  const tmsgheader header;
};

template<typename T, std::uint32_t MsgId, SoldatNetMessageType Type>
struct SoldatVariableSizeMessage
{
  static consteval std::uint8_t sGetMsgId() { return MsgId; }
  static consteval bool sIsReliableMessage() { return Type == SoldatNetMessageType::Reliable; }
  SoldatVariableSizeMessage(): header{MsgId} {}
  const tmsgheader header;
};

struct tmsg_stringmessage
{
  tmsgheader header;
  std::uint8_t num;
  std::uint8_t msgtype;
  std::array<char16_t, 0> text;
};
using pmsg_stringmessage = tmsg_stringmessage *;

struct tmsg_ping
{
  tmsgheader header;
  std::uint8_t pingticks;
  std::uint8_t pingnum;
};
using pmsg_ping = tmsg_ping *;

struct tmsg_pong
{
  tmsgheader header;
  std::uint8_t pingnum;
};
using pmsg_pong = tmsg_pong *;

// heartbeat type
// - every while information about frags, server status etc.

struct tmsg_heartbeat : SoldatConstSizeMessage<tmsg_heartbeat, msgid_heartbeat, SoldatNetMessageType::Unreliable>
{
  std::uint32_t mapid;
  std::array<std::uint16_t, 4> teamscore;
  std::array<bool, max_players> active;
  std::array<std::uint16_t, max_players> kills;
  std::array<std::uint8_t, max_players> caps;
  std::array<std::uint8_t, max_players> team;
  std::array<std::uint16_t, max_players> deaths;
  std::array<std::uint8_t, max_players> ping;
  std::array<std::uint16_t, max_players> realping;
  std::array<std::uint8_t, max_players> connectionquality;
  std::array<std::uint8_t, max_players> flags;
};
using pmsg_heartbeat = tmsg_heartbeat *;

static_assert(sizeof(tmsg_heartbeat) == 397, "TMsg_HeartBeat should be 397");

// serverspritesnapshot type
// - servers object status - uses for all objects in game

struct tmsg_serverspritesnapshot
{
  tmsgheader header;
  std::uint8_t num;
  tvector2 pos, velocity;
  std::int16_t mouseaimx, mouseaimy;
  std::uint8_t position;
  std::uint16_t keys16;
  std::uint8_t look;
  float vest;
  float health;
  std::uint8_t ammocount, grenadecount;
  std::uint8_t weaponnum, secondaryweaponnum;
  std::int32_t serverticks;
};
using pmsg_serverspritesnapshot = tmsg_serverspritesnapshot *;

struct tmsg_serverspritesnapshot_major
{
  tmsgheader header;
  std::uint8_t num;
  tvector2 pos, velocity;
  float health;
  std::int16_t mouseaimx, mouseaimy;
  std::uint8_t position;
  std::uint16_t keys16;
  std::int32_t serverticks;
};
using pmsg_serverspritesnapshot_major = tmsg_serverspritesnapshot_major *;

struct tmsg_servervars
{
  tmsgheader header;
  std::array<float, original_weapons> damage;
  std::array<std::uint8_t, original_weapons> ammo;
  std::array<std::uint16_t, original_weapons> reloadtime;
  std::array<float, original_weapons> speed;
  std::array<std::uint8_t, original_weapons> bulletstyle;
  std::array<std::uint16_t, original_weapons> startuptime;
  std::array<std::int16_t, original_weapons> bink;
  std::array<std::uint16_t, original_weapons> fireinterval;
  std::array<float, original_weapons> movementacc;
  std::array<float, original_weapons> bulletspread;
  std::array<std::uint16_t, original_weapons> recoil;
  std::array<float, original_weapons> push;
  std::array<float, original_weapons> inheritedvelocity;
  std::array<float, original_weapons> modifierhead;
  std::array<float, original_weapons> modifierchest;
  std::array<float, original_weapons> modifierlegs;
  std::array<std::uint8_t, original_weapons> nocollision;
  std::array<std::uint8_t, main_weapons> weaponactive;
};

static_assert(offsetof(tmsg_servervars, bulletstyle) == 221, "Bulletstyle should be 221");
static_assert(sizeof(tmsg_servervars) == 995, "TMsg_Servervars should be 995");

using pmsg_servervars = tmsg_servervars *;

struct tmsg_serversynccvars
{
  tmsgheader header;
  std::uint8_t itemcount;
  std::array<std::uint8_t, 0> data;
};
using pmsg_serversynccvars = tmsg_serversynccvars *;

// clientspritesnapshot type
// - current players status
struct tmsg_clientspritesnapshot
{
  tmsgheader header;
  std::uint8_t ammocount, secondaryammocount;
  std::uint8_t weaponnum, secondaryweaponnum;
  std::uint8_t position;
};
using pmsg_clientspritesnapshot = tmsg_clientspritesnapshot *;

struct tmsg_clientspritesnapshot_mov
{
  tmsgheader header;
  tvector2 pos, velocity;
  std::uint16_t keys16;
  std::int16_t mouseaimx, mouseaimy;
};
using pmsg_clientspritesnapshot_mov = tmsg_clientspritesnapshot_mov *;

struct tmsg_clientspritesnapshot_dead
{
  tmsgheader header;
  std::uint8_t camerafocus;
};
using pmsg_clientspritesnapshot_dead = tmsg_clientspritesnapshot_dead *;

// bulletsnapshot type
// - for server's bullet information

struct tmsg_bulletsnapshot
{
  tmsgheader header;
  std::uint8_t owner, weaponnum;
  tvector2 pos, velocity;
  std::uint16_t seed;
  bool forced; // createbullet() forced bullet?

  void Dump();
};
using pmsg_bulletsnapshot = tmsg_bulletsnapshot *;

// bulletsnapshot type
// - for clients' bullet information
struct tmsg_clientbulletsnapshot
{
  tmsgheader header;
  std::uint8_t weaponnum;
  tvector2 pos, velocity;
  std::uint16_t seed;
  std::int32_t clientticks;
};
using pmsg_clientbulletsnapshot = tmsg_clientbulletsnapshot *;

// serverskeletonsnapshot type
// - info on the sprites skeleton - used when sprite is deadmeat

struct tmsg_serverskeletonsnapshot
{
  tmsgheader header;
  std::uint8_t num;
  // byte constraints;
  std::int16_t respawncounter;
};
using pmsg_serverskeletonsnapshot = tmsg_serverskeletonsnapshot *;

// mapchange type
struct tmsg_mapchange
{
  tmsgheader header;
  std::int16_t counter;
  std::uint8_t mapnamelength; // number of characters in mapname (equivalent to Pascals' string)
  std::array<char, MAPNAME_CHARS> mapname;
  tsha1digest mapchecksum;
};
using pmsg_mapchange = tmsg_mapchange *;

// serverthingsnapshot type
// - info on the things in world that move
struct tmsg_serverthingsnapshot
{
  tmsgheader header;
  std::uint8_t num, owner, style, holdingsprite;
  std::array<tvector2, 4> oldpos, pos;
};
using pmsg_serverthingsnapshot = tmsg_serverthingsnapshot *;

// serverthingmustsnapshot type
// - info on the things in world
struct tmsg_serverthingmustsnapshot
{
  tmsgheader header;
  std::uint8_t num, owner, style, holdingsprite;
  std::array<tvector2, 4> pos, oldpos;
  std::int32_t timeout;
};
using pmsg_serverthingmustsnapshot = tmsg_serverthingmustsnapshot *;

// serverthingtakeninfo type
// - sent when thing is taken

struct tmsg_serverthingtaken
{
  tmsgheader header;
  std::uint8_t num, who;
  std::uint8_t style, ammocount;
};
using pmsg_serverthingtaken = tmsg_serverthingtaken *;

// spritedeath type
// - if sprite dies this is sent

struct tmsg_spritedeath
{
  tmsgheader header;
  std::uint8_t num, killer, killbullet, where;
  std::uint8_t constraints;
  std::array<tvector2, 16> pos, oldpos;
  float health;
  std::uint8_t onfire;
  std::int16_t respawncounter;
  float shotdistance, shotlife;
  std::uint8_t shotricochet;
};
using pmsg_spritedeath = tmsg_spritedeath *;

// request game type

struct tmsg_requestgame : SoldatVariableSizeMessage<tmsg_requestgame, msgid_requestgame, SoldatNetMessageType::Reliable>
{
  std::array<char, 6> version;
  std::uint8_t forwarded;
  std::uint8_t haveanticheat;
  std::array<char, PLAYERHWID_CHARS> hardwareid;
  std::array<char, 25> password;

  tmsg_requestgame(std::string_view pass)
  {
    std::memset(password.data() + password.size(),0xff, pass.size());
    *(password.data() + password.size() + pass.size()) = '\0';
    std::strcpy(password.data(), pass.data());
  }
  [[nodiscard]] std::int32_t GetSize() const { return sizeof(tmsg_requestgame) + std::strlen(password.data() + password.size()) + 1;}
  [[nodiscard]] static std::int32_t sCalculateSize(std::string_view password) { return sizeof(tmsg_requestgame) + length(password) + 1; }
};
static_assert(sizeof(tmsg_requestgame) == 45);
using pmsg_requestgame = tmsg_requestgame *;

// player info type

struct tmsg_playerinfo
{
  tmsgheader header;
  std::array<char, PLAYERNAME_CHARS> name;
  std::uint8_t look;
  std::uint8_t team;
  std::uint32_t shirtcolor, pantscolor, skincolor, haircolor, jetcolor;
  tsha1digest gamemodchecksum;
  tsha1digest custommodchecksum;
};
static_assert(sizeof(tmsg_playerinfo) == 87, "TMsg_PlayerInfo should be 87");

using pmsg_playerinfo = tmsg_playerinfo *;

// players list type
struct tmsg_playerslist
{
  tmsgheader header;
  std::array<char, MAPNAME_CHARS> modname;
  tsha1digest modchecksum;
  std::array<char, MAPNAME_CHARS> mapname;
  tsha1digest mapchecksum;
  std::uint8_t players;
  std::array<std::array<char, PLAYERNAME_CHARS>, max_players> name{0};
  std::array<std::uint32_t, max_players> shirtcolor, pantscolor, skincolor, haircolor, jetcolor;
  std::array<std::uint8_t, max_players> team;
  std::array<std::uint8_t, max_players> predduration;
  std::array<std::uint8_t, max_players> look;
  std::array<tvector2, max_players> pos;
  std::array<tvector2, max_players> vel;
  std::array<std::uint64_t, max_players> steamid;
  std::int32_t currenttime;
  std::int32_t serverticks;
  bool anticheatrequired;
};
static_assert(offsetof(tmsg_playerslist, players) == 169, "Players should be at 169 byte");
static_assert(offsetof(tmsg_playerslist, name) == 170, "Name should be at 170 byte");
static_assert(offsetof(tmsg_playerslist, shirtcolor) == 938, "ShirtColor should be at 938 byte");
static_assert(offsetof(tmsg_playerslist, pantscolor) == 1066, "ShirtColor should be at 1066 byte");
static_assert(offsetof(tmsg_playerslist, team) == 1578, "Team should be at 1578 byte");
static_assert(sizeof(tmsg_playerslist) == 2451, "TMsg_PlayersList should be 2451");
using pmsg_playerslist = tmsg_playerslist *;

// rejected connection type
struct tmsg_unaccepted
{
  tmsgheader header;
  std::uint8_t state;
  // soldat_version_chars = 6
  std::array<char, 6> version;
  std::array<char, 0> text;
};
using pmsg_unaccepted = tmsg_unaccepted *;

// new player type
struct tmsg_newplayer
{
  tmsgheader header;
  std::uint8_t num;
  std::uint8_t adoptspriteid;
  std::uint8_t jointype;
  std::array<char, PLAYERNAME_CHARS> name;
  std::uint32_t shirtcolor, pantscolor, skincolor, haircolor, jetcolor;
  std::uint8_t team;
  std::uint8_t look;
  tvector2 pos;
  std::uint64_t steamid;
};
using pmsg_newplayer = tmsg_newplayer *;

// server disconnect type

struct tmsg_serverdisconnect
{
  tmsgheader header;
};
using pmsg_serverdisconnect = tmsg_serverdisconnect *;

// player disconnect type
struct tmsg_playerdisconnect
{
  tmsgheader header;
  std::uint8_t num;
  std::uint8_t why;
};
using pmsg_playerdisconnect = tmsg_playerdisconnect *;

// idle animation type

struct tmsg_idleanimation
{
  tmsgheader header;
  std::uint8_t num;
  std::int16_t idlerandom;
};
using pmsg_idleanimation = tmsg_idleanimation *;

struct tmsg_clientfreecam
{
  tmsgheader header;
  std::uint8_t freecamon;
  tvector2 targetpos;
};
using pmsg_clientfreecam = tmsg_clientfreecam *;

// deltas

struct tmsg_serverspritedelta_movement
{
  tmsgheader header;
  std::uint8_t num;
  tvector2 pos, velocity;
  std::uint16_t keys16;
  std::int16_t mouseaimx, mouseaimy;
  std::int32_t servertick;
};
using pmsg_serverspritedelta_movement = tmsg_serverspritedelta_movement *;

struct tmsg_serverspritedelta_mouseaim
{
  tmsgheader header;
  std::uint8_t num;
  std::int16_t mouseaimx, mouseaimy;
};
using pmsg_serverspritedelta_mouseaim = tmsg_serverspritedelta_mouseaim *;

struct tmsg_serverspritedelta_weapons
{
  tmsgheader header;
  std::uint8_t num;
  std::uint8_t weaponnum, secondaryweaponnum;
  std::uint8_t ammocount;
};
using pmsg_serverspritedelta_weapons = tmsg_serverspritedelta_weapons *;

struct tmsg_serverspritedelta_helmet
{
  tmsgheader header;
  std::uint8_t num;
  std::uint8_t wearhelmet;
};
using pmsg_serverspritedelta_helmet = tmsg_serverspritedelta_helmet *;

struct tmsg_serverflaginfo
{
  tmsgheader header;
  std::uint8_t style, who;
};
using pmsg_serverflaginfo = tmsg_serverflaginfo *;

struct tmsg_serversyncmsg
{
  tmsgheader header;
  std::int32_t time;
  std::uint8_t pause;
};
using pmsg_serversyncmsg = tmsg_serversyncmsg *;

#ifdef enable_fae
using pmsg_faechallenge = tmsg_faechallenge *;
struct tmsg_faechallenge
{
  tmsgheader header;
  std::uint8_t inorder;
  tfaechallenge challenge;
}

using pmsg_faeresponse = tmsg_faeresponse *;
struct tmsg_faeresponse
{
  tmsgheader header;
  tfaeresponsebox response;
}
#endif

struct tmsg_forceposition
{
  tmsgheader header;
  tvector2 pos;
  std::uint8_t playerid;
};
using pmsg_forceposition = tmsg_forceposition *;

struct tmsg_forcevelocity
{
  tmsgheader header;
  tvector2 vel;
  std::uint8_t playerid;
};
using pmsg_forcevelocity = tmsg_forcevelocity *;

struct tmsg_forceweapon
{
  tmsgheader header;
  std::uint8_t weaponnum, secondaryweaponnum;
  std::uint8_t ammocount, secammocount;
};
using pmsg_forceweapon = tmsg_forceweapon *;

struct tmsg_changeteam
{
  tmsgheader header;
  std::uint8_t team;
};
using pmsg_changeteam = tmsg_changeteam *;

struct tmsg_requestthing
{
  tmsgheader header;
  std::uint8_t thingid;
};
using pmsg_requestthing = tmsg_requestthing *;

// voting messages

// voting on type

struct tmsg_voteon
{
  tmsgheader header;
  std::uint8_t votetype;
  std::uint16_t timer;
  std::uint8_t who;
  std::array<char, MAPNAME_CHARS> targetname;
  std::array<char, REASON_CHARS> reason;
};
using pmsg_voteon = tmsg_voteon *;

// voting off type

struct tmsg_voteoff
{
  tmsgheader header;
};
using pmsg_voteoff = tmsg_voteoff *;

// voting map list query

struct tmsg_votemap
{
  tmsgheader header;
  std::uint16_t mapid;
};
using pmsg_votemap = tmsg_votemap *;

// voting map list response
struct tmsg_votemapreply
{
  tmsgheader header;
  std::uint16_t count;
  std::array<char, MAPNAME_CHARS> mapname;
};
using pmsg_votemapreply = tmsg_votemapreply *;

// voting kick type

struct tmsg_votekick
{
  tmsgheader header;
  std::uint8_t ban;
  std::uint8_t num;
  std::array<char, REASON_CHARS> reason;
};
using pmsg_votekick = tmsg_votekick *;

// message packet
struct tmsg_serverspecialmessage
{
  tmsgheader header;
  std::uint8_t msgtype; // 0 - console, 1 - big text, 2 - world text
  std::uint8_t layerid; // only used for big text and world text
  std::int32_t delay;
  float scale;
  std::uint32_t color;
  float x, y;
  std::array<char, 0> text;
};
using pmsg_serverspecialmessage = tmsg_serverspecialmessage *;

// hide/show weapon in menu for specific player

struct tmsg_weaponactivemessage
{
  tmsgheader header;
  std::uint8_t active, weapon;
};
using pmsg_weaponactivemessage = tmsg_weaponactivemessage *;

struct tmsg_joinserver
{
  tmsgheader header;
  std::uint32_t ip;
  std::uint16_t port;
  std::array<char, 50> showmsg;
};
using pmsg_joinserver = tmsg_joinserver *;

struct tmsg_playsound
{ // server -> client
  tmsgheader header;
  std::array<char, 26> name;
  tvector2 emitter;
};
using pmsg_playsound = tmsg_playsound *;

struct tmsg_voicedata
{
  tmsgheader header;
  std::uint8_t speaker;
  std::array<std::uint8_t, 0> data;
};
using pmsg_voicedata = tmsg_voicedata *;

#pragma pack(pop)

#ifdef SCRIPT
bool ForceWeaponCalled;
#endif
