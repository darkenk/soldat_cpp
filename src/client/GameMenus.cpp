// automatically converted
#include "GameMenus.hpp"

/*#include "SDL2.h"*/
/*#include "SysUtils.h"*/
/*#include "Client.h"*/
/*#include "Weapons.h"*/
/*#include "Game.h"*/
/*#include "GameStrings.h"*/
/*#include "ClientGame.h"*/
/*#include "Sound.h"*/
/*#include "InterfaceGraphics.h"*/
/*#include "Constants.h"*/
/*#include "Net.h"*/
/*#include "NetworkClientConnection.h"*/
/*#include "NetworkClientSprite.h"*/
/*#include "Sprites.h"*/
/*#include "Cvar.h"*/
/*#include "NetworkClientGame.h"*/
/*#include "NetworkClientMessages.h"*/
/*#include "SteamTypes.h"*/
#include "Client.hpp"
#include "ClientGame.hpp"
#include "InterfaceGraphics.hpp"
#include "Sound.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "shared/misc/PortUtils.hpp"
#include "shared/misc/PortUtilsSoldat.hpp"
#include "shared/network/NetworkClient.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include "shared/network/NetworkClientGame.hpp"
#include "shared/network/NetworkClientMessages.hpp"
#include "shared/network/NetworkClientSprite.hpp"
#include <SDL2/SDL.h>

using string = std::string;

std::vector<tgamemenu> gamemenu;
pgamemenu hoveredmenu;
pgamebutton hoveredbutton;
std::int32_t hoveredbuttonindex;
pgamemenu escmenu;
pgamemenu teammenu;
pgamemenu limbomenu;
pgamemenu kickmenu;
pgamemenu mapmenu;
std::int32_t kickmenuindex = 0;
std::int32_t mapmenuindex = 0;

bool limbowasactive;

void initbutton(pgamemenu menu, std::int32_t button, const std::string &caption, std::int32_t x,
                std::int32_t y, std::int32_t w, std::int32_t h, bool active = true)
{
    menu->button.emplace_back();
    menu->button[button].active = active;
    menu->button[button].x1 = menu->x + x;
    menu->button[button].y1 = menu->y + y;
    menu->button[button].x2 = menu->x + x + w;
    menu->button[button].y2 = menu->y + y + h;
    menu->button[button].caption = caption;
}

void initgamemenus()
{
    std::int32_t i;
    std::string s;

    hoveredmenu = nullptr;
    hoveredbutton = nullptr;
    hoveredbuttonindex = 0;

    setlength(gamemenu, 5);
    escmenu = &gamemenu[0];
    teammenu = &gamemenu[1];
    limbomenu = &gamemenu[2];
    kickmenu = &gamemenu[3];
    mapmenu = &gamemenu[4];

    // esc menu

    escmenu->w = 300;
    escmenu->h = 200;

    if (CVar::r_scaleinterface)
    {
        escmenu->x = round((float)((gamewidth - escmenu->w)) / 2);
        escmenu->y = round((float)((gameheight - escmenu->h)) / 2);
    }
    else
    {
        escmenu->x = round((float)((renderwidth - escmenu->w)) / 2);
        escmenu->y = round((float)((renderheight - escmenu->h)) / 2);
    }

    //  SetLength(escmenu->Button, {$IFDEF STEAM}5{$ELSE}4{$ENDIF});
    initbutton(escmenu, 0, string("1 ") + ("Exit to menu"), 5, 1 * 25, 240, 25);
    initbutton(escmenu, 1, string("2 ") + ("Change map"), 5, 2 * 25, 240, 25);
    initbutton(escmenu, 2, string("3 ") + ("Kick player"), 5, 3 * 25, 240, 25);
    initbutton(escmenu, 3, string("4 ") + ("Change team"), 5, 4 * 25, 240, 25);
#ifdef STEAM
    initbutton(escmenu, 4, ("Server Website"), 5, 7 * 25, 240, 15);
#endif

    // team menu

    teammenu->w = 0;
    teammenu->h = 0;
    teammenu->x = 0;
    teammenu->y = 0;

    setlength(teammenu->button, 6);
    initbutton(teammenu, 0, string("0 ") + ("0 Player"), 40, 140 + 40 * 1, 215, 35);
    initbutton(teammenu, 1, string("1 ") + ("Alpha Team"), 40, 140 + 40 * 1, 215, 35);
    initbutton(teammenu, 2, string("2 ") + ("Bravo Team"), 40, 140 + 40 * 2, 215, 35);
    initbutton(teammenu, 3, string("3 ") + ("Charlie Team"), 40, 140 + 40 * 3, 215, 35);
    initbutton(teammenu, 4, string("4 ") + ("Delta Team"), 40, 140 + 40 * 4, 215, 35);
    initbutton(teammenu, 5, string("5 ") + ("Spectator"), 40, 140 + 40 * 5, 215, 35);

    // limbo menu

    limbomenu->w = 0;
    limbomenu->h = 0;
    limbomenu->x = 0;
    limbomenu->y = 0;

    setlength(limbomenu->button, main_weapons);

    for (i = 0; i <= main_weapons - 1; i++)
    {
        if (i < primary_weapons)
            s = (inttostr((i + 1) % 10)) + ' ' + (gundisplayname[guns[i + 1].num]);
        else
            s = (gundisplayname[guns[i + 1].num]);

        initbutton(limbomenu, i, s, 35, 154 + 18 * (i + ord(i >= primary_weapons)), 235, 16);
    }

    // kick menu

    kickmenu->w = 370;
    kickmenu->h = 90;
    kickmenu->x = 125;
    kickmenu->y = 355;

    setlength(kickmenu->button, 4);
    initbutton(kickmenu, 0, "<<<<", 15, 35, 90, 25);
    initbutton(kickmenu, 1, ">>>>", 265, 35, 90, 25);
    initbutton(kickmenu, 2, ("Kick"), 105, 55, 90, 25);
    initbutton(kickmenu, 3, ("Ban"), 195, 55, 80, 25);

    kickmenu->button[3].active = false; // TODO: ban not supported for now

    // map menu

    mapmenu->w = 370;
    mapmenu->h = 90;
    mapmenu->x = 125;
    mapmenu->y = 355;

    setlength(mapmenu->button, 3);
    initbutton(mapmenu, 0, "<<<<", 15, 35, 90, 25);
    initbutton(mapmenu, 1, ">>>>", 265, 35, 90, 25);
    initbutton(mapmenu, 2, ("Select"), 120, 55, 90, 25);
}

