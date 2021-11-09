// automatically converted

#include "Weapons.hpp"
#include "Constants.hpp"
#include "Logging.hpp"
#include "gfx.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"
/*#include "TraceLog.h"*/
/*#include "Constants.h"*/
/*#include "SysUtils.h"*/

PascalArray<tgun, 1, total_weapons> guns;
PascalArray<tgun, 1, total_weapons> defaultguns;
std::uint64_t defaultwmchecksum, loadedwmchecksum;

void createweapons(bool floatisticmode)
{
    createweaponsbase();
    createdefaultweapons(floatisticmode);
}

void createdefaultweapons(bool floatisticmode)
{
    tgun *gun;
    tgun *defaultgun;
    std::int32_t weaponindex;

    if (floatisticmode)
    {
        createfloatisticweapons();
    }
    else
    {
        createnormalweapons();
    }

    // Set defaults for weapon menu selection comparisons
    for (weaponindex = guns.StartIdx(); weaponindex <= guns.EndIdx(); weaponindex++)
    {
        gun = &guns[weaponindex];
        defaultgun = &defaultguns[weaponindex];

        defaultgun->hitmultiply = gun->hitmultiply;
        defaultgun->fireinterval = gun->fireinterval;
        defaultgun->ammo = gun->ammo;
        defaultgun->reloadtime = gun->reloadtime;
        defaultgun->speed = gun->speed;
        defaultgun->bulletstyle = gun->bulletstyle;
        defaultgun->startuptime = gun->startuptime;
        defaultgun->bink = gun->bink;
        defaultgun->movementacc = gun->movementacc;
        defaultgun->bulletspread = gun->bulletspread;
        defaultgun->recoil = gun->recoil;
        defaultgun->push = gun->push;
        defaultgun->inheritedvelocity = gun->inheritedvelocity;
        defaultgun->modifierlegs = gun->modifierlegs;
        defaultgun->modifierchest = gun->modifierchest;
        defaultgun->modifierhead = gun->modifierhead;
    }

    buildweapons();
}

