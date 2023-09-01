// automatically converted
#include "Util.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include <filesystem>
#include <fstream>
#include <physfs.h>

std::int32_t charcount(const char character, const std::string &str1)
{
  std::int32_t result = 0;
  for (std::size_t i = 0; i < str1.length(); i++)
  {
    if (character == str1[i])
    {
      result += 1;
    }
  }
  return result;
}

tstringarray splitstr(const std::string source, const char delimiter, std::int32_t limit)
{

  size_t last = 0;
  size_t next = 0;
  tstringarray ret;
  while (((next = source.find(delimiter, last)) != std::string::npos) && limit--)
  {
    ret.push_back(source.substr(last, next - last));
    last = next + 1;
  }
  ret.push_back(source.substr(last));
  return ret;
}

std::string getpiece(const std::string source, const char delimiter, const std::int32_t piece)
{
  return splitstr(source, delimiter, piece)[piece - 1];
}

std::int32_t posex(const std::string substr, std::string s, std::uint32_t offset)
{
  return s.find(substr, offset);
}

std::uint32_t colortohex(tcolor color)
{
  return color;
}

#if 0 // not used
tcolor std::stringtocolor(const std::string s)
{
    tcolor std::stringtocolor_result;
    result = strtoint(s);
    return std::stringtocolor_result;
}
#endif

std::string numberformat(std::uint32_t num)
{
  return std::to_string(num);
}

std::string overridefileext(const std::string &filename, const std::string &ext)
{
  auto result = filename;
  auto dotPos = filename.find_last_of('.');
  if (dotPos == std::string::npos)
  {
    return result;
  }
  if (filename.substr(dotPos) != ext)
  {
    result.replace(dotPos, result.length(), ext);
    if (not PHYSFS_exists(result.c_str()))
    {
      result = filename;
    }
  }
  return result;
}

// function MapExists(MapName: std::string; RootDirectory: std::string{$IFNDEF SERVER}; Checksum:
// TSHA1Digest{$ENDIF}): Boolean;
/*begin
  Result := False;
  if PHYSFS_exists(PChar('maps/' + MapName + '.pms')) then
    Result := True
  else if FileExists(RootDirectory + 'maps/' + MapName + '.smap') then
    if Sha1Match(GetMapChecksum(MapName, RootDirectory), Checksum) then
      Result := True;
end;*/

#if 0
std::string md5stringhelper(std::string text)
{
    std::string md5std::stringhelper_result;
    result = md5print(md5std::string(text));
    return md5std::stringhelper_result;
}
#endif

// makes sure the directory exists
// returns false on error and true if everything is allright
bool createdirifmissing(const std::string &dir)
{
  std::filesystem::path p(dir);
  if (std::filesystem::is_directory(p))
  {
    return true;
  }
  return std::filesystem::create_directory(p);
}

bool createfileifmissing(const std::string &filename)
{
  std::filesystem::path p(filename);
  if (std::filesystem::is_regular_file(p))
  {
    return true;
  }
  std::ofstream d(filename);
  return true;
}

std::string getsize(std::int64_t bytes)
{
  std::int64_t filesize;

  std::string getsize_result;

  if (bytes < 1024)
  {
    return std::to_string(bytes) + " B";
  }
  filesize = bytes / 1024;
  if (filesize > 1024)
  {
    filesize = filesize / 1024;

    if (filesize > 1024)
    {
      return std::to_string(filesize / 1024) + " Gb";
    }
    else
    {
      return std::to_string(filesize) + " Mb";
    }
  }
  return std::to_string(filesize) + " Kb";
}

bool verifymapchecksum(const tmapinfo &map, const tsha1digest &checksum,
                       const tsha1digest &defaultgamemodchecksum)
{
  return getmapchecksum(map, defaultgamemodchecksum) == checksum;
}

tsha1digest getmapchecksum(const tmapinfo &map, const tsha1digest &defaultgamemodchecksum)
{
  if (PHYSFS_exists((pchar)(std::string("maps/") + map.mapname + ".pms")))
  {
    return defaultgamemodchecksum;
  }
  else if (fileexists(map.path))
  {
    return sha1file(map.path);
  }
  return tsha1digest{};
}