void hideall()
{
    std::int32_t i;

    for (i = low(gamemenu); i <= high(gamemenu); i++)
        gamemenu[i].active = false;
}

void gamemenushow(pgamemenu menu, bool show)
{
    std::int32_t i;

    if (menu == escmenu)
    {
        if (show)
        {
            if (limbomenu->active)
                limbowasactive = true;

            hideall();
            fragsmenushow = false;
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

            if (CVar::cl_runs < 3)
                noobshow = true;

            // TODO: stop playing weather in escmenu
        }
        else
        {
            hideall();
            noobshow = false;
            if (limbowasactive)
                limbomenu->active = true;
        }
    }
    else if ((menu == teammenu) && show)
    {
        hideall();

        if (show)
            switch (CVar::sv_gamemode)
            {
            case gamestyle_ctf:
            case gamestyle_inf:
            case gamestyle_htf: {
                menu->button[0].active = false;
                menu->button[1].active = true;
                menu->button[2].active = true;
                menu->button[3].active = false;
                menu->button[4].active = false;
            }
            break;
            case gamestyle_teammatch: {
                menu->button[0].active = false;
                menu->button[1].active = true;
                menu->button[2].active = true;
                menu->button[3].active = true;
                menu->button[4].active = true;
            }
            break;
            default: {
                menu->button[0].active = true;
                menu->button[1].active = false;
                menu->button[2].active = false;
                menu->button[3].active = false;
                menu->button[4].active = false;
            }
            }
    }
    else if ((menu == mapmenu) && show)
    {
        clientvotemap(mapmenuindex);
        kickmenu->active = false;
    }
    else if ((menu == kickmenu) && show)
    {
        kickmenuindex = 1;
        mapmenu->active = false;

        if (playersnum < 1)
            menu = nullptr;
    }
    else if (menu == limbomenu)
    {
        menu->active = false;

        if (!show)
            limbowasactive = false;
        else if (weaponsingame == 0)
            menu = nullptr;
    }

    if (menu != nullptr)
        menu->active = show;

    gamemenumousemove();
}

