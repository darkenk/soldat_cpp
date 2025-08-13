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
  .gamemenu{},
  .hoveredmenu{},
  .hoveredbutton{},
  .hoveredbuttonindex{},
  .escmenu{},
  .teammenu{},
  .limbomenu{},
  .kickmenu{},
  .mapmenu{},
  .kickmenuindex = 0,
  .mapmenuindex = 0,
};

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

void GlobalStateGameMenus::initgamemenus()
{
  std::string s;

  gGlobalStateGameMenus.hoveredmenu = nullptr;
  gGlobalStateGameMenus.hoveredbutton = nullptr;
  gGlobalStateGameMenus.hoveredbuttonindex = 0;

  setlength(gGlobalStateGameMenus.gamemenu, 5);
  gGlobalStateGameMenus.escmenu = gGlobalStateGameMenus.gamemenu.data();
  gGlobalStateGameMenus.teammenu = &gGlobalStateGameMenus.gamemenu[1];
  gGlobalStateGameMenus.limbomenu = &gGlobalStateGameMenus.gamemenu[2];
  gGlobalStateGameMenus.kickmenu = &gGlobalStateGameMenus.gamemenu[3];
  gGlobalStateGameMenus.mapmenu = &gGlobalStateGameMenus.gamemenu[4];

  // esc menu

  gGlobalStateGameMenus.escmenu->w = 300;
  gGlobalStateGameMenus.escmenu->h = 200;

  if (CVar::r_scaleinterface)
  {
    gGlobalStateGameMenus.escmenu->x =
      round((float)((gGlobalStateGame.gamewidth - gGlobalStateGameMenus.escmenu->w)) / 2);
    gGlobalStateGameMenus.escmenu->y =
      round((float)((gGlobalStateGame.gameheight - gGlobalStateGameMenus.escmenu->h)) / 2);
  }
  else
  {
    gGlobalStateGameMenus.escmenu->x =
      round((float)((gGlobalStateClientGame.renderwidth - gGlobalStateGameMenus.escmenu->w)) / 2);
    gGlobalStateGameMenus.escmenu->y =
      round((float)((gGlobalStateClientGame.renderheight - gGlobalStateGameMenus.escmenu->h)) / 2);
  }

  initbutton(gGlobalStateGameMenus.escmenu, 0, string("1 ") + ("Exit to menu"), 5, 1 * 25, 240, 25);
  initbutton(gGlobalStateGameMenus.escmenu, 1, string("2 ") + ("Change map"), 5, 2 * 25, 240, 25);
  initbutton(gGlobalStateGameMenus.escmenu, 2, string("3 ") + ("Kick player"), 5, 3 * 25, 240, 25);
  initbutton(gGlobalStateGameMenus.escmenu, 3, string("4 ") + ("Change team"), 5, 4 * 25, 240, 25);

  // team menu

  gGlobalStateGameMenus.teammenu->w = 0;
  gGlobalStateGameMenus.teammenu->h = 0;
  gGlobalStateGameMenus.teammenu->x = 0;
  gGlobalStateGameMenus.teammenu->y = 0;

  setlength(gGlobalStateGameMenus.teammenu->button, 6);
  initbutton(gGlobalStateGameMenus.teammenu, 0, string("0 ") + ("0 Player"), 40, 140 + 40 * 1, 215,
             35);
  initbutton(gGlobalStateGameMenus.teammenu, 1, string("1 ") + ("Alpha Team"), 40, 140 + 40 * 1,
             215, 35);
  initbutton(gGlobalStateGameMenus.teammenu, 2, string("2 ") + ("Bravo Team"), 40, 140 + 40 * 2,
             215, 35);
  initbutton(gGlobalStateGameMenus.teammenu, 3, string("3 ") + ("Charlie Team"), 40, 140 + 40 * 3,
             215, 35);
  initbutton(gGlobalStateGameMenus.teammenu, 4, string("4 ") + ("Delta Team"), 40, 140 + 40 * 4,
             215, 35);
  initbutton(gGlobalStateGameMenus.teammenu, 5, string("5 ") + ("Spectator"), 40, 140 + 40 * 5, 215,
             35);

  // limbo menu

  gGlobalStateGameMenus.limbomenu->w = 0;
  gGlobalStateGameMenus.limbomenu->h = 0;
  gGlobalStateGameMenus.limbomenu->x = 0;
  gGlobalStateGameMenus.limbomenu->y = 0;

  setlength(gGlobalStateGameMenus.limbomenu->button, main_weapons);

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

    initbutton(gGlobalStateGameMenus.limbomenu, i, s, 35,
               154 + 18 * (i + ord(i >= primary_weapons)), 235, 16);
  }

  // kick menu

  gGlobalStateGameMenus.kickmenu->w = 370;
  gGlobalStateGameMenus.kickmenu->h = 90;
  gGlobalStateGameMenus.kickmenu->x = 125;
  gGlobalStateGameMenus.kickmenu->y = 355;

  setlength(gGlobalStateGameMenus.kickmenu->button, 4);
  initbutton(gGlobalStateGameMenus.kickmenu, 0, "<<<<", 15, 35, 90, 25);
  initbutton(gGlobalStateGameMenus.kickmenu, 1, ">>>>", 265, 35, 90, 25);
  initbutton(gGlobalStateGameMenus.kickmenu, 2, ("Kick"), 105, 55, 90, 25);
  initbutton(gGlobalStateGameMenus.kickmenu, 3, ("Ban"), 195, 55, 80, 25);

  gGlobalStateGameMenus.kickmenu->button[3].active = false; // TODO: ban not supported for now

  // map menu

  gGlobalStateGameMenus.mapmenu->w = 370;
  gGlobalStateGameMenus.mapmenu->h = 90;
  gGlobalStateGameMenus.mapmenu->x = 125;
  gGlobalStateGameMenus.mapmenu->y = 355;

  setlength(gGlobalStateGameMenus.mapmenu->button, 3);
  initbutton(gGlobalStateGameMenus.mapmenu, 0, "<<<<", 15, 35, 90, 25);
  initbutton(gGlobalStateGameMenus.mapmenu, 1, ">>>>", 265, 35, 90, 25);
  initbutton(gGlobalStateGameMenus.mapmenu, 2, ("Select"), 120, 55, 90, 25);
}

