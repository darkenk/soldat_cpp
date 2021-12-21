#pragma once

#include <array>
#include <cstdint>

//#define GFXID
/*$INCLUDE gfx.inc*/
/*$UNDEF GFXID*/

constexpr char default_font[] = "play-regular.ttf";

constexpr std::int32_t headstyle_none = 0;
constexpr std::int32_t headstyle_helmet = 34;
constexpr std::int32_t headstyle_hat = 124;

constexpr std::int32_t default_width = 640;
constexpr std::int32_t default_height = 480;

constexpr std::int32_t default_goalticks = 60;

constexpr std::int32_t scale = 3;

constexpr float max_fov = 1.78;
#ifdef SERVER
constexpr float max_game_width = 480 * max_fov;
#else
constexpr float min_fov = 1.25;
constexpr std::int32_t max_big_messages = 255;
#endif

constexpr float grav = 0.06;

// speeds
constexpr float runspeed = 0.118;
constexpr float runspeedup = runspeed / 6;
constexpr float flyspeed = 0.03;
constexpr float jumpspeed = 0.66;
constexpr float crouchrunspeed = runspeed / 0.6;
constexpr float pronespeed = runspeed * 4.0;
constexpr float rollspeed = runspeed / 1.2;
constexpr float jumpdirspeed = 0.30;
constexpr float jetspeed = 0.10;
constexpr float camspeed = 0.14;

constexpr std::int32_t cluster_grenades = 3;

// aimdistances
constexpr std::int32_t defaultaimdist = 7;
constexpr float sniperaimdist = 3.5;
constexpr float crouchaimdist = 4.5;
constexpr std::int32_t spectatoraimdist = 30;
constexpr float aimdistincr = 0.05;

constexpr std::int32_t bulletcheckarraysize = 20;
constexpr std::int32_t max_logfilesize = 512000;
constexpr std::int32_t sound_maxdist = 750;
constexpr std::int32_t sound_panwidth = 1000;
constexpr std::int32_t sound_meterlength = 2000;

// trails
constexpr std::int32_t bullettrail = 13;
constexpr std::int32_t m79trail = 6;

#ifndef SERVER
constexpr std::int32_t bulletlength = 21;
#endif

// healths
constexpr std::int32_t default_health = 150;
constexpr std::int32_t floatistic_health = 65;
constexpr std::int32_t brutaldeathhealth = -400;
constexpr std::int32_t headchopdeathhealth = -90;
constexpr std::int32_t helmetfallhealth = 70;
constexpr std::int32_t hurt_health = 25;

// time
constexpr std::int32_t permanent = -1000;
constexpr std::int32_t second = 60;
constexpr std::int32_t half_minute = second * 30;
constexpr std::int32_t minute = second * 60;
constexpr std::int32_t five_minutes = minute * 5;
constexpr std::int32_t twenty_minutes = minute * 20;
constexpr std::int32_t half_hour = minute * 30;
constexpr std::int32_t sixty_minutes = minute * 60;
constexpr std::int32_t hour = sixty_minutes;
constexpr std::int32_t day = hour * 24;

#ifndef SERVER
constexpr std::int32_t morechattext = 60;
constexpr std::int32_t maxchattext = 85;
// display time for chars
constexpr std::int32_t spacechardelay = 68;
constexpr std::int32_t chardelay = 25;
#endif
constexpr std::int32_t max_chatdelay = second * 7 + 40;

constexpr std::int32_t killconsole_separate_height = 8;

// sound
constexpr std::int32_t default_volume_setting = 50;

// animations
constexpr std::int32_t explosion_anims = 16;
constexpr std::int32_t smoke_anims = 10;

constexpr float explosion_impact_multiply = 3.75;
constexpr float explosion_deadimpact_multiply = 4.5;

constexpr std::int32_t bullet_timeout = second * 7;
constexpr std::int32_t grenade_timeout = second * 3;
constexpr std::int32_t m2bullet_timeout = second;
constexpr std::int32_t flamer_timeout = 32;
constexpr std::int32_t melee_timeout = 1;

