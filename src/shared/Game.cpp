// automatically converted

#include "Game.hpp"

/*#include "Server.h"*/
/*#include "Client.h"*/
/*#include "TraceLog.h"*/
/*#include "ServerHelper.h"*/
/*#include "ScriptDispatcher.h"*/
/*#include "Sound.h"*/
/*#include "GameMenus.h"*/
/*#include "ClientGame.h"*/
/*#include "GameRendering.h"*/
/*#include "GameStrings.h"*/
/*#include "InterfaceGraphics.h"*/
/*#include "Net.h"*/
/*#include "Demo.h"*/
/*#include "NetworkClientGame.h"*/
/*#include "NetworkServerGame.h"*/
/*#include "NetworkServerSprite.h"*/
#ifndef SERVER
#include "../client/Client.hpp"
#include "../client/ClientGame.hpp"
#include "../client/GameMenus.hpp"
#include "../client/GameRendering.hpp"
#include "../client/InterfaceGraphics.hpp"
#include "../client/Sound.hpp"
#else
#include "../server/Server.hpp"
#include "../server/ServerHelper.hpp"
#include "shared/Logging.hpp"
#include "shared/network/NetworkServerGame.hpp"
#include "shared/network/NetworkServerSprite.hpp"
#endif
#include "Cvar.hpp"
#include "Demo.hpp"
#include "Util.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include <chrono>

// various
// extern tconsole mainconsole;
// end

twaypoints botpath;

// TODO: move all dependent functions in this unit
std::int32_t ticks, tickspersecond; // Tick counter and Ticks persecond counter
// Ticks counter etc
std::int32_t frames, framespersecond, ticktime, ticktimelast;
std::int32_t goalticks = default_goalticks;

std::int32_t bullettimetimer;

#ifndef SERVER
std::int32_t gamewidth = default_width;
std::int32_t gameheight = default_height;

float gamewidthhalf = default_width;  // / 2;
float gameheighthalf = default_width; // / 2;
#endif
// Ping Impr - vars
PascalArray<PascalArray<tvector2, 0, max_oldpos>, 1, max_sprites> oldspritepos;

// survival vars
std::uint8_t alivenum;
std::array<std::int8_t, 6> teamalivenum;
PascalArray<std::int8_t, 0, 4> teamplayersnum;
bool survivalendround = false;
bool weaponscleaned = false;

std::int32_t ceasefiretime = default_ceasefire_time;
std::int32_t mapchangetime = default_mapchange_time;
std::int32_t mapchangecounter;
std::string mapchangename;
tmapinfo mapchange;
std::uint64_t mapchangeitemid;
tsha1digest mapchangechecksum;
std::int32_t timelimitcounter = 3600;
std::int32_t starthealth = 150;
std::int32_t timeleftsec, timeleftmin;
PascalArray<PascalArray<std::uint8_t, 1, main_weapons>, 1, max_sprites> weaponsel;

std::array<std::int32_t, 5> teamscore;
std::array<std::int32_t, 4> teamflag;

float sinuscounter = 0;

tpolymap map;

tsha1digest gamemodchecksum;
tsha1digest custommodchecksum;
tsha1digest mapchecksum;

std::int32_t mapindex;

PascalArray<tkillsort, 1, max_sprites> sortedplayers;
#ifndef SERVER
PascalArray<tkillsort, 1, max_sprites> sortedteamscore;

std::int32_t heartbeattime, heartbeattimewarnings;
#endif

// Sprites
// FIXME: client has frozen bullets when Sprite array position is "bad"
// if happens again change Sprite array to 0..MAX_SPRITES to "fix" it
// possible cause: out of range array read (index 0 instead of 1)
PascalArray<tsprite, 1, max_sprites> sprite; // player, game handling sprite
PascalArray<tbullet, 1, max_bullets> bullet; // bullet game handling sprite
#ifndef SERVER
PascalArray<tspark, 1, max_sparks> spark; // spark game handling sprite
#endif
PascalArray<tthing, 1, max_things> thing; // thing game handling sprite

// voting
bool voteactive = false;
std::uint8_t votetype = 0; // VOTE_MAP or VOTE_KICK
std::string votetarget = "";
std::string votestarter = "";
std::string votereason = "";
std::int32_t votetimeremaining = -1;
std::uint8_t votenumvotes = 0;
std::uint8_t votemaxvotes = 0;
PascalArray<bool, 1, max_sprites> votehasvoted;
PascalArray<std::int32_t, 1, max_sprites> votecooldown;
bool votekickreasontype = false;

