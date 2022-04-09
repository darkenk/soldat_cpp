// automatically converted

#include "SharedConfig.hpp"
#include "../server/Server.hpp"
#include "Constants.hpp"
#include "Game.hpp"
#include "common/Logging.hpp"
#include "common/Util.hpp"
#include "common/Weapons.hpp"
#include "common/gfx.hpp"
#include "common/misc/TIniFile.hpp"
#include "network/Net.hpp"
#include "shared/mechanics/Sprites.hpp"
#include <sstream>

namespace
{

void ReadConf(const TIniFile::Entries &conf, const std::string_view entry, std::string &out,
              bool allowEmpty = false)
{
  if (allowEmpty && !conf.contains(entry.data()))
  {
    out = "";
  }
  else
  {
    try
    {
      out = conf.at(entry.data());
    }
    catch (std::out_of_range &ex)
    {
      LogWarnG("{} is missing in config file", entry);
      throw ex;
    }
  }
}

void ReadConf(const TIniFile::Entries &conf, const std::string_view entry, bool &out,
              bool allowEmpty = false)
{
  std::string s;
  ReadConf(conf, entry, s, allowEmpty);
  if (s.empty())
  {
    out = false;
    return;
  }
  out = std::stoi(s) == 1;
}

void ReadConf(const TIniFile::Entries &conf, const std::string_view entry, float &out,
              bool allowEmpty = false)
{
  std::string s;
  ReadConf(conf, entry, s, allowEmpty);
  out = std::stof(s);
}

template <typename T>
void ReadConf(const TIniFile::Entries &conf, const std::string_view entry, T &out,
              bool allowEmpty = false) requires std::is_integral<T>::value
{
  std::string s;
  ReadConf(conf, entry, s, allowEmpty);
  if (s.empty())
  {
    out = T{0};
    return;
  }
  out = std::stoi(s);
}

void ReadConfColor(const TIniFile::Entries &conf, const std::string_view entry, std::uint32_t &out,
                   bool allowEmpty = false)
{
  std::string s;
  ReadConf(conf, entry, s, allowEmpty);
  SoldatAssert(s[0] == '$');
  s.erase(0, 1);
  out = std::stoi(s, 0, 16);
}

} // namespace

bool loadbotconfig(TIniFile &ini, tsprite &spritec, GunsDescription &guns)
{
  TIniFile::Entries conf;
  std::string filename;
  std::string favweaponname;
  std::uint8_t headgear;

  if (!ini.ReadSectionValues("BOT", conf))
  {
    return false;
  }
  try
  {
    ReadConf(conf, "Favourite_Weapon", favweaponname);
    spritec.brain.favweapon = weaponnametonum(favweaponname, guns);

    ReadConf(conf, "Secondary_Weapon", spritec.player->secwep);
    ReadConf(conf, "Friend", spritec.brain.friend_, true);
    ReadConf(conf, "Accuracy", spritec.brain.accuracy);
    spritec.brain.accuracy = trunc(spritec.brain.accuracy * ((float)(CVar::bots_difficulty) / 100));
    ReadConf(conf, "Shoot_Dead", spritec.brain.deadkill);
    ReadConf(conf, "Grenade_Frequency", spritec.brain.grenadefreq);
    ReadConf(conf, "OnStartUse", spritec.brain.use);

    ReadConf(conf, "Chat_Frequency", spritec.brain.chatfreq);
    spritec.brain.chatfreq = round(2.5 * spritec.brain.chatfreq);
    ReadConf(conf, "Chat_Kill", spritec.brain.chatkill, true);
    ReadConf(conf, "Chat_Dead", spritec.brain.chatdead, true);
    ReadConf(conf, "Chat_LowHealth", spritec.brain.chatlowhealth, true);
    ReadConf(conf, "Chat_SeeEnemy", spritec.brain.chatseeenemy, true);
    ReadConf(conf, "Chat_Winning", spritec.brain.chatwinning, true);

    ReadConf(conf, "Camping", spritec.brain.camper);

    ReadConf(conf, "Name", spritec.player->name);

    NotImplemented();
    // setlength(spritec.player.name, min(length(spritec.player.name), playername_chars));

    if (spritec.player->team == team_none)
      ReadConfColor(conf, "Color1", spritec.player->shirtcolor);
    ReadConfColor(conf, "Color2", spritec.player->pantscolor);
    ReadConfColor(conf, "Skin_Color", spritec.player->skincolor);
    ReadConfColor(conf, "Hair_Color", spritec.player->haircolor);
    spritec.player->jetcolor = (default_jetcolor & 0xffffff) + color_transparency_bot;

    ReadConf(conf, "Hair", spritec.player->hairstyle);

    ReadConf(conf, "Headgear", headgear);
    if (headgear == 0)
    {
      spritec.player->headcap = 0;
    }
    else if (headgear == 2)
    {
      spritec.player->headcap = GFX::GOSTEK_KAP;
    }
    else
    {
      spritec.player->headcap = GFX::GOSTEK_HELM;
    }

    spritec.wearhelmet = spritec.player->headcap == 0 ? 1 : 0;

    ReadConf(conf, "Chain", spritec.player->chain);
    ReadConf(conf, "Dummy", spritec.dummy, true);
    spritec.player->controlmethod = bot;
    spritec.freecontrols();
  }
  catch (std::out_of_range &ex)
  {
    LogWarnG("Cannot parse bot config");
    return false;
  }
  return true;
}

bool loadweaponsconfig(TIniFile &iniFile, std::string &modname, std::string &modversion,
                       GunsDescription &gunDesc)
{
  TIniFile::Entries conf;

  if (iniFile.ReadSectionValues("Info", conf))
  {
    ReadConf(conf, "Name", modname);
    ReadConf(conf, "Version", modversion);
  }
  else
  {
    LogWarnG("Weapon mod does not contain [Info] section. Skip loading");
    return false;
  }

  try
  {
    for (auto &gun : gunDesc)
    {
      conf.clear();
      if (!iniFile.ReadSectionValues(gun.ininame, conf))
      {
        continue;
      }
      ReadConf(conf, "Damage", gun.hitmultiply);
      ReadConf(conf, "FireInterval", gun.fireinterval);
      ReadConf(conf, "Ammo", gun.ammo);
      ReadConf(conf, "ReloadTime", gun.reloadtime);
      ReadConf(conf, "Speed", gun.speed);
      ReadConf(conf, "BulletStyle", gun.bulletstyle);
      ReadConf(conf, "StartUpTime", gun.startuptime);
      ReadConf(conf, "Bink", gun.bink);
      ReadConf(conf, "MovementAcc", gun.movementacc);
      ReadConf(conf, "BulletSpread", gun.bulletspread);
      ReadConf(conf, "Recoil", gun.recoil);
      ReadConf(conf, "Push", gun.push);
      ReadConf(conf, "InheritedVelocity", gun.inheritedvelocity);
      ReadConf(conf, "ModifierLegs", gun.modifierlegs);
      ReadConf(conf, "ModifierChest", gun.modifierchest);
      ReadConf(conf, "ModifierHead", gun.modifierhead);
      ReadConf(conf, "NoCollision", gun.nocollision, true);
    }
  }
  catch (std::out_of_range &ex)
  {
    LogWarnG("Cannot parse weapon config");
    return false;
  }

  return true;
}
