#pragma once

#include "common/FileUtility.hpp"
#include "misc/SoldatConfig.hpp"
#include <string>
#include <vector>

template <Config::Module M = Config::GetModule()>
void commandinit();
template <Config::Module M = Config::GetModule()>
void commanddeinit();
template <Config::Module M = Config::GetModule()>
bool parseinput(const std::string &input);
template <Config::Module M = Config::GetModule()>
bool parseinput(const std::string &input, std::uint8_t sender);
template <Config::Module M = Config::GetModule()>
bool loadconfig(const std::string &configname, FileUtility& fs);

constexpr std::int32_t max_commands = 1024;

enum tcommandflag
{
  cmd_init = 0x1,
  cmd_alias = 0x2,
  cmd_script = 0x4,
  cmd_deferred = 0x8,
  cmd_adminonly = 0x10,
  cmd_playeronly = 0x20,
  last_tcommandflag
};
typedef std::uint32_t tcommandflags;
typedef struct tcommand *pcommand;
typedef void (*tcommandfunction)(std::vector<std::string> &args, std::uint8_t sender);
struct tcommand
{
  std::string name;
  tcommandfunction functionptr;
  std::string description;
  tcommandflags flags;
};
typedef std::vector<std::uint8_t> tcommandtargets;

template <Config::Module M = Config::GetModule()>
pcommand commandadd(const std::string &commandnamevar, tcommandfunction commandptr,
                    const std::string &description, tcommandflags flags);
template <Config::Module M = Config::GetModule()>
void parsecommandline(int argc, const char *argv[]);
template <Config::Module M = Config::GetModule()>
void rundeferredcommands();
template <Config::Module M = Config::GetModule()>
tcommandtargets commandtarget(std::string target, std::uint8_t sender);