// NUMBER27's TIMING ROUTINES
std::uint64_t timeinmil, timeinmillast; // time in Milliseconds the computer has
// been running
std::uint64_t timepassed;          // Time in Milliseconds the program has been running
std::int32_t seconds, secondslast; // Seconds the program has been running

using string = std::string;

// Timing routine
void number27timing()
{
    timeinmillast = timeinmil;
    timeinmil = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();
    if (timeinmil - timeinmillast > 2000)
        timeinmillast = timeinmil; // safety precaution

    timepassed = timepassed + (timeinmil - timeinmillast);
    secondslast = seconds;
    seconds = trunc((float)(timepassed) / 1000);

    if (seconds != secondslast)
    { // new Second
        tickspersecond = ticks;
        ticks = 0;

        framespersecond = frames;
        frames = 0;
    }

    frames += 1;

    ticktimelast = ticktime;

    ticktime = trunc(timepassed / ((float)(1000) / goalticks));
}

void updategamestats()
{
    std::int32_t i;
    tstringlist s;

    // Game Stats save
    if (CVar::log_enable)
    {
        //    try

        s.add("In-Game Statistics");
        s.add(string("Players: ") + inttostr(playersnum));
        s.add(string("Map: ") + map.name);
        switch (CVar::sv_gamemode)
        {
        case gamestyle_deathmatch:
            s.add("Gamemode: Deathmatch");
            break;
        case gamestyle_pointmatch:
            s.add("Gamemode: Pointmatch");
            break;
        case gamestyle_teammatch:
            s.add("Gamemode: Teammatch");
            break;
        case gamestyle_ctf:
            s.add("Gamemode: Capture the Flag");
            break;
        case gamestyle_rambo:
            s.add("Gamemode: Rambomatch");
            break;
        case gamestyle_inf:
            s.add("Gamemode: Infiltration");
            break;
        case gamestyle_htf:
            s.add("Gamemode: Hold the Flag");
            break;
        }
        s.add(string("Timeleft: ") + inttostr(timeleftmin) + ':' + inttostr(timeleftsec));
        if (isteamgame())
        {
            s.add(string("Team 1: ") + inttostr(teamscore[1]));
            s.add(string("Team 2: ") + inttostr(teamscore[2]));
            s.add(string("Team 3: ") + inttostr(teamscore[3]));
            s.add(string("Team 4: ") + inttostr(teamscore[4]));
        }

        s.add("Players list: (name/kills/deaths/team/ping)");
        if (playersnum > 0)
            for (i = 1; i <= playersnum; i++)
            {
                s.add(sprite[sortedplayers[i].playernum].player->name);
                s.add(inttostr(sprite[sortedplayers[i].playernum].player->kills));
                s.add(inttostr(sprite[sortedplayers[i].playernum].player->deaths));
                s.add(inttostr(sprite[sortedplayers[i].playernum].player->team));
                s.add(inttostr(sprite[sortedplayers[i].playernum].player->realping));
            }
#ifndef SERVER
        s.add("");
        s.add("Server:");
        s.add(joinip + ':' + joinport);
#endif

        s.savetofile(userdirectory + "logs/gamestat.txt");
        //    finally
        //      FreeAndNil(S);
        //    end;
    }
}

void togglebullettime(bool turnon, std::int32_t duration)
{
#ifdef SERVER
    LogTraceG("ToggleBulletTime");
#endif

    if (turnon)
    {
        bullettimetimer = duration;
        goalticks = default_goalticks / 3;
    }
    else
        goalticks = default_goalticks;

    number27timing();
}

bool pointvisible(float x, float y, std::int32_t i)
{
#ifdef SERVER
    // TODO: check why numbers differ on server and client
    const std::int32_t game_width = max_game_width;
    const std::int32_t game_height = 480;
#else
    std::int32_t game_width;
    std::int32_t game_height;
#endif
    float sx, sy;

#ifdef SERVER
    LogTraceG("PointVisible");
#else
    // workaround because of variables instead of constants
    game_width = gamewidth;
    game_height = gameheight;
#endif

    bool result = false;

    if ((i > max_players) || (i < 1))
        return result;

    sx = spriteparts.pos[i].x - ((float)((spriteparts.pos[i].x - sprite[i].control.mouseaimx)) / 2);
    sy = spriteparts.pos[i].y - ((float)((spriteparts.pos[i].y - sprite[i].control.mouseaimy)) / 2);

    if ((x > (sx - game_width)) && (x < (sx + game_width)) && (y > (sy - game_height)) &&
        (y < (sy + game_height)))
        result = true;
    return result;
}

