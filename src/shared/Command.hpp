#pragma once

#include <string>
#include <vector>

void commandinit();
void commanddeinit();
bool parseinput(const std::string &input);
bool parseinput(const std::string &input, std::uint8_t sender);
bool loadconfig(const std::string &configname);

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

pcommand commandadd(const std::string &commandnamevar, tcommandfunction commandptr,
                    const std::string &description, tcommandflags flags);
void commandexecutealias(std::vector<std::string> &args, std::uint8_t sender);
void parsecommandline(int argc, const char *argv[]);
void rundeferredcommands();
tcommandtargets commandtarget(std::string target, std::uint8_t sender);
