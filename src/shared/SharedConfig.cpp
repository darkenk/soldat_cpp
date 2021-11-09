// automatically converted

#include "SharedConfig.hpp"
#include "../../server/Server.hpp"
#include "Constants.hpp"
#include "Game.hpp"
#include "Util.hpp"
#include "Weapons.hpp"
#include "network/Net.hpp"

/*#include "Game.h"*/
/*#include "IniFiles.h"*/
/*#include "Classes.h"*/
/*#include "SysUtils.h"*/
/*#include "StrUtils.h"*/
/*#include "Math.h"*/
/*#include "Util.h"*/
/*#include "Server.h"*/
/*#include "Net.h"*/
/*#include "Weapons.h"*/
/*#include "Constants.h"*/

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
#ifdef SERVER
bool loadbotconfig(const std::string filepath, tsprite &spritec)
{
    NotImplemented(NITag::OTHER);
    return false;
#if 0
    tmeminifile ini;
    tstringlist conf;
    std::string filename;
    std::string favweaponname;
    = '';
    std::uint8_t headgear;
    = 0;

    bool loadbotconfig_result;
    result = false;
    ini = nullptr;
    conf = nullptr;

    //  try
    //    if not FileExists(FilePath) then
    //      raise Exception.Create('Bot file not found');

    conf = tstringlist.create;
    ini = tmeminifile.create(filepath);

    if (ini.sectionexists("BOT"))
    {
        ini.readsectionvalues("BOT", conf);

        readconf(conf, "Favourite_Weapon", favweaponname);
        spritec.brain.favweapon = weaponnametonum(favweaponname);
        readconf(conf, "Secondary_Weapon", spritec.player.secwep);
        readconf(conf, "Friend", spritec.brain.friend_, true);
        readconf(conf, "Accuracy", spritec.brain.accuracy);
        spritec.brain.accuracy =
            trunc(spritec.brain.accuracy * ((float)(CVar::bots_difficulty) / 100));
        readconf(conf, "Shoot_Dead", spritec.brain.deadkill);
        readconf(conf, "Grenade_Frequency", spritec.brain.grenadefreq);
        readconf(conf, "OnStartUse", spritec.brain.use);

        readconf(conf, "Chat_Frequency", spritec.brain.chatfreq);
        spritec.brain.chatfreq = round(2.5 * spritec.brain.chatfreq);
        readconf(conf, "Chat_Kill", spritec.brain.chatkill, true);
        readconf(conf, "Chat_Dead", spritec.brain.chatdead, true);
        readconf(conf, "Chat_LowHealth", spritec.brain.chatlowhealth, true);
        readconf(conf, "Chat_SeeEnemy", spritec.brain.chatseeenemy, true);
        readconf(conf, "Chat_Winning", spritec.brain.chatwinning, true);

        readconf(conf, "Camping", spritec.brain.camper);

        readconf(conf, "Name", spritec.player.name);
        setlength(spritec.player.name, min(length(spritec.player.name), playername_chars));

        if (spritec.player.team == team_none)
            readconfcolor(conf, "Color1", spritec.player.shirtcolor);
        readconfcolor(conf, "Color2", spritec.player.pantscolor);
        readconfmagiccolor(conf, "Skin_Color", spritec.player.skincolor);
        readconfcolor(conf, "Hair_Color", spritec.player.haircolor);
        spritec.player.jetcolor = (default_jetcolor & 0xffffff) + color_transparency_bot;
        readconf(conf, "Hair", spritec.player.hairstyle);

        readconf(conf, "Headgear", headgear);
        if (headgear == 0)
            spritec.player.headcap = 0;
        else if (headgear == 2)
            spritec.player.headcap = gfx_gostek_kap;
        else
            spritec.player.headcap = gfx_gostek_helm;

        if (spritec.player.headcap == 0)
            spritec.wearhelmet = 0;
        else
            spritec.wearhelmet = 1;

        readconf(conf, "Chain", spritec.player.chain);
        readconf(conf, "Dummy", spritec.dummy, true);

        spritec.player.controlmethod = bot;
        spritec.freecontrols;
    }
    //    else
    //      raise Exception.Create('Section "[Bot]" not found');

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
    return loadbotconfig_result;
#endif
}
#endif
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
