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
#include "Logging.hpp"
#include "misc/Config.hpp"
#include <sstream>

#include <filesystem>
#include <fstream>
#include <map>

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
        mainconsole.console("Usage: exec \"filename.cfg\"", game_message_color);
        return;
    }
    loadconfig(args[1]);
}

static void commandtoggle(std::vector<std::string> &args, std::uint8_t sender)
{
    NotImplemented(NITag::OTHER);
#if 0
    CVarBase acvar;

    if (length(args) == 1)
    {
        mainconsole.console("Usage: toggle \"cvarname\" \"value\" \"value2\"", game_message_color);
        return;
    }
    acvar = CVarBase.find(args[1]);
    if (!assigned(acvar))
    {
        mainconsole.console(std::string("Toggle: Cvar ") + args[1] + " not found",
                            debug_message_color);
        return;
    }
    if (CVar::acvarasstring == args[2])
        acvar.parseandsetvalue(args[3]);
    else
        acvar.parseandsetvalue(args[2]);
#endif
}

void commandalias(std::vector<std::string> &args, std::uint8_t sender)
{
    std::string aliasname;

    if (length(args) == 1)
    {
        mainconsole.console("Usage: alias \"name\" \"command\"", game_message_color);
        return;
    }
    aliasname = args[1];
    NotImplemented(NITag::OTHER);
#if 0
    if ((tcvarbase.find(aliasname) != nullptr) && (commandfind(aliasname) != nullptr))
    {
        mainconsole.console("Cannot use this alias name because it's already used",
                            debug_message_color);
        return;
    }
    commandadd(aliasname, commandexecutealias, args[2], cmd_alias);
    mainconsole.console(std::string("New alias: Args[1] with command: ") + args[2],
                        game_message_color);
#endif
}

void commandexecutealias(std::vector<std::string> &args, std::uint8_t sender)
{
    pcommand commandptr;
    tstringlist inputparse;
    std::int32_t i;

    commandptr = commandfind(args[0]);
    if (!assigned(commandptr))
    {
        mainconsole.console("Cannot find alias command", debug_message_color);
        return;
    }
    NotImplemented(NITag::OTHER);
#if 0
    inputparse.delimiter = ';';
    inputparse.strictdelimiter = true;
    inputparse.delimitedtext = commandptr.description;
    for (i = 0; i <= inputparse.count - 1; i++)
        parseinput(inputparse[i]);
#endif
}

void commandecho(std::vector<std::string> &args, std::uint8_t sender)
{
    if (length(args) == 1)
    {
        mainconsole.console("Usage: echo \"text\"", game_message_color);
        return;
    }
    mainconsole.console(args[1], game_message_color);
}

void commandreset(std::vector<std::string> &args, std::uint8_t sender)
{
    NotImplemented(NITag::OTHER);
#if 0
    std::string cvarname;
    tcvarbase acvar;

    if (length(args) == 1)
    {
        mainconsole.console("Usage: reset \"cvarname\"", game_message_color);
        return;
    }
    cvarname = args[1];
    acvar = tcvarbase.find(cvarname);
    if (!assigned(acvar))
    {
        mainconsole.console(std::string("Reset: Cvar ") + cvarname + " not found",
                            debug_message_color);
        return;
    }
    acvar.reset(0);
    mainconsole.console(std::string("Reset: ") + cvarname + " set to: " + CVar::acvarasstring,
                        debug_message_color);
#endif
}

void commandcmdlist(std::vector<std::string> &args, std::uint8_t sender)
{
    NotImplemented(NITag::OTHER);
#if 0
    std::int32_t i;
    pcommand commandptr;

    for (i = 0; i <= commands.count - 1; i++)
    {
        commandptr = commands.items[i];
        mainconsole.console(commandptr->name + " - " + commandptr->description, game_message_color);
    }
#endif
}

void commandcvarlist(std::vector<std::string> &args, std::uint8_t sender)
{
    NotImplemented(NITag::OTHER);
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
        mainconsole.console(padright(acvar.name, 30) + " : " + CVar::acvarasstring + " : [" +
                                dumpflags(acvar) + " ] : " + acvar.description,
                            game_message_color);
    }
#endif
}