// for string delimiter
std::vector<std::string> split_string(std::string s, std::string delimiter)
{
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
  {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

bool getmapinfo(const std::string &mapname, const std::string &directory,
                tmapinfo &mapinfo) // dk out MapInfo
{
  tstringarray split;
  std::uint64_t itemid;

  bool result;
  result = false;

  if (mapname.rfind("workshop/") == 0)
  {
    split = split_string(mapname, "/");
    if (length(split) >= 2)
    {
      itemid = strtointdef(split[1], 0);
      if (itemid > 0)
      {
        mapinfo.workshopid = itemid;
        mapinfo.mapname = split[2];
        if (fileexists(directory + "maps/" + mapname + ".smap"))
        {
          mapinfo.path = directory + "maps/" + mapname + ".smap";
          mapinfo.name = mapname;
          result = true;
        }
      }
    }
  }
  else
  {
    if (PHYSFS_exists((pchar)(std::string("maps/") + mapname + ".pms")))
    {
      mapinfo.name = mapname;
      mapinfo.mapname = mapname;
      mapinfo.workshopid = 0;
      mapinfo.path = "smod";
      result = true;
    }
    else
    {
      if (fileexists(directory + "maps/" + mapname + ".smap"))
      {
        mapinfo.name = mapname;
        mapinfo.mapname = mapname;
        mapinfo.workshopid = 0;
        mapinfo.path = directory + "maps/" + mapname + ".smap";
        result = true;
      }
    }
  }
  return result;
}

#if 0

#ifdef DEVELOPMENT
// Source: https://github.com/correaelias/TypeUtils

typedef array of std::uint8_t tarray;

void tostr(const void *avalue, ptypeinfo atypeinfo)
{
    std::uint32_t i;
    tformatsettings formatsettings;
    pmanagedfield firstfield, field;
    sizeint elementsize;

    void tostr_result;
    formatsettings = default_(tformatsettings);

    switch (atypeinfo->kind)
    {
    case tkchar:
        result = quotedstr((char)(avalue));
        break;
    case tkwchar:
        result = quotedstr(ansistd::string(wchar(avalue)));
        break;
    case tkbool:
        result = booltostr((bool)(avalue), true);
        break;
    case tkint64:
        result = inttostr(int64(avalue));
        break;
    case tkqstd::uint64_t:
        result = inttostr(qstd::uint64_t(avalue));
        break;
    case tksstd::string:
        result = quotedstr(shortstd::string(avalue));
        break;
    case tkastd::string:
        result = quotedstr(ansistd::string(avalue));
        break;
    case tkwstd::string:
        result = quotedstr(ansistd::string(widestd::string(avalue)));
        break;
    case tkustd::string:
        result = quotedstr(ansistd::string(unicodestd::string(avalue)));
        break;
    case tkclass:
        result = tobject(avalue).tostd::string;
        break;
    case tkenumeration:
        result = getenumname(atypeinfo, (std::int32_t)(avalue));
        break;
    case tkset:
        result = settostd::string(atypeinfo, (std::int32_t)(avalue), true).replace(',', ", ");
        break;
    case tkvariant:
        result = vartostr(variant(avalue));
        break;
    case tkstd::int32_t: {
        switch (gettypedata(atypeinfo)->ordtype)
        {
        case otsstd::uint8_t:
            result = inttostr((std::int8_t)(avalue));
            break;
        case otustd::uint8_t:
            result = inttostr((std::uint8_t)(avalue));
            break;
        case otsstd::uint64_t:
            result = inttostr(std::int16_t(avalue));
            break;
        case otustd::uint64_t:
            result = inttostr((std::uint64_t)(avalue));
            break;
        case otslong:
            result = inttostr((std::uint32_t)(avalue));
            break;
        case otulong:
            result = inttostr(std::uint32_t(avalue));
            break;
            // otSQWord: Result := IntToStr(Int64(AValue));
            // otUQWord: Result := IntToStr(QWord(AValue));
        }
    }
    break;
    case tkfloat: {
        fillstd::uint8_t(formatsettings, sizeof(tformatsettings), 0);
        formatsettings.decimalseparator = '.';
        switch (gettypedata(atypeinfo)->floattype)
        {
        case ftfloat:
            result = formatfloat("0.######", float(avalue), formatsettings);
            break;
        case ftdouble:
            result = formatfloat("0.######", (double)(avalue), formatsettings);
            break;
        case ftdouble:
            result = formatfloat("0.######", double(avalue), formatsettings);
            break;
        case ftcomp:
            result = formatfloat("0.######", comp(avalue), formatsettings);
            break;
        case ftcurr:
            result = formatfloat("0.######", currency(avalue), formatsettings);
            break;
        }
    }
    break;
    case tkrecord: {
        result = '(';
        {
            void &with = gettypedata(atypeinfo);

#ifndef VER3_0 // ifdef needed because of a field rename in trunk (ManagedFldCount to \
    // TotalFieldCount)
            firstfield = pmanagedfield(std::uint8_t*(&totalfieldcount) + sizeof(totalfieldcount));
            for (i = 0; i <= totalfieldcount - 1; i++)
#else
            firstfield = pmanagedfield(std::uint8_t*(&managedfldcount) + sizeof(managedfldcount));
            for (i = 0; i <= managedfldcount - 1; i++)
#endif
            {
                if (i > 0)
                    result += ", ";
                field = pmanagedfield(std::uint32_t(std::uint8_t*(firstfield)) +
                                      (i * std::uint32_t(sizeof(tmanagedfield))));
                result += tostr(&{{std::uint8_t*(&avalue) + field->fldoffset}}, field->typeref);
            }
        }
        result += ')';
    }
    break;
    case tkarray: {
        result = '[';
        {
            void &with = gettypedata(atypeinfo);

            elementsize = arraydata.size / arraydata.elcount;
            for (i = 0; i <= arraydata.elcount - 1; i++)
            {
                if (i > 0)
                    result += ", ";
                result += tostr(&*{{std::uint32_t(std::uint8_t*(&avalue)) + (i * std::uint32_t(elementsize))}},
                                arraydata.eltype);
            }
        }
        result += ']';
    }
    break;
    case tkdynarray: {
        result = '[';
        {
            void &with = gettypedata(atypeinfo);

            for (i = 0; i <= length((tarray)(avalue)) - 1; i++)
            {
                if (i > 0)
                    result += ", ";
                result += tostr(&*{{std::uint32_t(std::uint8_t*(&(tarray)(avalue)[0])) + (i * std::uint32_t(elsize))}},
                                eltype2);
            }
        }
        result += ']';
    }
    break;
    default:
        result = format("%s@%p", set::of(atypeinfo->name, &avalue, eos));
    }
    return tostr_result;
}
#endif

#endif