bool pointvisible2(float x, float y, std::int32_t i)
{
// TODO: check why numbers differ on server and client
#ifdef SERVER
    const std::int32_t game_width = max_game_width;
    const std::int32_t game_height = 480;
#else
    const std::int32_t game_width = 600;
    const std::int32_t game_height = 440;
#endif
    float sx, sy;

#ifdef SERVER
    LogTraceG("PointVisible2");
#endif

    bool result = false;

    sx = spriteparts.pos[i].x;
    sy = spriteparts.pos[i].y;

    if ((x > (sx - game_width)) && (x < (sx + game_width)) && (y > (sy - game_height)) &&
        (y < (sy + game_height)))
        result = true;
    return result;
}

#ifndef SERVER
bool ispointonscreen(tvector2 point)
{
    float p1, p2;

    bool result;
    result = true;
    p1 = gamewidthhalf - (camerax - point.x);
    p2 = gameheighthalf - (cameray - point.y);
    if ((p1 < 0) || (p1 > gamewidth))
        result = false;
    if ((p2 < 0) || (p2 > gameheight))
        result = false;
    return result;
}
#endif

void startvote(std::uint8_t startervote, std::uint8_t typevote, std::string targetvote,
               std::string reasonvote)
{
    std::uint8_t i;

    voteactive = true;
    if ((startervote < 1) || (startervote > max_players))
        votestarter = "Server";
    else
    {
        votestarter = sprite[startervote].player->name;
        votecooldown[startervote] = default_vote_time;
#ifndef SERVER
        if (startervote == mysprite)
            if (votetype == vote_kick)
            {
                mainconsole.console(("You have voted to kick") + ' ' +
                                        (sprite[kickmenuindex].player->name) + ' ' +
                                        ("from the game"),
                                    vote_message_color);
                voteactive = false;
                NotImplemented(NITag::NETWORK, "No clientvotekick");
#if 0
                clientvotekick(strtoint(targetvote), true, "");
#endif
            }
#endif
    }
    votetype = typevote;
    votetarget = targetvote;
    votereason = reasonvote;
    votetimeremaining = default_voting_time;
    votenumvotes = 0;
    votemaxvotes = 0;
    for (auto &s : sprite)
    {
        if (!s.active)
        {
            continue;
        }
        if (s.player->controlmethod == human)
        {
            votemaxvotes = votemaxvotes + 1;
        }
    }
}

void stopvote()
{
    voteactive = false;
    votenumvotes = 0;
    votemaxvotes = 0;
    votetype = 0;
    votetarget = "";
    votestarter = "";
    votereason = "";
    votetimeremaining = -1;
    std::fill(std::begin(votehasvoted), std::end(votehasvoted), false);
}

void timervote()
{
#ifdef SERVER
    if (voteactive)
    {
#endif
        if (votetimeremaining > -1)
            votetimeremaining = votetimeremaining - 1;

        if (votetimeremaining == 0)
        {
            if (votetype == vote_map)
                mainconsole.console(
#ifdef SERVER
                    "No map has been voted",
#else
                ("No map has been voted"),
#endif
                    vote_message_color);
            stopvote();
        }
#ifdef SERVER
    }
#endif
}
#ifdef SERVER
void countvote(std::uint8_t voter)
{
    std::int32_t i;
    float edge;
    // Status: TMapInfo;

    if (voteactive && !votehasvoted[voter])
    {
        votenumvotes = votenumvotes + 1;
        votehasvoted[voter] = true;
        edge = (float)(votenumvotes) / votemaxvotes;
        if (edge >= ((float)(CVar::sv_votepercent) / 100))
        {
            if (votetype == vote_kick)
            {
                i = strtoint(votetarget);
                // There should be no permanent bans by votes. Reduced to 1 day.
                if (cheattag[i] == 0)
                    kickplayer(i, true, kick_voted, hour, "Vote Kicked");
                else
                    kickplayer(i, true, kick_voted, day, "Vote Kicked by Server");
                dobalancebots(1, sprite[i].player->team);
            }
            else if (votetype == vote_map)
            {
                if (!preparemapchange(votetarget))
                {
                    mainconsole.console(string("Map not found (") + votetarget + ')',
                                        warning_message_color);
                    mainconsole.console("No map has been voted", vote_message_color);
                }
            }
            stopvote();
            serversendvoteoff();
        }
    }
}
#endif