constexpr std::int32_t m2hitmultiply = 2;
constexpr std::int32_t m2gun_overaim = 4;
constexpr std::int32_t m2gun_overheat = 18;
constexpr std::int32_t gunresisttime = second * 20;

constexpr std::int32_t gun_radius = 10;
constexpr std::int32_t bow_radius = 20;
constexpr std::int32_t kit_radius = 12;
constexpr std::int32_t stat_radius = 15;

constexpr std::int32_t arrow_resist = 280;
constexpr float iluminatespeed = 0.085;
constexpr float minmovedelta = 0.63;

constexpr float posdelta = 60.0;
constexpr float veldelta = 0.27;

constexpr std::int32_t mouseaimdelta = 30;
constexpr std::int32_t spawnrandomvelocity = 25;

constexpr std::int32_t flag_timeout = second * 25;
constexpr std::int32_t waypointtimeout = second * 5 + 20; // = 320
constexpr std::int32_t waypoint_timeout = 480;            // TODO: why the duplication?

constexpr std::int32_t waypointseekradius = 21;

constexpr std::int32_t default_interest_time = second * 5 + 50;
constexpr std::int32_t flag_interest_time = second * 25;
constexpr std::int32_t bow_interest_time = second * 41 + 40;

constexpr std::int32_t default_mapchange_time = second * 5 + 20;

constexpr std::int32_t medikitthingsdiv = 23;
constexpr std::int32_t grenadekitthingsdiv = 23;

constexpr std::int32_t connectionproblem_time = second * 4;
constexpr std::int32_t connectionproblem_time2 = second * 5;

constexpr std::int32_t disconnection_time = second * 15;

constexpr std::int32_t killmessagewait = second * 4;
constexpr std::int32_t capturemessagewait = second * 6;
constexpr std::int32_t gamestartmessagewait = second * 5 + 20;
constexpr std::int32_t capturectfmessagewait = second * 7;

constexpr std::int32_t blood_random_normal = 10;
constexpr std::int32_t blood_random_low = 22;
constexpr std::int32_t blood_random_high = 6;

constexpr std::int32_t torch_random_normal = 6;
constexpr std::int32_t torch_random_low = 12;

constexpr std::int32_t fire_random_high = 30;
constexpr std::int32_t fire_random_normal = 50;
constexpr std::int32_t fire_random_low = 70;

constexpr std::int32_t clientmaxpositiondelta = 169;
constexpr std::int32_t default_ceasefire_time = 90;
constexpr std::int32_t predatoralpha = 5;
constexpr std::int32_t defaultvest = 100;

constexpr std::int32_t flamerbonustime = 600;
constexpr std::int32_t predatorbonustime = 1500;
constexpr std::int32_t berserkerbonustime = 900;

constexpr std::int32_t flamerbonus_random = 5;
constexpr std::int32_t predatorbonus_random = 5;
constexpr std::int32_t vestbonus_random = 4;
constexpr std::int32_t berserkerbonus_random = 4;
constexpr std::int32_t clusterbonus_random = 4;

constexpr std::int32_t bonus_none = 0;
constexpr std::int32_t bonus_grenades = 17;
constexpr std::int32_t bonus_flamegod = 18;
constexpr std::int32_t bonus_predator = 19;
constexpr std::int32_t bonus_vest = 20;
constexpr std::int32_t bonus_berserker = 21;
constexpr std::int32_t bonus_clusters = 22;

constexpr std::int32_t cursorsprite_distance = 15;
constexpr std::int32_t clientstopmove_retrys = 90;
constexpr std::int32_t multikillinterval = 180;

constexpr std::int32_t default_idletime = second * 8;
constexpr std::int32_t longer_idletime = half_minute;

constexpr std::int32_t fragsmenu_player_height = 15;
constexpr std::int32_t grenadeeffect_dist = 38;
constexpr std::int32_t htf_sec_point = 300;

