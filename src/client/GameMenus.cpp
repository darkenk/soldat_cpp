// automatically converted
#include "GameMenus.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <math.h>
#include <memory>

#include "Client.hpp"
#include "ClientGame.hpp"
#include "ControlGame.hpp"
#include "InterfaceGraphics.hpp"
#include "Input.hpp"
#include "Sound.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include "shared/network/NetworkClient.hpp"
#include "shared/network/NetworkClientConnection.hpp"
#include "shared/network/NetworkClientGame.hpp"
#include "shared/network/NetworkClientMessages.hpp"
#include "shared/network/NetworkClientSprite.hpp"
#include "common/Vector.hpp"
#include "common/WeaponSystem.hpp"
#include "common/Weapons.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "common/network/Net.hpp"
#include "shared/Constants.cpp.h"
#include "shared/mechanics/Sprites.hpp"
#include "shared/network/Net.hpp"

using string = std::string;

GlobalStateGameMenus gGlobalStateGameMenus{

};

void GlobalStateGameMenus::initbutton(pgamemenu menu, std::int32_t button,
                                      const std::string &caption, std::int32_t x, std::int32_t y,
                                      std::int32_t w, std::int32_t h, bool active)
{
  menu->button.emplace_back();
  menu->button[button].active = active;
  menu->button[button].x1 = menu->x + x;
  menu->button[button].y1 = menu->y + y;
  menu->button[button].x2 = menu->x + x + w;
  menu->button[button].y2 = menu->y + y + h;
  menu->button[button].caption = caption;
}

