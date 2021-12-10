// automatically converted
#include "NetworkClientThing.hpp"

#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/Sound.hpp"
#include "../Cvar.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../GameStrings.hpp"
#include "../mechanics/Things.hpp"
#include "NetworkClientSprite.hpp"
#include "NetworkUtils.hpp"
#include "common/Calc.hpp"

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

void clienthandleserverthingsnapshot(SteamNetworkingMessage_t *netmessage)
{
    tmsg_serverthingsnapshot *thingsnap;
    std::int32_t i, d;
    tvector2 a;

    if (!verifypacket(sizeof(tmsg_serverthingsnapshot), netmessage->m_cbSize,
                      msgid_serverthingsnapshot))
        return;

    thingsnap = pmsg_serverthingsnapshot(netmessage->m_pData);

    // assign received Thing info to thing
    i = thingsnap->num;

    if ((i < 1) || (i > max_things))
        return;

    a.x = thingsnap->pos[1].x;
    a.y = thingsnap->pos[1].y;

    if ((!thing[i].active) || (thing[i].style != thingsnap->style))
    {
        creatething(a, thingsnap->owner, thingsnap->style, i);

        for (d = 1; d <= 4; d++)
        {
            [[deprecated("minus 1")]] auto dminus1 = d - 1;
            thing[i].skeleton.pos[d].x = thingsnap->pos[dminus1].x;
            thing[i].skeleton.pos[d].y = thingsnap->pos[dminus1].y;
            thing[i].skeleton.oldpos[d].x = thingsnap->oldpos[dminus1].x;
            thing[i].skeleton.oldpos[d].y = thingsnap->oldpos[dminus1].y;
        }
    }

    thing[i].holdingsprite = thingsnap->holdingsprite;

    // is not holded anymore
    if (thing[i].holdingsprite == 0)
    {
        for (d = 1; d <= max_sprites; d++)
            if (sprite[d].active)
            {
                if (sprite[d].holdedthing == i)
                    sprite[d].holdedthing = 0;
            }
    }

    if ((thingsnap->owner > 0) && (thingsnap->owner < max_sprites + 1))
    {
        if (thing[i].holdingsprite > 0)
        {
            sprite[thingsnap->owner].holdedthing = i;
            sprite[thingsnap->owner].onground = false;
        }

        thing[i].color = sprite[thingsnap->owner].player->shirtcolor;
    }

    if ((thing[i].holdingsprite == 0) && (thing[i].style != object_stationary_gun))
        if ((distance(thing[i].skeleton.pos[1], thingsnap->pos[1]) > 10) &&
            (distance(thing[i].skeleton.pos[2], thingsnap->pos[2]) > 10))
            for (d = 1; d <= 4; d++)
            {
                [[deprecated("maybe change d scope from 1 to 0?")]] auto dminus1 = d - 1;
                thing[i].skeleton.pos[d].x = thingsnap->pos[dminus1].x;
                thing[i].skeleton.pos[d].y = thingsnap->pos[dminus1].y;
                thing[i].skeleton.oldpos[d].x = thingsnap->oldpos[dminus1].x;
                thing[i].skeleton.oldpos[d].y = thingsnap->oldpos[dminus1].y;
            }

    if ((thing[i].holdingsprite > 0) && (thing[i].style != object_parachute))
        if (distance(thing[i].skeleton.pos[1], spriteparts.pos[thing[i].holdingsprite]) > 330)
            for (d = 1; d <= 4; d++)
            {
                [[deprecated("maybe change d scope from 1 to 0?")]] auto dminus1 = d - 1;
                thing[i].skeleton.pos[d].x = thingsnap->pos[dminus1].x;
                thing[i].skeleton.pos[d].y = thingsnap->pos[dminus1].y;
                thing[i].skeleton.oldpos[d].x = thingsnap->oldpos[dminus1].x;
                thing[i].skeleton.oldpos[d].y = thingsnap->oldpos[dminus1].y;
            }

    thing[i].statictype = false;

    if (thing[i].style == object_rambo_bow)
        gamethingtarget = i;
}

