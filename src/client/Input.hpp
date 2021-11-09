#pragma once

/*#include "sysutils.h"*/
/*#include "Constants.h"*/
/*#include "Weapons.h"*/
/*#include "Classes.h"*/
/*#include "Strutils.h"*/
/*#include "SDL2.h"*/

#include <SDL2/SDL.h>
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

/*$scopedenums on*/
enum taction
{
    none,
    left,
    right,
    taction_jump,
    taction_crouch,
    fire,
    jet,
    taction_reload,
    changeweapon,
    voicechat,
    throwgrenade,
    dropweapon,
    taction_prone,
    flagthrow,
    statsmenu,
    gamestats,
    minimap,
    playername,
    fragslist,
    sniperline,
    radio,
    recorddemo,
    volumeup,
    volumedown,
    mousesensitivityup,
    mousesensitivitydown,
    cmd,
    chat,
    teamchat,
    snap,
    weapons,
    bind,
    last_taction
};

taction GetActionEnum(const std::string_view &name);

typedef struct tbind *pbind;
struct tbind
{
    taction action;
    std::uint32_t keyid;
    std::uint32_t keymod;
    std::string command;
};

constexpr std::int32_t km_none = 0;
constexpr std::int32_t km_alt = 1 << 0;
constexpr std::int32_t km_ctrl = 1 << 1;
constexpr std::int32_t km_shift = 1 << 2;

bool bindkey(const std::string &key, const std::string &action, const std::string &command,
             std::uint32_t modifier);
pbind findkeybind(std::uint32_t keymods, SDL_Scancode keycode);
void startinput();
void unbindall();

extern std::array<bool, 512> keystatus;
extern std::vector<tbind> binds;
extern SDL_Window *gamewindow;
extern SDL_GLContext gameglcontext;