void GlobalStateGameMenus::initgamemenus()
{
  std::string s;

  hoveredmenu = nullptr;
  hoveredbutton = nullptr;
  hoveredbuttonindex = 0;

  setlength(gamemenu, 5);
  escmenu = gamemenu.data();
  teammenu = &gamemenu[1];
  limbomenu = &gamemenu[2];
  kickmenu = &gamemenu[3];
  mapmenu = &gamemenu[4];

  // esc menu

  escmenu->w = 300;
  escmenu->h = 200;

  if (CVar::r_scaleinterface)
  {
    escmenu->x = round((float)((gGlobalStateGame.gamewidth - escmenu->w)) / 2);
    escmenu->y = round((float)((gGlobalStateGame.gameheight - escmenu->h)) / 2);
  }
  else
  {
    escmenu->x = round((float)((gGlobalStateClientGame.renderwidth - escmenu->w)) / 2);
    escmenu->y = round((float)((gGlobalStateClientGame.renderheight - escmenu->h)) / 2);
  }

  initbutton(escmenu, 0, string("1 ") + ("Exit to menu"), 5, 1 * 25, 240, 25);
  initbutton(escmenu, 1, string("2 ") + ("Change map"), 5, 2 * 25, 240, 25);
  initbutton(escmenu, 2, string("3 ") + ("Kick player"), 5, 3 * 25, 240, 25);
  initbutton(escmenu, 3, string("4 ") + ("Change team"), 5, 4 * 25, 240, 25);

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

  for (std::int32_t i = 0; i <= main_weapons - 1; i++)
  {
    if (i < primary_weapons)
    {
      const auto& gun = GS::GetWeaponSystem().GetGuns()[i + 1];
      s = (inttostr((i + 1) % 10)) + ' ' + (gGlobalStateClient.gundisplayname[gun.num]);
    }
    else
    {
      s = (gGlobalStateClient.gundisplayname[GS::GetWeaponSystem().GetGuns()[i + 1].num]);
    }

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

void GlobalStateGameMenus::hideall()
{
  std::int32_t i;

  for (i = low(gGlobalStateGameMenus.gamemenu); i <= high(gGlobalStateGameMenus.gamemenu); i++)
  {
    gGlobalStateGameMenus.gamemenu[i].active = false;
  }
}

void GlobalStateGameMenus::gamemenushow(pgamemenu menu, bool show)
{
  if (menu == escmenu)
  {
    if (show)
    {
      if (limbomenu->active)
      {
        limbowasactive = true;
      }

      hideall();
      gGlobalStateInterfaceGraphics.fragsmenushow = false;
      gGlobalStateInterfaceGraphics.statsmenushow = false;

      for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
      {
        gGlobalStateSound.stopsound(sprite.reloadsoundchannel);
        gGlobalStateSound.stopsound(sprite.jetssoundchannel);
        gGlobalStateSound.stopsound(sprite.gattlingsoundchannel);
        gGlobalStateSound.stopsound(sprite.gattlingsoundchannel2);
      }

      if (CVar::cl_runs < 3)
      {
        gGlobalStateInterfaceGraphics.noobshow = true;
      }

      // TODO: stop playing weather in escmenu
    }
    else
    {
      hideall();
      gGlobalStateInterfaceGraphics.noobshow = false;
      if (limbowasactive)
      {
        limbomenu->active = true;
      }
    }
  }
  else if ((menu == teammenu) && show)
  {
    hideall();

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

    if (GS::GetGame().GetPlayersNum() < 1)
    {
      menu = nullptr;
    }
  }
  else if (menu == limbomenu)
  {
    menu->active = false;

    if (!show)
    {
      limbowasactive = false;
    }
    else if (GS::GetWeaponSystem().GetWeaponsInGame() == 0)
    {
      menu = nullptr;
    }
  }

  if (menu != nullptr)
  {
    menu->active = show;
  }

  gamemenumousemove();
}

auto GlobalStateGameMenus::gamemenuaction(pgamemenu menu, std::int32_t buttonindex) -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;

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
        clientdisconnect(*gGlobalStateNetworkClient.GetNetwork());
        SDL_Event evt;
        evt.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&evt);
      }
      break;
      case 1:
        gamemenushow(mapmenu, !mapmenu->active);
        break;
      case 2:
        gamemenushow(kickmenu, !kickmenu->active);
        break;
      case 3: {
        result = (sprite_system.IsPlayerSpriteValid()) && (GS::GetGame().GetMapchangecounter() < 0);

        if (result)
        {
          gamemenushow(teammenu);
          GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
          gGlobalStateClient.selteam = 0;
        }
        else if ((!sprite_system.IsPlayerSpriteValid()) && GS::GetGame().isteamgame())
        {
          result = true;
          gamemenushow(teammenu);
        }
      }
      break;
      }
    }
    else if (menu == teammenu)
    {
      result = true;
      gamemenushow(menu, false);
      gGlobalStateClient.selteam = buttonindex;

      if ((!sprite_system.IsPlayerSpriteValid()) ||
          (buttonindex != sprite_system.GetPlayerSprite().player->team))
      {
        // NOTE this actually sends a change team request
        clientsendplayerinfo();
      }
    }
    else if (menu == kickmenu)
    {
      i = kickmenuindex;

      if (GS::GetGame().GetPlayersNum() < 1)
      {
        gamemenushow(kickmenu, false);
      }
      else
      {
        switch (buttonindex)
        {
        case 0: { // prev
          kickmenuindex = ((max_sprites + kickmenuindex - 2) % max_sprites) + 1;
          while (!(sprite_system.GetSprite(kickmenuindex).IsActive() or
                   sprite_system.GetSprite(kickmenuindex).player->demoplayer))
          {
            kickmenuindex = ((max_sprites + kickmenuindex - 2) % max_sprites) + 1;
          }

          result = (kickmenuindex != i);
        }
        break;

        case 1: { // next
          kickmenuindex = (kickmenuindex % max_sprites) + 1;
          while (!(sprite_system.GetSprite(kickmenuindex).IsActive() or
                   sprite_system.GetSprite(kickmenuindex).player->demoplayer))
          {
            kickmenuindex = (kickmenuindex % max_sprites) + 1;
          }

          result = (kickmenuindex != i);
        }
        break;

        case 2: { // kick
          result = (!sprite_system.IsPlayerSprite(kickmenuindex));

          if (result)
          {
            gamemenushow(escmenu, false);
            gGlobalStateClientGame.chattext = ' ';
            gGlobalStateClientGame.chatchanged = true;
            gGlobalStateControlGame.votekickreasontype = true;
            SDL_StartTextInput(gGlobalStateInput.gamewindow);
          }
        }
        break;
        }
      }
    }
    else if (menu == mapmenu)
    {
      if (GS::GetGame().GetPlayersNum() < 1)
      {
        gamemenushow(kickmenu, false);
      }
      else
      {
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
          if (mapmenuindex < gGlobalStateNetworkClient.votemapcount - 1)
          {
            mapmenuindex = mapmenuindex + 1;
            clientvotemap(mapmenuindex);
          }

          result = (mapmenuindex <= gGlobalStateNetworkClient.votemapcount - 1);
        }
        break;

        case 2: { // vote map
          gamemenushow(escmenu, false);
          clientsendstringmessage(string("votemap ") + (gGlobalStateNetworkClient.votemapname),
                                  msgtype_cmd);
        }
        break;
        }
      }
    }
    else if ((menu == limbomenu) && (sprite_system.IsPlayerSpriteValid()))
    {
      result = true;
      i = buttonindex + 1;
      auto &weaponSystem = GS::GetWeaponSystem();

      if (weaponSystem.IsEnabled(i) &&
          (GS::GetGame().GetWeaponsel()[gGlobalStateClient.mysprite][i] == 1))
      {
        if (i <= 10)
        {
          if (weaponSystem.IsEnabled(i) &&
              (GS::GetGame().GetWeaponsel()[gGlobalStateClient.mysprite][i] == 1))
          {
            sprite_system.GetPlayerSprite().selweapon = GS::GetWeaponSystem().GetGuns()[i].num;
          }

          if (sprite_system.GetPlayerSprite().selweapon > 0)
          {
            gamemenushow(limbomenu, false);
            if (!sprite_system.GetPlayerSprite().deadmeat and
                sprite_system.GetPlayerSprite().weapon.num != bow_num and
                sprite_system.GetPlayerSprite().weapon.num != bow2_num)
            {
              sprite_system.GetPlayerSprite().applyweaponbynum(
                sprite_system.GetPlayerSprite().selweapon, 1);
              clientspritesnapshot();
            }
          }
        }
        else
        {
          CVar::cl_player_secwep = (i - 11);
          sprite_system.GetPlayerSprite().player->secwep = i - 11;
          sprite_system.GetPlayerSprite().applyweaponbynum(GS::GetWeaponSystem().GetGuns()[i].num,
                                                           2);

          auto count = weaponSystem.CountEnabledPrimaryWeapons();

          if (count == 0)
          {
            gamemenushow(limbomenu, false);
            sprite_system.GetPlayerSprite().SetFirstWeapon(
              sprite_system.GetPlayerSprite().secondaryweapon);
            sprite_system.GetPlayerSprite().SetSecondWeapon(
              GS::GetWeaponSystem().GetGuns()[noweapon]);
          }

          if (!sprite_system.GetPlayerSprite().deadmeat)
          {
            clientspritesnapshot();
          }
        }
      }
    }

    if (result)
    {
      gGlobalStateSound.playsound(SfxEffect::menuclick);
    }
  }
  return result;
}

void GlobalStateGameMenus::gamemenumousemove()
{
  std::int32_t i;
  std::int32_t j;
  float x;
  float y;
  pgamebutton btn;

  hoveredmenu = nullptr;
  hoveredbutton = nullptr;
  hoveredbuttonindex = 0;

  x = gGlobalStateClientGame.mx * gGlobalStateInterfaceGraphics._rscala.x;
  y = gGlobalStateClientGame.my * gGlobalStateInterfaceGraphics._rscala.y;

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

auto GlobalStateGameMenus::gamemenuclick() -> bool
{
  if (hoveredbutton != nullptr)
  {
    return gamemenuaction(hoveredmenu, hoveredbuttonindex);
  }
  return false;
}
