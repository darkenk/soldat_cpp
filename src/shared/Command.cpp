// automatically converted

#include "Command.hpp"

#ifdef SERVER
#include "../server/Server.hpp"
#include "shared/network/NetworkUtils.hpp"
#else
#include "../client/Client.hpp"
#endif
#include "Cvar.hpp"
#include "Game.hpp"
#include "common/Logging.hpp"
#include "common/misc/Config.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

static bool deferredinitialized = false;
static std::map<std::string, tcommand *> commands;
static std::vector<std::string> deferredcommands;

static pcommand commandfind(const std::string &name)
{
  auto ret = commands.find(name);
  if (ret == commands.end())
  {
    return nullptr;
  }
  return ret->second;
}

/*$PUSH*/
/*$WARN 5024 OFF : Parameter "$1" not used*/
static void commandexec(std::vector<std::string> &args, std::uint8_t sender = 255)
{
  if (length(args) == 1)
  {
    GS::GetMainConsole().console("Usage: exec \"filename.cfg\"", game_message_color);
    return;
  }
  loadconfig(args[1]);
}

static void commandtoggle(std::vector<std::string> &args, std::uint8_t sender)
{
  NotImplemented();
#if 0
    CVarBase acvar;

    if (length(args) == 1)
    {
        GS::GetMainConsole().console("Usage: toggle \"cvarname\" \"value\" \"value2\"", game_message_color);
        return;
    }
    acvar = CVarBase.find(args[1]);
    if (!assigned(acvar))
    {
        GS::GetMainConsole().console(std::string("Toggle: Cvar ") + args[1] + " not found",
                            debug_message_color);
        return;
    }
    if (CVar::acvarasstring == args[2])
        acvar.parseandsetvalue(args[3]);
    else
        acvar.parseandsetvalue(args[2]);
#endif
}

static void commandalias(std::vector<std::string> &args, std::uint8_t sender)
{
  std::string aliasname;

  if (length(args) == 1)
  {
    GS::GetMainConsole().console("Usage: alias \"name\" \"command\"", game_message_color);
    return;
  }
  aliasname = args[1];
  NotImplemented();
#if 0
    if ((tcvarbase.find(aliasname) != nullptr) && (commandfind(aliasname) != nullptr))
    {
        GS::GetMainConsole().console("Cannot use this alias name because it's already used",
                            debug_message_color);
        return;
    }
    commandadd(aliasname, commandexecutealias, args[2], cmd_alias);
    GS::GetMainConsole().console(std::string("New alias: Args[1] with command: ") + args[2],
                        game_message_color);
#endif
}

static void commandexecutealias(std::vector<std::string> &args, std::uint8_t sender)
{
  pcommand commandptr;
  tstringlist inputparse;
  std::int32_t i;

  commandptr = commandfind(args[0]);
  if (!assigned(commandptr))
  {
    GS::GetMainConsole().console("Cannot find alias command", debug_message_color);
    return;
  }
  NotImplemented();
#if 0
    inputparse.delimiter = ';';
    inputparse.strictdelimiter = true;
    inputparse.delimitedtext = commandptr.description;
    for (i = 0; i <= inputparse.count - 1; i++)
        parseinput(inputparse[i]);
#endif
}

static void commandecho(std::vector<std::string> &args, std::uint8_t sender)
{
  if (length(args) == 1)
  {
    GS::GetMainConsole().console("Usage: echo \"text\"", game_message_color);
    return;
  }
  GS::GetMainConsole().console(args[1], game_message_color);
}

static void commandreset(std::vector<std::string> &args, std::uint8_t sender)
{
  NotImplemented();
#if 0
    std::string cvarname;
    tcvarbase acvar;

    if (length(args) == 1)
    {
        GS::GetMainConsole().console("Usage: reset \"cvarname\"", game_message_color);
        return;
    }
    cvarname = args[1];
    acvar = tcvarbase.find(cvarname);
    if (!assigned(acvar))
    {
        GS::GetMainConsole().console(std::string("Reset: Cvar ") + cvarname + " not found",
                            debug_message_color);
        return;
    }
    acvar.reset(0);
    GS::GetMainConsole().console(std::string("Reset: ") + cvarname + " set to: " + CVar::acvarasstring,
                        debug_message_color);
#endif
}