void createweaponsbase()
{
    tgun *gun;

    // Desert Eagle
    gun = &guns[eagle];
    gun->name = "Desert Eagles";
    gun->ininame = gun->name;
    gun->num = eagle_num;
    gun->texturenum = GFX::WEAPONS_DEAGLES;
    gun->cliptexturenum = GFX::WEAPONS_DEAGLES_CLIP;
    gun->clipreload = true;
    gun->bulletimagestyle = GFX::WEAPONS_DEAGLES_BULLET;
    gun->firestyle = GFX::WEAPONS_DEAGLES_FIRE;
    gun->firemode = 2;

    // MP5
    gun = &guns[mp5];
    gun->name = "HK MP5";
    gun->ininame = gun->name;
    gun->num = mp5_num;
    gun->texturenum = GFX::WEAPONS_MP5;
    gun->cliptexturenum = GFX::WEAPONS_MP5_CLIP;
    gun->clipreload = true;
    gun->bulletimagestyle = GFX::WEAPONS_MP5_BULLET;
    gun->firestyle = GFX::WEAPONS_MP5_FIRE;
    gun->firemode = 0;

    // AK-74
    gun = &guns[ak74];
    gun->name = "Ak-74";
    gun->ininame = gun->name;
    gun->num = ak74_num;
    gun->texturenum = GFX::WEAPONS_AK74;
    gun->cliptexturenum = GFX::WEAPONS_AK74_CLIP;
    gun->clipreload = true;
    gun->bulletimagestyle = GFX::WEAPONS_AK74_BULLET;
    gun->firestyle = GFX::WEAPONS_AK74_FIRE;
    gun->firemode = 0;

    // Steyr AUG
    gun = &guns[steyraug];
    gun->name = "Steyr AUG";
    gun->ininame = gun->name;
    gun->num = steyraug_num;
    gun->texturenum = GFX::WEAPONS_STEYR;
    gun->cliptexturenum = GFX::WEAPONS_STEYR_CLIP;
    gun->clipreload = true;
    gun->bulletimagestyle = GFX::WEAPONS_STEYR_BULLET;
    gun->firestyle = GFX::WEAPONS_STEYR_FIRE;
    gun->firemode = 0;

    // SPAS-12
    gun = &guns[spas12];
    gun->name = "Spas-12";
    gun->ininame = gun->name;
    gun->num = spas12_num;
    gun->texturenum = GFX::WEAPONS_SPAS;
    gun->cliptexturenum = 0;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_SPAS_FIRE;
    gun->firemode = 2;

    // Ruger 77
    gun = &guns[ruger77];
    gun->name = "Ruger 77";
    gun->ininame = gun->name;
    gun->num = ruger77_num;
    gun->texturenum = GFX::WEAPONS_RUGER;
    gun->cliptexturenum = 0;
    gun->clipreload = false;
    gun->bulletimagestyle = GFX::WEAPONS_RUGER_BULLET;
    gun->firestyle = GFX::WEAPONS_RUGER_FIRE;
    gun->firemode = 2;

    // M79 grenade launcher
    gun = &guns[m79];
    gun->name = "M79";
    gun->ininame = gun->name;
    gun->num = m79_num;
    gun->texturenum = GFX::WEAPONS_M79;
    gun->cliptexturenum = GFX::WEAPONS_M79_CLIP;
    gun->clipreload = true;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_M79_FIRE;
    gun->firemode = 0;

    // Barrett M82A1
    gun = &guns[barrett];
    gun->name = "Barrett M82A1";
    gun->ininame = "Barret M82A1";
    gun->num = barrett_num;
    gun->texturenum = GFX::WEAPONS_BARRETT;
    gun->cliptexturenum = GFX::WEAPONS_BARRETT_CLIP;
    gun->clipreload = true;
    gun->bulletimagestyle = GFX::WEAPONS_BARRETT_BULLET;
    gun->firestyle = GFX::WEAPONS_BARRETT_FIRE;
    gun->firemode = 2;

    // M249
    gun = &guns[m249];
    gun->name = "FN Minimi";
    gun->ininame = gun->name;
    gun->num = m249_num;
    gun->texturenum = GFX::WEAPONS_MINIMI;
    gun->cliptexturenum = GFX::WEAPONS_MINIMI_CLIP;
    gun->clipreload = true;
    gun->bulletimagestyle = GFX::WEAPONS_MINIMI_BULLET;
    gun->firestyle = GFX::WEAPONS_MINIMI_FIRE;
    gun->firemode = 0;

    // Minigun
    gun = &guns[minigun];
    gun->name = "XM214 Minigun";
    gun->ininame = gun->name;
    gun->num = minigun_num;
    gun->texturenum = GFX::WEAPONS_MINIGUN;
    gun->cliptexturenum = 0;
    gun->clipreload = false;
    gun->bulletimagestyle = GFX::WEAPONS_MINIGUN_BULLET;
    gun->firestyle = GFX::WEAPONS_MINIGUN_FIRE;
    gun->firemode = 0;

    // Colt 1911
    gun = &guns[colt];
    gun->name = "USSOCOM";
    gun->ininame = gun->name;
    gun->num = colt_num;
    gun->texturenum = GFX::WEAPONS_SOCOM;
    gun->cliptexturenum = GFX::WEAPONS_SOCOM_CLIP;
    gun->clipreload = true;
    gun->bulletimagestyle = GFX::WEAPONS_COLT_BULLET;
    gun->firestyle = GFX::WEAPONS_SOCOM_FIRE;
    gun->firemode = 2;

    // Knife
    gun = &guns[knife];
    gun->name = "Combat Knife";
    gun->ininame = gun->name;
    gun->num = knife_num;
    gun->texturenum = GFX::WEAPONS_KNIFE;
    gun->cliptexturenum = 0;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = 0;
    gun->firemode = 0;

    // Chainsaw
    gun = &guns[chainsaw];
    gun->name = "Chainsaw";
    gun->ininame = gun->name;
    gun->num = chainsaw_num;
    gun->texturenum = GFX::WEAPONS_CHAINSAW;
    gun->cliptexturenum = 0;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_CHAINSAW_FIRE;
    gun->firemode = 0;

    // M72 LAW
    gun = &guns[law];
    gun->name = "LAW";
    gun->ininame = "M72 LAW";
    gun->num = law_num;
    gun->texturenum = GFX::WEAPONS_LAW;
    gun->cliptexturenum = 0;
    gun->clipreload = true;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_LAW_FIRE;
    gun->firemode = 0;

    // Rambo Bow with flame
    gun = &guns[bow2];
    gun->name = "Flame Bow";
    gun->ininame = "Flamed Arrows";
    gun->num = bow2_num;
    gun->texturenum = GFX::WEAPONS_BOW;
    gun->cliptexturenum = GFX::WEAPONS_BOW_S;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_BOW_FIRE;
    gun->firemode = 0;

    // Rambo Bow
    gun = &guns[bow];
    gun->name = "Bow";
    gun->ininame = "Rambo Bow";
    gun->num = bow_num;
    gun->texturenum = GFX::WEAPONS_BOW;
    gun->cliptexturenum = GFX::WEAPONS_BOW_S;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_BOW_FIRE;
    gun->firemode = 0;

    // Flamethrower
    gun = &guns[flamer];
    gun->name = "Flamer";
    gun->ininame = gun->name;
    gun->num = flamer_num;
    gun->texturenum = GFX::WEAPONS_FLAMER;
    gun->cliptexturenum = GFX::WEAPONS_FLAMER;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_FLAMER_FIRE;
    gun->firemode = 0;

    // M2
    gun = &guns[m2];
    gun->name = "M2 MG";
    gun->ininame = "Stationary Gun";
    gun->num = m2_num;
    gun->texturenum = GFX::WEAPONS_MINIGUN;
    gun->cliptexturenum = 0;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = 0;
    gun->firemode = 0;

    // No weapon
    gun = &guns[noweapon];
    gun->name = "Hands";
    gun->ininame = "Punch";
    gun->num = noweapon_num;
    gun->texturenum = 0;
    gun->cliptexturenum = 0;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = 0;
    gun->firemode = 0;

    // Frag grenade
    gun = &guns[fraggrenade];
    gun->name = "Frag Grenade";
    gun->ininame = "Grenade";
    gun->num = fraggrenade_num;
    gun->texturenum = GFX::WEAPONS_FRAG_GRENADE;
    gun->cliptexturenum = GFX::WEAPONS_FRAG_GRENADE;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_AK74_FIRE;
    gun->firemode = 0;

    // TODO(skoskav): Add a proper entry for cluster nade and thrown knife
    // Cluster grenade
    gun = &guns[clustergrenade];
    gun->name = "Frag Grenade";
    gun->ininame = "";
    gun->num = clustergrenade_num;
    gun->texturenum = GFX::WEAPONS_FRAG_GRENADE;
    gun->cliptexturenum = GFX::WEAPONS_FRAG_GRENADE;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_AK74_FIRE;
    gun->firemode = 0;

    // Cluster
    gun = &guns[cluster];
    gun->name = "Frag Grenade";
    gun->ininame = "";
    gun->num = cluster_num;
    gun->texturenum = GFX::WEAPONS_FRAG_GRENADE;
    gun->cliptexturenum = GFX::WEAPONS_FRAG_GRENADE;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = GFX::WEAPONS_AK74_FIRE;
    gun->firemode = 0;

    // Thrown knife
    gun = &guns[thrownknife];
    gun->name = "Combat Knife";
    gun->ininame = "";
    gun->num = thrownknife_num;
    gun->texturenum = GFX::WEAPONS_KNIFE;
    gun->cliptexturenum = 0;
    gun->clipreload = false;
    gun->bulletimagestyle = 0;
    gun->firestyle = 0;
    gun->firemode = 0;
}