constexpr std::int32_t background_width = 64;

constexpr std::int32_t max_admin_flood_ips = 200;
constexpr std::int32_t max_last_admin_ips = 5;

constexpr std::int32_t waverespawn_time_mulitplier = 1;

constexpr float para_speed = -0.5 * 0.06; // GRAV
constexpr std::int32_t para_distance = 500;

constexpr std::int32_t max_oldpos = 125;
#ifdef SERVER
constexpr std::int32_t max_pushtick = 0;
#else
constexpr std::int32_t max_pushtick = 125;
#endif
constexpr float max_inaccuracy = 0.5;

constexpr std::int32_t thing_push_multiplier = 9;
constexpr std::int32_t thing_collision_cooldown = 60;

constexpr std::int32_t fireinterval_net = 5;
constexpr std::int32_t melee_dist = 12;

constexpr std::array<const char *, 16> multikillmessage = {
    {"DOUBLE KILL", "TRIPLE KILL", "MULTI KILL", "MULTI KILL X2", "SERIAL KILL", "INSANE KILLS",
     "GIMME MORE!", "MASTA KILLA!", "MASTA KILLA!", "MASTA KILLA!", "STOP IT!!!!",
     "MERCY!!!!!!!!!!", "CHEATER!!!!!!!!", "Phased-plasma rifle in the forty watt range",
     "Hey, just what you see, pal", "just what you see, pal..."}};

constexpr std::int32_t default_jetcolor = 0xffffbd24;

constexpr std::int32_t idle_kick = minute * 3;
constexpr std::int32_t menu_time = second;
constexpr std::int32_t lessbleed_time = second * 2;
constexpr std::int32_t nobleed_time = second * 5;
constexpr std::int32_t onfire_time = second * 4;

constexpr std::int32_t survival_respawntime = second * 5;
constexpr std::int32_t default_vote_time = minute * 2;
constexpr std::int32_t default_voting_time = second * 20;

#ifndef SERVER
constexpr std::int32_t wep_restrict_width = 64;
constexpr std::int32_t wep_restrict_height = 64;
constexpr std::int32_t gos_restrict_width = 16;
constexpr std::int32_t gos_restrict_height = 16;
#endif

constexpr std::int32_t textstyle = 0;
constexpr std::int32_t horizontal = 1;
constexpr std::int32_t vertical = 2;

// Colors
constexpr std::int32_t default_message_color = 0xeeccffaa;
constexpr std::int32_t debug_message_color = 0xeeff8989;
constexpr std::int32_t game_message_color = 0xee71f981;
constexpr std::int32_t warning_message_color = 0xeee36952;

constexpr std::int32_t server_message_color = 0xf9fbda22;
constexpr std::int32_t client_message_color = 0xf9fcd822;

constexpr std::int32_t enter_message_color = 0xf1c3c3c3;

constexpr std::int32_t abovechat_message_color = 0xfdfdf9;
constexpr std::int32_t chat_message_color = 0xeeeffeea;
constexpr std::int32_t teamchat_message_color = 0xeefeda7c;

constexpr std::int32_t kill_message_color = 0xffea3530;
constexpr std::int32_t suicide_message_color = 0xd6b3a717;
constexpr std::int32_t die_message_color = 0xffc53025;

constexpr std::int32_t death_message_color = 0xee801304;
constexpr std::int32_t killer_message_color = 0xee52d119;

constexpr std::int32_t gamestart_message_color = 0xffd3ca34;

constexpr std::int32_t capture_message_color = 0xff77d334;
constexpr std::int32_t return_message_color = 0xff71a331;

constexpr std::int32_t alpha_message_color = 0xffdf3131;
constexpr std::int32_t bravo_message_color = 0xff3131df;
constexpr std::int32_t charlie_message_color = 0xffdfdf31;
constexpr std::int32_t delta_message_color = 0xff31df31;

