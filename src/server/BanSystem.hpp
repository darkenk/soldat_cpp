#pragma once

#include "common/Constants.hpp"
#include <cstdint>
#include <string>
#include <vector>

/*#include "Constants.h"*/

void addbannedip(const std::string &ip, std::string reason, std::int32_t duration = Constants::PERMAMENT);
bool delbannedip(const std::string &ip);
bool checkbannedip(const std::string &ip);
std::int32_t findban(const std::string &ip);
void loadbannedlist(const std::string &filename);
void savebannedlist(const std::string &filename);
// bans hardwareids
void addbannedhw(const std::string &hw, std::string reason, std::int32_t duration = Constants::PERMAMENT);
bool delbannedhw(const std::string &hw);
bool checkbannedhw(const std::string &hw);
std::int32_t findbanhw(const std::string &hw);
void loadbannedlisthw(const std::string &filename);
void savebannedlisthw(const std::string &filename);
void updateipbanlist();
void updatehwbanlist();

struct tbanip
{
  std::string ip;
  std::int32_t time;
  std::string reason;
};

struct tbanhw
{
  std::string hw;
  std::int32_t time;
  std::string reason;
};

struct GlobalStateBanSystem
{
  std::vector<tbanip> bannediplist;
  std::vector<tbanhw> bannedhwlist;
  std::string lastban;
  std::string lastbanhw;
};

extern GlobalStateBanSystem gGlobalStateBanSystem;