void createnormalweapons()
{
    tgun *gun;

    // Desert Eagle
    gun = &guns[eagle];
    gun->hitmultiply = 1.81;
    gun->fireinterval = 24;
    gun->ammo = 7;
    gun->reloadtime = 87;
    gun->speed = 19;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.009;
    gun->bulletspread = 0.15;
    gun->recoil = 0;
    gun->push = 0.0176;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 0.95;
    gun->modifierlegs = 0.85;

    // MP5
    gun = &guns[mp5];
    gun->hitmultiply = 1.01;
    gun->fireinterval = 6;
    gun->ammo = 30;
    gun->reloadtime = 105;
    gun->speed = 18.9;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0.14;
    gun->recoil = 0;
    gun->push = 0.0112;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 0.95;
    gun->modifierlegs = 0.85;

    // AK-74
    gun = &guns[ak74];
    gun->hitmultiply = 1.004;
    gun->fireinterval = 10;
    gun->ammo = 35;
    gun->reloadtime = 165;
    gun->speed = 24.6;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = -12;
    gun->movementacc = 0.011;
    gun->bulletspread = 0.025;
    gun->recoil = 0;
    gun->push = 0.01376;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 0.95;
    gun->modifierlegs = 0.85;

    // Steyr AUG
    gun = &guns[steyraug];
    gun->hitmultiply = 0.71;
    gun->fireinterval = 7;
    gun->ammo = 25;
    gun->reloadtime = 125;
    gun->speed = 26;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0.075;
    gun->recoil = 0;
    gun->push = 0.0084;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 0.95;
    gun->modifierlegs = 0.85;

    // SPAS-12
    gun = &guns[spas12];
    gun->hitmultiply = 1.22;
    gun->fireinterval = 32;
    gun->ammo = 7;
    gun->reloadtime = 175;
    gun->speed = 14;
    gun->bulletstyle = bullet_style_shotgun;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0.8;
    gun->recoil = 0;
    gun->push = 0.0188;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 0.95;
    gun->modifierlegs = 0.85;

    // Ruger 77
    gun = &guns[ruger77];
    gun->hitmultiply = 2.49;
    gun->fireinterval = 45;
    gun->ammo = 4;
    gun->reloadtime = 78;
    gun->speed = 33;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.03;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.012;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.2;
    gun->modifierchest = 1.05;
    gun->modifierlegs = 1;

    // M79 grenade launcher
    gun = &guns[m79];
    gun->hitmultiply = 1550;
    gun->fireinterval = 6;
    gun->ammo = 1;
    gun->reloadtime = 178;
    gun->speed = 10.7;
    gun->bulletstyle = bullet_style_m79;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.036;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.15;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.9;

    // Barrett M82A1
    gun = &guns[barrett];
    gun->hitmultiply = 4.45;
    gun->fireinterval = 225;
    gun->ammo = 10;
    gun->reloadtime = 70;
    gun->speed = 55;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 19;
    gun->bink = 65;
    gun->movementacc = 0.05;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.018;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1;
    gun->modifierchest = 1;
    gun->modifierlegs = 1;

    // M249
    gun = &guns[m249];
    gun->hitmultiply = 0.85;
    gun->fireinterval = 9;
    gun->ammo = 50;
    gun->reloadtime = 250;
    gun->speed = 27;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.013;
    gun->bulletspread = 0.064;
    gun->recoil = 0;
    gun->push = 0.0128;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 0.95;
    gun->modifierlegs = 0.85;

    // Minigun
    gun = &guns[minigun];
    gun->hitmultiply = 0.468;
    gun->fireinterval = 3;
    gun->ammo = 100;
    gun->reloadtime = 480;
    gun->speed = 29;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 25;
    gun->bink = 0;
    gun->movementacc = 0.0625;
    gun->bulletspread = 0.3;
    gun->recoil = 0;
    gun->push = 0.0104;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 0.95;
    gun->modifierlegs = 0.85;

    // Colt 1911
    gun = &guns[colt];
    gun->hitmultiply = 1.49;
    gun->fireinterval = 10;
    gun->ammo = 14;
    gun->reloadtime = 60;
    gun->speed = 18;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.02;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 0.95;
    gun->modifierlegs = 0.85;

    // Knife
    gun = &guns[knife];
    gun->hitmultiply = 2150;
    gun->fireinterval = 6;
    gun->ammo = 1;
    gun->reloadtime = 3;
    gun->speed = 6;
    gun->bulletstyle = bullet_style_knife;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.12;
    gun->inheritedvelocity = 0;
    gun->modifierhead = 1.15;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.9;

    // Chainsaw
    gun = &guns[chainsaw];
    gun->hitmultiply = 50;
    gun->fireinterval = 2;
    gun->ammo = 200;
    gun->reloadtime = 110;
    gun->speed = 8;
    gun->bulletstyle = bullet_style_knife;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.0028;
    gun->inheritedvelocity = 0;
    gun->modifierhead = 1.15;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.9;

    // M72 LAW
    gun = &guns[law];
    gun->hitmultiply = 1550;
    gun->fireinterval = 6;
    gun->ammo = 1;
    gun->reloadtime = 300;
    gun->speed = 23;
    gun->bulletstyle = bullet_style_law;
    gun->startuptime = 13;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.028;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.15;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.9;

    // Rambo Bow with flame
    gun = &guns[bow2];
    gun->hitmultiply = 8;
    gun->fireinterval = 10;
    gun->ammo = 1;
    gun->reloadtime = 39;
    gun->speed = 18;
    gun->bulletstyle = bullet_style_flamearrow;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.15;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.9;

    // Rambo Bow
    gun = &guns[bow];
    gun->hitmultiply = 12;
    gun->fireinterval = 10;
    gun->ammo = 1;
    gun->reloadtime = 25;
    gun->speed = 21;
    gun->bulletstyle = bullet_style_arrow;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.0148;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.15;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.9;

    // Flamethrower
    gun = &guns[flamer];
    gun->hitmultiply = 19;
    gun->fireinterval = 6;
    gun->ammo = 200;
    gun->reloadtime = 5;
    gun->speed = 10.5;
    gun->bulletstyle = bullet_style_flame;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.016;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.15;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.9;

    // M2
    gun = &guns[m2];
    gun->hitmultiply = 1.8;
    gun->fireinterval = 10;
    gun->ammo = 100;
    gun->reloadtime = 366;
    gun->speed = 36;
    gun->bulletstyle = bullet_style_m2;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.0088;
    gun->inheritedvelocity = 0;
    gun->modifierhead = 1.1;
    gun->modifierchest = 0.95;
    gun->modifierlegs = 0.85;

    // No weapon
    gun = &guns[noweapon];
    gun->hitmultiply = 330;
    gun->fireinterval = 6;
    gun->ammo = 1;
    gun->reloadtime = 3;
    gun->speed = 5;
    gun->bulletstyle = bullet_style_punch;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0;
    gun->inheritedvelocity = 0;
    gun->modifierhead = 1.15;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.9;

    // Frag grenade
    gun = &guns[fraggrenade];
    gun->hitmultiply = 1500;
    gun->fireinterval = 80;
    gun->ammo = 1;
    gun->reloadtime = 20;
    gun->speed = 5;
    gun->bulletstyle = bullet_style_fragnade;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0;
    gun->inheritedvelocity = 1;
    gun->modifierhead = 1;
    gun->modifierchest = 1;
    gun->modifierlegs = 1;
}

