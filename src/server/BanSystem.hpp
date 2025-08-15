#pragma once

#include "common/Constants.hpp"
#include <cstdint>
#include <string>
#include <vector>

/*#include "Constants.h"*/

// bans hardwareids

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
  bool checkbannedhw(const std::string &hw);
  bool checkbannedip(const std::string &ip);
  bool delbannedhw(const std::string &hw);
  bool delbannedip(const std::string &ip);
  std::int32_t findban(const std::string &ip);
  std::int32_t findbanhw(const std::string &hw);
  void addbannedhw(const std::string &hw, std::string reason,
                   std::int32_t duration = Constants::PERMAMENT);
  void addbannedip(const std::string &ip, std::string reason,
                   std::int32_t duration = Constants::PERMAMENT);
  void loadbannedlist(const std::string &filename);
  void loadbannedlisthw(const std::string &filename);
  void savebannedlist(const std::string &filename);
  void savebannedlisthw(const std::string &filename);
  void updatehwbanlist();
  void updateipbanlist();
  std::vector<tbanip> bannediplist = {};
  std::vector<tbanhw> bannedhwlist = {};
  std::string lastban = {};
  std::string lastbanhw = {};
};

extern GlobalStateBanSystem gGlobalStateBanSystem;
