// automatically converted
#include "NetworkClientThing.hpp"

#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/Sound.hpp"
#include "../Cvar.hpp"
#include "../Game.hpp"
#include "NetworkClient.hpp"
#include "NetworkClientSprite.hpp"
#include "NetworkUtils.hpp"
#include "common/Calc.hpp"
#include "common/GameStrings.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"

void clienthandleserverthingsnapshot(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  tvector2 a;

  if (!verifypacket(sizeof(tmsg_serverthingsnapshot), netmessage->size, msgid_serverthingsnapshot))
  {
    return;
  }

  auto *thingsnap = pmsg_serverthingsnapshot(netmessage->packet);

  // assign received Thing info to thing
  const auto i = thingsnap->num;

  if ((thingsnap->num < 1) || (thingsnap->num > max_things))
  {
    return;
  }

  a.x = thingsnap->pos[1].x;
  a.y = thingsnap->pos[1].y;

  auto &things = GS::GetThingSystem().GetThings();
  auto &thing = things[i];

  if ((!thing.active) || (thing.style != thingsnap->style))
  {
    creatething(a, thingsnap->owner, thingsnap->style, i);

    for (auto d = 1; d <= 4; d++)
    {
      [[deprecated("minus 1")]] auto dminus1 = d - 1;
      thing.skeleton.pos[d].x = thingsnap->pos[dminus1].x;
      thing.skeleton.pos[d].y = thingsnap->pos[dminus1].y;
      thing.skeleton.oldpos[d].x = thingsnap->oldpos[dminus1].x;
      thing.skeleton.oldpos[d].y = thingsnap->oldpos[dminus1].y;
    }
  }

  thing.holdingsprite = thingsnap->holdingsprite;

  // is not holded anymore
  if (thing.holdingsprite == 0)
  {
    for (auto &sprite : sprite_system.GetActiveSprites())
    {
      if (sprite.holdedthing == i)
      {
        sprite.holdedthing = 0;
      }
    }
  }

  if ((thingsnap->owner > 0) && (thingsnap->owner < max_sprites + 1))
  {
    if (thing.holdingsprite > 0)
    {
      sprite_system.GetSprite(thingsnap->owner).holdedthing = i;
      sprite_system.GetSprite(thingsnap->owner).onground = false;
    }

    thing.color = sprite_system.GetSprite(thingsnap->owner).player->shirtcolor;
  }

  if ((thing.holdingsprite == 0) && (thing.style != object_stationary_gun))
  {
    if ((distance(thing.skeleton.pos[1], thingsnap->pos[1]) > 10) &&
        (distance(thing.skeleton.pos[2], thingsnap->pos[2]) > 10))
    {
      for (auto d = 1; d <= 4; d++)
      {
        [[deprecated("maybe change d scope from 1 to 0?")]] auto dminus1 = d - 1;
        thing.skeleton.pos[d].x = thingsnap->pos[dminus1].x;
        thing.skeleton.pos[d].y = thingsnap->pos[dminus1].y;
        thing.skeleton.oldpos[d].x = thingsnap->oldpos[dminus1].x;
        thing.skeleton.oldpos[d].y = thingsnap->oldpos[dminus1].y;
      }
    }
  }

  if ((thing.holdingsprite > 0) && (thing.style != object_parachute))
  {
    auto &spritePartsPos = sprite_system.GetSpritePartsPos(thing.holdingsprite);
    if (distance(thing.skeleton.pos[1], spritePartsPos) > 330)
    {
      for (auto d = 1; d <= 4; d++)
      {
        [[deprecated("maybe change d scope from 1 to 0?")]] auto dminus1 = d - 1;
        thing.skeleton.pos[d].x = thingsnap->pos[dminus1].x;
        thing.skeleton.pos[d].y = thingsnap->pos[dminus1].y;
        thing.skeleton.oldpos[d].x = thingsnap->oldpos[dminus1].x;
        thing.skeleton.oldpos[d].y = thingsnap->oldpos[dminus1].y;
      }
    }
  }

  thing.statictype = false;

  if (thing.style == object_rambo_bow)
  {
    gamethingtarget = i;
  }
}