void createfloatisticweapons()
{
    tgun *gun;

    // Desert Eagle
    gun = &guns[eagle];
    gun->hitmultiply = 1.66;
    gun->fireinterval = 27;
    gun->ammo = 7;
    gun->reloadtime = 106;
    gun->speed = 19;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.02;
    gun->bulletspread = 0.1;
    gun->recoil = 55;
    gun->push = 0.0164;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // MP5
    gun = &guns[mp5];
    gun->hitmultiply = 0.94;
    gun->fireinterval = 6;
    gun->ammo = 30;
    gun->reloadtime = 110;
    gun->speed = 18.9;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = -10;
    gun->movementacc = 0.01;
    gun->bulletspread = 0.03;
    gun->recoil = 9;
    gun->push = 0.0164;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // AK-74
    gun = &guns[ak74];
    gun->hitmultiply = 1.08;
    gun->fireinterval = 11;
    gun->ammo = 35;
    gun->reloadtime = 158;
    gun->speed = 24;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = -10;
    gun->movementacc = 0.02;
    gun->bulletspread = 0;
    gun->recoil = 13;
    gun->push = 0.0132;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Steyr AUG
    gun = &guns[steyraug];
    gun->hitmultiply = 0.68;
    gun->fireinterval = 7;
    gun->ammo = 30;
    gun->reloadtime = 126;
    gun->speed = 26;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = -9;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 11;
    gun->push = 0.012;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // SPAS-12
    gun = &guns[spas12];
    gun->hitmultiply = 1.2;
    gun->fireinterval = 35;
    gun->ammo = 7;
    gun->reloadtime = 175;
    gun->speed = 13.2;
    gun->bulletstyle = bullet_style_shotgun;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0.8;
    gun->recoil = 65;
    gun->push = 0.0224;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Ruger 77
    gun = &guns[ruger77];
    gun->hitmultiply = 2.22;
    gun->fireinterval = 52;
    gun->ammo = 4;
    gun->reloadtime = 104;
    gun->speed = 33;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = 14;
    gun->movementacc = 0.03;
    gun->bulletspread = 0;
    gun->recoil = 54;
    gun->push = 0.0096;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // M79 grenade launcher
    gun = &guns[m79];
    gun->hitmultiply = 1600;
    gun->fireinterval = 6;
    gun->ammo = 1;
    gun->reloadtime = 173;
    gun->speed = 11.4;
    gun->bulletstyle = bullet_style_m79;
    gun->startuptime = 0;
    gun->bink = 45;
    gun->movementacc = 0.03;
    gun->bulletspread = 0;
    gun->recoil = 420;
    gun->push = 0.024;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Barrett M82A1
    gun = &guns[barrett];
    gun->hitmultiply = 4.95;
    gun->fireinterval = 200;
    gun->ammo = 10;
    gun->reloadtime = 170;
    gun->speed = 55;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 16;
    gun->bink = 80;
    gun->movementacc = 0.07;
    gun->bulletspread = 0;
    gun->recoil = 0;
    gun->push = 0.0056;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // M249
    gun = &guns[m249];
    gun->hitmultiply = 0.81;
    gun->fireinterval = 10;
    gun->ammo = 50;
    gun->reloadtime = 261;
    gun->speed = 27;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = -8;
    gun->movementacc = 0.02;
    gun->bulletspread = 0;
    gun->recoil = 8;
    gun->push = 0.0116;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Minigun
    gun = &guns[minigun];
    gun->hitmultiply = 0.43;
    gun->fireinterval = 4;
    gun->ammo = 100;
    gun->reloadtime = 320;
    gun->speed = 29;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 33;
    gun->bink = -2;
    gun->movementacc = 0.01;
    gun->bulletspread = 0.1;
    gun->recoil = 4;
    gun->push = 0.0108;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Colt 1911
    gun = &guns[colt];
    gun->hitmultiply = 1.30;
    gun->fireinterval = 12;
    gun->ammo = 12;
    gun->reloadtime = 72;
    gun->speed = 18;
    gun->bulletstyle = bullet_style_plain;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.02;
    gun->bulletspread = 0;
    gun->recoil = 28;
    gun->push = 0.0172;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Knife
    gun = &guns[knife];
    gun->hitmultiply = 2250;
    gun->fireinterval = 6;
    gun->ammo = 1;
    gun->reloadtime = 3;
    gun->speed = 6;
    gun->bulletstyle = bullet_style_knife;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 10;
    gun->push = 0.028;
    gun->inheritedvelocity = 0;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Chainsaw
    gun = &guns[chainsaw];
    gun->hitmultiply = 21;
    gun->fireinterval = 2;
    gun->ammo = 200;
    gun->reloadtime = 110;
    gun->speed = 7.6;
    gun->bulletstyle = bullet_style_knife;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 1;
    gun->push = 0.0028;
    gun->inheritedvelocity = 0;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // M72 LAW
    gun = &guns[law];
    gun->hitmultiply = 1500;
    gun->fireinterval = 30;
    gun->ammo = 1;
    gun->reloadtime = 495;
    gun->speed = 23;
    gun->bulletstyle = bullet_style_law;
    gun->startuptime = 12;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 9;
    gun->push = 0.012;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Rambo Bow with flame
    gun = &guns[bow2];
    gun->hitmultiply = 8;
    gun->fireinterval = 10;
    gun->ammo = 1;
    gun->reloadtime = 39;
    gun->speed = 18;
    gun->bulletstyle = bullet_style_flamearrow;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 10;
    gun->push = 0;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Rambo Bow
    gun = &guns[bow];
    gun->hitmultiply = 12;
    gun->fireinterval = 10;
    gun->ammo = 1;
    gun->reloadtime = 25;
    gun->speed = 21;
    gun->bulletstyle = bullet_style_arrow;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 10;
    gun->push = 0.0148;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Flamethrower
    gun = &guns[flamer];
    gun->hitmultiply = 12;
    gun->fireinterval = 6;
    gun->ammo = 200;
    gun->reloadtime = 5;
    gun->speed = 12.5;
    gun->bulletstyle = bullet_style_flame;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 10;
    gun->push = 0.016;
    gun->inheritedvelocity = 0.5;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // M2
    gun = &guns[m2];
    gun->hitmultiply = 1.55;
    gun->fireinterval = 14;
    gun->ammo = 100;
    gun->reloadtime = 366;
    gun->speed = 36;
    gun->bulletstyle = bullet_style_m2;
    gun->startuptime = 21;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 10;
    gun->push = 0.0088;
    gun->inheritedvelocity = 0;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // No weapon
    gun = &guns[noweapon];
    gun->hitmultiply = 330;
    gun->fireinterval = 6;
    gun->ammo = 1;
    gun->reloadtime = 3;
    gun->speed = 5;
    gun->bulletstyle = bullet_style_punch;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 10;
    gun->push = 0;
    gun->inheritedvelocity = 0;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;

    // Frag grenade
    gun = &guns[fraggrenade];
    gun->hitmultiply = 1500;
    gun->fireinterval = 80;
    gun->ammo = 1;
    gun->reloadtime = 20;
    gun->speed = 5;
    gun->bulletstyle = bullet_style_fragnade;
    gun->startuptime = 0;
    gun->bink = 0;
    gun->movementacc = 0.01;
    gun->bulletspread = 0;
    gun->recoil = 10;
    gun->push = 0;
    gun->inheritedvelocity = 1;
    gun->modifierhead = 1.1;
    gun->modifierchest = 1;
    gun->modifierlegs = 0.6;
}

