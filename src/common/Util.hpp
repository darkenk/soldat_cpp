#pragma once

#include "misc/SHA1Helper.hpp"
#include "FileUtility.hpp"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

using tcolor = std::uint32_t;
const std::uint32_t min_tcolor = 0;
const std::uint32_t max_tcolor = 0xffffffff;

using tstringarray = std::vector<std::string>;

struct tmapinfo
{
  std::string name;
  std::string mapname;
  std::uint64_t workshopid = -1;
  std::string path;
};
// how often does character appear in str
std::int32_t charcount(const char, const std::string &str1);

// splits a string, ie: 1|2|3 with a limit of 1, into 1 and 2|3
tstringarray splitstr(const std::string source, const char delimiter, std::int32_t limit = -1);

// Gets a specific piece of a string
std::string getpiece(const std::string source, const char delimiter, const std::int32_t piece);

// TODO: posex function returns values moved by 1 in comparision to pascal
std::int32_t posex(const std::string substr, std::string s, std::uint32_t offset = 1);

template <typename T>
T iif(const bool condition, T truepart, T falsepart)
{
  return condition ? truepart : falsepart;
}

template <typename T>
T &choose(const std::int32_t index, T choices[])
{
  return choices[index];
}

// TODO: colortohex - I don't floatly get how it should work
std::uint32_t colortohex(tcolor color);
#if 0 // not used
tcolor stringtocolor(const std::string s);
#endif

std::string numberformat(std::uint32_t num);

std::string overridefileext(FileUtility& fs, const std::string &filename, const std::string &ext);
// function MapExists(MapName: string; RootDirectory: string{$IFNDEF SERVER}; Checksum:
// TSHA1Digest{$ENDIF}): Boolean;
#if 0 // not now
std::string md5stringhelper(std::string text);
#endif
bool createdirifmissing(const std::string &dir);
bool createfileifmissing(const std::string &filename);
std::string getsize(int64_t bytes);
tsha1digest getmapchecksum(FileUtility &fs, const tmapinfo &map, const tsha1digest &defaultgamemodchecksum);
bool getmapinfo(FileUtility& fs, const std::string &mapname, const std::string &directory,
                tmapinfo &mapinfo); // dk out MapInfo
bool verifymapchecksum(FileUtility &fs, const tmapinfo &map, const tsha1digest &checksum,
                       const tsha1digest &defaultgamemodchecksum);
#ifdef DEVELOPMENT
void tostr(const void *avalue, ptypeinfo atypeinfo);
#endif