void hideall()
{
  std::int32_t i;

  for (i = low(gGlobalStateGameMenus.gamemenu); i <= high(gGlobalStateGameMenus.gamemenu); i++)
  {
    gGlobalStateGameMenus.gamemenu[i].active = false;
  }
}

void GlobalStateGameMenus::gamemenushow(pgamemenu menu, bool show)
{
  if (menu == gGlobalStateGameMenus.escmenu)
  {
    if (show)
    {
      if (gGlobalStateGameMenus.limbomenu->active)
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
        gGlobalStateGameMenus.limbomenu->active = true;
      }
    }
  }
  else if ((menu == gGlobalStateGameMenus.teammenu) && show)
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
  else if ((menu == gGlobalStateGameMenus.mapmenu) && show)
  {
    clientvotemap(gGlobalStateGameMenus.mapmenuindex);
    gGlobalStateGameMenus.kickmenu->active = false;
  }
  else if ((menu == gGlobalStateGameMenus.kickmenu) && show)
  {
    gGlobalStateGameMenus.kickmenuindex = 1;
    gGlobalStateGameMenus.mapmenu->active = false;

    if (GS::GetGame().GetPlayersNum() < 1)
    {
      menu = nullptr;
    }
  }
  else if (menu == gGlobalStateGameMenus.limbomenu)
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

  gGlobalStateGameMenus.gamemenumousemove();
}