void showmapchangescoreboard()
{
    showmapchangescoreboard("EXIT*!*");
}

void showmapchangescoreboard(const std::string nextmap)
{
#ifndef SERVER
    std::int32_t i;
#endif

    mapchangename = nextmap;
    mapchangecounter = mapchangetime;
#ifndef SERVER
    gamemenushow(limbomenu, false);
    fragsmenushow = true;
    statsmenushow = false;
    for (i = 1; i <= max_players; i++)
    {
        if (sprite[i].active)
        {
            stopsound(sprite[i].reloadsoundchannel);
            stopsound(sprite[i].jetssoundchannel);
            stopsound(sprite[i].gattlingsoundchannel);
            stopsound(sprite[i].gattlingsoundchannel2);
        }
    }
#endif
}

bool isteamgame()
{
    bool isteamgame_result;
    switch (CVar::sv_gamemode)
    {
    case gamestyle_teammatch:
    case gamestyle_ctf:
    case gamestyle_inf:
    case gamestyle_htf:
        isteamgame_result = true;
        break;
    default:
        isteamgame_result = false;
    }
    return isteamgame_result;
}

void changemap()
{
#ifdef SERVER
    tvector2 a;
#endif
    std::int32_t j;
    std::int32_t secwep;
#ifndef SERVER
    tmapinfo mapchangestatus;
#endif

#ifdef SERVER
    //  try
    LogDebugG("ChangeMap");
    mapslist.clear();

    if (fileexists(userdirectory + std::string(CVar::sv_maplist)))
    {
        mapslist.loadfromfile(userdirectory + std::string(CVar::sv_maplist));
        auto it = std::remove(mapslist.begin(), mapslist.end(), "");
        mapslist.erase(it, mapslist.end());
    }

    for (auto &w : botpath.waypoint)
    {
        w.active = false;
        w.id = 0;
        w.pathnum = 0;
    }

    if (!map.loadmap(mapchange))
    {
        mainconsole.console(string("Error: Could not load map (") + mapchange.name + ')',
                            debug_message_color);
        nextmap();
        return;
    }
#endif
#ifndef SERVER
    mapchanged = true;
    demorecorder.stoprecord();

    if (getmapinfo(mapchangename, userdirectory, mapchangestatus) &&
        verifymapchecksum(mapchangestatus, mapchangechecksum))
    {
        if (!map.loadmap(mapchangestatus, CVar::r_forcebg, CVar::r_forcebg_color1,
                         CVar::r_forcebg_color2))
        {
            rendergameinfo(("Could not load map: ") + (mapchangename));
            exittomenu();
            return;
        }
        // Map.Name := MapChangeName;
    }
    else
    {
        exittomenu();
        joinserver();
        return;
    }
#endif

    for (auto &b : bullet)
    {
        b.kill();
    }
    for (auto &t : thing)
    {
        t.kill();
    }
#ifndef SERVER
    for (auto &s : spark)
    {
        s.kill();
    }
#endif

    {
        auto i = 0;
        for (auto &s : sprite)
        {
            i++;
            if (s.active && s.isnotspectator())
            {
                randomizestart(spriteparts.pos[i], s.player->team);
                s.respawn();
                s.player->kills = 0;
                s.player->deaths = 0;
                s.player->flags = 0;
                s.bonustime = 0;
                s.bonusstyle = bonus_none;
#ifndef SERVER
                s.selweapon = 0;
#endif
                s.freecontrols();
                s.weapon = guns[noweapon];

                secwep = s.player->secwep + 1;

                if ((secwep >= 1) && (secwep <= secondary_weapons) &&
                    (weaponactive[primary_weapons + secwep] == 1))
                    s.secondaryweapon = guns[primary_weapons + secwep];
                else
                    s.secondaryweapon = guns[noweapon];

                s.respawncounter = 0;
            }
        }
    }

#ifndef SERVER
    for (auto &w : weaponsel)
    {
        for (auto i = 1; i <= primary_weapons; i++)
        {
            w[i] = 1;
        }
    }
#endif

    if (CVar::sv_advancemode)
    {
        for (auto &w : weaponsel)
        {
            for (auto i = 1; i <= primary_weapons; i++)
            {
                w[i] = 0;
            }
        }

#ifndef SERVER
        if (mysprite > 0)
            for (auto i = 1; i <= main_weapons; i++)
                limbomenu->button[i - 1].active = (bool)(weaponsel[mysprite][i]);
#endif
    }

    for (auto i = 1; i <= 4; i++)
    {
        teamscore[i] = 0;
    }

    for (auto i = 1; i <= 2; i++)
    {
        teamflag[i] = 0;
    }

#ifndef SERVER
    fragsmenushow = false;
    statsmenushow = false;

    if (mysprite > 0)
        gamemenushow(limbomenu);
#endif

#ifdef SERVER
    // add yellow flag
    if ((CVar::sv_gamemode == gamestyle_pointmatch) || (CVar::sv_gamemode == gamestyle_htf))
    {
        randomizestart(a, 14);
        teamflag[1] = creatething(a, 255, object_pointmatch_flag, 255);
    }

    if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf))
    {
        // red flag
        if (randomizestart(a, 5))
            teamflag[1] = creatething(a, 255, object_alpha_flag, 255);

        // blue flag
        if (randomizestart(a, 6))
            teamflag[2] = creatething(a, 255, object_bravo_flag, 255);
    }

    if (CVar::sv_gamemode == gamestyle_rambo)
    {
        randomizestart(a, 15);
        creatething(a, 255, object_rambo_bow, 255);
    }

    if (!CVar::sv_survivalmode)
    {
        // spawn medikits
        spawnthings(object_medical_kit, map.medikits);

        // spawn grenadekits
        if (CVar::sv_maxgrenades > 0)
            spawnthings(object_grenade_kit, map.grenades);
    }

    // stat gun
    if (CVar::sv_stationaryguns)
    {
        for (auto &s : map.spawnpoints)
        {
            if (!s.active)
            {
                continue;
            }
            if (s.team != 16)
            {
                continue;
            }
            a.x = s.x;
            a.y = s.y;
            creatething(a, 255, object_stationary_gun, 255);
        }
    }