void clienthandleserverthingmustsnapshot(SteamNetworkingMessage_t *netmessage)
{
    tmsg_serverthingmustsnapshot *thingmustsnap;
    std::int32_t i, d;
    tvector2 a;
    tsprite *spritethingowner;
    std::int32_t weaponthing;

    if (!verifypacket(sizeof(tmsg_serverthingmustsnapshot), netmessage->m_cbSize,
                      msgid_serverthingmustsnapshot))
        return;

    thingmustsnap = pmsg_serverthingmustsnapshot(netmessage->m_pData);

    // assign received Thing info to thing
    i = thingmustsnap->num;

    if ((i < 1) || (i > max_things))
        return;

    if ((thingmustsnap->owner > 0) && (thingmustsnap->owner < max_sprites + 1))
        spritethingowner = &sprite[thingmustsnap->owner];
    else
        spritethingowner = nullptr;

    if ((!thing[i].active) or (thing[i].style != thingmustsnap->style))
    {
        a.x = thingmustsnap->pos[1].x;
        a.y = thingmustsnap->pos[1].y;
        creatething(a, thingmustsnap->owner, thingmustsnap->style, i);

        [[deprecated("it was d = 1, d <= 4")]] auto d = 0;
        for (d = 0; d < 4; d++)
        {
            thing[i].skeleton.pos[d + 1].x = thingmustsnap->pos[d].x;
            thing[i].skeleton.pos[d + 1].y = thingmustsnap->pos[d].y;
            thing[i].skeleton.oldpos[d + 1].x = thingmustsnap->oldpos[d].x;
            thing[i].skeleton.oldpos[d + 1].y = thingmustsnap->oldpos[d].y;
        }

        if (spritethingowner != nullptr)
        {
            switch (thingmustsnap->style)
            {
            case object_ussocom:
                weaponthing = colt_num;
                break;
            case object_desert_eagle:
                weaponthing = eagle_num;
                break;
            case object_hk_mp5:
                weaponthing = mp5_num;
                break;
            case object_ak74:
                weaponthing = ak74_num;
                break;
            case object_steyr_aug:
                weaponthing = steyraug_num;
                break;
            case object_spas12:
                weaponthing = spas12_num;
                break;
            case object_ruger77:
                weaponthing = ruger77_num;
                break;
            case object_m79:
                weaponthing = m79_num;
                break;
            case object_barret_m82a1:
                weaponthing = barrett_num;
                break;
            case object_minimi:
                weaponthing = m249_num;
                break;
            case object_minigun:
                weaponthing = minigun_num;
                break;
                // OBJECT_COMBAT_KNIFE: WeaponThing := KNIFE_NUM;
            case object_chainsaw:
                weaponthing = chainsaw_num;
                break;
            case object_law:
                weaponthing = law_num;
                break;
            default:
                weaponthing = -1;
            }

            if (weaponthing > -1)
            {
                if (spritethingowner->weapon.num == weaponthing)
                    spritethingowner->applyweaponbynum(noweapon_num, 1);
                else if (spritethingowner->secondaryweapon.num == weaponthing)
                    spritethingowner->applyweaponbynum(noweapon_num, 2);
                if ((spritethingowner->num == mysprite) && !spritethingowner->deadmeat)
                {
                    clientspritesnapshot();
                }
            }
        }
    }

    thing[i].owner = thingmustsnap->owner;
    thing[i].holdingsprite = thingmustsnap->holdingsprite;

    // is not holded anymore
    if (thing[i].holdingsprite == 0)
    {
        for (d = 1; d <= max_sprites; d++)
            if (sprite[d].active)
            {
                if (sprite[d].holdedthing == i)
                    sprite[d].holdedthing = 0;
            }
    }

    if ((thingmustsnap->owner > 0) && (thingmustsnap->owner < max_sprites + 1))
    {
        if (thing[i].holdingsprite > 0)
        {
            spritethingowner->holdedthing = i;
            spritethingowner->onground = false;
        }

        thing[i].color = spritethingowner->player->shirtcolor;
    }

    if ((thing[i].holdingsprite == 0) && (!(thing[i].style == object_stationary_gun)))
        for (d = 1; d <= 4; d++)
        {
            [[deprecated("indexing")]] auto dminus1 = d - 1;
            thing[i].skeleton.pos[d].x = thingmustsnap->pos[dminus1].x;
            thing[i].skeleton.pos[d].y = thingmustsnap->pos[dminus1].y;
            thing[i].skeleton.oldpos[d].x = thingmustsnap->oldpos[dminus1].x;
            thing[i].skeleton.oldpos[d].y = thingmustsnap->oldpos[dminus1].y;
        }

    thing[i].timeout = thingmustsnap->timeout;
    thing[i].statictype = false;
    if (thing[i].style == object_rambo_bow)
        gamethingtarget = i;
}