void buildweapons()
{
    tgun *gun;
    std::int32_t weaponindex;

    // TODO(skoskav): Add a proper entry for cluster nade and thrown knife
    // Cluster grenade
    gun = &guns[clustergrenade];
    gun->hitmultiply = guns[fraggrenade].hitmultiply;
    gun->fireinterval = guns[fraggrenade].fireinterval;
    gun->ammo = guns[fraggrenade].ammo;
    gun->reloadtime = guns[fraggrenade].reloadtime;
    gun->speed = guns[fraggrenade].speed;
    gun->bulletstyle = bullet_style_clusternade;
    gun->startuptime = guns[fraggrenade].startuptime;
    gun->bink = guns[fraggrenade].bink;
    gun->movementacc = guns[fraggrenade].movementacc;
    gun->bulletspread = guns[fraggrenade].bulletspread;
    gun->recoil = guns[fraggrenade].recoil;
    gun->push = guns[fraggrenade].push;
    gun->inheritedvelocity = guns[fraggrenade].inheritedvelocity;

    // Cluster
    gun = &guns[cluster];
    gun->hitmultiply = guns[clustergrenade].hitmultiply;
    gun->fireinterval = guns[clustergrenade].fireinterval;
    gun->ammo = guns[clustergrenade].ammo;
    gun->reloadtime = guns[clustergrenade].reloadtime;
    gun->speed = guns[clustergrenade].speed;
    gun->bulletstyle = bullet_style_cluster;
    gun->startuptime = guns[clustergrenade].startuptime;
    gun->bink = guns[clustergrenade].bink;
    gun->movementacc = guns[clustergrenade].movementacc;
    gun->bulletspread = guns[clustergrenade].bulletspread;
    gun->recoil = guns[clustergrenade].recoil;
    gun->push = guns[clustergrenade].push;
    gun->inheritedvelocity = guns[clustergrenade].inheritedvelocity;

    // Thrown knife
    gun = &guns[thrownknife];
    gun->hitmultiply = guns[knife].hitmultiply;
    gun->fireinterval = guns[knife].fireinterval;
    gun->ammo = guns[knife].ammo;
    gun->reloadtime = guns[knife].reloadtime;
    gun->speed = guns[knife].speed;
    gun->bulletstyle = bullet_style_thrownknife;
    gun->startuptime = guns[knife].startuptime;
    gun->bink = guns[knife].bink;
    gun->movementacc = guns[knife].movementacc;
    gun->bulletspread = guns[knife].bulletspread;
    gun->recoil = guns[knife].recoil;
    gun->push = guns[knife].push;
    gun->inheritedvelocity = guns[knife].inheritedvelocity;

    for (weaponindex = guns.StartIdx(); weaponindex <= guns.EndIdx(); weaponindex++)
    {
        gun = &guns[weaponindex];

        gun->fireintervalprev = gun->fireinterval;
        gun->fireintervalcount = gun->fireinterval;
        gun->ammocount = gun->ammo;
        gun->reloadtimeprev = gun->reloadtime;
        gun->reloadtimecount = gun->reloadtime;
        gun->startuptimecount = gun->startuptime;

        // Set timings for when to let out and in a magazine, if at all
        if (gun->clipreload)
        {
            gun->clipouttime = gun->reloadtime * 0.8;
            gun->clipintime = gun->reloadtime * 0.3;
        }
        else
        {
            gun->clipouttime = 0;
            gun->clipintime = 0;
        }

        // Set bullet lifetime
        switch (gun->bulletstyle)
        {
        case bullet_style_fragnade:
        case bullet_style_clusternade:
            gun->timeout = Constants::GRENADE_TIMEOUT;
            break;
        case bullet_style_flame:
            gun->timeout = Constants::FLAMER_TIMEOUT;
            break;
        case bullet_style_punch:
        case bullet_style_knife:
            gun->timeout = Constants::MELEE_TIMEOUT;
            break;
        case bullet_style_m2:
            gun->timeout = Constants::M2BULLET_TIMEOUT;
            break;
        default:
            gun->timeout = Constants::BULLET_TIMEOUT;
        }
    }

    // Force M79 reload on spawn
    guns[m79].ammocount = 0;
}
// TODO weapons off for now
#if OPTION_Q == 1 /*$Q-*/
#define NoOverflowCheck
#endif
#if OPTION_R == 1 /*$R-*/
#define NoRangeCheck
#endif
std::uint32_t createwmchecksum()
{
    std::uint32_t hash;
    std::int32_t weaponindex;
    tgun *gun;

    std::uint32_t result;
    LogTraceG("CreateWMChecksum");

    // djb2 hashing algorithm
    hash = 5381;

    for (weaponindex = 1; weaponindex <= original_weapons; weaponindex++)
    {
        gun = &guns[weaponindex];

        hash += (hash << 5) + round(1000.0 * gun->hitmultiply);
        hash += (hash << 5) + round(1000.0 * gun->fireinterval);
        hash += (hash << 5) + round(1000.0 * gun->ammo);
        hash += (hash << 5) + round(1000.0 * gun->reloadtime);
        hash += (hash << 5) + round(1000.0 * gun->speed);
        hash += (hash << 5) + round(1000.0 * gun->bulletstyle);
        hash += (hash << 5) + round(1000.0 * gun->startuptime);
        hash += (hash << 5) + round(1000.0 * gun->bink);
        hash += (hash << 5) + round(1000.0 * gun->movementacc);
        hash += (hash << 5) + round(1000.0 * gun->bulletspread);
        hash += (hash << 5) + round(1000.0 * gun->recoil);
        hash += (hash << 5) + round(1000.0 * gun->push);
        hash += (hash << 5) + round(1000.0 * gun->inheritedvelocity);
    }

    result = hash;
    return result;
}