void clienthandleserverthingmustsnapshot(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_serverthingmustsnapshot *thingmustsnap;
  std::int32_t d;
  tvector2 a;
  tsprite *spritethingowner;
  std::int32_t weaponthing;

  if (!verifypacket(sizeof(tmsg_serverthingmustsnapshot), netmessage->size,
                    msgid_serverthingmustsnapshot))
  {
    return;
  }

  thingmustsnap = pmsg_serverthingmustsnapshot(netmessage->packet);

  // assign received Thing info to thing
  const auto i = thingmustsnap->num;

  if ((i < 1) || (i > max_things))
  {
    return;
  }

  auto &things = GS::GetThingSystem().GetThings();
  auto &thing = things[i];

  if ((thingmustsnap->owner > 0) && (thingmustsnap->owner < max_sprites + 1))
  {
    spritethingowner = &sprite_system.GetSprite(thingmustsnap->owner);
  }
  else
  {
    spritethingowner = nullptr;
  }

  if ((!thing.active) or (thing.style != thingmustsnap->style))
  {
    a.x = thingmustsnap->pos[1].x;
    a.y = thingmustsnap->pos[1].y;
    creatething(a, thingmustsnap->owner, thingmustsnap->style, i);

    [[deprecated("it was d = 1, d <= 4")]] auto d = 0;
    for (d = 0; d < 4; d++)
    {
      thing.skeleton.pos[d + 1].x = thingmustsnap->pos[d].x;
      thing.skeleton.pos[d + 1].y = thingmustsnap->pos[d].y;
      thing.skeleton.oldpos[d + 1].x = thingmustsnap->oldpos[d].x;
      thing.skeleton.oldpos[d + 1].y = thingmustsnap->oldpos[d].y;
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
        {
          spritethingowner->applyweaponbynum(noweapon_num, 1);
        }
        else if (spritethingowner->secondaryweapon.num == weaponthing)
        {
          spritethingowner->applyweaponbynum(noweapon_num, 2);
        }
        if ((sprite_system.IsPlayerSprite(spritethingowner->num)) && !spritethingowner->deadmeat)
        {
          clientspritesnapshot();
        }
      }
    }
  }

  thing.owner = thingmustsnap->owner;
  thing.holdingsprite = thingmustsnap->holdingsprite;

  // is not holded anymore
  if (thing.holdingsprite == 0)
  {
    for (auto &sprite : sprite_system.GetActiveSprites())
    {
      if (sprite.holdedthing == i)
      {
        sprite.holdedthing = 0;
      }
    }
  }

  if ((thingmustsnap->owner > 0) && (thingmustsnap->owner < max_sprites + 1))
  {
    if (thing.holdingsprite > 0)
    {
      spritethingowner->holdedthing = i;
      spritethingowner->onground = false;
    }

    thing.color = spritethingowner->player->shirtcolor;
  }

  if ((thing.holdingsprite == 0) && (!(thing.style == object_stationary_gun)))
  {
    for (d = 1; d <= 4; d++)
    {
      [[deprecated("indexing")]] auto dminus1 = d - 1;
      thing.skeleton.pos[d].x = thingmustsnap->pos[dminus1].x;
      thing.skeleton.pos[d].y = thingmustsnap->pos[dminus1].y;
      thing.skeleton.oldpos[d].x = thingmustsnap->oldpos[dminus1].x;
      thing.skeleton.oldpos[d].y = thingmustsnap->oldpos[dminus1].y;
    }
  }

  thing.timeout = thingmustsnap->timeout;
  thing.statictype = false;
  if (thing.style == object_rambo_bow)
  {
    gamethingtarget = i;
  }
}