constexpr std::int32_t alphaj_message_color = 0xffe15353;
constexpr std::int32_t bravoj_message_color = 0xff5353e1;
constexpr std::int32_t charliej_message_color = 0xffdfdf53;
constexpr std::int32_t deltaj_message_color = 0xff53df53;

constexpr std::int32_t bonus_message_color = 0xffef3121;
constexpr std::int32_t vote_message_color = 0xeeddee99;
constexpr std::int32_t music_message_color = 0xeeadfe99;
constexpr std::int32_t info_message_color = 0xeedddea2;
constexpr std::int32_t reginfo_message_color = 0xeea2dedd;
constexpr std::int32_t mode_message_color = 0xee81da41;

constexpr std::int32_t outofscreen_message_color = 0x99df99;
constexpr std::int32_t outofscreendead_message_color = 0x983333;
constexpr std::int32_t outofscreenflag_message_color = 0xdcdc33;

constexpr std::int32_t ac_message_color = 0xeee739b1;

constexpr std::int32_t alpha_k_message_color = 0xebffe3e3;
constexpr std::int32_t bravo_k_message_color = 0xebd3e3ff;
constexpr std::int32_t charlie_k_message_color = 0xebffffe3;
constexpr std::int32_t delta_k_message_color = 0xebd3ffe3;

constexpr std::int32_t alpha_d_message_color = 0xebdab0b0;
constexpr std::int32_t bravo_d_message_color = 0xeba0b0da;
constexpr std::int32_t charlie_d_message_color = 0xebd0d0b0;
constexpr std::int32_t delta_d_message_color = 0xeba0d0ba;
constexpr std::int32_t spectator_d_message_color = 0xebd3b727;

constexpr std::int32_t alpha_c_message_color = 0xf5fee8e8;
constexpr std::int32_t bravo_c_message_color = 0xf5e3e8fe;
constexpr std::int32_t charlie_c_message_color = 0xf5fefee8;
constexpr std::int32_t delta_c_message_color = 0xf5e8fee8;
constexpr std::int32_t spectator_c_message_color = 0xf5df7ab0;

// 0 represents in some cases all players
constexpr std::int32_t all_players = 0;

// Player teams
constexpr std::int32_t team_none = 0;
constexpr std::int32_t team_alpha = 1;
constexpr std::int32_t team_bravo = 2;
constexpr std::int32_t team_charlie = 3;
constexpr std::int32_t team_delta = 4;
constexpr std::int32_t team_spectator = 5;

// Game styles
constexpr std::int32_t gamestyle_deathmatch = 0;
constexpr std::int32_t gamestyle_pointmatch = 1;
constexpr std::int32_t gamestyle_teammatch = 2;
constexpr std::int32_t gamestyle_ctf = 3;
constexpr std::int32_t gamestyle_rambo = 4;
constexpr std::int32_t gamestyle_inf = 5;
constexpr std::int32_t gamestyle_htf = 6;

// Vote types
constexpr std::int32_t vote_map = 0;
constexpr std::int32_t vote_kick = 1;

constexpr std::int32_t color_transparency_unregistered = 0xff000000;
constexpr std::int32_t color_transparency_registered = 0xfe000000;
constexpr std::int32_t color_transparency_special = 0xfd000000;
constexpr std::int32_t color_transparency_bot = 0xfb000000;

// Polygon types
constexpr std::int32_t pt_onlybullets = 1;
constexpr std::int32_t pt_onlyplayers = 2;
constexpr std::int32_t pt_doesntcollide = 3;
constexpr std::int32_t pt_ice = 4;
constexpr std::int32_t pt_deadly = 5;
constexpr std::int32_t pt_bloodydeadly = 6;
constexpr std::int32_t pt_hurts = 7;
constexpr std::int32_t pt_regenerates = 8;
constexpr std::int32_t pt_lava = 9;
constexpr std::int32_t pt_alphabullets = 10;
constexpr std::int32_t pt_alphaplayers = 11;
constexpr std::int32_t pt_bravobullets = 12;
constexpr std::int32_t pt_bravoplayers = 13;
constexpr std::int32_t pt_charliebullets = 14;
constexpr std::int32_t pt_charlieplayers = 15;
constexpr std::int32_t pt_deltabullets = 16;
constexpr std::int32_t pt_deltaplayers = 17;
constexpr std::int32_t pt_bouncy = 18;
constexpr std::int32_t pt_explosive = 19;
constexpr std::int32_t pt_hurtflaggers = 20;
constexpr std::int32_t pt_flaggercollides = 21;
constexpr std::int32_t pt_nonflaggercollides = 22;
constexpr std::int32_t pt_flagcollides = 23;

