// automatically converted

#include "ClientGame.hpp"

#include "Client.hpp"
#include "GameMenus.hpp"
#include "GameRendering.hpp"
#include "Gfx.hpp"
#include "InterfaceGraphics.hpp"
#include "UpdateFrame.hpp"
#include "shared/Cvar.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include "shared/network/NetworkClientSprite.hpp"
#include <SDL2/SDL.h>
#include <Tracy.hpp>
#include <thread>

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

using string = std::string;

tvector2 mouseprev;
float mx, my;
bool mapchanged = false;
bool chatchanged = true; // used for blinking chat input
// DK_FIXME set it to true for now. I want to see something on screen
bool shouldrenderframes = true; // false during game request phase

// us std::uint8_t  action snap
std::uint8_t actionsnap = 1;
bool actionsnaptaken = false;
std::int32_t capscreen = 255;
std::uint8_t showscreen = false;
std::uint8_t screencounter = 255;

// resolution
bool isfullscreen;
std::int32_t screenwidth = default_width;
std::int32_t screenheight = default_height;
std::int32_t renderwidth = 0;
std::int32_t renderheight = 0;
std::int32_t windowwidth = 0;
std::int32_t windowheight = 0;

// cha std::uint8_t f
std::string chattext, lastchattext, firechattext;
std::uint8_t chattype;
std::string completionbase = "";
std::int32_t completionbaseseparator;
std::uint8_t currenttabcompleteplayer = 0;
std::uint8_t cursorposition = 0;
bool tabcompletepressed;
std::int32_t chattimecounter;

std::int32_t clientstopmovingcounter = 99999;
bool forceclientspritesnapshotmov;
std::uint32_t lastforceclientspritesnapshotmovtick;
std::int32_t menutimer;

struct tframetiming
{
    std::int64_t frequency;
    std::int64_t starttime;
    double prevtime;
    double prevrendertime;
    double accumulator;
    double mindeltatime;
    double elapsed;
    std::int32_t counter;
    std::int32_t fps;
    double fpsaccum;
};

tframetiming frametiming;

void resetframetiming()
{
    frametiming.frequency = SDL_GetPerformanceFrequency();
    frametiming.starttime = SDL_GetPerformanceCounter();

    frametiming.prevtime = getcurrenttime();
    frametiming.prevrendertime = frametiming.prevtime;
    frametiming.accumulator = 0;
    frametiming.mindeltatime = 0;
    frametiming.elapsed = 0;

    frametiming.counter = 0;
    frametiming.fps = 0;
    frametiming.fpsaccum = 0;

    if (CVar::r_fpslimit)
        frametiming.mindeltatime = 1.0 / CVar::r_maxfps;

    GS::GetGame().SetTickTime(0);
    GS::GetGame().SetTickTimeLast(0);
}

float getcurrenttime()
{
    auto x = SDL_GetPerformanceCounter();
    return (float)((x - frametiming.starttime)) / frametiming.frequency;
}

void bigmessage(const std::string &text, std::int32_t delay, std::uint32_t col)
{
    float w, s;

    gfxtextpixelratio(vector2(1, 1));
    setfontstyle(font_big);

    w = rectwidth(gfxtextmetrics(text));
    s = 4.8 * ((float)(renderheight) / 480);

    bigx[1] = 0;
    bigtext[1] = text;
    bigdelay[1] = delay;
    bigscale[1] = std::fmin(1 / 4.8, (0.7 * renderwidth / w) / s);
    bigcolor[1] = col;
    bigposx[1] = (float)((renderwidth - s * w * bigscale[1])) / 2;
    bigposy[1] = 420 * _iscala.y;

    if (CVar::r_scaleinterface)
        bigposx[1] = bigposx[1] * ((float)(gamewidth) / renderwidth);
}

// In-game nickname tab completion
void tabcomplete()
{
    std::int32_t i;
    std::int32_t chattextlen, completionbaselen;
    std::int32_t offset, lastseparator;
    std::int32_t continuedtabcompleteplayer, next, availablechatspace;
    std::string spacefittedname;

    if (mysprite < 1)
        return;

    chattextlen = length(chattext);

    if ((chattextlen > 1) && (chattext[2] == '^'))
        offset = 1;
    else
        offset = 0;

    // If not already tab-completing, save and use this base text for tab completetion
    if (currenttabcompleteplayer == 0)
    {
        NotImplemented(NITag::OTHER, "string operation");
#if 0
        // Find where the current std::uint64_t starts
        lastseparator = lastdelimiter(' ', string(chattext));

        if (lastseparator < offset)
            lastseparator = offset;

        completionbaselen = chattextlen - lastseparator;
        completionbase = ansimidstr(string(chattext), lastseparator + 1, completionbaselen);
        completionbaseseparator = lastseparator;
#endif
    }

    // Next potential match
    continuedtabcompleteplayer = (currenttabcompleteplayer + 1) % max_players;

    if (chattextlen > offset) // Dont complete if chat is empty
    {
        for (i = continuedtabcompleteplayer; i <= (continuedtabcompleteplayer + max_players - 1);
             i++)
        {
            next = ((i - 1) % max_players) + 1;
            auto &sprite = SpriteSystem::Get().GetSprite(next);
            if (sprite.IsActive() && (!sprite.player->demoplayer) && (next != mysprite))
            {
                if ((completionbase == "") ||
                    std::string::npos != sprite.player->name.find(completionbase))
                {
                    availablechatspace = maxchattext - completionbaseseparator;
                    spacefittedname = sprite.player->name.substr(0, availablechatspace);
                    chattext = chattext.substr(0, completionbaseseparator) + spacefittedname;
                    currenttabcompleteplayer = next;
                    cursorposition = length((chattext));
                    tabcompletepressed = true;
                    break;
                }
            }
        }
    }
}