void clienthandlethingtaken(SteamNetworkingMessage_t *netmessage)
{
    tmsg_serverthingtaken *thingtakensnap;
    std::int32_t i, j, n;
    tmsg_requestthing requestthingmsg;
    std::uint8_t weaponindex;
    std::uint32_t capcolor = capture_message_color;
    std::string bigcaptext;
    std::string smallcaptext;

    if (!verifypacket(sizeof(tmsg_serverthingtaken), netmessage->m_cbSize, msgid_thingtaken))
        return;

    thingtakensnap = pmsg_serverthingtaken(netmessage->m_pData);

    i = thingtakensnap->num;
    if ((i < 1) || (i > max_things))
        return;

    if (thingtakensnap->who == 255)
        thing[i].kill();

    if ((thingtakensnap->who < 1) || (thingtakensnap->who > max_sprites))
        return;

    if (!sprite[thingtakensnap->who].active)
        return;

    if ((!thing[i].active) && ((thingtakensnap->style == object_stationary_gun) ||
                               (thingtakensnap->style < object_ussocom)))
    {
        // request the new thing if not active
        requestthingmsg.header.id = msgid_requestthing;
        requestthingmsg.thingid = i;
        GetNetwork()->senddata(&requestthingmsg, sizeof(requestthingmsg),
                               k_nSteamNetworkingSend_Unreliable);
        return;
    }

    thing[i].style = thingtakensnap->style;

    j = thingtakensnap->who;
    if (sprite[j].weapon.num == guns[noweapon].num)
        n = 1;
    else
        n = 2;

    switch (thing[i].style)
    {
    case object_alpha_flag:
    case object_bravo_flag:
    case object_pointmatch_flag: {
        // capture sound
        playsound(sfx_capture, thing[i].skeleton.pos[1]);
        thing[i].holdingsprite = thingtakensnap->who;
        thing[i].statictype = false;

        j = thingtakensnap->who;

        switch (CVar::sv_gamemode)
        {
        case gamestyle_pointmatch:
        case gamestyle_inf:
            capcolor = capture_message_color;
            break;
        case gamestyle_htf:
        case gamestyle_ctf:
            switch (sprite[j].player->team)
            {
            case team_alpha:
                capcolor = alpha_message_color;
                break;
            case team_bravo:
                capcolor = bravo_message_color;
                break;
            }
            break;
        }

        smallcaptext = "";

        switch (CVar::sv_gamemode)
        {
        case gamestyle_pointmatch:
        case gamestyle_htf: {
            bigcaptext = iif(j == mysprite, _("You got the Flag!"), _("Yellow Flag captured!"));
            smallcaptext = _("{} got the Yellow Flag");
        }
        break;
        case gamestyle_ctf:
            if (sprite[j].player->team == thing[i].style)
            {
                switch (sprite[j].player->team)
                {
                case team_alpha: {
                    bigcaptext = _("Red Flag returned!");
                    smallcaptext = _("{} returned the Red Flag");
                }
                break;
                case team_bravo: {
                    bigcaptext = _("Blue Flag returned!");
                    smallcaptext = _("{} returned the Blue Flag");
                }
                break;
                }
                thing[i].respawn();
            }
            else
            {
                switch (sprite[j].player->team)
                {
                case team_alpha: {
                    bigcaptext =
                        iif(j == mysprite, _("You got the Blue Flag!"), _("Blue Flag captured!"));
                    smallcaptext = _("{} captured the Blue Flag");
                }
                break;
                case team_bravo: {
                    bigcaptext =
                        iif(j == mysprite, _("You got the Red Flag!"), _("Red Flag captured!"));
                    smallcaptext = _("{} captured the Red Flag");
                }
                break;
                }
            }
            break;
        case gamestyle_inf:
            if (sprite[j].player->team == thing[i].style)
            {
                if (sprite[j].player->team == team_bravo)
                {
                    bigcaptext = iif(j == mysprite, _("You returned the Objective!"),
                                     _("Objective returned!"));
                    smallcaptext = _("{} returned the Objective");
                }
                thing[i].respawn();
            }
            else
            {
                if (sprite[j].player->team == team_alpha)
                {
                    bigcaptext =
                        iif(j == mysprite, _("You got the Objective!"), _("Objective captured!"));
                    smallcaptext = _("{} captured the Objective");
                }
            }
            break;
        }

        if (smallcaptext != "")
        {
            bigmessage(bigcaptext, capturemessagewait, capcolor);
            NotImplemented(NITag::NETWORK);
#if 0
            GetMainConsole().console(smallcaptext, (sprite[j].player->name), capcolor);
#endif
        }
    }
    break;
    case object_ussocom:
    case object_desert_eagle:
    case object_hk_mp5:
    case object_ak74:
    case object_steyr_aug:
    case object_spas12:
    case object_ruger77:
    case object_m79:
    case object_barret_m82a1:
    case object_minimi:
    case object_minigun: {
        // Objects 1-3 are flags, so we need for WeaponIndex subtract by flags+1
        weaponindex = weaponnumtoindex(thing[i].style - (object_num_flags + 1), guns);
        sprite[thingtakensnap->who].applyweaponbynum(guns[weaponindex].num, n,
                                                     thingtakensnap->ammocount);
        if ((thingtakensnap->who == mysprite) && !sprite[mysprite].deadmeat)
            clientspritesnapshot();
    }
    break;
    case object_rambo_bow: {
        playsound(sfx_takebow, thing[i].skeleton.pos[1]);
        sprite[thingtakensnap->who].applyweaponbynum(guns[bow].num, 1, 1);
        sprite[thingtakensnap->who].applyweaponbynum(guns[bow2].num, 2, 1);
        sprite[thingtakensnap->who].weapon.ammocount = 1;
        sprite[thingtakensnap->who].weapon.fireinterval = 10;
        sprite[thingtakensnap->who].wearhelmet = 1;
        thing[i].kill();
        gamethingtarget = 0;

        if ((thingtakensnap->who == mysprite) && !sprite[mysprite].deadmeat)
            clientspritesnapshot();

        if (thingtakensnap->who == mysprite)
            bigmessage(_("You got the Bow!"), capturemessagewait, capture_message_color);
        else
            bigmessage(wideformat(_("{} got the Bow!"), sprite[thingtakensnap->who].player->name),
                       capturemessagewait, capture_message_color);
    }
    break;
    case object_medical_kit: {
        playsound(sfx_takemedikit, thing[i].skeleton.pos[1]);
        sprite[thingtakensnap->who].SetHealth(starthealth);
        thing[i].kill();
    }
    break;
    case object_grenade_kit: {
        playsound(sfx_pickupgun, thing[i].skeleton.pos[1]);
        sprite[thingtakensnap->who].tertiaryweapon = guns[fraggrenade];
        sprite[thingtakensnap->who].tertiaryweapon.ammocount = CVar::sv_maxgrenades;
        thing[i].kill();
    }
    break;
    case object_flamer_kit: {
        playsound(sfx_godflame, thing[i].skeleton.pos[1]);
        sprite[thingtakensnap->who].bonustime = flamerbonustime;
        sprite[thingtakensnap->who].bonusstyle = bonus_flamegod;
        sprite[thingtakensnap->who].applyweaponbynum(sprite[thingtakensnap->who].weapon.num, 2, -1,
                                                     true);
        sprite[thingtakensnap->who].applyweaponbynum(guns[flamer].num, 1);
        if (thingtakensnap->who == mysprite)
        {
            bigmessage(_("Flame God Mode!"), capturemessagewait, bonus_message_color);
            if (!sprite[mysprite].deadmeat)
                clientspritesnapshot();
        }
        thing[i].kill();
    }
    break;
    case object_predator_kit: {
        playsound(sfx_predator, thing[i].skeleton.pos[1]);
        sprite[thingtakensnap->who].alpha = predatoralpha;
        sprite[thingtakensnap->who].bonustime = predatorbonustime;
        sprite[thingtakensnap->who].bonusstyle = bonus_predator;
        if (thingtakensnap->who == mysprite)
            bigmessage(_("Predator Mode!"), capturemessagewait, bonus_message_color);
        thing[i].kill();
    }
    break;
    case object_vest_kit: {
        playsound(sfx_vesttake, thing[i].skeleton.pos[1]);
        sprite[thingtakensnap->who].vest = defaultvest;
        if (thingtakensnap->who == mysprite)
            bigmessage(_("Bulletproof Vest!"), capturemessagewait, capture_message_color);
        thing[i].kill();
    }
    break;
    case object_berserk_kit: {
        playsound(sfx_berserker, thing[i].skeleton.pos[1]);
        sprite[thingtakensnap->who].bonusstyle = bonus_berserker;
        sprite[thingtakensnap->who].bonustime = berserkerbonustime;
        if (thingtakensnap->who == mysprite)
            bigmessage(_("Berserker Mode!"), capturemessagewait, bonus_message_color);
        thing[i].kill();
    }
    break;
    case object_cluster_kit: {
        playsound(sfx_pickupgun, thing[i].skeleton.pos[1]);
        sprite[thingtakensnap->who].tertiaryweapon = guns[clustergrenade];
        sprite[thingtakensnap->who].tertiaryweapon.ammocount = cluster_grenades;
        if (thingtakensnap->who == mysprite)
            bigmessage(_("Cluster Grenades!"), capturemessagewait, capture_message_color);
        thing[i].kill();
    }
    break;
    case object_combat_knife:
    case object_chainsaw:
    case object_law: {
        // There are in total OBJECT_NUM_NONWEAPON non-weapon objects before the
        // knife so we need to subtract it+1 for the WeaponIndex (like before)
        weaponindex = weaponnumtoindex(thing[i].style - (object_num_nonweapon + 1), guns);
        sprite[thingtakensnap->who].applyweaponbynum(guns[weaponindex].num, n,
                                                     thingtakensnap->ammocount);
        if ((thingtakensnap->who == mysprite) && !sprite[mysprite].deadmeat)
            clientspritesnapshot();
    }
    break;
    case object_stationary_gun: {
        thing[i].statictype = true;
        sprite[thingtakensnap->who].stat = i;
        playsound(sfx_m2use, spriteparts.pos[thingtakensnap->who]);
    }
    break;
    }

    if (((thing[i].style > object_pointmatch_flag) && (thing[i].style < object_rambo_bow)) ||
        ((thing[i].style > object_parachute) && (thing[i].style < object_stationary_gun)))
    {
        playsound(sfx_pickupgun, thing[i].skeleton.pos[1]);
        sprite[thingtakensnap->who].weapon.fireintervalprev =
            sprite[thingtakensnap->who].weapon.fireinterval;
        sprite[thingtakensnap->who].weapon.fireintervalcount =
            sprite[thingtakensnap->who].weapon.fireinterval;
        thing[i].kill();
    }
}