// Game objects
constexpr std::int32_t object_num_nonweapon = 12;
constexpr std::int32_t object_num_flags = 3;

constexpr std::int32_t object_alpha_flag = 1;
constexpr std::int32_t object_bravo_flag = 2;
constexpr std::int32_t object_pointmatch_flag = 3;
constexpr std::int32_t object_ussocom = 4;
constexpr std::int32_t object_desert_eagle = 5;
constexpr std::int32_t object_hk_mp5 = 6;
constexpr std::int32_t object_ak74 = 7;
constexpr std::int32_t object_steyr_aug = 8;
constexpr std::int32_t object_spas12 = 9;
constexpr std::int32_t object_ruger77 = 10;
constexpr std::int32_t object_m79 = 11;
constexpr std::int32_t object_barret_m82a1 = 12;
constexpr std::int32_t object_minimi = 13;
constexpr std::int32_t object_minigun = 14;
constexpr std::int32_t object_rambo_bow = 15;
constexpr std::int32_t object_medical_kit = 16;
constexpr std::int32_t object_grenade_kit = 17;
constexpr std::int32_t object_flamer_kit = 18;
constexpr std::int32_t object_predator_kit = 19;
constexpr std::int32_t object_vest_kit = 20;
constexpr std::int32_t object_berserk_kit = 21;
constexpr std::int32_t object_cluster_kit = 22;
constexpr std::int32_t object_parachute = 23;
constexpr std::int32_t object_combat_knife = 24;
constexpr std::int32_t object_chainsaw = 25;
constexpr std::int32_t object_law = 26;
constexpr std::int32_t object_stationary_gun = 27;