bool gamemenuaction(pgamemenu menu, std::int32_t buttonindex)
{
    std::int32_t i, count;

    bool result = false;

    if ((buttonindex >= 0) && (buttonindex < length(menu->button)) &&
        menu->button[buttonindex].active)
    {
        if (menu == escmenu)
        {
            result = true;

            switch (buttonindex)
            {
            case 0: {
                clientdisconnect();

                exit(0);
            }
            break;
            case 1:
                gamemenushow(mapmenu, !mapmenu->active);
                break;
            case 2:
                gamemenushow(kickmenu, !kickmenu->active);
                break;
            case 3: {
                result = (mysprite > 0) && (mapchangecounter < 0);

                if (result)
                {
                    gamemenushow(teammenu);
                    mapchangecounter = -60;
                    selteam = 0;
                }
                else if ((mysprite == 0) && isteamgame())
                {
                    result = true;
                    gamemenushow(teammenu);
                }
            }
            break;
#ifdef STEAM
            case 4: {
                if (CVar::sv_website != "")
                    steamapi.friends.activategameoverlaytowebpage(
                        (pchar)(CVar::sv_website), k_eactivategameoverlaytowebpagemode_default);
            }
            break;
#endif
            }
        }
        else if (menu == teammenu)
        {
            result = true;
            gamemenushow(menu, false);
            selteam = buttonindex;

            if ((mysprite == 0) || (buttonindex != sprite[mysprite].player->team))
            {
                // NOTE this actually sends a change team request
                clientsendplayerinfo();
            }
        }
        else if (menu == kickmenu)
        {
            i = kickmenuindex;

            if (playersnum < 1)
            {
                gamemenushow(kickmenu, false);
            }
            else
                switch (buttonindex)
                {
                case 0: { // prev
                    kickmenuindex = ((max_sprites + kickmenuindex - 2) % max_sprites) + 1;
                    while (
                        !(sprite[kickmenuindex].active or sprite[kickmenuindex].player->demoplayer))
                        kickmenuindex = ((max_sprites + kickmenuindex - 2) % max_sprites) + 1;

                    result = (kickmenuindex != i);
                }
                break;

                case 1: { // next
                    kickmenuindex = (kickmenuindex % max_sprites) + 1;
                    while (
                        !(sprite[kickmenuindex].active or sprite[kickmenuindex].player->demoplayer))
                        kickmenuindex = (kickmenuindex % max_sprites) + 1;

                    result = (kickmenuindex != i);
                }
                break;

                case 2: { // kick
                    result = (kickmenuindex != mysprite);

                    if (result)
                    {
                        gamemenushow(escmenu, false);
                        chattext = ' ';
                        chatchanged = true;
                        votekickreasontype = true;
                        SDL_StartTextInput();
                    }
                }
                break;
                }
        }
        else if (menu == mapmenu)
        {
            if (playersnum < 1)
            {
                gamemenushow(kickmenu, false);
            }
            else
                switch (buttonindex)
                {
                case 0: { // prev
                    if (mapmenuindex > 0)
                    {
                        mapmenuindex = mapmenuindex - 1;
                        clientvotemap(mapmenuindex);
                    }

                    result = (kickmenuindex != 0);
                }
                break;

                case 1: { // next
                    if (mapmenuindex < votemapcount - 1)
                    {
                        mapmenuindex = mapmenuindex + 1;
                        clientvotemap(mapmenuindex);
                    }

                    result = (mapmenuindex <= votemapcount - 1);
                }
                break;

                case 2: { // vote map
                    gamemenushow(escmenu, false);
                    clientsendstringmessage(string("votemap ") + (votemapname), msgtype_cmd);
                }
                break;
                }
        }
        else if ((menu == limbomenu) && (mysprite > 0))
        {
            result = true;
            i = buttonindex + 1;

            if ((weaponactive[i] == 1) && (weaponsel[mysprite][i] == 1))
            {
                if (i <= 10)
                {
                    if ((weaponactive[i] == 1) && (weaponsel[mysprite][i] == 1))
                        sprite[mysprite].selweapon = guns[i].num;

                    if (sprite[mysprite].selweapon > 0)
                    {
                        gamemenushow(limbomenu, false);
                        if (!sprite[mysprite].deadmeat and
                            sprite[mysprite].weapon.num != guns[bow].num and
                            sprite[mysprite].weapon.num != guns[bow2].num)
                        {
                            sprite[mysprite].applyweaponbynum(sprite[mysprite].selweapon, 1);
                            clientspritesnapshot();
                        }
                    }
                }
                else
                {
                    CVar::cl_player_secwep = (i - 11);
                    sprite[mysprite].player->secwep = i - 11;
                    sprite[mysprite].applyweaponbynum(guns[i].num, 2);

                    count = 0;
                    for (i = 1; i <= primary_weapons; i++)
                        count += weaponactive[i];

                    if (count == 0)
                    {
                        gamemenushow(limbomenu, false);
                        sprite[mysprite].weapon = sprite[mysprite].secondaryweapon;
                        sprite[mysprite].secondaryweapon = guns[noweapon];
                    }

                    if (!sprite[mysprite].deadmeat)
                        clientspritesnapshot();
                }
            }
        }

        if (result)
            playsound(sfx_menuclick);
    }
    return result;
}

void gamemenumousemove()
{
    std::int32_t i, j;
    float x, y;
    pgamebutton btn;

    hoveredmenu = nullptr;
    hoveredbutton = nullptr;
    hoveredbuttonindex = 0;

    x = mx * _rscala.x;
    y = my * _rscala.y;

    for (i = low(gamemenu); i <= high(gamemenu); i++)
    {
        if (gamemenu[i].active)
        {
            for (j = low(gamemenu[i].button); j <= high(gamemenu[i].button); j++)
            {
                btn = &gamemenu[i].button[j];

                if (btn->active && (x > btn->x1) && (x < btn->x2) && (y > btn->y1) && (y < btn->y2))
                {
                    hoveredmenu = &gamemenu[i];
                    hoveredbutton = btn;
                    hoveredbuttonindex = j;
                    return;
                }
            }
        }
    }
}

bool gamemenuclick()
{
    if (hoveredbutton != nullptr)
        return gamemenuaction(hoveredmenu, hoveredbuttonindex);
    return false;
}