// Resets the stats of all weapons
void resetweaponstats()
{
    for (auto &w : wepstats)
    {
        w.shots = 0;
        w.hits = 0;
        w.kills = 0;
        w.headshots = 0;
        w.accuracy = 0;
    }
}

std::int32_t getgamefps()
{
    return frametiming.fps;
}

void gameloop()
{
    ZoneScopedN("gameloop");
    std::int32_t maincontrol;
    std::int32_t heavysendersnum;
    float adjust;
    double currenttime, frametime, simtime;
    double framepercent, dt;
    bool gamepaused;

    gamepaused = (GS::GetGame().GetMapchangecounter() >= 0);

    currenttime = getcurrenttime();

    frametime = currenttime - frametiming.prevtime;

    frametiming.fpsaccum = frametiming.fpsaccum + frametime;

    auto &game = GS::GetGame();

    frametiming.prevtime = currenttime;
    game.SetTickTimeLast(game.GetTickTime());

    if (frametime > 2)
        frametime = 0;

    dt = (float)(1) / game.GetGoalTicks();

    frametiming.accumulator = frametiming.accumulator + frametime;
    game.SetTickTime(game.GetTickTime() + trunc((float)(frametiming.accumulator) / dt));

    simtime = (game.GetTickTime() - game.GetTickTimeLast()) * dt;
    frametiming.accumulator = frametiming.accumulator - simtime;
    framepercent = std::fmin(1, std::fmax(0, (float)(frametiming.accumulator) / dt));

    for (maincontrol = 1; maincontrol <= (game.GetTickTime() - game.GetTickTimeLast());
         maincontrol++)
    { // frame rate independant code
        if (!gamepaused)
            frametiming.elapsed = frametiming.elapsed + ((float)(1) / default_goalticks);

        clienttickcount += 1;
        // Update main tick counter
        maintickcounter += 1;

        if (menutimer > -1)
            menutimer -= 1;

        // General game updating
        update_frame();

        if (demorecorder.active() && (maintickcounter % CVar::demo_rate == 0))
            demorecorder.saveposition();

        if ((game.GetMapchangecounter() < 0) && (escmenu->active))
        {
            // DEMO
            if (demorecorder.active())
                demorecorder.savenextframe();
            if (demoplayer.active())
                demoplayer.processdemo();
        }

        // Radio Cooldown
        if ((maintickcounter % second == 0) && (radiocooldown > 0) && (CVar::sv_radio))
            radiocooldown -= 1;

        // Packet rate send adjusting
        if (packetadjusting == 1)
        {
            heavysendersnum = playersnum - spectatorsnum;

            if (heavysendersnum < 5)
                adjust = 0.75;
            else if (heavysendersnum < 9)
                adjust = 0.87;
            else
                adjust = 1.0;
        }
        else
            adjust = 1.0;

        if ((mysprite > 0) && (!demoplayer.active()))
        {
            // connection problems
            if ((game.GetMapchangecounter() < 0) && escmenu->active)
                noheartbeattime += 1;

            if (noheartbeattime > connectionproblem_time)
            {
                if (maintickcounter % 120 == 0)
                {
                    if (noheartbeattime > disconnection_time)
                    {
                        GetMainConsole().console(("Connection timeout"), warning_message_color);
                    }
                    else
                    {
                        GetMainConsole().console(("Connection problem"), warning_message_color);
                    }
                }

                clientstopmovingcounter = 0;
            }

            if (noheartbeattime == disconnection_time)
            {
                GS::GetGame().showmapchangescoreboard();

                gamemenushow(teammenu, false);

                GetMainConsole().console(("Connection timeout"), warning_message_color);

                clientdisconnect();
            }

            if (noheartbeattime < 0)
                noheartbeattime = 0;

            clientstopmovingcounter -= 1;

            auto &sprite = SpriteSystem::Get().GetSprite(mysprite);

            if (connection == INTERNET)
            {
                if (sprite.IsActive())
                {
                    if (!sprite.deadmeat)
                    {
                        if ((maintickcounter % (std::int32_t)round(7 * adjust) == 1) &&
                            (maintickcounter % (std::int32_t)round(5 * adjust) != 0))
                            clientspritesnapshot();
                        if ((maintickcounter % (std::int32_t)round(5 * adjust) == 0) ||
                            forceclientspritesnapshotmov)
                            clientspritesnapshotmov();
                    }
                    else if (maintickcounter % (std::int32_t)round(30 * adjust) == 0)
                        clientspritesnapshotdead();
                }
            }
            else if (connection == LAN)
            {
                if (!sprite.deadmeat)
                {
                    if (maintickcounter % (std::int32_t)round(4 * adjust) == 0)
                        clientspritesnapshot();

                    if ((maintickcounter % (std::int32_t)round(3 * adjust) == 0) ||
                        forceclientspritesnapshotmov)
                        clientspritesnapshotmov();
                }
                else if (maintickcounter % (std::int32_t)round(15 * adjust) == 0)
                    clientspritesnapshotdead();
            }

            forceclientspritesnapshotmov = false;
        } // playing

        // UDP.FlushMsg;
    } // Client

    // this shouldn't happen but still done for safety
    if (frametiming.prevrendertime > currenttime)
        frametiming.prevrendertime = currenttime - frametiming.mindeltatime;

    if (shouldrenderframes &&
        ((currenttime - frametiming.prevrendertime) >= frametiming.mindeltatime))
    {
        frametiming.prevrendertime = currenttime;
        frametiming.counter += 1;

        if (frametiming.counter >= 30)
        {
            frametiming.fps = round((float)(frametiming.counter) / frametiming.fpsaccum);
            frametiming.counter = 0;
            frametiming.fpsaccum = 0;
        }

        if (gamepaused)
            renderframe(frametiming.elapsed, framepercent, true);
        else
            renderframe(frametiming.elapsed - dt * (1 - framepercent), framepercent, false);
    }

    if ((game.GetMapchangecounter() < 0) && (game.GetMapchangecounter() > -59))
    {
        if (game.GetMapchangename() == "EXIT*!*")
        {
            exittomenu();
        }
    }

    if (mapchanged)
    {
        mapchanged = false;
        resetframetiming();
    }

    if (CVar::r_sleeptime > 0)
    {
        ZoneScopedN("Sleeping");
        std::this_thread::sleep_for(std::chrono::milliseconds((std::int32_t)CVar::r_sleeptime));
    }
}