// Sound effects
constexpr std::int32_t sfx_ak74_fire = 1;
constexpr std::int32_t sfx_rocketz = 2;
constexpr std::int32_t sfx_ak74_reload = 3;
constexpr std::int32_t sfx_m249_fire = 5;
constexpr std::int32_t sfx_ruger77_fire = 6;
constexpr std::int32_t sfx_ruger77_reload = 7;
constexpr std::int32_t sfx_m249_reload = 8;
constexpr std::int32_t sfx_mp5_fire = 9;
constexpr std::int32_t sfx_mp5_reload = 10;
constexpr std::int32_t sfx_spas12_fire = 11;
constexpr std::int32_t sfx_spas12_reload = 12;
constexpr std::int32_t sfx_standup = 13;
constexpr std::int32_t sfx_fall = 14;
constexpr std::int32_t sfx_spawn = 15;
constexpr std::int32_t sfx_m79_fire = 16;
constexpr std::int32_t sfx_m79_explosion = 17;
constexpr std::int32_t sfx_m79_reload = 18;
constexpr std::int32_t sfx_grenade_throw = 19;
constexpr std::int32_t sfx_grenade_explosion = 20;
constexpr std::int32_t sfx_grenade_bounce = 21;
constexpr std::int32_t sfx_bryzg = 22;
constexpr std::int32_t sfx_infiltmus = 23;
constexpr std::int32_t sfx_headchop = 24;
constexpr std::int32_t sfx_explosion_erg = 25;
constexpr std::int32_t sfx_water_step = 26;
constexpr std::int32_t sfx_bulletby = 27;
constexpr std::int32_t sfx_bodyfall = 28;
constexpr std::int32_t sfx_deserteagle_fire = 29;
constexpr std::int32_t sfx_deserteagle_reload = 30;
constexpr std::int32_t sfx_steyraug_fire = 31;
constexpr std::int32_t sfx_steyraug_reload = 32;
constexpr std::int32_t sfx_barretm82_fire = 33;
constexpr std::int32_t sfx_barretm82_reload = 34;
constexpr std::int32_t sfx_minigun_fire = 35;
constexpr std::int32_t sfx_minigun_reload = 36;
constexpr std::int32_t sfx_minigun_start = 37;
constexpr std::int32_t sfx_minigun_end = 38;
constexpr std::int32_t sfx_pickupgun = 39;
constexpr std::int32_t sfx_capture = 40;
constexpr std::int32_t sfx_colt1911_fire = 41;
constexpr std::int32_t sfx_colt1911_reload = 42;
constexpr std::int32_t sfx_changeweapon = 43;
constexpr std::int32_t sfx_shell = 44;
constexpr std::int32_t sfx_shell2 = 45;
constexpr std::int32_t sfx_dead_hit = 46;
constexpr std::int32_t sfx_throwgun = 47;
constexpr std::int32_t sfx_bow_fire = 48;
constexpr std::int32_t sfx_takebow = 49;
constexpr std::int32_t sfx_takemedikit = 50;
constexpr std::int32_t sfx_wermusic = 51;
constexpr std::int32_t sfx_ts = 52;
constexpr std::int32_t sfx_ctf = 53;
constexpr std::int32_t sfx_berserker = 54;
constexpr std::int32_t sfx_godflame = 55;
constexpr std::int32_t sfx_flamer = 56;
constexpr std::int32_t sfx_predator = 57;
constexpr std::int32_t sfx_killberserk = 58;
constexpr std::int32_t sfx_vesthit = 59;
constexpr std::int32_t sfx_burn = 60;
constexpr std::int32_t sfx_vesttake = 61;
constexpr std::int32_t sfx_clustergrenade = 62;
constexpr std::int32_t sfx_cluster_explosion = 63;
constexpr std::int32_t sfx_grenade_pullout = 64;
constexpr std::int32_t sfx_spit = 65;
constexpr std::int32_t sfx_stuff = 66;
constexpr std::int32_t sfx_smoke = 67;
constexpr std::int32_t sfx_match = 68;
constexpr std::int32_t sfx_roar = 69;
constexpr std::int32_t sfx_step = 70;
constexpr std::int32_t sfx_step2 = 71;
constexpr std::int32_t sfx_step3 = 72;
constexpr std::int32_t sfx_step4 = 73;
constexpr std::int32_t sfx_hum = 74;
constexpr std::int32_t sfx_ric = 75;
constexpr std::int32_t sfx_ric2 = 76;
constexpr std::int32_t sfx_ric3 = 77;
constexpr std::int32_t sfx_ric4 = 78;
constexpr std::int32_t sfx_dist_m79 = 79;
constexpr std::int32_t sfx_dist_grenade = 80;
constexpr std::int32_t sfx_dist_gun1 = 81;
constexpr std::int32_t sfx_dist_gun2 = 82;
constexpr std::int32_t sfx_dist_gun3 = 83;
constexpr std::int32_t sfx_dist_gun4 = 84;
constexpr std::int32_t sfx_death = 85;
constexpr std::int32_t sfx_death2 = 86;
constexpr std::int32_t sfx_death3 = 87;
constexpr std::int32_t sfx_crouch_move = 88;
constexpr std::int32_t sfx_hit_arg = 89;
constexpr std::int32_t sfx_hit_arg2 = 90;
constexpr std::int32_t sfx_hit_arg3 = 91;
constexpr std::int32_t sfx_goprone = 92;
constexpr std::int32_t sfx_roll = 93;
constexpr std::int32_t sfx_fall_hard = 94;
constexpr std::int32_t sfx_onfire = 95;
constexpr std::int32_t sfx_firecrack = 96;
constexpr std::int32_t sfx_scope = 97;
constexpr std::int32_t sfx_scopeback = 98;
constexpr std::int32_t sfx_playerdeath = 99;
constexpr std::int32_t sfx_changespin = 100;
constexpr std::int32_t sfx_arg = 101;
constexpr std::int32_t sfx_lava = 102;
constexpr std::int32_t sfx_regenerate = 103;
constexpr std::int32_t sfx_prone_move = 104;
constexpr std::int32_t sfx_jump = 105;
constexpr std::int32_t sfx_crouch = 106;
constexpr std::int32_t sfx_crouch_movel = 107;
constexpr std::int32_t sfx_step5 = 108;
constexpr std::int32_t sfx_step6 = 109;
constexpr std::int32_t sfx_step7 = 110;
constexpr std::int32_t sfx_step8 = 111;
constexpr std::int32_t sfx_stop = 112;
constexpr std::int32_t sfx_bulletby2 = 113;
constexpr std::int32_t sfx_bulletby3 = 114;
constexpr std::int32_t sfx_bulletby4 = 115;
constexpr std::int32_t sfx_bulletby5 = 116;
constexpr std::int32_t sfx_weaponhit = 117;
constexpr std::int32_t sfx_clipfall = 118;
constexpr std::int32_t sfx_bonecrack = 119;
constexpr std::int32_t sfx_gaugeshell = 120;
constexpr std::int32_t sfx_colliderhit = 121;
constexpr std::int32_t sfx_kit_fall = 122;
constexpr std::int32_t sfx_kit_fall2 = 123;
constexpr std::int32_t sfx_flag = 124;
constexpr std::int32_t sfx_flag2 = 125;
constexpr std::int32_t sfx_takegun = 126;
constexpr std::int32_t sfx_infilt_point = 127;
constexpr std::int32_t sfx_menuclick = 128;
constexpr std::int32_t sfx_knife = 129;
constexpr std::int32_t sfx_slash = 130;
constexpr std::int32_t sfx_chainsaw_d = 131;
constexpr std::int32_t sfx_chainsaw_m = 132;
constexpr std::int32_t sfx_chainsaw_r = 133;
constexpr std::int32_t sfx_piss = 134;
constexpr std::int32_t sfx_law = 135;
constexpr std::int32_t sfx_chainsaw_o = 136;
constexpr std::int32_t sfx_m2fire = 137;
constexpr std::int32_t sfx_m2explode = 138;
constexpr std::int32_t sfx_m2overheat = 139;
constexpr std::int32_t sfx_signal = 140;
constexpr std::int32_t sfx_m2use = 141;
constexpr std::int32_t sfx_scoperun = 142;
constexpr std::int32_t sfx_mercy = 143;
constexpr std::int32_t sfx_ric5 = 144;
constexpr std::int32_t sfx_ric6 = 145;
constexpr std::int32_t sfx_ric7 = 146;
constexpr std::int32_t sfx_law_start = 147;
constexpr std::int32_t sfx_law_end = 148;
constexpr std::int32_t sfx_boomheadshot = 149;
constexpr std::int32_t sfx_snapshot = 150;
constexpr std::int32_t sfx_radio_efcup = 151;
constexpr std::int32_t sfx_radio_efcmid = 152;
constexpr std::int32_t sfx_radio_efcdown = 153;
constexpr std::int32_t sfx_radio_ffcup = 154;
constexpr std::int32_t sfx_radio_ffcmid = 155;
constexpr std::int32_t sfx_radio_ffcdown = 156;
constexpr std::int32_t sfx_radio_esup = 157;
constexpr std::int32_t sfx_radio_esmid = 158;
constexpr std::int32_t sfx_radio_esdown = 159;
constexpr std::int32_t sfx_bounce = 160;
constexpr std::int32_t sfx_rain = 161;
constexpr std::int32_t sfx_snow = 162;
constexpr std::int32_t sfx_wind = 163;
