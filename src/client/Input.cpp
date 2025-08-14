// automatically converted
#include "Input.hpp"

#include <SDL3/SDL_keyboard.h>
#include <spdlog/fmt/bundled/core.h>
#include <spdlog/fmt/bundled/format.h>
#include <map>
#include <utility>

#include "common/Logging.hpp"
#include "common/misc/PortUtilsSoldat.hpp"

/*#include "Server.h"*/
/*#include "Client.h"*/
/*#include "GameRendering.h"*/
/*#include "typinfo.h"*/
/*#include "TraceLog.h"*/
GlobalStateInput gGlobalStateInput{
  .keystatus{},
  .binds{},
  .gamewindow{},
};

auto constexpr LOG = "input";

auto GlobalStateInput::bindkey(const std::string &key, const std::string &action,
                               const std::string &command, std::uint32_t modifier) -> bool
{
  bool bindkey_result = false;
  tbind bind;

  if (key.find("mouse") != std::string::npos)
  {
    std::string i = " ";
    i[0] = key[5];
    bind.keyid = 300 + strtointdef(i, 1);
  }
  else
  {
    bind.keyid = SDL_GetScancodeFromName(key.c_str());
  }

  if (bind.keyid == 0)
  {
    LogWarn(LOG, "Key {} is invalid", key);
    return bindkey_result;
  }

  if (findkeybind(modifier, (SDL_Scancode)bind.keyid) != nullptr)
  {
    LogWarn(LOG, "Key {} is already binded", key);
    return bindkey_result;
  }

  bind.action = GetActionEnum(action.substr(1));

  bind.command = command;
  bind.keymod = modifier;

  LogInfo(LOG, "BindKey id: {} Key: {} ({}), Mod: {} Command: {}", gGlobalStateInput.binds.size(),
          key, bind.keyid, bind.keymod, command);

  gGlobalStateInput.binds.push_back(bind);
  bindkey_result = true;
  return bindkey_result;
}

auto GlobalStateInput::findkeybind(std::uint32_t keymods, SDL_Scancode keycode) -> pbind
{
  pbind findkeybind_result = nullptr;

  for (auto &bind : gGlobalStateInput.binds)
  {
    if ((bind.keyid == keycode) && (((bind.keymod & keymods) != 0) || (bind.keymod == keymods)))
    {
      findkeybind_result = &bind;
      return findkeybind_result;
    }
  }
  return findkeybind_result;
}

void GlobalStateInput::unbindall() { gGlobalStateInput.binds.clear(); }

void GlobalStateInput::startinput()
{
  //SDL_SetWindowRelativeMouseMode(gamewindow, true);
  SDL_StopTextInput(gGlobalStateInput.gamewindow);
}

auto GlobalStateInput::GetActionEnum(const std::string_view &name) -> taction
{
  static const std::map<std::string_view, taction> m{
    {"none", none},
    {"left", left},
    {"right", right},
    {"jump", taction_jump},
    {"crouch", taction_crouch},
    {"fire", fire},
    {"jet", jet},
    {"reload", taction_reload},
    {"changeweapon", changeweapon},
    {"voicechat", voicechat},
    {"throwgrenade", throwgrenade},
    {"dropweapon", dropweapon},
    {"prone", taction_prone},
    {"flagthrow", flagthrow},
    {"statsmenu", statsmenu},
    {"gamestats", gamestats},
    {"minimap", minimap},
    {"playername", playername},
    {"fragslist", fragslist},
    {"sniperline", sniperline},
    {"radio", radio},
    {"recorddemo", recorddemo},
    {"volumeup", volumeup},
    {"volumedown", volumedown},
    {"mousesensitivityup", mousesensitivityup},
    {"mousesensitivitydown", mousesensitivitydown},
    {"cmd", cmd},
    {"chat", chat},
    {"teamchat", teamchat},
    {"snap", snap},
    {"weapons", weapons},
    {"bind", bind},
    {"last_taction", last_taction},
  };
  return m.at(name);
}