#ifdef NoRangeCheck /*$R+*/ /*$UNDEF NoRangeCheck*/
#endif
#ifdef NoOverflowCheck /*$Q+*/ /*$UNDEF NoOverflowCheck*/
#endif

std::int32_t weaponnumtoindex(std::uint8_t num)
{
    std::uint8_t weaponindex;

    std::uint32_t result;
    for (weaponindex = guns.StartIdx(); weaponindex <= guns.EndIdx(); weaponindex++)
    {
        if (num == guns[weaponindex].num)
        {
            result = weaponindex;
            return result;
        }
    }
    result = -1;
    return result;
}

std::int32_t weaponnametonum(const std::string &name)
{
    std::uint8_t i;

    std::int32_t result;
    for (i = 1; i <= high(guns); i++)
    {
        if (name == guns[i].name)
        {
            result = guns[i].num;
            return result;
        }
    }
    result = -1;
    return result;
}

std::string weaponnumtoname(std::int32_t num)
{
    std::string result;
    switch (num)
    {
    case eagle_num:
        result = guns[eagle].name;
        break;
    case mp5_num:
        result = guns[mp5].name;
        break;
    case ak74_num:
        result = guns[ak74].name;
        break;
    case steyraug_num:
        result = guns[steyraug].name;
        break;
    case spas12_num:
        result = guns[spas12].name;
        break;
    case ruger77_num:
        result = guns[ruger77].name;
        break;
    case m79_num:
        result = guns[m79].name;
        break;
    case barrett_num:
        result = guns[barrett].name;
        break;
    case m249_num:
        result = guns[m249].name;
        break;
    case minigun_num:
        result = guns[minigun].name;
        break;
    case colt_num:
        result = guns[colt].name;
        break;
    case knife_num:
        result = guns[knife].name;
        break;
    case chainsaw_num:
        result = guns[chainsaw].name;
        break;
    case law_num:
        result = guns[law].name;
        break;
    case bow2_num:
        result = guns[bow2].name;
        break;
    case bow_num:
        result = guns[bow].name;
        break;
    case flamer_num:
        result = guns[flamer].name;
        break;
    case m2_num:
        result = guns[m2].name;
        break;
    case noweapon_num:
        result = guns[noweapon].name;
        break;
    case fraggrenade_num:
        result = guns[fraggrenade].name;
        break;
    case clustergrenade_num:
        result = guns[clustergrenade].name;
        break;
    case cluster_num:
        result = guns[cluster].name;
        break;
    case thrownknife_num:
        result = guns[thrownknife].name;
        break;
    default:
        result = "";
    }
    return result;
}

