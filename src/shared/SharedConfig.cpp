// automatically converted

#include "SharedConfig.hpp"
#include "../server/Server.hpp"
#include "Constants.hpp"
#include "Game.hpp"
#include "Logging.hpp"
#include "Util.hpp"
#include "Weapons.hpp"
#include "misc/TIniFile.hpp"
#include "network/Net.hpp"
#include "shared/gfx.hpp"
#include <sstream>

// procedure ReadConfColor(conf: TStringList; const SectionName: string; var VarName: LongWord);
// overload; begin
//  try
//    if conf.Values[SectionName] <> '' then
//    begin
//      VarName := LongWord(ColorToHex(StringToColor(conf.Values[SectionName])))
//    end;
//    else
//      raise Exception.Create('Value "' + SectionName + '" not found');
//  except
//    raise Exception.Create('Value "' + SectionName + '" is not a number');
//  end;
// end;

// procedure ReadConfMagicColor(conf: TStringList; const SectionName: string; var VarName:
// LongWord); overload; begin
//  try
//    if conf.Values[SectionName] <> '' then
//      VarName := StringToColor(conf.Values[SectionName])
//    else
//      raise Exception.Create('Value "' + SectionName + '" not found');
//  except
//    raise Exception.Create('Value "' + SectionName + '" is not a number');
//  end;
// end;

// procedure ReadConf(conf: TStringList; const SectionName: string; var VarName: Integer); overload;
// begin
//  try
//    if conf.Values[SectionName] <> '' then
//      VarName := StrToInt(conf.Values[SectionName])
//    else
//      raise Exception.Create('Value "' + SectionName + '" not found');
//  except
//    raise Exception.Create('Value "' + SectionName + '" is not a number');
//  end;
// end;
//
// procedure ReadConf(conf: TStringList; const SectionName: string; var VarName: Byte); overload;
// begin
//  try
//    if conf.Values[SectionName] <> '' then
//      VarName := StrToInt(conf.Values[SectionName])
//    else
//      raise Exception.Create('Value "' + SectionName + '" not found');
//  except
//    raise Exception.Create('Value "' + SectionName + '" is not a number');
//  end;
// end;
//
// procedure ReadConf(conf: TStringList; const SectionName: string; var VarName: string;
//  AllowBlank: Boolean = False); overload;
// begin
//  if (conf.Values[SectionName] <> '') or AllowBlank then
//    VarName := conf.Values[SectionName]
//  else
//    raise Exception.Create('Value "' + SectionName + '" not found');
// end;
//
// procedure ReadConf(conf: TStringList; const SectionName: string; var VarName: Boolean;
//  AllowBlank: Boolean = False); overload;
// begin
//  if (conf.Values[SectionName] <> '') or AllowBlank then
//    VarName := conf.Values[SectionName] = '1'
//  else
//    raise Exception.Create('Value "' + SectionName + '" not found');
// end;
//
// procedure ReadWMConf(conf: TStringList; const SectionName: string; var VarName: Word); overload;
// begin
//  try
//    if conf.Values[SectionName] <> '' then
//      VarName := StrToInt(conf.Values[SectionName]);
//  except
//    raise Exception.Create('Value "' + SectionName + '" is not a number');
//  end;
// end;
//
// procedure ReadWMConf(conf: TStringList; const SectionName: string; var VarName: SmallInt);
// overload; begin
//  try
//    if conf.Values[SectionName] <> '' then
//      VarName := StrToInt(conf.Values[SectionName]);
//  except
//    raise Exception.Create('Value "' + SectionName + '" is not a number');
//  end;
// end;
//
// procedure ReadWMConf(conf: TStringList; const SectionName: string; var VarName: Byte); overload;
// begin
//  try
//    if conf.Values[SectionName] <> '' then
//      VarName := StrToInt(conf.Values[SectionName]);
//  except
//    raise Exception.Create('Value "' + SectionName + '" is not a number');
//  end;
// end;
//
// procedure ReadWMConf(conf: TStringList; const SectionName: string; var VarName: Single);
// overload; begin
//  try
//    if conf.Values[SectionName] <> '' then
//      VarName := StrToFloat(conf.Values[SectionName]);
//  except
//    raise Exception.Create('Value "' + SectionName + '" is not a number');
//  end;
// end;
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
        out = conf.at(entry.data());
    }
}