auto GlobalStateGameMenus::gamemenuaction(pgamemenu menu, std::int32_t buttonindex) -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;

  bool result = false;

  if ((buttonindex >= 0) && (buttonindex < length(menu->button)) &&
      menu->button[buttonindex].active)
  {
    if (menu == gGlobalStateGameMenus.escmenu)
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
        gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.mapmenu,
                                           !gGlobalStateGameMenus.mapmenu->active);
        break;
      case 2:
        gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.kickmenu,
                                           !gGlobalStateGameMenus.kickmenu->active);
        break;
      case 3: {
        result = (sprite_system.IsPlayerSpriteValid()) && (GS::GetGame().GetMapchangecounter() < 0);

        if (result)
        {
          gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.teammenu);
          GS::GetGame().SetMapchangecounter(GS::GetGame().GetMapchangecounter() - 60);
          gGlobalStateClient.selteam = 0;
        }
        else if ((!sprite_system.IsPlayerSpriteValid()) && GS::GetGame().isteamgame())
        {
          result = true;
          gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.teammenu);
        }
      }
      break;
      }
    }
    else if (menu == gGlobalStateGameMenus.teammenu)
    {
      result = true;
      gGlobalStateGameMenus.gamemenushow(menu, false);
      gGlobalStateClient.selteam = buttonindex;

      if ((!sprite_system.IsPlayerSpriteValid()) ||
          (buttonindex != sprite_system.GetPlayerSprite().player->team))
      {
        // NOTE this actually sends a change team request
        clientsendplayerinfo();
      }
    }
    else if (menu == gGlobalStateGameMenus.kickmenu)
    {
      i = gGlobalStateGameMenus.kickmenuindex;

      if (GS::GetGame().GetPlayersNum() < 1)
      {
        gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.kickmenu, false);
      }
      else
      {
        switch (buttonindex)
        {
        case 0: { // prev
          gGlobalStateGameMenus.kickmenuindex =
            ((max_sprites + gGlobalStateGameMenus.kickmenuindex - 2) % max_sprites) + 1;
          while (!(sprite_system.GetSprite(gGlobalStateGameMenus.kickmenuindex).IsActive() or
                   sprite_system.GetSprite(gGlobalStateGameMenus.kickmenuindex).player->demoplayer))
          {
            gGlobalStateGameMenus.kickmenuindex =
              ((max_sprites + gGlobalStateGameMenus.kickmenuindex - 2) % max_sprites) + 1;
          }

          result = (gGlobalStateGameMenus.kickmenuindex != i);
        }
        break;

        case 1: { // next
          gGlobalStateGameMenus.kickmenuindex =
            (gGlobalStateGameMenus.kickmenuindex % max_sprites) + 1;
          while (!(sprite_system.GetSprite(gGlobalStateGameMenus.kickmenuindex).IsActive() or
                   sprite_system.GetSprite(gGlobalStateGameMenus.kickmenuindex).player->demoplayer))
          {
            gGlobalStateGameMenus.kickmenuindex =
              (gGlobalStateGameMenus.kickmenuindex % max_sprites) + 1;
          }

          result = (gGlobalStateGameMenus.kickmenuindex != i);
        }
        break;

        case 2: { // kick
          result = (!sprite_system.IsPlayerSprite(gGlobalStateGameMenus.kickmenuindex));

          if (result)
          {
            gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.escmenu, false);
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
    else if (menu == gGlobalStateGameMenus.mapmenu)
    {
      if (GS::GetGame().GetPlayersNum() < 1)
      {
        gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.kickmenu, false);
      }
      else
      {
        switch (buttonindex)
        {
        case 0: { // prev
          if (gGlobalStateGameMenus.mapmenuindex > 0)
          {
            gGlobalStateGameMenus.mapmenuindex = gGlobalStateGameMenus.mapmenuindex - 1;
            clientvotemap(gGlobalStateGameMenus.mapmenuindex);
          }

          result = (gGlobalStateGameMenus.kickmenuindex != 0);
        }
        break;

        case 1: { // next
          if (gGlobalStateGameMenus.mapmenuindex < gGlobalStateNetworkClient.votemapcount - 1)
          {
            gGlobalStateGameMenus.mapmenuindex = gGlobalStateGameMenus.mapmenuindex + 1;
            clientvotemap(gGlobalStateGameMenus.mapmenuindex);
          }

          result =
            (gGlobalStateGameMenus.mapmenuindex <= gGlobalStateNetworkClient.votemapcount - 1);
        }
        break;

        case 2: { // vote map
          gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.escmenu, false);
          clientsendstringmessage(string("votemap ") + (gGlobalStateNetworkClient.votemapname),
                                  msgtype_cmd);
        }
        break;
        }
      }
    }
    else if ((menu == gGlobalStateGameMenus.limbomenu) && (sprite_system.IsPlayerSpriteValid()))
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
            gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.limbomenu, false);
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
            gGlobalStateGameMenus.gamemenushow(gGlobalStateGameMenus.limbomenu, false);
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

  gGlobalStateGameMenus.hoveredmenu = nullptr;
  gGlobalStateGameMenus.hoveredbutton = nullptr;
  gGlobalStateGameMenus.hoveredbuttonindex = 0;

  x = gGlobalStateClientGame.mx * gGlobalStateInterfaceGraphics._rscala.x;
  y = gGlobalStateClientGame.my * gGlobalStateInterfaceGraphics._rscala.y;

  for (i = low(gGlobalStateGameMenus.gamemenu); i <= high(gGlobalStateGameMenus.gamemenu); i++)
  {
    if (gGlobalStateGameMenus.gamemenu[i].active)
    {
      for (j = low(gGlobalStateGameMenus.gamemenu[i].button);
           j <= high(gGlobalStateGameMenus.gamemenu[i].button); j++)
      {
        btn = &gGlobalStateGameMenus.gamemenu[i].button[j];

        if (btn->active && (x > btn->x1) && (x < btn->x2) && (y > btn->y1) && (y < btn->y2))
        {
          gGlobalStateGameMenus.hoveredmenu = &gGlobalStateGameMenus.gamemenu[i];
          gGlobalStateGameMenus.hoveredbutton = btn;
          gGlobalStateGameMenus.hoveredbuttonindex = j;
          return;
        }
      }
    }
  }
}

auto GlobalStateGameMenus::gamemenuclick() -> bool
{
  if (gGlobalStateGameMenus.hoveredbutton != nullptr)
  {
    return gGlobalStateGameMenus.gamemenuaction(gGlobalStateGameMenus.hoveredmenu,
                                                gGlobalStateGameMenus.hoveredbuttonindex);
  }
  return false;
}
