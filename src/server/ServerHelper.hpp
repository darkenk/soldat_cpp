#pragma once

#include <string>
#include <vector>
#include <cinttypes>
#include <cstdint>

std::string checknextmap();
#if 0
void writeln1(variant s);;
#endif
std::string idtoname(std::int32_t id);
std::string teamtoname(std::int32_t id);
std::int32_t nametoid(const std::string &name);
std::string nametohw(const std::string &name);
std::int32_t findlowestteam(const std::vector<std::int32_t> &arr);
void savetxtlists();
void savemaplist();
std::uint32_t rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b);
std::uint8_t fixteam(std::uint8_t team);
#if 0
std::string weaponnamebynum(std::int32_t num);
#endif
void writepid();
std::int32_t soldat_getpid();
void writeconsole(std::uint8_t id, std::string text, std::uint32_t colour);
void updatewaverespawntime();
std::string randombot();
void dobalancebots(std::uint8_t leftgame, std::uint8_t newteam);