void ReadConf(const TIniFile::Entries &conf, const std::string_view entry, bool &out,
              bool allowEmpty = false)
{
    std::string s;
    ReadConf(conf, entry, s, allowEmpty);
    if (s.empty())
    {
        out = true;
        return;
    }
    out = std::stoi(s) == 1;
}

template <typename T>
void ReadConf(const TIniFile::Entries &conf, const std::string_view entry, T &out,
              bool allowEmpty = false)
{
    std::string s;
    ReadConf(conf, entry, s, allowEmpty);
    out = std::stoi(s);
}

void ReadConfColor(const TIniFile::Entries &conf, const std::string_view entry, std::uint32_t &out,
                   bool allowEmpty = false)
{
    std::string s;
    ReadConf(conf, entry, s, allowEmpty);
    Assert(s[0] == '$');
    s.erase(0, 1);
    out = std::stoi(s, 0, 16);
}

} // namespace

bool loadbotconfig(TIniFile &ini, tsprite &spritec)
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
        spritec.brain.favweapon = weaponnametonum(favweaponname);

        ReadConf(conf, "Secondary_Weapon", spritec.player->secwep);
        ReadConf(conf, "Friend", spritec.brain.friend_, true);
        ReadConf(conf, "Accuracy", spritec.brain.accuracy);
        spritec.brain.accuracy =
            trunc(spritec.brain.accuracy * ((float)(CVar::bots_difficulty) / 100));
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

        NotImplemented(NITag::OTHER);
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

bool loadweaponsconfig(const std::string &filepath)
{
    NotImplemented(NITag::OTHER);
    return false;
#if 0
    tmeminifile ini;
    tstringlist conf;
    std::int32_t weaponindex;
    struct tgun *gun;
    std::string filename;

    bool loadweaponsconfig_result;
    result = false;
    ini = nullptr;
    conf = nullptr;

    if (!fileexists(filepath))
        return loadweaponsconfig_result;

    //  try
    conf = tstringlist.create;
    ini = tmeminifile.create(filepath);

    if (ini.sectionexists("Info"))
    {
        ini.readsectionvalues("Info", conf);
        readconf(conf, "Name", wmname);
        readconf(conf, "Version", wmversion);
    }
    //    else
    //      raise Exception.Create('Section "[Info]" not found');

    for (weaponindex = 1; weaponindex <= original_weapons; weaponindex++)
    {
        gun = &guns[weaponindex];

        if (ini.sectionexists(gun.ininame))
        {
            ini.readsectionvalues(gun.ininame, conf);

            readwmconf(conf, "Damage", gun.hitmultiply);
            readwmconf(conf, "FireInterval", gun.fireinterval);
            readwmconf(conf, "Ammo", gun.ammo);
            readwmconf(conf, "ReloadTime", gun.reloadtime);
            readwmconf(conf, "Speed", gun.speed);
            readwmconf(conf, "BulletStyle", gun.bulletstyle);
            readwmconf(conf, "StartUpTime", gun.startuptime);
            readwmconf(conf, "Bink", gun.bink);
            readwmconf(conf, "MovementAcc", gun.movementacc);
            readwmconf(conf, "BulletSpread", gun.bulletspread);
            readwmconf(conf, "Recoil", gun.recoil);
            readwmconf(conf, "Push", gun.push);
            readwmconf(conf, "InheritedVelocity", gun.inheritedvelocity);
            readwmconf(conf, "ModifierLegs", gun.modifierlegs);
            readwmconf(conf, "ModifierChest", gun.modifierchest);
            readwmconf(conf, "ModifierHead", gun.modifierhead);
            readwmconf(conf, "NoCollision", gun.nocollision);
        }
    }

    buildweapons();

    result = true;
    //  except
    //    on e : Exception do
    //    begin
    //      Filename := RightStr(FilePath, Length(FilePath) - LastDelimiter('\', FilePath));
    //      mainconsole.console(Filename + ': ' + e.message, WARNING_MESSAGE_COLOR);
    //    end;
    //  end;

    ini.free;
    conf.free;
    return loadweaponsconfig_result;
#endif
}
