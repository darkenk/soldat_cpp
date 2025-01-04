// automatically converted
#include "Input.hpp"
#include "common/Logging.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"

/*#include "Server.h"*/
/*#include "Client.h"*/
/*#include "GameRendering.h"*/
/*#include "typinfo.h"*/
/*#include "TraceLog.h"*/
std::array<bool, 512> keystatus;
std::vector<tbind> binds;
SDL_Window *gamewindow;
SDL_GLContext gameglcontext;

auto constexpr LOG = "input";

bool bindkey(const std::string &key, const std::string &action, const std::string &command,
             std::uint32_t modifier)
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

  LogInfo(LOG, "BindKey id: {} Key: {} ({}), Mod: {} Command: {}", binds.size(), key, bind.keyid,
          bind.keymod, command);

  binds.push_back(bind);
  bindkey_result = true;
  return bindkey_result;
}

pbind findkeybind(std::uint32_t keymods, SDL_Scancode keycode)
{
  pbind findkeybind_result = nullptr;

  for (auto &bind : binds)
  {
    if ((bind.keyid == keycode) && (((bind.keymod & keymods) != 0) || (bind.keymod == keymods)))
    {
      findkeybind_result = &bind;
      return findkeybind_result;
    }
  }
  return findkeybind_result;
}

void unbindall()
{
  binds.clear();
}

void startinput()
{
  //SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_StopTextInput();
}

taction GetActionEnum(const std::string_view &name)
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