#endif
#ifndef SERVER
    heartbeattime = maintickcounter;
    heartbeattimewarnings = 0;

    if ((mysprite > 0) && sprite[mysprite].isnotspectator())
    {
        camerafollowsprite = mysprite;
    }
    else
    {
        // If in freecam or the previous followee is gone, then find a new followee
        if ((camerafollowsprite == 0) or !sprite[camerafollowsprite].active)
        {
            camerafollowsprite = getcameratarget(0);
            // If no appropriate player found, then just center the camera
            if (camerafollowsprite == 0)
            {
                camerax = 0;
                cameray = 0;
            }
        }
    }

    if (!escmenu->active)
    {
        mx = gamewidthhalf;
        my = gameheighthalf;
        mouseprev.x = mx;
        mouseprev.y = my;
    }

    // Spawn sound
    if (mysprite > 0)
        playsound(sfx_spawn, spriteparts.pos[mysprite]);

#endif
    // DEMO
    if (CVar::demo_autorecord)
    {
        if (demorecorder.active())
            demorecorder.stoprecord();

        NotImplemented(NITag::OTHER, "no current time function");
#if 0
        demorecorder.startrecord(userdirectory + "demos/" +
                                 formatdatetime("yyyy-mm-dd_hh-nn-ss_", now()) + map.name + ".sdm");
#endif
    }

    sortplayers();

    mapchangecounter = -60;

    timelimitcounter = CVar::sv_timelimit;

#ifdef SCRIPT
    scrptdispatcher.onaftermapchange(map.name);
#endif
#ifdef SERVER
    serverspritesnapshotmajor(netw);
    //  except
    //    on e: exception do
    //      WriteLn('Error changing map ' + e.message);
    //  end;
#endif
}

