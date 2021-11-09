#pragma once

#include <string_view>

namespace Constants
{
constexpr std::string_view DEFAULT_FONT = "play-regular.ttf";

constexpr std::int32_t HEADSTYLE_NONE = 0;
constexpr std::int32_t HEADSTYLE_HELMET = 34;
constexpr std::int32_t HEADSTYLE_HAT = 124;

constexpr std::int32_t DEFAULT_WIDTH = 640;
constexpr std::int32_t DEFAULT_HEIGHT = 480;

constexpr std::int32_t DEFAULT_GOALTICKS = 60;

constexpr std::int32_t SCALE = 3;

constexpr float MAX_FOV = 1.78;
constexpr float MIN_FOV = 1.25;
constexpr float MAX_BIG_MESSAGES = 255;

constexpr float RUNSPEED = 0.118;
constexpr float RUNSPEEDUP = RUNSPEED / 6.0f;
constexpr float FLYSPEED = 0.03;
constexpr float JUMPSEED = 0.66;
constexpr float CROUCHRUNSPEED = RUNSPEED / 0.6f;
constexpr float PRONESPEED = RUNSPEED * 4.0f;
constexpr float ROLLSPEED = RUNSPEED / 1.2f;
constexpr float JUMPDIRSPEED = 0.30;
constexpr float JETSPEED = 0.10;
constexpr float CAMSPEED = 0.14;

constexpr std::int32_t CLUSTER_GRENADES = 3;

// aimdistances
constexpr float DEFAULTAIMDIST = 7.0;
constexpr float SNIPERAIMDIST = 3.5;
constexpr float CROUCHAIMDIST = 4.5;
constexpr float SPECTATORAIMDIST = 30.0;
constexpr float AIMDISTINC = 0.05;

constexpr std::int32_t BULLETCHECKARRAYSIZE = 20;
constexpr std::int32_t MAX_LOGFILESIZE = 512000;
constexpr std::int32_t SOUND_MAXDIST = 750;
constexpr std::int32_t SOUND_PANWIDTH = 1000;
constexpr std::int32_t SOUND_METERLENGTH = 2000;

// trails
constexpr std::int32_t BULLETTRAIL = 13;
constexpr std::int32_t M79TRAIL = 6;

constexpr std::int32_t BULLETLENGTH = 21;

// healths
constexpr std::int32_t DEFAULT_HEALTH = 150;
constexpr std::int32_t REALISTIC_HEALTH = 65;
constexpr std::int32_t BRUTALDEATHHEALTH = -400;
constexpr std::int32_t HEADCOPDEATHHEALTH = -90;
constexpr std::int32_t HELMETFALLHEALTH = 70;
constexpr std::int32_t HURT_HEALTH = 25;

// time
constexpr std::int32_t PERMAMENT = -1000;
constexpr std::int32_t SECOND = 60;
constexpr std::int32_t HALF_MINUTE = SECOND * 30;
constexpr std::int32_t MINUTE = SECOND * 60;
constexpr std::int32_t FIVE_MINUTES = MINUTE * 5;
constexpr std::int32_t TWENTY_MINUTES = MINUTE * 20;
constexpr std::int32_t HALF_HOUR = MINUTE * 30;
constexpr std::int32_t SIXTY_MINUTES = MINUTE * 60;
constexpr std::int32_t HOUR = SIXTY_MINUTES;
constexpr std::int32_t DAY = HOUR * 25;

constexpr std::int32_t MORECHATTEXT = 60;
constexpr std::int32_t MAXCHATTEXT = 85;
// display time for chars
constexpr std::int32_t SPACECHARDELAY = 68;
constexpr std::int32_t CHARDELAY = 25;

constexpr std::int32_t MAX_CHATDELAY = SECOND * 7 + 40;

constexpr std::int32_t KILLCONSOLE_SEPARATE_HEIGHT = 8;

// sound
constexpr std::int32_t DEFAULT_VOLUME_SETTING = 50;

// animations
constexpr std::int32_t EXPLOSION_ANIMS = 16;
constexpr std::int32_t SMOKE_ANIMS = 10;

constexpr float EXPLOSION_IMPACT_MULTIPLY = 3.75;
constexpr float EXPLOSION_DEADIMPACT_MULTIPLY = 4.5;

constexpr std::int32_t BULLET_TIMEOUT = SECOND * 7;
constexpr std::int32_t GRENADE_TIMEOUT = SECOND * 3;
constexpr std::int32_t M2BULLET_TIMEOUT = SECOND;
constexpr std::int32_t FLAMER_TIMEOUT = 32;
constexpr std::int32_t MELEE_TIMEOUT = 1;

constexpr std::int32_t M2HITMULTIPLY = 2;
constexpr std::int32_t M2GUN_VERAIM = 4;
constexpr std::int32_t M2GUN_OVERHEAT = 18;
constexpr std::int32_t GUNRESITTIME = SECOND * 20;

constexpr std::int32_t GUN_RADIUS = 10;
constexpr std::int32_t BOW_RADIUS = 20;
constexpr std::int32_t KIT_RADIUS = 12;
constexpr std::int32_t STAT_RADIUS = 15;

constexpr std::int32_t ARROW_RESIST = 280;
constexpr float ILUMINATESPEED = 0.085;
constexpr float MINMOVEDELTA = 0.65;

constexpr float POSDELTA = 60.0;
constexpr float VELDELTA = 0.27;

constexpr std::int32_t MOUSEAIMDELTA = 30;
constexpr std::int32_t SPAWNRANDOMVELOCITY = 25;

constexpr std::int32_t FLAG_TIMEOUT = SECOND * 25;
constexpr std::int32_t WAYPOINTTIMEOUT = SECOND * 5 + 20;
constexpr std::int32_t WAYPOINT_TIMEOUT = 480;

constexpr std::int32_t WAYPOINTSEEKRADIUS = 21;
constexpr std::int32_t DEFAULT_INTEREST_TIME = SECOND * 5 + 50;
constexpr std::int32_t FLAG_INTEREST_TIME = SECOND * 25;
constexpr std::int32_t BOW_INTEREST_TIME = SECOND * 41 + 40;

constexpr std::int32_t DEFAULT_MAPCHANGE_TIME = SECOND * 5 + 20;

constexpr std::int32_t MEDIKITTHINGSDIV = 23;
constexpr std::int32_t GRENADEKITTHINGSDIV = 23;

constexpr std::int32_t CONNECTIONPROBLEM_TIME = SECOND * 4;
constexpr std::int32_t CONNECTIONPROBLEM_TIME2 = SECOND * 5;

constexpr std::int32_t DISCONNECTION_TIME = SECOND * 15;

constexpr std::int32_t KILLMESSAGEWAIT = SECOND * 4;
constexpr std::int32_t CAPTUREMESSAGEWAIT = SECOND * 6;
constexpr std::int32_t GAMESTARTMESSAGEWAIT = SECOND * 5 + 20;
constexpr std::int32_t CAPTURECTFMESSAGEWAIT = SECOND * 7;

constexpr std::int32_t BLOOD_RANDOM_NORMAL = 10;
constexpr std::int32_t BLOOD_RANDOM_LOW = 22;
constexpr std::int32_t BLOOD_RANDOM_HIGH = 6;

constexpr std::int32_t TORCH_RANDOM_NORMAL = 6;
constexpr std::int32_t TORCH_RANDOM_LOW = 12;

constexpr std::int32_t FIRE_RANDOM_HIGH = 30;
constexpr std::int32_t FIRE_RANDOM_NORMAL = 50;
constexpr std::int32_t FIRE_RANDOM_LOW = 70;

constexpr std::int32_t CLIENTMAXPOSITIONDELTA = 169;
constexpr std::int32_t DEFAULT_CEASEFIRE_TIME = 90;
constexpr std::int32_t PREDATORALPHA = 5;
constexpr std::int32_t DEFAULTVEST = 100;

constexpr std::int32_t FLAMERBONUSTIME = 600;
constexpr std::int32_t PREDATORBONUSTIME = 1500;
constexpr std::int32_t BERSERKERBONUSTIME = 900;

constexpr std::int32_t FLAMERBONUS_RANDOM = 5;
constexpr std::int32_t PREDATORBONUS_RANDOM = 5;
constexpr std::int32_t VESTBONUS_RANDOM = 4;
constexpr std::int32_t BERSERKERBONUS_RANDOM = 4;
constexpr std::int32_t CLUSTERBONUS_RANDOM = 4;

constexpr std::int32_t BONUS_NONE = 0;
constexpr std::int32_t BONUS_GRENADES = 17;
constexpr std::int32_t BONUS_FLAMEGOD = 18;
constexpr std::int32_t BONUS_PREDATOR = 19;
constexpr std::int32_t BONUS_VEST = 20;
constexpr std::int32_t BONUS_BERSERKER = 21;
constexpr std::int32_t BONUS_CLUSTERS = 22;

constexpr std::int32_t CURSORSPRITE_DISTANCE = 15;
constexpr std::int32_t CLIENTSTOPMOVE_RETRYS = 90;
constexpr std::int32_t MULTIKILLINTERVAL = 180;

constexpr std::int32_t DEFAULT_IDLETIME = SECOND * 8;
constexpr std::int32_t LONGER_IDLETIME = HALF_MINUTE;

constexpr std::int32_t FRAGSMENU_PLAYER_HEIGHT = 15;
constexpr std::int32_t GRENADEEFFECT_DIST = 38;
constexpr std::int32_t HTF_SEC_POINT = 300;

constexpr std::int32_t BACKGROUND_WIDTH = 64;

constexpr std::int32_t MAX_ADMIN_FLOOD_IPS = 200;
constexpr std::int32_t MAX_LAST_ADMIN_IPS = 5;

constexpr std::int32_t WAVERESPAWN_TIME_MULITPLIER = 1;

constexpr float PARA_SPEED = -0.5 * 0.06; // GRAV
constexpr std::int32_t PARA_DISTANCE = 500;

constexpr std::int32_t MAX_OLDPOS = 125;
#if SERVER
constexpr std::int32_t MAX_PUSHTICK = 0;
#else
constexpr std::int32_t MAX_PUSHTICK = 125;
#endif
constexpr float MAX_INACCURACY = 0.5;

constexpr std::int32_t THING_PUSH_MULTIPLIER = 9;
constexpr std::int32_t THING_COLLISION_COOLDOWN = 60;

constexpr std::int32_t FIREINTERVAL_NET = 5;
constexpr std::int32_t MELEE_DIST = 12;

/*   MULTIKILLMESSAGE: array[2..17] of WideString = ('DOUBLE KILL', 'TRIPLE KILL', */
// 'MULTI KILL', 'MULTI KILL X2', 'SERIAL KILL', 'INSANE KILLS', 'GIMME MORE!',
// 'MASTA KILLA!', 'MASTA KILLA!', 'MASTA KILLA!', 'STOP IT!!!!',
// 'MERCY!!!!!!!!!!', 'CHEATER!!!!!!!!',
// 'Phased-plasma rifle in the forty watt range',
/* 'Hey, just what you see, pal', 'just what you see, pal...'); */

constexpr std::int32_t DEFAULT_JETCOLOR = 0xFFFFBD24;

constexpr std::int32_t IDLE_KICK = MINUTE * 3;
constexpr std::int32_t MENU_TIME = SECOND;
constexpr std::int32_t LESSBLEED_TIME = SECOND * 2;
constexpr std::int32_t NOBLEED_TIME = SECOND * 5;
constexpr std::int32_t ONFIRE_TIME = SECOND * 4;

constexpr std::int32_t SURVIVAL_RESPAWNTIME = SECOND * 5;
constexpr std::int32_t DEFAULT_VOTE_TIME = MINUTE * 2;
constexpr std::int32_t DEFAULT_VOTING_TIME = SECOND * 20;

#if !SERVER
constexpr std::int32_t WEP_RESTRICT_WIDTH = 64;
constexpr std::int32_t WEP_RESTRICT_HEIGHT = 64;
constexpr std::int32_t GOS_RESTRICT_WIDTH = 16;
constexpr std::int32_t GOS_RESTRICT_HEIGHT = 16;
#endif

constexpr std::int32_t TEXTSTYLE = 0;
constexpr std::int32_t HORIZONTAL = 1;
constexpr std::int32_t VERTICAL = 2;

// Colors
constexpr std::int32_t DEFAULT_MESSAGE_COLOR = 0xEECCFFAA;
constexpr std::int32_t DEBUG_MESSAGE_COLOR = 0xEEFF8989;
constexpr std::int32_t GAME_MESSAGE_COLOR = 0xEE71F981;
constexpr std::int32_t WARNING_MESSAGE_COLOR = 0xEEE36952;

constexpr std::int32_t SERVER_MESSAGE_COLOR = 0xF9FBDA22;
constexpr std::int32_t CLIENT_MESSAGE_COLOR = 0xF9FCD822;

constexpr std::int32_t ENTER_MESSAGE_COLOR = 0xF1C3C3C3;

constexpr std::int32_t ABOVECHAT_MESSAGE_COLOR = 0xFDFDF9;
constexpr std::int32_t CHAT_MESSAGE_COLOR = 0xEEEFFEEA;
constexpr std::int32_t TEAMCHAT_MESSAGE_COLOR = 0xEEFEDA7C;

constexpr std::int32_t KILL_MESSAGE_COLOR = 0xFFEA3530;
constexpr std::int32_t SUICIDE_MESSAGE_COLOR = 0xD6B3A717;
constexpr std::int32_t DIE_MESSAGE_COLOR = 0xFFC53025;

constexpr std::int32_t DEATH_MESSAGE_COLOR = 0xEE801304;
constexpr std::int32_t KILLER_MESSAGE_COLOR = 0xEE52D119;

constexpr std::int32_t GAMESTART_MESSAGE_COLOR = 0xFFD3CA34;

constexpr std::int32_t CAPTURE_MESSAGE_COLOR = 0xFF77D334;
constexpr std::int32_t RETURN_MESSAGE_COLOR = 0xFF71A331;

constexpr std::int32_t ALPHA_MESSAGE_COLOR = 0xFFDF3131;
constexpr std::int32_t BRAVO_MESSAGE_COLOR = 0xFF3131DF;
constexpr std::int32_t CHARLIE_MESSAGE_COLOR = 0xFFDFDF31;
constexpr std::int32_t DELTA_MESSAGE_COLOR = 0xFF31DF31;

constexpr std::int32_t ALPHAJ_MESSAGE_COLOR = 0xFFE15353;
constexpr std::int32_t BRAVOJ_MESSAGE_COLOR = 0xFF5353E1;
constexpr std::int32_t CHARLIEJ_MESSAGE_COLOR = 0xFFDFDF53;
constexpr std::int32_t DELTAJ_MESSAGE_COLOR = 0xFF53DF53;

constexpr std::int32_t BONUS_MESSAGE_COLOR = 0xFFEF3121;
constexpr std::int32_t VOTE_MESSAGE_COLOR = 0xEEDDEE99;
constexpr std::int32_t MUSIC_MESSAGE_COLOR = 0xEEADFE99;
constexpr std::int32_t INFO_MESSAGE_COLOR = 0xEEDDDEA2;
constexpr std::int32_t REGINFO_MESSAGE_COLOR = 0xEEA2DEDD;
constexpr std::int32_t MODE_MESSAGE_COLOR = 0xEE81DA41;

constexpr std::int32_t OUTOFSCREEN_MESSAGE_COLOR = 0x99DF99;
constexpr std::int32_t OUTOFSCREENDEAD_MESSAGE_COLOR = 0x983333;
constexpr std::int32_t OUTOFSCREENFLAG_MESSAGE_COLOR = 0xDCDC33;

constexpr std::int32_t AC_MESSAGE_COLOR = 0xEEE739B1;

constexpr std::int32_t ALPHA_K_MESSAGE_COLOR = 0xEBFFE3E3;
constexpr std::int32_t BRAVO_K_MESSAGE_COLOR = 0xEBD3E3FF;
constexpr std::int32_t CHARLIE_K_MESSAGE_COLOR = 0xEBFFFFE3;
constexpr std::int32_t DELTA_K_MESSAGE_COLOR = 0xEBD3FFE3;

constexpr std::int32_t ALPHA_D_MESSAGE_COLOR = 0xEBDAB0B0;
constexpr std::int32_t BRAVO_D_MESSAGE_COLOR = 0xEBA0B0DA;
constexpr std::int32_t CHARLIE_D_MESSAGE_COLOR = 0xEBD0D0B0;
constexpr std::int32_t DELTA_D_MESSAGE_COLOR = 0xEBA0D0BA;
constexpr std::int32_t SPECTATOR_D_MESSAGE_COLOR = 0xEBD3B727;

constexpr std::int32_t ALPHA_C_MESSAGE_COLOR = 0xF5FEE8E8;
constexpr std::int32_t BRAVO_C_MESSAGE_COLOR = 0xF5E3E8FE;
constexpr std::int32_t CHARLIE_C_MESSAGE_COLOR = 0xF5FEFEE8;
constexpr std::int32_t DELTA_C_MESSAGE_COLOR = 0xF5E8FEE8;
constexpr std::int32_t SPECTATOR_C_MESSAGE_COLOR = 0xF5DF7AB0;

// 0 represents in some cases all players
constexpr std::int32_t ALL_PLAYERS = 0;

// Player teams
constexpr std::int32_t TEAM_NONE = 0;
constexpr std::int32_t TEAM_ALPHA = 1;
constexpr std::int32_t TEAM_BRAVO = 2;
constexpr std::int32_t TEAM_CHARLIE = 3;
constexpr std::int32_t TEAM_DELTA = 4;
constexpr std::int32_t TEAM_SPECTATOR = 5;

// Game styles
constexpr std::int32_t GAMESTYLE_DEATHMATCH = 0;
constexpr std::int32_t GAMESTYLE_POINTMATCH = 1;
constexpr std::int32_t GAMESTYLE_TEAMMATCH = 2;
constexpr std::int32_t GAMESTYLE_CTF = 3;
constexpr std::int32_t GAMESTYLE_RAMBO = 4;
constexpr std::int32_t GAMESTYLE_INF = 5;
constexpr std::int32_t GAMESTYLE_HTF = 6;

// Vote types
constexpr std::int32_t VOTE_MAP = 0;
constexpr std::int32_t VOTE_KICK = 1;

constexpr std::int32_t COLOR_TRANSPARENCY_UNREGISTERED = 0xFF000000;
constexpr std::int32_t COLOR_TRANSPARENCY_REGISTERED = 0xFE000000;
constexpr std::int32_t COLOR_TRANSPARENCY_SPECIAL = 0xFD000000;
constexpr std::int32_t COLOR_TRANSPARENCY_BOT = 0xFB000000;

// Polygon types
constexpr std::int32_t PT_ONLYBULLETS = 1;
constexpr std::int32_t PT_ONLYPLAYERS = 2;
constexpr std::int32_t PT_DOESNTCOLLIDE = 3;
constexpr std::int32_t PT_ICE = 4;
constexpr std::int32_t PT_DEADLY = 5;
constexpr std::int32_t PT_BLOODYDEADLY = 6;
constexpr std::int32_t PT_HURTS = 7;
constexpr std::int32_t PT_REGENERATES = 8;
constexpr std::int32_t PT_LAVA = 9;
constexpr std::int32_t PT_ALPHABULLETS = 10;
constexpr std::int32_t PT_ALPHAPLAYERS = 11;
constexpr std::int32_t PT_BRAVOBULLETS = 12;
constexpr std::int32_t PT_BRAVOPLAYERS = 13;
constexpr std::int32_t PT_CHARLIEBULLETS = 14;
constexpr std::int32_t PT_CHARLIEPLAYERS = 15;
constexpr std::int32_t PT_DELTABULLETS = 16;
constexpr std::int32_t PT_DELTAPLAYERS = 17;
constexpr std::int32_t PT_BOUNCY = 18;
constexpr std::int32_t PT_EXPLOSIVE = 19;
constexpr std::int32_t PT_HURTFLAGGERS = 20;
constexpr std::int32_t PT_FLAGGERCOLLIDES = 21;
constexpr std::int32_t PT_NONFLAGGERCOLLIDES = 22;
constexpr std::int32_t PT_FLAGCOLLIDES = 23;

// Game objects
constexpr std::int32_t OBJECT_NUM_NONWEAPON = 12;
constexpr std::int32_t OBJECT_NUM_FLAGS = 3;

constexpr std::int32_t OBJECT_ALPHA_FLAG = 1;
constexpr std::int32_t OBJECT_BRAVO_FLAG = 2;
constexpr std::int32_t OBJECT_POINTMATCH_FLAG = 3;
constexpr std::int32_t OBJECT_USSOCOM = 4;
constexpr std::int32_t OBJECT_DESERT_EAGLE = 5;
constexpr std::int32_t OBJECT_HK_MP5 = 6;
constexpr std::int32_t OBJECT_AK74 = 7;
constexpr std::int32_t OBJECT_STEYR_AUG = 8;
constexpr std::int32_t OBJECT_SPAS12 = 9;
constexpr std::int32_t OBJECT_RUGER77 = 10;
constexpr std::int32_t OBJECT_M79 = 11;
constexpr std::int32_t OBJECT_BARRET_M82A1 = 12;
constexpr std::int32_t OBJECT_MINIMI = 13;
constexpr std::int32_t OBJECT_MINIGUN = 14;
constexpr std::int32_t OBJECT_RAMBO_BOW = 15;
constexpr std::int32_t OBJECT_MEDICAL_KIT = 16;
constexpr std::int32_t OBJECT_GRENADE_KIT = 17;
constexpr std::int32_t OBJECT_FLAMER_KIT = 18;
constexpr std::int32_t OBJECT_PREDATOR_KIT = 19;
constexpr std::int32_t OBJECT_VEST_KIT = 20;
constexpr std::int32_t OBJECT_BERSERK_KIT = 21;
constexpr std::int32_t OBJECT_CLUSTER_KIT = 22;
constexpr std::int32_t OBJECT_PARACHUTE = 23;
constexpr std::int32_t OBJECT_COMBAT_KNIFE = 24;
constexpr std::int32_t OBJECT_CHAINSAW = 25;
constexpr std::int32_t OBJECT_LAW = 26;
constexpr std::int32_t OBJECT_STATIONARY_GUN = 27;

// Sound effects
constexpr std::int32_t SFX_AK74_FIRE = 1;
constexpr std::int32_t SFX_ROCKETZ = 2;
constexpr std::int32_t SFX_AK74_RELOAD = 3;
constexpr std::int32_t SFX_M249_FIRE = 5;
constexpr std::int32_t SFX_RUGER77_FIRE = 6;
constexpr std::int32_t SFX_RUGER77_RELOAD = 7;
constexpr std::int32_t SFX_M249_RELOAD = 8;
constexpr std::int32_t SFX_MP5_FIRE = 9;
constexpr std::int32_t SFX_MP5_RELOAD = 10;
constexpr std::int32_t SFX_SPAS12_FIRE = 11;
constexpr std::int32_t SFX_SPAS12_RELOAD = 12;
constexpr std::int32_t SFX_STANDUP = 13;
constexpr std::int32_t SFX_FALL = 14;
constexpr std::int32_t SFX_SPAWN = 15;
constexpr std::int32_t SFX_M79_FIRE = 16;
constexpr std::int32_t SFX_M79_EXPLOSION = 17;
constexpr std::int32_t SFX_M79_RELOAD = 18;
constexpr std::int32_t SFX_GRENADE_THROW = 19;
constexpr std::int32_t SFX_GRENADE_EXPLOSION = 20;
constexpr std::int32_t SFX_GRENADE_BOUNCE = 21;
constexpr std::int32_t SFX_BRYZG = 22;
constexpr std::int32_t SFX_INFILTMUS = 23;
constexpr std::int32_t SFX_HEADCHOP = 24;
constexpr std::int32_t SFX_EXPLOSION_ERG = 25;
constexpr std::int32_t SFX_WATER_STEP = 26;
constexpr std::int32_t SFX_BULLETBY = 27;
constexpr std::int32_t SFX_BODYFALL = 28;
constexpr std::int32_t SFX_DESERTEAGLE_FIRE = 29;
constexpr std::int32_t SFX_DESERTEAGLE_RELOAD = 30;
constexpr std::int32_t SFX_STEYRAUG_FIRE = 31;
constexpr std::int32_t SFX_STEYRAUG_RELOAD = 32;
constexpr std::int32_t SFX_BARRETM82_FIRE = 33;
constexpr std::int32_t SFX_BARRETM82_RELOAD = 34;
constexpr std::int32_t SFX_MINIGUN_FIRE = 35;
constexpr std::int32_t SFX_MINIGUN_RELOAD = 36;
constexpr std::int32_t SFX_MINIGUN_START = 37;
constexpr std::int32_t SFX_MINIGUN_END = 38;
constexpr std::int32_t SFX_PICKUPGUN = 39;
constexpr std::int32_t SFX_CAPTURE = 40;
constexpr std::int32_t SFX_COLT1911_FIRE = 41;
constexpr std::int32_t SFX_COLT1911_RELOAD = 42;
constexpr std::int32_t SFX_CHANGEWEAPON = 43;
constexpr std::int32_t SFX_SHELL = 44;
constexpr std::int32_t SFX_SHELL2 = 45;
constexpr std::int32_t SFX_DEAD_HIT = 46;
constexpr std::int32_t SFX_THROWGUN = 47;
constexpr std::int32_t SFX_BOW_FIRE = 48;
constexpr std::int32_t SFX_TAKEBOW = 49;
constexpr std::int32_t SFX_TAKEMEDIKIT = 50;
constexpr std::int32_t SFX_WERMUSIC = 51;
constexpr std::int32_t SFX_TS = 52;
constexpr std::int32_t SFX_CTF = 53;
constexpr std::int32_t SFX_BERSERKER = 54;
constexpr std::int32_t SFX_GODFLAME = 55;
constexpr std::int32_t SFX_FLAMER = 56;
constexpr std::int32_t SFX_PREDATOR = 57;
constexpr std::int32_t SFX_KILLBERSERK = 58;
constexpr std::int32_t SFX_VESTHIT = 59;
constexpr std::int32_t SFX_BURN = 60;
constexpr std::int32_t SFX_VESTTAKE = 61;
constexpr std::int32_t SFX_CLUSTERGRENADE = 62;
constexpr std::int32_t SFX_CLUSTER_EXPLOSION = 63;
constexpr std::int32_t SFX_GRENADE_PULLOUT = 64;
constexpr std::int32_t SFX_SPIT = 65;
constexpr std::int32_t SFX_STUFF = 66;
constexpr std::int32_t SFX_SMOKE = 67;
constexpr std::int32_t SFX_MATCH = 68;
constexpr std::int32_t SFX_ROAR = 69;
constexpr std::int32_t SFX_STEP = 70;
constexpr std::int32_t SFX_STEP2 = 71;
constexpr std::int32_t SFX_STEP3 = 72;
constexpr std::int32_t SFX_STEP4 = 73;
constexpr std::int32_t SFX_HUM = 74;
constexpr std::int32_t SFX_RIC = 75;
constexpr std::int32_t SFX_RIC2 = 76;
constexpr std::int32_t SFX_RIC3 = 77;
constexpr std::int32_t SFX_RIC4 = 78;
constexpr std::int32_t SFX_DIST_M79 = 79;
constexpr std::int32_t SFX_DIST_GRENADE = 80;
constexpr std::int32_t SFX_DIST_GUN1 = 81;
constexpr std::int32_t SFX_DIST_GUN2 = 82;
constexpr std::int32_t SFX_DIST_GUN3 = 83;
constexpr std::int32_t SFX_DIST_GUN4 = 84;
constexpr std::int32_t SFX_DEATH = 85;
constexpr std::int32_t SFX_DEATH2 = 86;
constexpr std::int32_t SFX_DEATH3 = 87;
constexpr std::int32_t SFX_CROUCH_MOVE = 88;
constexpr std::int32_t SFX_HIT_ARG = 89;
constexpr std::int32_t SFX_HIT_ARG2 = 90;
constexpr std::int32_t SFX_HIT_ARG3 = 91;
constexpr std::int32_t SFX_GOPRONE = 92;
constexpr std::int32_t SFX_ROLL = 93;
constexpr std::int32_t SFX_FALL_HARD = 94;
constexpr std::int32_t SFX_ONFIRE = 95;
constexpr std::int32_t SFX_FIRECRACK = 96;
constexpr std::int32_t SFX_SCOPE = 97;
constexpr std::int32_t SFX_SCOPEBACK = 98;
constexpr std::int32_t SFX_PLAYERDEATH = 99;
constexpr std::int32_t SFX_CHANGESPIN = 100;
constexpr std::int32_t SFX_ARG = 101;
constexpr std::int32_t SFX_LAVA = 102;
constexpr std::int32_t SFX_REGENERATE = 103;
constexpr std::int32_t SFX_PRONE_MOVE = 104;
constexpr std::int32_t SFX_JUMP = 105;
constexpr std::int32_t SFX_CROUCH = 106;
constexpr std::int32_t SFX_CROUCH_MOVEL = 107;
constexpr std::int32_t SFX_STEP5 = 108;
constexpr std::int32_t SFX_STEP6 = 109;
constexpr std::int32_t SFX_STEP7 = 110;
constexpr std::int32_t SFX_STEP8 = 111;
constexpr std::int32_t SFX_STOP = 112;
constexpr std::int32_t SFX_BULLETBY2 = 113;
constexpr std::int32_t SFX_BULLETBY3 = 114;
constexpr std::int32_t SFX_BULLETBY4 = 115;
constexpr std::int32_t SFX_BULLETBY5 = 116;
constexpr std::int32_t SFX_WEAPONHIT = 117;
constexpr std::int32_t SFX_CLIPFALL = 118;
constexpr std::int32_t SFX_BONECRACK = 119;
constexpr std::int32_t SFX_GAUGESHELL = 120;
constexpr std::int32_t SFX_COLLIDERHIT = 121;
constexpr std::int32_t SFX_KIT_FALL = 122;
constexpr std::int32_t SFX_KIT_FALL2 = 123;
constexpr std::int32_t SFX_FLAG = 124;
constexpr std::int32_t SFX_FLAG2 = 125;
constexpr std::int32_t SFX_TAKEGUN = 126;
constexpr std::int32_t SFX_INFILT_POINT = 127;
constexpr std::int32_t SFX_MENUCLICK = 128;
constexpr std::int32_t SFX_KNIFE = 129;
constexpr std::int32_t SFX_SLASH = 130;
constexpr std::int32_t SFX_CHAINSAW_D = 131;
constexpr std::int32_t SFX_CHAINSAW_M = 132;
constexpr std::int32_t SFX_CHAINSAW_R = 133;
constexpr std::int32_t SFX_PISS = 134;
constexpr std::int32_t SFX_LAW = 135;
constexpr std::int32_t SFX_CHAINSAW_O = 136;
constexpr std::int32_t SFX_M2FIRE = 137;
constexpr std::int32_t SFX_M2EXPLODE = 138;
constexpr std::int32_t SFX_M2OVERHEAT = 139;
constexpr std::int32_t SFX_SIGNAL = 140;
constexpr std::int32_t SFX_M2USE = 141;
constexpr std::int32_t SFX_SCOPERUN = 142;
constexpr std::int32_t SFX_MERCY = 143;
constexpr std::int32_t SFX_RIC5 = 144;
constexpr std::int32_t SFX_RIC6 = 145;
constexpr std::int32_t SFX_RIC7 = 146;
constexpr std::int32_t SFX_LAW_START = 147;
constexpr std::int32_t SFX_LAW_END = 148;
constexpr std::int32_t SFX_BOOMHEADSHOT = 149;
constexpr std::int32_t SFX_SNAPSHOT = 150;
constexpr std::int32_t SFX_RADIO_EFCUP = 151;
constexpr std::int32_t SFX_RADIO_EFCMID = 152;
constexpr std::int32_t SFX_RADIO_EFCDOWN = 153;
constexpr std::int32_t SFX_RADIO_FFCUP = 154;
constexpr std::int32_t SFX_RADIO_FFCMID = 155;
constexpr std::int32_t SFX_RADIO_FFCDOWN = 156;
constexpr std::int32_t SFX_RADIO_ESUP = 157;
constexpr std::int32_t SFX_RADIO_ESMID = 158;
constexpr std::int32_t SFX_RADIO_ESDOWN = 159;
constexpr std::int32_t SFX_BOUNCE = 160;
constexpr std::int32_t SFX_RAIN = 161;
constexpr std::int32_t SFX_SNOW = 162;
constexpr std::int32_t SFX_WIND = 163;
}; // namespace Constants

#include "Constants.cpp.h"