static void commandcmdlist(std::vector<std::string> &args, std::uint8_t sender)
{
  NotImplemented();
#if 0
    std::int32_t i;
    pcommand commandptr;

    for (i = 0; i <= commands.count - 1; i++)
    {
        commandptr = commands.items[i];
        GS::GetMainConsole().console(commandptr->name + " - " + commandptr->description, game_message_color);
    }
#endif
}

static void commandcvarlist(std::vector<std::string> &args, std::uint8_t sender)
{
  NotImplemented();
#if 0
    std::int32_t i;
    tcvarbase acvar;

    for (i = 0; i <= cvars.count - 1; i++)
    {
        acvar = cvars.items[i];
#ifndef SERVER
#ifndef DEVELOPMENT
        // Hide temporary sync cvars in client
        if ((acvar.flags.has(cvar_server)) || (acvar.flags.has(cvar_sync)))
            continue;
#endif
#endif
        if (length(args) == 2)
            if (!ansicontainsstr(acvar.name, args[1]))
                continue;
        GS::GetMainConsole().console(padright(acvar.name, 30) + " : " + CVar::acvarasstring + " : [" +
                                dumpflags(acvar) + " ] : " + acvar.description,
                            game_message_color);
    }
#endif
}

static void commandinc(std::vector<std::string> &args, std::uint8_t sender)
{
  NotImplemented();
#if 0
    std::string cvarname;
    tcvarbase acvar;
    tfloatcvar floatcvar;
    tintcvar tintegercvar;
    float newfloatvalue;
    std::int32_t newstd::int32_tvalue;

    if (length(args) <= 4)
    {
        GS::GetMainConsole().console("Usage: inc \"cvarname\" \"min\" \"max\" \"delta\"",
                            game_message_color);
        return;
    }

    cvarname = args[1];
    acvar = tcvarbase.find(cvarname);
    if (!assigned(acvar))
    {
        GS::GetMainConsole().console(string("Inc: Cvar ") + cvarname + " not found", debug_message_color);
        return;
    }

    if (acvar == tfloatcvar) // dk = is
    {
        floatcvar = tfloatcvar(acvar);
        newfloatvalue = CVar::floatcvar + strtofloatdef(args[4], 0.0);
        if ((newfloatvalue >= strtofloatdef(args[2], 0.0)) &&
            (newfloatvalue <= strtofloatdef(args[3], 1.0)))
            floatcvar.setvalue(newfloatvalue);
    }

    if (acvar == tintegercvar) // dk = iterates
    {
        integercvar = tintegercvar(acvar);
        newstd::int32_tvalue = std::int32CVar::_tcvar + strtointdef(args[4], 0);
        if ((newstd::int32_tvalue >= strtointdef(args[2], 0)) &&
            (newstd::int32_tvalue <= strtointdef(args[3], 1)))
            integercvar.setvalue(newstd::int32_tvalue);
    }
#endif
}