std::uint8_t weaponnuminternaltoexternal(std::uint8_t num)
{
    std::uint8_t result;
    switch (num)
    {
    case knife_num:
        result = 14;
        break;
    case chainsaw_num:
        result = 15;
        break;
    case law_num:
        result = 16;
        break;
    case flamer_num:
        result = 11;
        break;
    case bow_num:
        result = 12;
        break;
    case bow2_num:
        result = 13;
        break;
    default:
        result = num;
    }
    return result;
}

std::uint8_t weaponnumexternaltointernal(std::uint8_t num)
{
    std::uint8_t result;
    switch (num)
    {
    case 11:
        result = flamer_num;
        break;
    case 12:
        result = bow_num;
        break;
    case 13:
        result = bow2_num;
        break;
    case 14:
        result = knife_num;
        break;
    case 15:
        result = chainsaw_num;
        break;
    case 16:
        result = law_num;
        break;
    default:
        result = num;
    }
    return result;
}

std::string weaponnamebynum(std::int32_t num)
{
    std::int32_t weaponindex;

    std::string result;
    result = "";

    for (weaponindex = guns.StartIdx(); weaponindex <= guns.EndIdx(); weaponindex++)
    {
        if (num == guns[weaponindex].num)
        {
            result = guns[weaponindex].name;
            break;
        }
    }
    return result;
}

bool ismainweaponindex(std::int16_t weaponindex)
{
    bool result;
    result = (weaponindex >= 1) && (weaponindex <= main_weapons);
    return result;
}

bool issecondaryweaponindex(std::int16_t weaponindex)
{
    bool result;
    result = (weaponindex >= primary_weapons + 1) && (weaponindex <= main_weapons);
    return result;
}

bool isdoubleweaponindex(std::int16_t weaponindex)
{
    bool result;
    result = (weaponindex >= 1) && (weaponindex <= double_weapons);
    return result;
}

std::uint32_t calculatebink(std::uint32_t accumulated, std::uint32_t bink)
{
    // Adding bink has diminishing returns as more gets accumulated
    std::uint64_t result;
    result = accumulated + bink -
             round(accumulated * ((float)(accumulated) / ((10 * bink) + accumulated)));
    return result;
}