void commandinc(std::vector<std::string> &args, std::uint8_t sender)
{
    NotImplemented(NITag::OTHER);
#if 0
    std::string cvarname;
    tcvarbase acvar;
    tfloatcvar floatcvar;
    tintcvar tintegercvar;
    float newfloatvalue;
    std::int32_t newstd::int32_tvalue;

    if (length(args) <= 4)
    {
        mainconsole.console("Usage: inc \"cvarname\" \"min\" \"max\" \"delta\"",
                            game_message_color);
        return;
    }

    cvarname = args[1];
    acvar = tcvarbase.find(cvarname);
    if (!assigned(acvar))
    {
        mainconsole.console(string("Inc: Cvar ") + cvarname + " not found", debug_message_color);
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
        mainconsole.console("Usage: netconfig \"id\" \"value\"", game_message_color);
        return;
    }

    if (udp.networkingutil.getconfigvalueinfo(esteamnetworkingconfigvalue(strtoint(args[1])), &name,
                                              &outdatatype, &outscope, &outnextvalue))
    {
        if (outdatatype == k_esteamnetworkingconfig_int32)
        {
            cbresult = sizeof(std::int32_t);
            std::int32_tvalue = strtointdef(args[2], 0);
            setresult = udp.networkingutil.setconfigvalue(
                esteamnetworkingconfigvalue(strtoint(args[1])), k_esteamnetworkingconfig_global, 0,
                outdatatype, &std::int32_tvalue);
            mainconsole.console(format("[NET] NetConfig: Set %S to %D, result: %S",
                                       set::of(std::string(name[0]), std::int32_tvalue,
                                               setresult.tostring(tuseboolstrs.true), eos)),
                                debug_message_color #ifdef SERVER, sender #endif);
        }
        else if (outdatatype == k_esteamnetworkingconfig_float)
        {
            cbresult = sizeof(float);
            floatvalue = strtofloatdef(args[2], 0.0);
            setresult = udp.networkingutil.setconfigvalue(
                esteamnetworkingconfigvalue(strtoint(args[1])), k_esteamnetworkingconfig_global, 0,
                outdatatype, &floatvalue);
            mainconsole.console(format("[NET] NetConfig: Set %S to %F, result: %S",
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
        // mainconsole.Console('Usage: netconfig "id" "value"', GAME_MESSAGE_COLOR);
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
                mainconsole.console(format("[NET] NetConfig: %S is %D",
                                           set::of(std::string(name[0]), std::int32_tvalue, eos)),
                                    debug_message_color #ifdef SERVER, sender #endif);
        }
        else if (outdatatype == k_esteamnetworkingconfig_float)
        {
            cbresult = sizeof(float);
            if (udp.networkingutil.getconfigvalue(outnextvalue, k_esteamnetworkingconfig_global, 0,
                                                  &outdatatype, floatvalue,
                                                  &cbresult) == k_esteamnetworkinggetconfigvalue_ok)
                mainconsole.console(format("[NET] NetConfig: %S is %F",
                                           set::of(std::string(name[0]), floatvalue, eos)),
                                    debug_message_color #ifdef SERVER, sender #endif);
        }
    }
}

void commandnetloglevel(std::vector<std::string> &args, std::uint8_t sender)
{
    if (length(args) == 1)
    {
        mainconsole.console("Usage: netconfig_loglevel \"level\"",
                            debug_message_color #ifdef SERVER, sender #endif);
        return;
    }

    udp.setdebuglevel(esteamnetworkingsocketsdebugoutputtype(strtointdef(args[1], 4)));
    mainconsole.console(string("[NET] GNS log level set to ") + args[1],
                        debug_message_color #ifdef SERVER, sender #endif);
}
/*$POP*/
#endif

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

bool parseinput(const std::string &input)
{
    return parseinput(input, 0);
}

bool parseinput(const std::string &input, std::uint8_t sender)
{
    tstringlist inputparse;

    pcommand commandptr;
#if 0
    tcvarbase acvar;
#endif
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
                if (!((sender == 255) or (isremoteadminip(sprite[sender].player->ip) or
                                          isadminip(sprite[sender].player->ip))))
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
#if 0
    acvar = tcvarbase.find(inputarray[0]);
    if (acvar != nullptr)
    {
#ifndef SERVER
        if (acvar.flags.has(cvar_sync))
        {
            result = false;
            return result;
        }
#endif
        if (length(inputarray) == 1)
        {
            mainconsole.console(format("%s is \"%s\" (%s)", set::of(acvar.name, CVar::acvarasstring,
                                                                    acvar.description, eos)),
                                debug_message_color #ifdef SERVER, sender #endif);
        }
        else if (length(inputarray) == 2)
        {
            if (!acvar.parseandsetvalue(inputarray[1]))
            {
                mainconsole.console(
                    format("Unable to set %s: %s", set::of(acvar.name, acvar.geterrormessage, eos)),
                    debug_message_color #ifdef SERVER, sender #endif);
            }
            else
            {
                mainconsole.console(format("%s is now set to: \"%s\"",
                                           set::of(acvar.name, CVar::acvarasstring, eos)),
                                    debug_message_color #ifdef SERVER, sender #endif);
            }
        }
        result = true;
    }
    return result;
#endif
    return result;
}

bool loadconfig(const std::string &configname)
{
    std::string path;
    std::string line;

    bool result = false;
    path = userdirectory + "configs/" + configname;
    if (!std::filesystem::exists(path))
    {
        mainconsole.console(std::string("No such config file: ") + configname,
                            warning_message_color);
        return result;
    }
    std::ifstream configfile{path.c_str()};
    while (configfile.good() && !configfile.eof())
    {
        std::getline(configfile, line);
        line.erase(line.begin(), std::find_if(line.begin(), line.end(),
                                              [](auto ch) { return !std::isspace(ch); }));
        if (line.starts_with("//"))
        {
            continue;
        }
        parseinput(line);
    }
    result = true;
    return result;
}

void parsecommandline()
{
    std::string commandline = "";
    tstringlist commandlineparser;
    std::int32_t i;

    NotImplemented(NITag::OTHER);
#if 0

    for (i = 1; i <= argc; i++)
    {
        if (commandline != "")
            commandline = commandline + ' ';
        commandline = commandline + argv[i];
    }

    commandlineparser = tstringlist.create;
    commandlineparser.delimiter = '-';
    commandlineparser.strictdelimiter = true;
    commandlineparser.delimitedtext = commandline;

    for (i = 0; i <= commandlineparser.count - 1; i++)
    {
        if (commandlineparser[i] != "")
            parseinput(commandlineparser[i]);
    }
#endif
}

tcommandtargets commandtarget(std::string target, std::uint8_t sender);

static void addplayer(std::uint8_t id, std::vector<std::uint8_t> &players)
{
    setlength(players, length(players) + 1);
    players[high(players)] = sprite[id].num;
}

tcommandtargets commandtarget(std::string target, std::uint8_t sender)
{
    std::vector<std::uint8_t> players;
    std::int32_t j;
    std::int32_t targetid;

    tcommandtargets result;
    targetid = strtointdef(target, 0);
    setlength(players, 0);

    for (j = 1; j <= max_players; j++)
    {
        if (sprite[j].active)
        {
            if ((targetid != 0) && (j == targetid))
            {
                addplayer(j, players);
                break;
            }

            if (sprite[j].player->name == target)
            {
                addplayer(sprite[j].num, players);
                break;
            }

            if (target == "@all")
            {
                addplayer(sprite[j].num, players);
            }
            else if (target == "@bots")
            {
                if (sprite[j].player->controlmethod == bot)
                    addplayer(sprite[j].num, players);
            }
            else if (target == "@humans")
            {
                if (sprite[j].player->controlmethod == human)
                    addplayer(sprite[j].num, players);
            }
            else if (target == "@alive")
            {
                if (!sprite[j].deadmeat)
                    addplayer(sprite[j].num, players);
            }
            else if (target == "@dead")
            {
                if (sprite[j].deadmeat)
                    addplayer(sprite[j].num, players);
            }
            else if (target == "@aim")
            {
                if ((sender > 0) && (sender <= max_players))
                    if (sprite[sender].player->camera == sprite[j].num)
                    {
                        addplayer(sprite[j].num, players);
                    }
            }
            else if (target == "@me")
            {
                if ((sender > 0) && (sender <= max_players))
                    if (sprite[sender].num == sprite[j].num)
                        addplayer(sprite[sender].num, players);
            }
            else if (target == "@!me")
            {
                if ((sender > 0) && (sender <= max_players))
                    if (!(sprite[sender].num == sprite[j].num))
                        addplayer(sprite[j].num, players);
            }
            else if (target == "@none")
            {
                if (sprite[j].player->team == team_none)
                    addplayer(sprite[j].num, players);
            }
            else if (target == "@alpha")
            {
                if (sprite[j].player->team == team_alpha)
                    addplayer(sprite[j].num, players);
            }
            else if (target == "@bravo")
            {
                if (sprite[j].player->team == team_bravo)
                    addplayer(sprite[j].num, players);
            }
            else if (target == "@charlie")
            {
                if (sprite[j].player->team == team_charlie)
                    addplayer(sprite[j].num, players);
            }
            else if (target == "@delta")
            {
                if (sprite[j].player->team == team_delta)
                    addplayer(sprite[j].num, players);
            }
            else if (target == "@spec")
            {
                if (sprite[j].player->team == team_spectator)
                    addplayer(sprite[j].num, players);
            }
        }
    }
    result = players;
    return result;
}

void rundeferredcommands()
{
    deferredinitialized = true;
    for (auto &command : deferredcommands)
    {
        parseinput(command);
    }
    deferredcommands.clear();
}

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
    NotImplemented(NITag::OTHER, "Missing development commands");
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

void commanddeinit()
{
    for (auto &c : commands)
    {
        delete c.second;
    }
    commands.clear();
}