#ifdef DEVELOPMENT
/*$PUSH*/
/*$WARN 5027 OFF*/
void commandnetconfig(std::vector<std::string> &args, std::uint8_t sender)
{
  array<0, 256, pansichar> name;
  esteamnetworkingconfigdatatype outdatatype;
  esteamnetworkingconfigscope outscope;
  esteamnetworkingconfigvalue outnextvalue;
  float floatvalue;
  = 0.0;
  std::int32_t std::int32_tvalue;
  = 0;
  csize_t cbresult;
  = 0;
  bool setresult;
  = false;

  cbresult = 0;

  if (length(args) <= 3)
  {
    GS::GetMainConsole().console("Usage: netconfig \"id\" \"value\"", game_message_color);
    return;
  }

  if (udp.networkingutil.getconfigvalueinfo(esteamnetworkingconfigvalue(strtoint(args[1])), &name,
                                            &outdatatype, &outscope, &outnextvalue))
  {
    if (outdatatype == k_esteamnetworkingconfig_int32)
    {
      cbresult = sizeof(std::int32_t);
      std::int32_tvalue = strtointdef(args[2], 0);
      setresult = udp.networkingutil.setconfigvalue(esteamnetworkingconfigvalue(strtoint(args[1])),
                                                    k_esteamnetworkingconfig_global, 0, outdatatype,
                                                    &std::int32_tvalue);
      GS::GetMainConsole().console(format("[NET] NetConfig: Set %S to %D, result: %S",
                                      set::of(std::string(name[0]), std::int32_tvalue,
                                              setresult.tostring(tuseboolstrs.true), eos)),
                               debug_message_color #ifdef SERVER, sender #endif);
    }
    else if (outdatatype == k_esteamnetworkingconfig_float)
    {
      cbresult = sizeof(float);
      floatvalue = strtofloatdef(args[2], 0.0);
      setresult = udp.networkingutil.setconfigvalue(esteamnetworkingconfigvalue(strtoint(args[1])),
                                                    k_esteamnetworkingconfig_global, 0, outdatatype,
                                                    &floatvalue);
      GS::GetMainConsole().console(format("[NET] NetConfig: Set %S to %F, result: %S",
                                      set::of(std::string(name[0]), floatvalue,
                                              setresult.tostring(tuseboolstrs.true), eos)),
                               debug_message_color #ifdef SERVER, sender #endif);
    }
  }
}

void commandnetconfglist(std::vector<std::string> &args, std::uint8_t sender)
{
  array<0, 256, pansichar> name;
  esteamnetworkingconfigdatatype outdatatype;
  esteamnetworkingconfigscope outscope;
  esteamnetworkingconfigvalue outnextvalue;
  float floatvalue;
  = 0.0;
  std::int32_t std::int32_tvalue;
  = 0;
  csize_t cbresult;
  = 0;

  if (length(args) <= 4)
  {
    ;
    // GS::GetMainConsole().Console('Usage: netconfig "id" "value"', GAME_MESSAGE_COLOR);
    // Exit;
  }

  outnextvalue = udp.networkingutil.getfirstconfigvalue(0);

#ifdef DEVELOPMENT
  std::int32_tvalue = 1;
  udp.networkingutil.setconfigvalue(k_esteamnetworkingconfig_enumeratedevvars,
                                    k_esteamnetworkingconfig_global, 0,
                                    k_esteamnetworkingconfig_int32, &std::int32_tvalue);
#endif
  while (udp.networkingutil.getconfigvalueinfo(outnextvalue, &name, &outdatatype, &outscope,
                                               &outnextvalue))
  {
    if (outdatatype == k_esteamnetworkingconfig_int32)
    {
      cbresult = sizeof(std::int32_t);

      if (udp.networkingutil.getconfigvalue(outnextvalue, k_esteamnetworkingconfig_global, 0,
                                            &outdatatype, std::int32_tvalue,
                                            &cbresult) == k_esteamnetworkinggetconfigvalue_ok)
        GS::GetMainConsole().console(format("[NET] NetConfig: %S is %D",
                                        set::of(std::string(name[0]), std::int32_tvalue, eos)),
                                 debug_message_color #ifdef SERVER, sender #endif);
    }
    else if (outdatatype == k_esteamnetworkingconfig_float)
    {
      cbresult = sizeof(float);
      if (udp.networkingutil.getconfigvalue(outnextvalue, k_esteamnetworkingconfig_global, 0,
                                            &outdatatype, floatvalue,
                                            &cbresult) == k_esteamnetworkinggetconfigvalue_ok)
        GS::GetMainConsole().console(
          format("[NET] NetConfig: %S is %F", set::of(std::string(name[0]), floatvalue, eos)),
          debug_message_color #ifdef SERVER, sender #endif);
    }
  }
}

void commandnetloglevel(std::vector<std::string> &args, std::uint8_t sender)
{
  if (length(args) == 1)
  {
    GS::GetMainConsole().console("Usage: netconfig_loglevel \"level\"",
                             debug_message_color #ifdef SERVER, sender #endif);
    return;
  }

  udp.setdebuglevel(esteamnetworkingsocketsdebugoutputtype(strtointdef(args[1], 4)));
  GS::GetMainConsole().console(string("[NET] GNS log level set to ") + args[1],
                           debug_message_color #ifdef SERVER, sender #endif);
}
/*$POP*/
#endif

template <Config::Module M>
pcommand commandadd(const std::string &commandnamevar, tcommandfunction commandptr,
                    const std::string &description, tcommandflags flags)
{
  pcommand newcommand;
  std::string commandname;

  pcommand result = nullptr;
  commandname = lowercase(commandnamevar);
  if (commandfind(commandname) != nullptr)
  {
    LogDebugG("CommandAdd: {} is already set", commandname);
    return result;
  }
  LogDebugG("CommandAdd: {} Description: {}", commandname, description);
  newcommand = new tcommand;
  newcommand->name = commandname;
  newcommand->functionptr = commandptr;
  newcommand->description = description;
  newcommand->flags = flags;

  commands[commandname] = newcommand;
  return newcommand;
}

template <Config::Module M>
bool parseinput(const std::string &input)
{
  return parseinput(input, 0);
}

template <typename T>
bool SetValue(const std::string &cvarName, const std::string &value)
{
  auto &cvi = CVarBase<T>::Find(cvarName);
  if (!cvi.IsValid())
  {
    return false;
  }
  auto ret = cvi.ParseAndSetValue(value);
  if (ret)
  {
    LogInfoG("Set CVar variable {} to {}", cvarName, value);
  }
  return ret;
}

template <Config::Module M>
bool parseinput(const std::string &input, std::uint8_t sender)
{
  tstringlist inputparse;

  pcommand commandptr;
  tcommandfunction commandfunction;

  bool result;
  result = false;

  if (length(input) == 0)
    return result;

  std::istringstream iss(input);
  std::vector<std::string> inputarray(std::istream_iterator<std::string>{iss},
                                      std::istream_iterator<std::string>());

  for (auto &s : inputarray)
  {
    if (s.starts_with("\""))
    {
      s.erase(s.begin());
    }
    if (s.ends_with("\""))
    {
      s.erase(s.end() - 1);
    }
  }

  commandptr = commandfind(inputarray[0]);

  if (commandptr != nullptr)
  {
    if ((commandptr->flags & cmd_deferred) && (!deferredinitialized))
    {
      deferredcommands.push_back(input);
    }
    else
    {
#ifdef SERVER
      if (commandptr->flags & cmd_adminonly)
      {
        if (!((sender == 255) or
              (isremoteadminip(SpriteSystem::Get().GetSprite(sender).player->ip) or
               isadminip(SpriteSystem::Get().GetSprite(sender).player->ip))))
          return result;
      }
      if (commandptr->flags & (cmd_playeronly))
        if ((sender == 0) || (sender > max_players + 1))
          return result;
#endif
      commandfunction = commandptr->functionptr;
      commandfunction(inputarray, sender);
    }
    result = true;
    return result;
  }
  if (inputarray.size() != 2)
  {
#ifdef SERVER
    GS::GetMainConsole().console("Cannot parse " + input, debug_message_color, sender);
#else
    GS::GetMainConsole().console("Cannot parse " + input, debug_message_color);
#endif
    return false;
  }

  if (SetValue<std::int32_t>(inputarray[0], inputarray[1]))
  {
    return true;
  }
  if (SetValue<bool>(inputarray[0], inputarray[1]))
  {
    return true;
  }
  if (SetValue<std::string>(inputarray[0], inputarray[1]))
  {
    return true;
  }
  if (SetValue<float>(inputarray[0], inputarray[1]))
  {
    return true;
  }
#ifdef SERVER
  GS::GetMainConsole().console("Cannot set variable " + input, debug_message_color, sender);
#else
  GS::GetMainConsole().console("Cannot set variable " + input, debug_message_color);
#endif
  return false;
}

template <Config::Module M>
bool loadconfig(const std::string &configname)
{
  std::string path;
  std::string line;

  bool result = false;
  path = GS::GetGame().GetUserDirectory() + "configs/" + configname;
  if (!std::filesystem::exists(path))
  {
    GS::GetMainConsole().console(std::string("No such config file: ") + configname,
                             warning_message_color);
    return result;
  }
  std::ifstream configfile{path.c_str()};
  while (configfile.good() && !configfile.eof())
  {
    std::getline(configfile, line);
    line.erase(line.begin(),
               std::find_if(line.begin(), line.end(), [](auto ch) { return !std::isspace(ch); }));
    if (line.starts_with("//"))
    {
      continue;
    }
    parseinput(line);
  }
  result = true;
  return result;
}

template <Config::Module M>
void parsecommandline(int argc, const char *argv[])
{
  std::vector<std::string> commands;
  std::int32_t i;

  for (i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      commands.push_back(argv[i] + 1);
      continue;
    }
    commands.back() += " ";
    commands.back() += argv[i];
  }
  for (const auto &c : commands)
  {
    parseinput(c);
  }
}

static void addplayer(std::uint8_t id, std::vector<std::uint8_t> &players)
{
  setlength(players, length(players) + 1);
  players[high(players)] = SpriteSystem::Get().GetSprite(id).num;
}

template <Config::Module M>
tcommandtargets commandtarget(std::string target, std::uint8_t sender)
{
  std::vector<std::uint8_t> players;
  std::int32_t targetid;

  targetid = strtointdef(target, 0);
  setlength(players, 0);

  for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
  {
    if ((targetid != 0) && (sprite.num == targetid))
    {
      addplayer(sprite.num, players);
      break;
    }

    if (sprite.player->name == target)
    {
      addplayer(sprite.num, players);
      break;
    }

    if (target == "@all")
    {
      addplayer(sprite.num, players);
    }
    else if (target == "@bots")
    {
      if (sprite.player->controlmethod == bot)
        addplayer(sprite.num, players);
    }
    else if (target == "@humans")
    {
      if (sprite.player->controlmethod == human)
        addplayer(sprite.num, players);
    }
    else if (target == "@alive")
    {
      if (!sprite.deadmeat)
        addplayer(sprite.num, players);
    }
    else if (target == "@dead")
    {
      if (sprite.deadmeat)
        addplayer(sprite.num, players);
    }
    else if (target == "@aim")
    {
      if ((sender > 0) && (sender <= max_players))
        if (SpriteSystem::Get().GetSprite(sender).player->camera == sprite.num)
        {
          addplayer(sprite.num, players);
        }
    }
    else if (target == "@me")
    {
      if ((sender > 0) && (sender <= max_players))
        if (SpriteSystem::Get().GetSprite(sender).num == sprite.num)
          addplayer(SpriteSystem::Get().GetSprite(sender).num, players);
    }
    else if (target == "@!me")
    {
      if ((sender > 0) && (sender <= max_players))
        if (!(SpriteSystem::Get().GetSprite(sender).num == sprite.num))
          addplayer(sprite.num, players);
    }
    else if (target == "@none")
    {
      if (sprite.player->team == team_none)
        addplayer(sprite.num, players);
    }
    else if (target == "@alpha")
    {
      if (sprite.player->team == team_alpha)
        addplayer(sprite.num, players);
    }
    else if (target == "@bravo")
    {
      if (sprite.player->team == team_bravo)
        addplayer(sprite.num, players);
    }
    else if (target == "@charlie")
    {
      if (sprite.player->team == team_charlie)
        addplayer(sprite.num, players);
    }
    else if (target == "@delta")
    {
      if (sprite.player->team == team_delta)
        addplayer(sprite.num, players);
    }
    else if (target == "@spec")
    {
      if (sprite.player->team == team_spectator)
        addplayer(sprite.num, players);
    }
  }
  return players;
}

template <Config::Module M>
void rundeferredcommands()
{
  deferredinitialized = true;
  for (auto &command : deferredcommands)
  {
    parseinput(command);
  }
  deferredcommands.clear();
}

template <Config::Module M>
void commandinit()
{
  commandadd("echo", commandecho, "echo text", 0);
  commandadd("exec", commandexec, "executes fileconfig", 0);
  commandadd("cmdlist", commandcmdlist, "list of commands", 0);
  commandadd("cvarlist", commandcvarlist, "list of cvars", 0);
  commandadd("reset", commandreset, "resets cvar to default value", 0);
  commandadd("alias", commandalias, "creates alias", 0);
  commandadd("toggle", commandtoggle, "toggles cvar between two values", 0);
  commandadd("inc", commandinc, "increments cvar value", 0);
  NotImplemented("Missing development commands");
#if 0
    if (!Config::IsMaster())
    {
        commandadd("netconfig", commandnetconfig, "Set GNS config" 0);
        commandadd("netconfig_conn", commandnetconfig,
                   "Set GNS config for specific connection handle", 0);
        commandadd("netconfig_list", commandnetconfglist, "List GNS cvars", 0);
        commandadd("netconfig_loglevel", commandnetloglevel, "Set GNS log level", 0);
    }
#endif
}

template <Config::Module M>
void commanddeinit()
{
  for (auto &c : commands)
  {
    delete c.second;
  }
  commands.clear();
}

template void commandinit();
template void commanddeinit();
template bool parseinput(const std::string &input);
template bool parseinput(const std::string &input, std::uint8_t sender);
template bool loadconfig(const std::string &configname);

template pcommand commandadd(const std::string &commandnamevar, tcommandfunction commandptr,
                             const std::string &description, tcommandflags flags);
template void parsecommandline(int argc, const char *argv[]);
template void rundeferredcommands();
template tcommandtargets commandtarget(std::string target, std::uint8_t sender);