std::uint8_t getcameratarget(bool backwards)
{
    std::uint8_t newcam, numloops;
    bool validcam;

    validcam = false;
    newcam = camerafollowsprite;
    numloops = 0;

    do
    {
        numloops += 1;
        if (numloops == 33)
        { // Shit, way too many loops...
            newcam = 0;
            validcam = true;
            break;
        }

        if (!backwards)
            newcam += 1;
        else
            newcam -= 1;
        if (newcam > max_sprites)
            newcam = 1;
        else if (newcam < 1)
            newcam = max_sprites;

        if (!SpriteSystem::Get().GetSprite(newcam).active)
            continue; // Sprite slot empty
        if (SpriteSystem::Get().GetSprite(newcam).deadmeat)
            continue; // Sprite is dead
        if (SpriteSystem::Get().GetSprite(newcam).isspectator())
            continue; // Sprite is a spectator

        if (SpriteSystem::Get().GetSprite(mysprite).control.up && (!CVar::sv_realisticmode) &&
            SpriteSystem::Get().GetSprite(mysprite).isnotspectator())
        {
            newcam = 0;
            validcam = true;
            break;
        } // Freecam if not Realistic

        if (SpriteSystem::Get().GetSprite(mysprite).isspectator())
        {
            if (SpriteSystem::Get().GetSprite(mysprite).control.up)
            {
                newcam = 0;
                validcam = true;
                break;
            }
            else
            { // Allow spectators to go into Free Cam
                validcam = true;
                break;
            } // Let spectator view all players
        }

        if (SpriteSystem::Get().GetSprite(newcam).isnotinsameteam(
                SpriteSystem::Get().GetSprite(mysprite)))
            continue; // Dont swap camera to a player not on my team

        validcam = true;
    } while (!validcam);

    return iif(validcam, newcam, camerafollowsprite);
}

#ifdef STEAM
void getmicdata()
{
    evoiceresult availablevoice;
    std::uint32_t availablevoicebytes;
    array of std::uint8_t voicedata;

    availablevoice = steamapi.user.getavailablevoice(&availablevoicebytes, nullptr, 0);

    if ((availablevoice == k_evoiceresultok) && (availablevoicebytes > 0))
    {
        setlength(voicedata, availablevoicebytes);
        availablevoice =
            steamapi.user.getvoice(true, voicedata, availablevoicebytes, &availablevoicebytes,
                                   false, nullptr, 0, nullptr, 0);

        if ((availablevoice == k_evoiceresultok) && (availablevoicebytes > 0))
            clientsendvoicedata(voicedata, availablevoicebytes);
    }
}
#endif