void clienthandlethingtaken(NetworkContext *netmessage)
{
  auto &sprite_system = SpriteSystem::Get();
  tmsg_serverthingtaken *thingtakensnap;
  std::int32_t j;
  std::int32_t n;
  tmsg_requestthing requestthingmsg;
  std::uint8_t weaponindex;
  std::uint32_t capcolor = capture_message_color;
  std::string bigcaptext;
  std::string smallcaptext;

  if (!verifypacket(sizeof(tmsg_serverthingtaken), netmessage->size, msgid_thingtaken))
  {
    return;
  }

  auto &guns = GS::GetWeaponSystem().GetGuns();

  thingtakensnap = pmsg_serverthingtaken(netmessage->packet);

  const auto i = thingtakensnap->num;
  if ((i < 1) || (i > max_things))
  {
    return;
  }

  auto &things = GS::GetThingSystem().GetThings();
  auto &thing = things[i];

  if (thingtakensnap->who == 255)
  {
    thing.kill();
  }

  if ((thingtakensnap->who < 1) || (thingtakensnap->who > max_sprites))
  {
    return;
  }

  if (!sprite_system.GetSprite(thingtakensnap->who).active)
  {
    return;
  }

  if ((!thing.active) && ((thingtakensnap->style == object_stationary_gun) ||
                          (thingtakensnap->style < object_ussocom)))
  {
    // request the new thing if not active
    requestthingmsg.header.id = msgid_requestthing;
    requestthingmsg.thingid = i;
    GetNetwork()->SendData(&requestthingmsg, sizeof(requestthingmsg),
                           false);
    return;
  }

  thing.style = thingtakensnap->style;

  j = thingtakensnap->who;
  if (sprite_system.GetSprite(j).weapon.num == noweapon_num)
  {
    n = 1;
  }
  else
  {
    n = 2;
  }

  switch (thing.style)
  {
  case object_alpha_flag:
  case object_bravo_flag:
  case object_pointmatch_flag: {
    // capture sound
    playsound(SfxEffect::capture, thing.skeleton.pos[1]);
    thing.holdingsprite = thingtakensnap->who;
    thing.statictype = false;

    j = thingtakensnap->who;

    switch (CVar::sv_gamemode)
    {
    case gamestyle_pointmatch:
    case gamestyle_inf:
      capcolor = capture_message_color;
      break;
    case gamestyle_htf:
    case gamestyle_ctf:
      switch (sprite_system.GetSprite(j).player->team)
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
      bigcaptext =
        iif(sprite_system.IsPlayerSprite(j), _("You got the Flag!"), _("Yellow Flag captured!"));
      smallcaptext = _("{} got the Yellow Flag");
    }
    break;
    case gamestyle_ctf:
      if (sprite_system.GetSprite(j).player->team == thing.style)
      {
        switch (sprite_system.GetSprite(j).player->team)
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
        thing.respawn();
      }
      else
      {
        switch (sprite_system.GetSprite(j).player->team)
        {
        case team_alpha: {
          bigcaptext = iif(sprite_system.IsPlayerSprite(j), _("You got the Blue Flag!"),
                           _("Blue Flag captured!"));
          smallcaptext = _("{} captured the Blue Flag");
        }
        break;
        case team_bravo: {
          bigcaptext = iif(sprite_system.IsPlayerSprite(j), _("You got the Red Flag!"),
                           _("Red Flag captured!"));
          smallcaptext = _("{} captured the Red Flag");
        }
        break;
        }
      }
      break;
    case gamestyle_inf:
      if (sprite_system.GetSprite(j).player->team == thing.style)
      {
        if (sprite_system.GetSprite(j).player->team == team_bravo)
        {
          bigcaptext = iif(sprite_system.IsPlayerSprite(j), _("You returned the Objective!"),
                           _("Objective returned!"));
          smallcaptext = _("{} returned the Objective");
        }
        thing.respawn();
      }
      else
      {
        if (sprite_system.GetSprite(j).player->team == team_alpha)
        {
          bigcaptext = iif(sprite_system.IsPlayerSprite(j), _("You got the Objective!"),
                           _("Objective captured!"));
          smallcaptext = _("{} captured the Objective");
        }
      }
      break;
    }

    if (!smallcaptext.empty())
    {
      bigmessage(bigcaptext, capturemessagewait, capcolor);
      NotImplemented("network");
#if 0
            GS::GetMainConsole().console(smallcaptext, (SpriteSystem::Get().GetSprite(j).player->name), capcolor);
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
    weaponindex = weaponnumtoindex(thing.style - (object_num_flags + 1), guns);
    sprite_system.GetSprite(thingtakensnap->who)
      .applyweaponbynum(guns[weaponindex].num, n, thingtakensnap->ammocount);
    if ((sprite_system.IsPlayerSprite(thingtakensnap->who)) &&
        !sprite_system.GetPlayerSprite().deadmeat)
    {
      clientspritesnapshot();
    }
  }
  break;
  case object_rambo_bow: {
    playsound(SfxEffect::takebow, thing.skeleton.pos[1]);
    sprite_system.GetSprite(thingtakensnap->who).applyweaponbynum(bow_num, 1, 1);
    sprite_system.GetSprite(thingtakensnap->who).applyweaponbynum(bow2_num, 2, 1);
    sprite_system.GetSprite(thingtakensnap->who).weapon.ammocount = 1;
    sprite_system.GetSprite(thingtakensnap->who).weapon.fireinterval = 10;
    sprite_system.GetSprite(thingtakensnap->who).wearhelmet = 1;
    thing.kill();
    gamethingtarget = 0;

    if ((sprite_system.IsPlayerSprite(thingtakensnap->who)) &&
        !sprite_system.GetPlayerSprite().deadmeat)
    {
      clientspritesnapshot();
    }

    if (sprite_system.IsPlayerSprite(thingtakensnap->who))
    {
      bigmessage(_("You got the Bow!"), capturemessagewait, capture_message_color);
    }
    else
    {
      bigmessage(
        wideformat(_("{} got the Bow!"), sprite_system.GetSprite(thingtakensnap->who).player->name),
        capturemessagewait, capture_message_color);
    }
  }
  break;
  case object_medical_kit: {
    playsound(SfxEffect::takemedikit, thing.skeleton.pos[1]);
    sprite_system.GetSprite(thingtakensnap->who).SetHealth(GS::GetGame().GetStarthealth());
    thing.kill();
  }
  break;
  case object_grenade_kit: {
    playsound(SfxEffect::pickupgun, thing.skeleton.pos[1]);
    sprite_system.GetSprite(thingtakensnap->who).SetThirdWeapon(guns[fraggrenade]);
    sprite_system.GetSprite(thingtakensnap->who).tertiaryweapon.ammocount = CVar::sv_maxgrenades;
    thing.kill();
  }
  break;
  case object_flamer_kit: {
    playsound(SfxEffect::godflame, thing.skeleton.pos[1]);
    sprite_system.GetSprite(thingtakensnap->who).bonustime = flamerbonustime;
    sprite_system.GetSprite(thingtakensnap->who).bonusstyle = bonus_flamegod;
    sprite_system.GetSprite(thingtakensnap->who)
      .applyweaponbynum(sprite_system.GetSprite(thingtakensnap->who).weapon.num, 2, -1, true);
    sprite_system.GetSprite(thingtakensnap->who).applyweaponbynum(flamer_num, 1);
    if (sprite_system.IsPlayerSprite(thingtakensnap->who))
    {
      bigmessage(_("Flame God Mode!"), capturemessagewait, bonus_message_color);
      if (!sprite_system.GetPlayerSprite().deadmeat)
      {
        clientspritesnapshot();
      }
    }
    thing.kill();
  }
  break;
  case object_predator_kit: {
    playsound(SfxEffect::predator, thing.skeleton.pos[1]);
    sprite_system.GetSprite(thingtakensnap->who).alpha = predatoralpha;
    sprite_system.GetSprite(thingtakensnap->who).bonustime = predatorbonustime;
    sprite_system.GetSprite(thingtakensnap->who).bonusstyle = bonus_predator;
    if (sprite_system.IsPlayerSprite(thingtakensnap->who))
    {
      bigmessage(_("Predator Mode!"), capturemessagewait, bonus_message_color);
    }
    thing.kill();
  }
  break;
  case object_vest_kit: {
    playsound(SfxEffect::vesttake, thing.skeleton.pos[1]);
    sprite_system.GetSprite(thingtakensnap->who).vest = defaultvest;
    if (sprite_system.IsPlayerSprite(thingtakensnap->who))
    {
      bigmessage(_("Bulletproof Vest!"), capturemessagewait, capture_message_color);
    }
    thing.kill();
  }
  break;
  case object_berserk_kit: {
    playsound(SfxEffect::berserker, thing.skeleton.pos[1]);
    sprite_system.GetSprite(thingtakensnap->who).bonusstyle = bonus_berserker;
    sprite_system.GetSprite(thingtakensnap->who).bonustime = berserkerbonustime;
    if (sprite_system.IsPlayerSprite(thingtakensnap->who))
    {
      bigmessage(_("Berserker Mode!"), capturemessagewait, bonus_message_color);
    }
    thing.kill();
  }
  break;
  case object_cluster_kit: {
    playsound(SfxEffect::pickupgun, thing.skeleton.pos[1]);
    sprite_system.GetSprite(thingtakensnap->who).SetThirdWeapon(guns[clustergrenade]);
    sprite_system.GetSprite(thingtakensnap->who).tertiaryweapon.ammocount = cluster_grenades;
    if (sprite_system.IsPlayerSprite(thingtakensnap->who))
    {
      bigmessage(_("Cluster Grenades!"), capturemessagewait, capture_message_color);
    }
    thing.kill();
  }
  break;
  case object_combat_knife:
  case object_chainsaw:
  case object_law: {
    // There are in total OBJECT_NUM_NONWEAPON non-weapon objects before the
    // knife so we need to subtract it+1 for the WeaponIndex (like before)
    weaponindex = weaponnumtoindex(thing.style - (object_num_nonweapon + 1), guns);
    sprite_system.GetSprite(thingtakensnap->who)
      .applyweaponbynum(guns[weaponindex].num, n, thingtakensnap->ammocount);
    if ((sprite_system.IsPlayerSprite(thingtakensnap->who)) &&
        !sprite_system.GetPlayerSprite().deadmeat)
    {
      clientspritesnapshot();
    }
  }
  break;
  case object_stationary_gun: {
    thing.statictype = true;
    sprite_system.GetSprite(thingtakensnap->who).stat = i;
    const auto &spritePartsPos = sprite_system.GetSpritePartsPos(thingtakensnap->who);
    playsound(SfxEffect::m2use, spritePartsPos);
  }
  break;
  }

  if (((thing.style > object_pointmatch_flag) && (thing.style < object_rambo_bow)) ||
      ((thing.style > object_parachute) && (thing.style < object_stationary_gun)))
  {
    playsound(SfxEffect::pickupgun, thing.skeleton.pos[1]);
    sprite_system.GetSprite(thingtakensnap->who).weapon.fireintervalprev =
      sprite_system.GetSprite(thingtakensnap->who).weapon.fireinterval;
    sprite_system.GetSprite(thingtakensnap->who).weapon.fireintervalcount =
      sprite_system.GetSprite(thingtakensnap->who).weapon.fireinterval;
    thing.kill();
  }
}