void sortplayers()
{
    std::int32_t j;
    tkillsort temp;

    playersnum = 0;
    botsnum = 0;
    spectatorsnum = 0;
    std::fill(std::begin(playersteamnum), std::end(playersteamnum), 0);

    for (auto &sp : sortedplayers)
    {
        sp.kills = 0;
        sp.deaths = 0;
        sp.flags = 0;
        sp.playernum = 0;
    }

    {
        auto i = 0;
        for (auto &s : sprite)
        {
            i++;
            if (s.active && (!s.player->demoplayer))
            {
                playersnum += 1;
                if (s.player->controlmethod == bot)
                    botsnum += 1;

                if (s.isspectator())
                    spectatorsnum += 1;

                if (s.isnotsolo() && s.isnotspectator())
                    playersteamnum[s.player->team] += 1;

                if (s.isnotspectator())
                {
                    sortedplayers[playersnum].kills = s.player->kills;
                    sortedplayers[playersnum].deaths = s.player->deaths;
                    sortedplayers[playersnum].flags = s.player->flags;
                    sortedplayers[playersnum].playernum = i;
                }
                else
                {
                    sortedplayers[playersnum].kills = 0;
                    sortedplayers[playersnum].deaths = std::numeric_limits<std::int32_t>::max();
                    sortedplayers[playersnum].flags = 0;
                    sortedplayers[playersnum].playernum = i;
                }

                // Kill Limit
                if (mapchangecounter < 1)
                    if (!isteamgame())
                        if (s.player->kills >= CVar::sv_killlimit)
                        {
#ifndef SERVER
                            camerafollowsprite = i;
                            if (escmenu->active)
                            {
                                mx = gamewidthhalf;
                                my = gameheighthalf;
                                mouseprev.x = mx;
                                mouseprev.y = my;
                            }
#else
                            nextmap();
#endif
                        }
            }
        }
    }

    // sort by caps first if new score board
    if (playersnum > 0)
        for (auto i = 1; i <= playersnum; i++)
        {
            for (j = i + 1; j <= playersnum; j++)
            {
                if (sortedplayers[j].flags > sortedplayers[i].flags)
                {
                    temp = sortedplayers[i];
                    sortedplayers[i] = sortedplayers[j];
                    sortedplayers[j] = temp;
                }
            }
        }

    // sort by kills
    if (playersnum > 0)
        for (auto i = 1; i <= playersnum; i++)
            for (j = i + 1; j <= playersnum; j++)
                if (sortedplayers[j].flags == sortedplayers[i].flags)
                    if (sortedplayers[j].kills > sortedplayers[i].kills)
                    {
                        temp = sortedplayers[i];
                        sortedplayers[i] = sortedplayers[j];
                        sortedplayers[j] = temp;
                    } // if

    // final sort by deaths
    if (playersnum > 0)
        for (auto i = 1; i <= playersnum; i++)
            for (j = i + 1; j <= playersnum; j++)
                if (sortedplayers[j].flags == sortedplayers[i].flags)
                    if (sortedplayers[j].kills == sortedplayers[i].kills)
                        if (sortedplayers[j].deaths < sortedplayers[i].deaths)
                        {
                            temp = sortedplayers[i];
                            sortedplayers[i] = sortedplayers[j];
                            sortedplayers[j] = temp;
                        }

#ifndef SERVER
    // Sort Team Score
    for (auto i = 1; i <= 4; i++)
    {
        sortedteamscore[i].kills = teamscore[i];
        sortedteamscore[i].playernum = i;
    }

    sortedteamscore[1].color =
        (std::uint32_t(CVar::ui_status_transparency) << 24) | 0xd20f05; // ARGB
    sortedteamscore[2].color = (std::uint32_t(CVar::ui_status_transparency) << 24) | 0x50fd2;
    sortedteamscore[3].color = (std::uint32_t(CVar::ui_status_transparency) << 24) | 0xd2d205;
    sortedteamscore[4].color = (std::uint32_t(CVar::ui_status_transparency) << 24) | 0x5d205;

    for (auto i = 1; i <= 4; i++)
        for (j = i + 1; j <= 4; j++)
            if (sortedteamscore[j].kills > sortedteamscore[i].kills)
            {
                temp = sortedteamscore[i];
                sortedteamscore[i] = sortedteamscore[j];
                sortedteamscore[j] = temp;
            }
#endif

#ifdef SERVER
    // Team - Kill Limit
    if (mapchangecounter < 1)
    {
        for (auto i = 1; i <= 4; i++)
        {
            if (teamscore[i] >= CVar::sv_killlimit)
            {
                nextmap();
                break;
            }
        }
    }
    // Wave respawn time
    updatewaverespawntime();
#endif

#ifndef SERVER
    teamplayersnum[0] = 0;
    teamplayersnum[1] = 0;
    teamplayersnum[2] = 0;
    teamplayersnum[3] = 0;
    teamplayersnum[4] = 0;
#else
    teamalivenum[1] = 0;
    teamalivenum[2] = 0;
    teamalivenum[3] = 0;
    teamalivenum[4] = 0;
#endif

    for (auto &s : sprite)
    {
        if (s.active)
        {
            if (team_alpha >= s.player->team && s.player->team <= team_delta)
#ifdef SERVER
                teamalivenum[s.player->team] += 1;
#else
                teamplayersnum[s.player->team] += 1;
#endif
        }
    }
}
