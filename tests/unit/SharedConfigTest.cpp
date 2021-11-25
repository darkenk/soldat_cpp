#include "shared/SharedConfig.hpp"
#include "shared/Cvar.hpp"
#include "common/gfx.hpp"
#include "shared/misc/TIniFile.hpp"
#include "shared/misc/TMemoryStream.hpp"
#include <gtest/gtest.h>

static_assert(Config::GetModule() == Config::Module::TEST_MODULE,
              "Tests should be compiled with default module set to TEST_MODULE");

static_assert(Config::IsTest(), "Tests should be compiled with default module set to TEST_MODULE");

// just dummy to satisfy linker
void tsprite::freecontrols()
{
}

constexpr std::string_view BotIni = R"([BOT]
Name=Kruger
Color1=$00D85C12
Color2=$0097C69D
Skin_Color=$00E6B478
Hair_Color=$00515C79
Favourite_Weapon=Ruger 77
Secondary_Weapon=0
Friend=
Accuracy=8
Shoot_Dead=0
Grenade_Frequency=500
Camping=255
OnStartUse=255
Hair=4
Headgear=1
Chain=0
Chat_Frequency=6
Chat_Kill=You've just been erased
Chat_Dead=
Chat_LowHealth=
Chat_SeeEnemy=Drop your gun
)";

TEST(SharedConfigTest, SpritecHasSetBotControlMethod)
{
    TIniFile tf{ReadAsMemoryStream(BotIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    EXPECT_EQ(true, loadbotconfig(tf, sprite));
    EXPECT_EQ(bot, sprite.player->controlmethod);
    delete sprite.player;
}

TEST(SharedConfigTest, ReadFavouriteWeapon)
{
    TIniFile tf{ReadAsMemoryStream(BotIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    createweapons(false);
    EXPECT_EQ(true, loadbotconfig(tf, sprite));
    EXPECT_EQ(ruger77_num, sprite.brain.favweapon);
    delete sprite.player;
}

TEST(SharedConfigTest, ReadSecondaryWeapon)
{
    TIniFile tf{ReadAsMemoryStream(BotIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    createweapons(false);
    EXPECT_EQ(true, loadbotconfig(tf, sprite));
    EXPECT_EQ(0, sprite.player->secwep);
    delete sprite.player;
}

TEST(SharedConfigTest, FriendValueCanBeEmpty)
{
    TIniFile tf{ReadAsMemoryStream(BotIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    createweapons(false);
    EXPECT_EQ(true, loadbotconfig(tf, sprite));
    EXPECT_EQ("", sprite.brain.friend_);
    delete sprite.player;
}

TEST(SharedConfigTest, ShootSettings)
{
    TIniFile tf{ReadAsMemoryStream(BotIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    createweapons(false);
    CVar::bots_difficulty = 100;
    EXPECT_EQ(true, loadbotconfig(tf, sprite));
    EXPECT_EQ(8, sprite.brain.accuracy);
    EXPECT_EQ(0, sprite.brain.deadkill);
    EXPECT_EQ(500, sprite.brain.grenadefreq);
    EXPECT_EQ(255, sprite.brain.use);
    delete sprite.player;
}

TEST(SharedConfigTest, ParseChatSettings)
{
    TIniFile tf{ReadAsMemoryStream(BotIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    createweapons(false);
    EXPECT_EQ(true, loadbotconfig(tf, sprite));
    EXPECT_EQ(2.5 * 6, sprite.brain.chatfreq);
    EXPECT_EQ("You've just been erased", sprite.brain.chatkill);
    EXPECT_EQ("", sprite.brain.chatdead);
    EXPECT_EQ("", sprite.brain.chatlowhealth);
    EXPECT_EQ("Drop your gun", sprite.brain.chatseeenemy);
    EXPECT_EQ("", sprite.brain.chatwinning);
    delete sprite.player;
}

TEST(SharedConfigTest, ReadNameCamper)
{
    TIniFile tf{ReadAsMemoryStream(BotIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    createweapons(false);
    EXPECT_EQ(true, loadbotconfig(tf, sprite));
    EXPECT_EQ(255, sprite.brain.camper);
    EXPECT_STREQ("Kruger", sprite.player->name.c_str());
    delete sprite.player;
}

TEST(SharedConfigTest, ReadColor)
{
    TIniFile tf{ReadAsMemoryStream(BotIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    sprite.player->team = team_none;
    createweapons(false);
    EXPECT_EQ(true, loadbotconfig(tf, sprite));
    EXPECT_EQ(0x00D85C12, sprite.player->shirtcolor);
    EXPECT_EQ(0x0097C69D, sprite.player->pantscolor);
    EXPECT_EQ(0x00E6B478, sprite.player->skincolor);
    EXPECT_EQ(0x00515C79, sprite.player->haircolor);
    delete sprite.player;
}

TEST(SharedConfigTest, ReadHeadConfig)
{
    TIniFile tf{ReadAsMemoryStream(BotIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    createweapons(false);
    EXPECT_EQ(true, loadbotconfig(tf, sprite));
    EXPECT_EQ(4, sprite.player->hairstyle);
    EXPECT_EQ(GFX::GOSTEK_HELM, sprite.player->headcap);
    EXPECT_EQ(0, sprite.wearhelmet);
    EXPECT_EQ(0, sprite.player->chain);
    EXPECT_EQ(false, sprite.dummy);
    delete sprite.player;
}

TEST(SharedConfigTest, ReadInvalidOptionsReturnFalse)
{
    auto invalidIni = R"([BOT])";
    TIniFile tf{ReadAsMemoryStream(invalidIni)};
    tsprite sprite;
    sprite.player = new tplayer();
    createweapons(false);
    EXPECT_EQ(false, loadbotconfig(tf, sprite));
    delete sprite.player;
}

auto constexpr WeaponConfig = R"(
[Info]
Name=Default mod
Version=1.7.1

[Rambo Bow]
Damage=12
FireInterval=10
Ammo=1
ReloadTime=25
Speed=21
BulletStyle=7
StartUpTime=0
Bink=0
MovementAcc=0
BulletSpread=0
Recoil=0
Push=0.0148
InheritedVelocity=0.5
ModifierHead=1.15
ModifierChest=1
ModifierLegs=0.9
)";

TEST(SharedConfigTest, ReadWeaponsConfigModInfo)
{
    TIniFile tf{ReadAsMemoryStream(WeaponConfig)};
    std::string modname;
    std::string version;
    GunsDescription gunDesc;
    loadweaponsconfig(tf, modname, version, gunDesc);
    EXPECT_EQ("Default mod", modname);
    EXPECT_EQ("1.7.1", version);
}

TEST(SharedConfigTest, ReadWeaponsConfigRamboBow)
{
    TIniFile tf{ReadAsMemoryStream(WeaponConfig)};
    std::string modname;
    std::string version;
    GunsDescription gunDesc;
    gunDesc[bow].ininame = "Rambo Bow";
    EXPECT_EQ(true, loadweaponsconfig(tf, modname, version, gunDesc));
    const auto &gun = gunDesc[bow];
    EXPECT_EQ(12, gun.hitmultiply);
    EXPECT_EQ(10, gun.fireinterval);
    EXPECT_EQ(1, gun.ammo);
    EXPECT_EQ(25, gun.reloadtime);
    EXPECT_EQ(21, gun.speed);
    EXPECT_EQ(7, gun.bulletstyle);
    EXPECT_EQ(0, gun.startuptime);
    EXPECT_EQ(0, gun.bink);
    EXPECT_EQ(0, gun.movementacc);
    EXPECT_EQ(0, gun.bulletspread);
    EXPECT_EQ(0, gun.recoil);
    EXPECT_EQ(0.0148f, gun.push);
    EXPECT_EQ(0.5f, gun.inheritedvelocity);
    EXPECT_EQ(0.9f, gun.modifierlegs);
    EXPECT_EQ(1.0f, gun.modifierchest);
    EXPECT_EQ(1.15f, gun.modifierhead);
    EXPECT_EQ(0, gun.nocollision);
}

TEST(SharedConfigTest, ReadWeaponsReturnsFalseInCaseOfMissingEntries)
{
    auto constexpr InvalidConfig = R"([Info]
        Name=Default mod
        Version=1.7.1

        [Rambo Bow]
        Damage=12)";
    TIniFile tf{ReadAsMemoryStream(InvalidConfig)};
    std::string modname;
    std::string version;
    GunsDescription gunDesc;
    gunDesc[bow].ininame = "Rambo Bow";
    EXPECT_EQ(false, loadweaponsconfig(tf, modname, version, gunDesc));
}
