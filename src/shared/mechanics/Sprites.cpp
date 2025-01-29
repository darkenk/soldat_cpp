// automatically converted

#include "Sprites.hpp"
#ifndef SERVER
#include "../../client/Client.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/Sound.hpp"
#include "../../shared/network/NetworkClientSprite.hpp"
#else
#include "../../server/Server.hpp"
#include "../../server/ServerHelper.hpp"
#include "common/LogFile.hpp"
#include "../network/NetworkServerConnection.hpp"
#include "../network/NetworkServerGame.hpp"
#include "../network/NetworkServerMessages.hpp"
#include "../network/NetworkServerSprite.hpp"
#include "../network/NetworkServerThing.hpp"
#endif
#include "../Cvar.hpp"
#include "../Game.hpp"
#include "Control.hpp"
#include "SpriteSystem.hpp"
#include "common/Calc.hpp"
#include "common/Console.hpp"
#include "common/GameStrings.hpp"
#include "common/Logging.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"
#include <Tracy.hpp>
#include <client/ClientGame.hpp>
#if !SOLDAT_UTBOT
#include <numbers>
#endif // !SOLDAT_UTBOT

#ifndef SERVER
bool wasreloading;
#endif

auto constexpr LOG = "sprites";

#if SOLDAT_UTBOT
constexpr auto pi = 3.14159265358979323846;
#else
using std::numbers::pi;
#endif // SOLDAT_UTBOT

namespace
{
bool weaponscleaned = false;
}

template <Config::Module M>
auto createsprite(tvector2 &spos, SpriteId id, std::shared_ptr<tplayer> player,
                  const tsprite::Style style) -> std::int32_t
{
  LogDebug(LOG, "CreateSprite");
  auto &game = GS::GetGame();
  auto &map = game.GetMap();
  auto &anim = AnimationSystem::Get();
  auto &weapon = GS::GetWeaponSystem();
  auto &spriteSystem = SpriteSystem::Get();

  auto &sprite = spriteSystem.CreateSprite(id);

  // replace player object
  if (sprite.player != nullptr)
  {
    sprite.player->spritenum = 0;
  }
  sprite.player = std::static_pointer_cast<std::conditional_t<Config::IsServer(M), TServerPlayer, tplayer>>(player);
  sprite.player->spritenum = sprite.num;
  sprite.ceasefirecounter = game.GetCeasefiretime();

  if (CVar::sv_survivalmode)
  {
    sprite.ceasefirecounter = sprite.ceasefirecounter * 3;
  }

  auto &guns = weapon.GetGuns();
  sprite.brain.pissedoff = 0;
  sprite.SetThirdWeapon(guns[fraggrenade]);

  sprite.bodyanimation = anim.GetAnimation(AnimationType::Stand);
  sprite.legsanimation = anim.GetAnimation(AnimationType::Stand);

  sprite.bgstate.backgroundstatus = background_transition;
  sprite.bgstate.backgroundpoly = background_poly_unknown;

  if (sprite.player->team == team_spectator)
  {
    spos.x = min_sectorz * map.GetSectorsDivision() * 0.8;
    spos.y = min_sectorz * map.GetSectorsDivision() * 0.8;
  }

  // activate sprite part
  spriteSystem.CreateSpritePart(spos, tvector2(0.f, 0.f), 1, sprite.num);

  // create skeleton
  sprite.skeleton = anim.GetSkeleton(Gostek);
  sprite.skeleton.vdamping = 0.9945;

  sprite.SetHealth(game.GetStarthealth());

  sprite.SetFirstWeapon(guns[noweapon]);

  const std::int32_t secwep = sprite.player->secwep + 1;
  auto secGun = noweapon;
  if ((secwep >= 1) && (secwep <= secondary_weapons) &&
      (weapon.IsEnabled(primary_weapons + secwep)))
  {
    secGun = primary_weapons + secwep;
  }
  sprite.SetSecondWeapon(guns[secGun]);

  sprite.jetscount = map.startjet;
  if constexpr(Config::IsClient(M))
  {
    sprite.jetscountprev = map.startjet;
  }
  sprite.tertiaryweapon.ammocount = CVar::sv_maxgrenades / 2;

  if (sprite.player->headcap == 0)
  {
    sprite.wearhelmet = 0;
  }

  sprite.brain.targetnum = 1;
  sprite.brain.waypointtimeoutcounter = waypointtimeout;

#ifndef SERVER
  const std::int32_t i = sprite.num;
  sprite.reloadsoundchannel = i - 1;
  sprite.jetssoundchannel = 1 * max_sprites + i - 1;
  sprite.gattlingsoundchannel = 2 * max_sprites + i - 1;
  sprite.gattlingsoundchannel2 = 3 * max_sprites + i - 1;
  sprite.moveskeleton(spos.x, spos.y, false);
#else
  sprite.moveskeleton(0, 0, false);
#endif

  sprite.bulletcount = Random(std::numeric_limits<std::uint16_t>::max()); // FIXME wat?

  game.sortplayers(); // sort the players frag list
  return sprite.num;
}

template <Config::Module M>
auto teamcollides(PolygonType polytype, std::int32_t team, const bool bullet) -> bool
{
  bool result;
  result = true;
  if (bullet)
  {
    if ((polytype == poly_type_red_bullets) || (polytype == poly_type_red_player))
    {
      result = (team == team_alpha) && (polytype == poly_type_red_bullets);
    }
    else if ((polytype == poly_type_blue_bullets) || (polytype == poly_type_blue_player))
    {
      result = (team == team_bravo) && (polytype == poly_type_yellow_bullets);
    }
    else if ((polytype == poly_type_yellow_bullets) || (polytype == poly_type_yellow_player))
    {
      result = (team == team_charlie) && (polytype == poly_type_yellow_bullets);
    }
    else if ((polytype == poly_type_green_bullets) || (polytype == poly_type_green_player))
    {
      result = (team == team_delta) && (polytype == poly_type_green_bullets);
    }
  }
  else
  {
    if (((polytype == poly_type_red_bullets) && (team == team_alpha)) ||
        (((polytype == poly_type_red_bullets) || (polytype == poly_type_red_player)) &&
         (team != team_alpha)))
    {
      result = false;
    }
    else if (((polytype == poly_type_blue_bullets) && (team == team_bravo)) ||
             (((polytype == poly_type_blue_bullets) || (polytype == poly_type_blue_player)) &&
              (team != team_bravo)))
    {
      result = false;
    }
    else if (((polytype == poly_type_yellow_bullets) && (team == team_charlie)) ||
             (((polytype == poly_type_yellow_bullets) || (polytype == poly_type_yellow_player)) &&
              (team != team_charlie)))
    {
      result = false;
    }
    else if (((polytype == poly_type_green_bullets) && (team == team_delta)) ||
             (((polytype == poly_type_green_bullets) || (polytype == poly_type_green_player)) &&
              (team != team_delta)))
    {
      result = false;
    }
  }
  if (polytype == poly_type_non_flagger_collides)
  {
    result = false;
  }
  return result;
}

template <Config::Module M>
Sprite<M>::Sprite(const uint8_t _num, bool _active) : active{_active}, num{_num}
{
  // clear push wait list
  for (auto j = 0; j <= max_pushtick; j++)
  {
    nextpush[j].x = 0;
    nextpush[j].y = 0;
  }
  freecontrols();
}

template <Config::Module M>
void Sprite<M>::update()
{
  ZoneScopedN("Sprite::Update");
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
#ifndef SERVER
  std::int32_t k;
  std::int32_t rnd;
  tvector2 m3;
  tvector2 m4;
  SfxEffect weaponreloadsound;
#endif
  tvector2 mouseaim;
  tvector2 p;
  tvector2 m;
  // rotation vars
  tvector2 rnorm;
  tvector2 legvector;
  float bodyy;
  float arms;
  float legdistance = 0.0;

  auto &map = GS::GetGame().GetMap();

#ifdef SERVER
  LogTraceG("TSprite.Update");
#endif

  SoldatAssert(&sprite_system.GetSprite(num) == this);

  jetscountprev = jetscount;
  weapon.reloadtimeprev = weapon.reloadtimecount;
  weapon.fireintervalprev = weapon.fireintervalcount;

  bodyy = 0;

  auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);
  auto &spriteVelocity = sprite_system.GetVelocity(num);

  spriteVelocity = vec2add(spriteVelocity, nextpush[0]);
#ifndef SERVER
  {
    ZoneScopedN("NextPushCopy");
    for (i = 0; i <= max_pushtick - 1; i++)
    {
      nextpush[i] = nextpush[i + 1];
    }
  }
#endif
  nextpush[max_pushtick].x = 0;
  nextpush[max_pushtick].y = 0;

  // reload spas after shooting delay is over
  if (autoreloadwhencanfire && ((weapon.num != spas12_num) || (weapon.fireintervalcount == 0)))
  {
    autoreloadwhencanfire = false;

    if ((weapon.num == spas12_num) && (bodyanimation.id != AnimationType::Roll) &&
        (bodyanimation.id != AnimationType::RollBack) &&
        (bodyanimation.id != AnimationType::Change) && (weapon.ammocount != weapon.ammo))
    {
      bodyapplyanimation(AnimationType::Reload, 1);
    }
  }

#ifndef SERVER
  if (clientstopmovingcounter > 0)
#else
  if (((player->controlmethod == human) && (noclientupdatetime[num] < clientstopmove_retrys)) ||
      (player->controlmethod == bot))
#endif
  {
    controlsprite(sprite_system.GetSprite(num));
  }

  if (isspectator())
  {
    deadmeat = true;

#ifndef SERVER
    if (sprite_system.IsPlayerSprite(num))
    {
      respawncounter = 19999;
      gamemenushow(limbomenu, false);
    }
#endif
  }

  skeleton.oldpos[21] = skeleton.pos[21];
  skeleton.oldpos[23] = skeleton.pos[23];
  skeleton.oldpos[25] = skeleton.pos[25];
  skeleton.pos[21] = skeleton.pos[9];
  skeleton.pos[23] = skeleton.pos[12];
  skeleton.pos[25] = skeleton.pos[5];
  if (!deadmeat)
  {
    NotImplemented("This code has no effect");
    vec2add(skeleton.pos[21], spriteVelocity);
    vec2add(skeleton.pos[23], spriteVelocity);
    vec2add(skeleton.pos[25], spriteVelocity);
  }

  switch (position)
  {
  case pos_stand:
    bodyy = 8;
    break;
  case pos_crouch:
    bodyy = 9;
    break;
  case pos_prone: {
    if (bodyanimation.id == AnimationType::Prone)
    {
      if (bodyanimation.currframe > 9)
      {
        bodyy = -2;
      }
      else
      {
        bodyy = 14 - bodyanimation.currframe;
      }
    }
    else
    {
      bodyy = 9;
    }

    if (bodyanimation.id == AnimationType::ProneMove)
    {
      bodyy = 0;
    }
  }
  break;
  }

  if (bodyanimation.id == AnimationType::GetUp)
  {
    if (bodyanimation.currframe > 18)
    {
      bodyy = 8;
    }
    else
    {
      bodyy = 4;
    }
  }

  if (flaggrabcooldown > 0)
  {
    flaggrabcooldown -= 1;
  }

  // Reset the background poly test before collision checks on the corpse
  if (deadmeat)
  {
    bgstate.backgroundtestprepare();
  }

  if (control.mouseaimx >= spritePartsPos.x)
  {
    direction = 1;
  }
  else
  {
    direction = -1;
  }

  for (i = 1; i <= 20; i++)
  {
    if (skeleton.active[i] && !deadmeat)
    {
      skeleton.oldpos[i] = skeleton.pos[i];

      if (!halfdead)
      {
        // legs
        if ((i == 1) || (i == 4) || (i == 2) || (i == 3) || (i == 5) || (i == 6) || (i == 17) ||
            (i == 18))
        {
          skeleton.pos[i].x =
            spritePartsPos.x + direction * legsanimation.frames[legsanimation.currframe].pos[i].x;
          skeleton.pos[i].y =
            spritePartsPos.y + legsanimation.frames[legsanimation.currframe].pos[i].y;
        }
      }

      // body
      if ((i == 7) || (i == 8) || (i == 9) || (i == 10) || (i == 11) || (i == 12) || (i == 13) ||
          (i == 14) || (i == 15) || (i == 16) || (i == 19) || (i == 20))
      {
        skeleton.pos[i].x =
          spritePartsPos.x + direction * bodyanimation.frames[bodyanimation.currframe].pos[i].x;
        if (!halfdead)
        {
          skeleton.pos[i].y = (skeleton.pos[6].y - (spritePartsPos.y - bodyy)) + spritePartsPos.y +
                              bodyanimation.frames[bodyanimation.currframe].pos[i].y;
        }
        else
        {
          skeleton.pos[i].y =
            9 + spritePartsPos.y + bodyanimation.frames[bodyanimation.currframe].pos[i].y;
        }
      }
    }
  }

  if (!deadmeat)
  {
    ZoneScopedN("RotateParts");
    // Rotate parts
    // head
    i = 12;
    {
      p.x = skeleton.pos[i].x;
      p.y = skeleton.pos[i].y;
      mouseaim.x = control.mouseaimx;
      mouseaim.y = control.mouseaimy;
      rnorm = vec2subtract(p, mouseaim);
      vec2normalize(rnorm, rnorm);
      vec2scale(rnorm, rnorm, 0.1);
      skeleton.pos[i].x = skeleton.pos[9].x - direction * rnorm.y;
      skeleton.pos[i].y = skeleton.pos[9].y + direction * rnorm.x;

      vec2scale(rnorm, rnorm, 50);
      skeleton.pos[23].x = skeleton.pos[9].x - direction * rnorm.y;
      skeleton.pos[23].y = skeleton.pos[9].y + direction * rnorm.x;
    }

    if (bodyanimation.id == AnimationType::Throw)
    {
      arms = -5;
    }
    else
    {
      arms = -7;
    }

    // arm
    i = 15;
    if ((bodyanimation.id != AnimationType::Reload) &&
        (bodyanimation.id != AnimationType::ReloadBow) &&
        (bodyanimation.id != AnimationType::ClipIn) &&
        (bodyanimation.id != AnimationType::ClipOut) &&
        (bodyanimation.id != AnimationType::SlideBack) &&
        (bodyanimation.id != AnimationType::Change) &&
        (bodyanimation.id != AnimationType::ThrowWeapon) &&
        (bodyanimation.id != AnimationType::WeaponNone) &&
        (bodyanimation.id != AnimationType::Punch) && (bodyanimation.id != AnimationType::Roll) &&
        (bodyanimation.id != AnimationType::RollBack) &&
        (bodyanimation.id != AnimationType::Cigar) && (bodyanimation.id != AnimationType::Match) &&
        (bodyanimation.id != AnimationType::Smoke) && (bodyanimation.id != AnimationType::Wipe) &&
        (bodyanimation.id != AnimationType::TakeOff) &&
        (bodyanimation.id != AnimationType::Groin) && (bodyanimation.id != AnimationType::Piss) &&
        (bodyanimation.id != AnimationType::Mercy) && (bodyanimation.id != AnimationType::Mercy2) &&
        (bodyanimation.id != AnimationType::Victory) && (bodyanimation.id != AnimationType::Own) &&
        (bodyanimation.id != AnimationType::Melee))
    {
      p.x = skeleton.pos[i].x;
      p.y = skeleton.pos[i].y;
      mouseaim.x = control.mouseaimx;
      mouseaim.y = control.mouseaimy;
      rnorm = vec2subtract(p, mouseaim);
      vec2normalize(rnorm, rnorm);
      vec2scale(rnorm, rnorm, arms);
      m.x = skeleton.pos[16].x;
      m.y = skeleton.pos[16].y;
      p = vec2add(m, rnorm);
      skeleton.pos[i].x = p.x;
      skeleton.pos[i].y = p.y;
    }

    if (bodyanimation.id == AnimationType::Throw)
    {
      arms = -6;
    }
    else
    {
      arms = -8;
    }

    // arm
    i = 19;
    if ((bodyanimation.id != AnimationType::Reload) &&
        (bodyanimation.id != AnimationType::ReloadBow) &&
        (bodyanimation.id != AnimationType::ClipIn) &&
        (bodyanimation.id != AnimationType::ClipOut) &&
        (bodyanimation.id != AnimationType::SlideBack) &&
        (bodyanimation.id != AnimationType::Change) &&
        (bodyanimation.id != AnimationType::ThrowWeapon) &&
        (bodyanimation.id != AnimationType::WeaponNone) &&
        (bodyanimation.id != AnimationType::Punch) && (bodyanimation.id != AnimationType::Roll) &&
        (bodyanimation.id != AnimationType::RollBack) &&
        (bodyanimation.id != AnimationType::Cigar) && (bodyanimation.id != AnimationType::Match) &&
        (bodyanimation.id != AnimationType::Smoke) && (bodyanimation.id != AnimationType::Wipe) &&
        (bodyanimation.id != AnimationType::TakeOff) &&
        (bodyanimation.id != AnimationType::Groin) && (bodyanimation.id != AnimationType::Piss) &&
        (bodyanimation.id != AnimationType::Mercy) && (bodyanimation.id != AnimationType::Mercy2) &&
        (bodyanimation.id != AnimationType::Victory) && (bodyanimation.id != AnimationType::Own) &&
        (bodyanimation.id != AnimationType::Melee))
    {
      p.x = skeleton.pos[i].x;
      p.y = skeleton.pos[i].y;
      mouseaim.x = control.mouseaimx;
      mouseaim.y = control.mouseaimy;
      rnorm = vec2subtract(p, mouseaim);
      vec2normalize(rnorm, rnorm);
      vec2scale(rnorm, rnorm, arms);
      m.x = skeleton.pos[16].x;
      m.y = skeleton.pos[16].y - 4;
      p = vec2add(m, rnorm);
      skeleton.pos[i].x = p.x;
      skeleton.pos[i].y = p.y;
    }
  }

  {
    ZoneScopedN("DeadMeatLoop");
    for (i = 1; i <= 20; i++)
    {
      // dead part
      if (deadmeat || (halfdead && isnotspectator()))
      {
        if ((i != 17) && (i != 18) && (i != 19) && (i != 20) && (i != 8) && (i != 7) && (i < 21))
        {
          onground = checkskeletonmapcollision(i, skeleton.pos[i].x, skeleton.pos[i].y);
        }

#ifndef SERVER
        // bleed
        // check where constraints are cut then BLEED
        for (k = 1; k <= skeleton.constraintcount; k++)
        {
          if (!skeleton.constraints[k].active and
              ((skeleton.constraints[k].parta == i) || (skeleton.constraints[k].partb == i)))
          {
            m4 = skeleton.pos[i];
            m4.y = m4.y + 2;
            m3 = vec2subtract(skeleton.pos[i], skeleton.oldpos[i]);
            vec2scale(m3, m3, 0.35);

            if (sparkscount > 300)
            {
              rnd = blood_random_low;
            }
            else if (sparkscount > 50)
            {
              rnd = blood_random_normal;
            }
            else
            {
              rnd = blood_random_high;
            }

            if (deadtime > lessbleed_time)
            {
              rnd = 2 * rnd;
            }
            if (deadtime > nobleed_time)
            {
              rnd = 100 * rnd;
            }

            if (CVar::r_maxsparks < (max_sparks - 10))
            {
              rnd = 2 * rnd;
            }

            if ((k != 10) && (k != 11))
            {
              if (Random(rnd) == 0)
              {
                createspark(m4, m3, 5, num, 85 - Random(25));
              }
              else if (Random(rnd / 3) == 0)
              {
                createspark(m4, m3, 4, num, 85 - Random(25));
              }
            }
          }
        } // bleed

        // fire
        if (deadtime < onfire_time)
        {
          if (onfire > 0)
          {
            if (i % onfire == 0)
            {
              m4 = skeleton.pos[i];
              m4.y = m4.y + 3;
              m3 = vec2subtract(skeleton.pos[i], skeleton.oldpos[i]);
              vec2scale(m3, m3, 0.3);

              rnd = fire_random_normal;
              if (sparkscount > 170)
              {
                rnd = fire_random_low;
              }
              if (sparkscount < 17)
              {
                rnd = fire_random_high;
              }

              if (CVar::r_maxsparks < (max_sparks - 10))
              {
                rnd = 2 * rnd;
              }

              if (Random(rnd) == 0)
              {
                createspark(m4, m3, 36, num, 35);
                if (Random(8) == 0)
                {
                  playsound(SfxEffect::onfire, spritePartsPos);
                }
                if (Random(2) == 0)
                {
                  playsound(SfxEffect::firecrack, spritePartsPos);
                }
              }
              else if (Random(rnd / 3) == 0)
              {
                createspark(m4, m3, 37, num, 75);
              }
            }
          }
        }
#endif
      }
    }
  }

  // If no background poly contact in CheckSkeletonMapCollision() then reset any background poly
  // status
  if (deadmeat)
  {
    bgstate.backgroundtestreset();
  }

#ifdef SERVER
  LogTraceG("TSprite.Update 2");
#endif
  auto &things = GS::GetThingSystem().GetThings();

  if (!deadmeat)
  {
    ZoneScopedN("DoAnimation");
    switch (style)
    {
    case tsprite::Style::Default: {
      bodyanimation.DoAnimation();
      legsanimation.DoAnimation();

      checkoutofbounds();

      onground = false;

#ifndef SERVER
      if (olddeadmeat)
      {
        respawn();
        olddeadmeat = deadmeat;
      }
#endif

      // Reset the background poly test before collision checks
      bgstate.backgroundtestprepare();

      // head
      checkmapcollision(spritePartsPos.x - 3.5, spritePartsPos.y - 12, 1);

      checkmapcollision(spritePartsPos.x + 3.5, spritePartsPos.y - 12, 1);

      bodyy = 0;
      arms = 0;

      // Walking either left or right (though only one can be active at once)
      if (control.left ^ control.right)
      {
        // If walking in facing direction
        if (control.left ^ (direction == 1))
        {
          arms = 0.25;
        }
        else
        { // Walking backwards
          bodyy = 0.25;
        }
      }

      // If a leg is inside a polygon, caused by the modification of ArmS and
      // BodyY, this is there to not lose contact to ground on slope polygons
      if (bodyy == 0)
      {
        legvector = vector2(spritePartsPos.x + 2, spritePartsPos.y + 1.9);
        if (map.raycast(legvector, legvector, legdistance, 10))
        {
          bodyy = 0.25;
        }
      }
      if (arms == 0)
      {
        legvector = vector2(spritePartsPos.x - 2, spritePartsPos.y + 1.9);
        if (map.raycast(legvector, legvector, legdistance, 10))
        {
          arms = 0.25;
        }
      }

      // Legs collison check. If collided then don't check the other side as a possible double
      // CheckMapCollision collision would result in too much of a ground repelling force.
      onground = checkmapcollision(spritePartsPos.x + 2, spritePartsPos.y + 2 - bodyy, 0);

      onground =
        onground || checkmapcollision(spritePartsPos.x - 2, spritePartsPos.y + 2 - arms, 0);

      // radius collison check
      ongroundforlaw = checkradiusmapcollision(spritePartsPos.x, spritePartsPos.y - 1, onground);

      onground = checkmapverticescollision(spritePartsPos.x, spritePartsPos.y, 3,
                                           onground || ongroundforlaw) ||
                 onground;

      // Change the permanent state if the player has had the same OnGround state for two
      // frames in a row
      if ((onground ^ ongroundlastframe) == 0)
      {
        ongroundpermanent = onground;
      }

      ongroundlastframe = onground;

      // If no background poly contact then reset any background poly status
      bgstate.backgroundtestreset();

      // WEAPON HANDLING
#ifndef SERVER
      if ((sprite_system.IsPlayerSprite(num)) || (weapon.fireinterval <= fireinterval_net) or
          !GS::GetGame().pointvisible(spritePartsPos.x, spritePartsPos.y, camerafollowsprite))
#endif
      {
        if ((weapon.fireintervalcount > 0) &&
            ((weapon.ammocount > 0) || (weapon.num == spas12_num)))
        {
          weapon.fireintervalprev = weapon.fireintervalcount;
          weapon.fireintervalcount -= 1;
        }
      }

      // If fire button is released, then the reload can begin
      if (!control.fire)
      {
        canautoreloadspas = true;
      }

      // reload
      if ((weapon.ammocount == 0) &&
          ((weapon.num == chainsaw_num) || ((bodyanimation.id != AnimationType::Roll) &&
                                            (bodyanimation.id != AnimationType::RollBack) &&
                                            (bodyanimation.id != AnimationType::Melee) &&
                                            (bodyanimation.id != AnimationType::Change) &&
                                            (bodyanimation.id != AnimationType::Throw) &&
                                            (bodyanimation.id != AnimationType::ThrowWeapon))))
      {
#ifndef SERVER
        if (reloadsoundchannel > -2)
        {
          setsoundpaused(reloadsoundchannel, false);
        }
#endif

        if (bodyanimation.id != AnimationType::GetUp)
        {
          // spas is unique - it does the fire interval delay AND THEN reloads. all other
          // weapons do the opposite.
          if (weapon.num == spas12_num)
          {
            if ((weapon.fireintervalcount == 0) && canautoreloadspas)
            {
              bodyapplyanimation(AnimationType::Reload, 1);
            }
          }
          else if ((weapon.num == bow_num) || (weapon.num == bow2_num))
          {
            bodyapplyanimation(AnimationType::ReloadBow, 1);
          }
          else if ((bodyanimation.id != AnimationType::ClipIn) &&
                   (bodyanimation.id != AnimationType::SlideBack))
          {
            // Don't show reload animation for chainsaw if one of these
            // animations are already ongoing
            if ((weapon.num != chainsaw_num) || ((bodyanimation.id != AnimationType::Roll) &&
                                                 (bodyanimation.id != AnimationType::RollBack) &&
                                                 (bodyanimation.id != AnimationType::Melee) &&
                                                 (bodyanimation.id != AnimationType::Change) &&
                                                 (bodyanimation.id != AnimationType::Throw) &&
                                                 (bodyanimation.id != AnimationType::ThrowWeapon)))
            {
              bodyapplyanimation(AnimationType::ClipOut, 1);
            }
          }

          burstcount = 0;
        }

#ifndef SERVER
        // play reload sound
        if (weapon.reloadtimecount == weapon.reloadtime)
        {

          if (weapon.num == eagle_num)
          {
            weaponreloadsound = SfxEffect::deserteagle_reload;
          }
          else if (weapon.num == mp5_num)
          {
            weaponreloadsound = SfxEffect::mp5_reload;
          }
          else if (weapon.num == ak74_num)
          {
            weaponreloadsound = SfxEffect::ak74_reload;
          }
          else if (weapon.num == steyraug_num)
          {
            weaponreloadsound = SfxEffect::steyraug_reload;
          }
          else if (weapon.num == ruger77_num)
          {
            weaponreloadsound = SfxEffect::ruger77_reload;
          }
          else if (weapon.num == m79_num)
          {
            weaponreloadsound = SfxEffect::m79_reload;
          }
          else if (weapon.num == barrett_num)
          {
            weaponreloadsound = SfxEffect::barretm82_reload;
          }
          else if (weapon.num == m249_num)
          {
            weaponreloadsound = SfxEffect::m249_reload;
          }
          else if (weapon.num == minigun_num)
          {
            weaponreloadsound = SfxEffect::minigun_reload;
          }
          else if (weapon.num == colt_num)
          {
            weaponreloadsound = SfxEffect::colt1911_reload;
          }
          else
          {
            weaponreloadsound = SfxEffect::INVALID;
          }

          if (weaponreloadsound != SfxEffect::INVALID)
          {
            playsound(weaponreloadsound, spritePartsPos, reloadsoundchannel);
          }
        }

        m3.x = skeleton.pos[15].x;
        m3.y = skeleton.pos[15].y + 6;
        m4.x = spriteVelocity.x;
        m4.y = spriteVelocity.y - 0.001;
        if (weapon.reloadtimecount == weapon.clipouttime)
        {
          if (weapon.num == eagle_num)
          {
            createspark(m3, m4, 18, num, 255);
            m3.x = skeleton.pos[15].x - 2;
            m3.y = skeleton.pos[15].y + 7;
            m4.x = spriteVelocity.x + 0.3;
            m4.y = spriteVelocity.y - 0.003;
            createspark(m3, m4, 18, num, 255);
          }
          else if (weapon.num == mp5_num)
          {
            createspark(m3, m4, 11, num, 255);
          }
          else if (weapon.num == ak74_num)
          {
            createspark(m3, m4, 9, num, 255);
          }
          else if (weapon.num == steyraug_num)
          {
            createspark(m3, m4, 19, num, 255);
          }
          else if (weapon.num == barrett_num)
          {
            createspark(m3, m4, 20, num, 255);
          }
          else if (weapon.num == m249_num)
          {
            createspark(m3, m4, 10, num, 255);
          }
          else if (weapon.num == colt_num)
          {
            createspark(m3, m4, 23, num, 255);
          }
        }
#endif

        if (weapon.num != spas12_num)
        {
          // Spas doesn't use the reload time.
          // If it ever does, be sure to put this back outside.
          weapon.reloadtimeprev = weapon.reloadtimecount;
          if (weapon.reloadtimecount > 0)
          {
            weapon.reloadtimecount -= 1;
          }

          // spas waits for fire interval to hit 0.
          // doing this next line for the spas would cause it to never reload when empty.
          weapon.fireintervalprev = weapon.fireinterval;
          weapon.fireintervalcount = weapon.fireinterval;

          if (weapon.reloadtimecount < 1)
          {
            weapon.reloadtimeprev = weapon.reloadtime;
            weapon.fireintervalprev = weapon.fireinterval;
            weapon.reloadtimecount = weapon.reloadtime;
            weapon.fireintervalcount = weapon.fireinterval;
            weapon.startuptimecount = weapon.startuptime;
            weapon.ammocount = weapon.ammo;
          }
        }
      }

      // weapon jam fix?
      // TODO: check if server or client do stuff wrong here...
      if (weapon.ammocount == 0)
      {
#ifdef SERVER
        if (weapon.reloadtimecount < 1)
        {
          weapon.reloadtimecount = weapon.reloadtime;
          weapon.fireintervalcount = weapon.fireinterval;
          weapon.startuptimecount = weapon.startuptime;
          weapon.ammocount = weapon.ammo;
        }
        if (weapon.reloadtimecount > weapon.reloadtime)
        {
          weapon.reloadtimecount = weapon.reloadtime;
        }
#endif

        if (weapon.num != spas12_num)
        {
          if (weapon.reloadtimecount < 1)
          {
#ifdef SERVER
            bodyapplyanimation(AnimationType::Change, 36);
#endif
            weapon.reloadtimeprev = weapon.reloadtime;
            weapon.fireintervalprev = weapon.fireinterval;
            weapon.reloadtimecount = weapon.reloadtime;
            weapon.fireintervalcount = weapon.fireinterval;
            weapon.startuptimecount = weapon.startuptime;
            weapon.ammocount = weapon.ammo;
          }

#ifndef SERVER
          if (weapon.reloadtimecount > weapon.reloadtime)
          {
            weapon.reloadtimeprev = weapon.reloadtime;
            weapon.reloadtimecount = weapon.reloadtime;
          }

          // didn't we just do this right above? :S
          if (weapon.num != spas12_num)
          {
            if (weapon.reloadtimecount < 1)
            {
              bodyapplyanimation(AnimationType::Change, 36);
              weapon.reloadtimeprev = weapon.reloadtime;
              weapon.fireintervalprev = weapon.fireinterval;
              weapon.reloadtimecount = weapon.reloadtime;
              weapon.fireintervalcount = weapon.fireinterval;
              weapon.startuptimecount = weapon.startuptime;
              weapon.ammocount = weapon.ammo;
            }
          }
#endif
        }
      }

#ifndef SERVER
      // chainsaw smoke
      if ((weapon.num == chainsaw_num) && (stat == 0))
      {
        if (CVar::r_maxsparks > (max_sparks - 10))
        {
          if (GS::GetGame().GetMainTickCounter() % 15 == 0)
          {
            m3 = skeleton.pos[9];
            m3.y = m3.y - 2;
            m3.x = m3.x + direction * 3;
            m4.x = 0;
            m4.y = -0.25;
            createspark(m3, m4, 1, num, 20);
            if (weapon.ammocount == 0)
            {
              playsound(SfxEffect::chainsaw_o, spritePartsPos, gattlingsoundchannel);
            }
            else
            {
              playsound(SfxEffect::chainsaw_m, spritePartsPos, defaultchannel);
            }
          }
        }

        if (control.fire)
        {
          if (weapon.ammocount > 0)
          {
            playsound(SfxEffect::chainsaw_r, spritePartsPos, gattlingsoundchannel);
          }
        }
      }

      // LAW, chansaw smoke
      if ((weapon.num == law_num) || (weapon.num == chainsaw_num))
      {
        if (weapon.ammocount == 0)
        {
          if (CVar::r_maxsparks > (max_sparks - 10))
          {
            if (Random(4) == 0)
            {
              m3 = skeleton.pos[9];
              m3.y = m3.y - 2 - 1 + (float)(Random(60)) / 10;
              m3.x = m3.x + direction * 3 - 8 + (float)(Random(80)) / 10;
              m4.x = 0;
              m4.y = -0.3;
              createspark(m3, m4, 1, num, 20);
            }
          }
        }
      }

      // flame arrow on fire
      if (weapon.num == bow2_num)
      {
        if (Random(10) == 0)
        {
          m3.x = skeleton.pos[15].x + direction * 6;
          m3.y = skeleton.pos[15].y - 5;
          createspark(m3, vector2(0, -0.5), 36, num, 40);
        }
      }
#endif

      // JETS
      if
#ifndef SERVER
        ((clientstopmovingcounter > 0))
#else
        (((player->controlmethod == human) && (noclientupdatetime[num] < clientstopmove_retrys)) ||
         (player->controlmethod == bot))
#endif
      {
        if ((jetscount < map.startjet) && !control.jetpack)
        {
          if (onground || (GS::GetGame().GetMainTickCounter() % 2 == 0))
          {
            jetscount += 1;
          }
        }
      }

      if (ceasefirecounter > -1)
      {
        ceasefirecounter = ceasefirecounter - 1;
        alpha = round(fabs(100 + 70 * sin(GS::GetGame().GetSinusCounter())));
      }
      else
      {
        alpha = 255;
      }

      if (bonusstyle == bonus_predator)
      {
        alpha = predatoralpha;
      }

#ifndef SERVER
      // bleed when BERSERKER
      if (bonusstyle == bonus_berserker)
      {
        m4 = skeleton.pos[19];
        m4.x = m4.x - 5 + Random(11);
        m4.y = m4.y - 5 + Random(11);
        m3 = vec2subtract(skeleton.pos[19], skeleton.oldpos[19]);
        m3.y = m3.y - 1.38;
        rnd = blood_random_high;

        if (CVar::r_maxsparks < (max_sparks - 10))
        {
          rnd = 2 * rnd;
        }

        if (Random(rnd) == 0)
        {
          createspark(m4, m3, 5, num, 55 - Random(20));
        }
      }

      if (bonusstyle == bonus_flamegod)
      {
        m4 = skeleton.pos[19];
        m4.x = m4.x - 5 + Random(11);
        m4.y = m4.y - 5 + Random(11);
        m3 = vec2subtract(skeleton.pos[19], skeleton.oldpos[19]);
        m3.y = m3.y - 1.38;
        rnd = blood_random_high;

        if (CVar::r_maxsparks < (max_sparks - 10))
        {
          rnd = 2 * rnd;
        }

        if (Random(rnd) == 0)
        {
          createspark(m4, m3, 36, num, 40 - Random(10));
        }
      }

      // bleed when hurt
      if (Health < hurt_health)
      {
        m4 = skeleton.pos[5];
        m4.x = m4.x + 2;
        m4.y = m4.y;
        m3 = vec2subtract(skeleton.pos[5], skeleton.oldpos[5]);
        rnd = blood_random_normal;

        if (CVar::r_maxsparks < (max_sparks - 10))
        {
          rnd = 2 * rnd;
        }

        if (Random(rnd) == 0)
        {
          createspark(m4, m3, 4, num, 65 - Random(10));
        }
      }
#endif

      // BONUS time
      if (bonustime > -1)
      {
        bonustime = bonustime - 1;
        if (bonustime < 1)
        {
          switch (bonusstyle)
          {
          case bonus_predator:
            alpha = 255;
            break;
          }
          bonusstyle = bonus_none;
        }
      }
      else
      {
        bonusstyle = bonus_none;
      }

      // MULITKILL TIMER

      if (multikilltime > -1)
      {
        multikilltime = multikilltime - 1;
      }
      else
      {
        multikills = 0;
      }

      // gain health from bow
      if ((GS::GetGame().GetMainTickCounter() % 3 == 0) &&
          ((weapon.num == bow_num) || (weapon.num == bow2_num)) &&
          (Health < (GS::GetGame().GetStarthealth())))
      {
        Health = Health + 1;
      }

#ifndef SERVER
      // smoke
      if (hascigar == 10)
      {
        if (GS::GetGame().GetMainTickCounter() % 160 == 0)
        {
          {
            m3 = skeleton.pos[9];
            m3.y = m3.y - 2;
            m3.x = m3.x + direction * 4;
            m4.x = 0;
            m4.y = -0.75;
            createspark(m3, m4, 31, num, 55);
            if (Random(2) == 0)
            {
              m3 = skeleton.pos[9];
              m3.y = m3.y - 2;
              m3.x = m3.x + direction * 4.1;
              m4.x = 0;
              m4.y = -0.69;
              createspark(m3, m4, 31, num, 55);
              playsound(SfxEffect::smoke, spritePartsPos);
              if (Random(2) == 0)
              {
                m3 = skeleton.pos[9];
                m3.y = m3.y - 2;
                m3.x = m3.x + direction * 3.9;
                m4.x = 0;
                m4.y = -0.81;
                createspark(m3, m4, 31, num, 55);
              }
            }
          }
        }
      }

      // winter breath
      if (map.weather == 3)
      {
        if (CVar::r_maxsparks > (max_sparks - 10))
        {
          if (GS::GetGame().GetMainTickCounter() % 160 == 0)
          {
            {
              m3 = skeleton.pos[9];
              m3.y = m3.y - 2;
              m3.x = m3.x + direction * 4;
              m4.x = 0;
              m4.y = -0.75;
              createspark(m3, m4, 31, num, 55);
            }
          }
        }
      }
#endif

      // parachuter
      para = 0;
      if ((holdedthing > 0) && (holdedthing < max_things + 1))
      {
        if (things[holdedthing].style == object_parachute)
        {
          para = 1;
        }
      }

      if (para == 1)
      {
        auto &spriteForces = sprite_system.GetForces(num);
        spriteForces.y = para_speed;

        auto ceasefire = ceasefirecounter < 1;
        if constexpr(Config::IsClient(M))
        {
          ceasefire = (CVar::sv_survivalmode &&
                       (ceasefirecounter < GS::GetGame().GetCeasefiretime() * 3 - 30)) ||
                      (ceasefirecounter < GS::GetGame().GetCeasefiretime() - 30);
        }
        if (ceasefire)
        {
          if (onground || control.jetpack)
          {
            if ((holdedthing > 0) && (holdedthing < max_things + 1))
            {
              things[holdedthing].holdingsprite = 0;
              things[holdedthing].skeleton.constraintcount -= 1;
              things[holdedthing].timeout = 3 * 60;
              holdedthing = 0;
            }
          }
        }
      }

#ifdef SERVER
      LogTraceG("TSprite.Update 3e");
#endif

      skeleton.doverlettimestepfor(22, 29);
      skeleton.doverlettimestepfor(24, 30);

#ifndef SERVER
      CopyOldSpritePos();
#endif
    }
    break; // 1
    default:
      NotImplemented("generic", "I think this is not used");
      break;
    } // switch
  }

  if (deadmeat)
  {
    if (isnotspectator())
    {
      // physically integrate skeleton particles
      skeleton.doverlettimestep();
      spritePartsPos = skeleton.pos[12];

      CopyOldSpritePos();

      checkskeletonoutofbounds();

      // Respawn Countdown
#ifdef SERVER
      if (respawncounter < 1)
      {
        respawn();
        serverspritesnapshotmajorfloat(num, netw);
      }
#endif

      respawncounter = respawncounter - 1;

#ifndef SERVER
      if (respawncounter < -360)
      {
        respawncounter = 0;
        respawn();
      }
#endif

#ifdef SERVER
      if (CVar::sv_survivalmode)
      {
        if (respawncounter == 1)
        {
          if (!GS::GetGame().GetSurvivalEndRound())
          {
            respawncounter += 2;
          }
          else
          {
            if (respawncounter < 3)
            {
              for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
              {
                if (!sprite.deadmeat)
                {
                  sprite.healthhit(4000, sprite.num, 1, -1, p);
                  sprite.player->deaths -= 1;
                }
              }
            }

            if (CVar::sv_gamemode != gamestyle_htf)
            {
              if ((GS::GetGame().GetTeamFlag(1) > 0) && (GS::GetGame().GetTeamFlag(2) > 0))
              {
                if (!things[GS::GetGame().GetTeamFlag(1)].inbase)
                {
                  things[GS::GetGame().GetTeamFlag(1)].respawn();
                }
                if (!things[GS::GetGame().GetTeamFlag(2)].inbase)
                {
                  things[GS::GetGame().GetTeamFlag(2)].respawn();
                }
              }
            }
          }
        }
      }
#endif

      // parachuter
      para = 0;
      if ((holdedthing > 0) && (holdedthing < max_things + 1))
      {
        if (things[holdedthing].style == object_parachute)
        {
          para = 1;
        }
      }

      if (para == 1)
      {
        skeleton.forces[12].y = 25 * para_speed;
        if (onground)
        {
          if ((holdedthing > 0) && (holdedthing < max_things + 1))
          {
            things[holdedthing].holdingsprite = 0;
            things[holdedthing].skeleton.constraintcount -= 1;
            things[holdedthing].timeout = 3 * 60;
            holdedthing = 0;
          }
        }
      }

      deadtime += 1;
    }
  } // DeadMeat

  // Safety
  if (spriteVelocity.x > max_velocity)
  {
    spriteVelocity.x = max_velocity;
  }
  if (spriteVelocity.x < -max_velocity)
  {
    spriteVelocity.x = -max_velocity;
  }
  if (spriteVelocity.y > max_velocity)
  {
    spriteVelocity.y = max_velocity;
  }
  if (spriteVelocity.y < -max_velocity)
  {
    spriteVelocity.y = -max_velocity;
  }
}

template <Config::Module M>
void Sprite<M>::kill()
{
  auto &sprite_system = SpriteSystem::Get();
  bool left;

  auto &things = GS::GetThingSystem().GetThings();

#ifdef SERVER
  LogTraceG("TSprite.Kill");
#endif

  // Debug('[Sprite] Deactivate ' + IntToStr(Num));
  active = false;
#ifndef SERVER
  muted = false;
#endif

#ifndef SERVER
  stopsound(reloadsoundchannel);
  stopsound(jetssoundchannel);
  stopsound(gattlingsoundchannel);
  stopsound(gattlingsoundchannel2);
#endif

  if (num > 0)
  {
    skeleton.destroy();
    sprite_system.DestroySpritePart(num);
  }

  if ((holdedthing > 0) && (holdedthing < max_things + 1))
  {
    if (things[holdedthing].style < object_ussocom)
    {
      things[holdedthing].holdingsprite = 0;
      holdedthing = 0;
    }
  }

  if (stat > 0)
  {
    things[stat].statictype = false;
    stat = 0;
  }

  if (isnotsolo())
  {
    left = false;
    for (auto &sprite : sprite_system.GetActiveSprites())
    {
      if (isinsameteam(sprite) && (sprite.num != num))
      {
        left = true;
      }
    }

    if (!left)
    {
      GS::GetGame().SetTeamScore(player->team, team_none);
    }
  }

#ifdef SERVER
  if (num > 0)
  {
    noclientupdatetime[num] = 0;
    messagesasecnum[num] = 0;
    floodwarnings[num] = 0;
    pingwarnings[num] = 0;
  }
#endif

  // sort the players frag list
  GS::GetGame().sortplayers();
}

#ifndef SERVER
// TODO move into Sprite
void selectdefaultweapons(std::uint8_t sprite_id)
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  std::int32_t j;
  std::int32_t k;
  SoldatAssert(sprite_system.IsPlayerSprite(sprite_id));

  auto &weaponSystem = GS::GetWeaponSystem();

  i = weaponSystem.CountEnabledPrimaryWeapons();

  auto &weaponsel = GS::GetGame().GetWeaponsel();

  if (i == 1)
  {
    for (j = 1; j <= primary_weapons; j++)
    {
      if (weaponSystem.IsEnabled(j))
      {
        weaponsel[sprite_id][j] = 1;
        limbomenu->button[j - 1].active = true;
        sprite_system.GetSprite(sprite_id).selweapon = j;

        if (limbomenu->active && !sprite_system.GetSprite(sprite_id).deadmeat)
        {
          sprite_system.GetSprite(sprite_id).applyweaponbynum(
            sprite_system.GetSprite(sprite_id).selweapon, 1);
          clientspritesnapshot();
        }
        break;
      }
    }
  }

  k = weaponSystem.CountEnabledSecondaryWeapons();

  if (k == 1)
  {
    for (j = primary_weapons + 1; j <= main_weapons; j++)
    {
      if (weaponSystem.IsEnabled(j))
      {
        weaponsel[sprite_id][j] = 1;
        limbomenu->button[j - 1].active = true;
        sprite_system.GetSprite(sprite_id).player->secwep = j - primary_weapons - 1;

        CVar::cl_player_secwep = (sprite_system.GetSprite(sprite_id).player->secwep);

        if (limbomenu->active && !sprite_system.GetSprite(sprite_id).deadmeat)
        {
          sprite_system.GetSprite(sprite_id).applyweaponbynum(
            GS::GetWeaponSystem().GetGuns()[j].num, 2);
        }
        clientspritesnapshot();
        break;
      }
    }
  }
}

auto deg2rad(float deg) -> float
{
  float result;
  result = deg / (180 / std::numbers::pi);
  return result;
}
#endif

template <Config::Module M>
void Sprite<M>::die(std::int32_t how, std::int32_t who, std::int32_t where, std::int32_t what,
                    tvector2 impact)
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  std::int32_t j;
  float k;
  std::string s;
  tvector2 a;
#ifndef SERVER
  tvector2 b;
#else
  std::string s2;
#endif

  LogTraceG("TSprite.Die");

  if ((who < 1) || (who > max_sprites))
  {
    return;
  }
  if (what > max_bullets)
  {
    return;
  }

  auto &guns = GS::GetWeaponSystem().GetGuns();
  auto &bullet = GS::GetBulletSystem().GetBullets();
  auto &things = GS::GetThingSystem().GetThings();

  if (!deadmeat)
  {
    // bullet time
    if (CVar::sv_bullettime)
    {
      if (GS::GetGame().IsDefaultGoalTicks())
      {
        k = 0;
        for (auto &sprite : sprite_system.GetActiveSprites())
        {
          auto &spritePartsPos = sprite_system.GetSpritePartsPos(sprite.num);
          auto &spritePartsPosWho = sprite_system.GetSpritePartsPos(who);
          if (sprite.active && (sprite.num != who) && (!sprite.player->demoplayer) and
              sprite.isnotspectator())
          {
            if (distance(spritePartsPos, spritePartsPosWho) > bullettime_mindistance)
            {
              k = 1;
            }
          }
        }

        if (k < 1)
        {
          GS::GetGame().togglebullettime(true);
        }
      }
    }

#ifdef SERVER
    if ((CVar::sv_gamemode == gamestyle_inf) || (CVar::sv_gamemode == gamestyle_teammatch) ||
        (CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_htf))
    {
      respawncounter = waverespawncounter + CVar::sv_respawntime_minwave;
    }
    else
    {
      respawncounter = CVar::sv_respawntime;
    }
#endif
    player->deaths += 1;

#ifdef SERVER
    if (what > 0)
    {
      if (((bullet[what].style == bullet_style_arrow) && (CVar::sv_gamemode != gamestyle_rambo)) ||
          ((bullet[what].style == bullet_style_flamearrow) &&
           (CVar::sv_gamemode != gamestyle_rambo)))
      {
        if (!bullet[what].dontcheat)
        {
          kickplayer(sprite_system.GetSprite(who).num, true, kick_cheat, day, "Not allowed weapon");
          return;
        }
      }
    }

    // Anti-Team Killer Protection
    if (CVar::sv_punishtk)
    {
      if (isinsameteam(sprite_system.GetSprite(who)) &&
          !(CVar::sv_gamemode == gamestyle_deathmatch) && !(CVar::sv_gamemode == gamestyle_rambo) &&
          !(player->name == sprite_system.GetSprite(who).player->name))
      {
        sprite_system.GetSprite(who).player->tkwarnings += 1;
        GS::GetMainConsole().console(
          sprite_system.GetSprite(who).player->name + " Team Killed " + player->name +
            " (Warning #" + inttostr(sprite_system.GetSprite(who).player->tkwarnings) + ')',
          game_message_color);
        serversendstringmessage(std::string("TK Warning #") +
                                  (inttostr(sprite_system.GetSprite(who).player->tkwarnings)) +
                                  ". Max Warnings: " + (inttostr(CVar::sv_warnings_tk)),
                                who, 255, msgtype_pub);
        if (sprite_system.GetSprite(who).player->tkwarnings > (CVar::sv_warnings_tk / 2))
        {
          sprite_system.GetSprite(who).vest = 0;
          sprite_system.GetSprite(who).healthhit(200, who, 1, 1, a);
          serversendstringmessage((sprite_system.GetSprite(who).player->name) +
                                    " has been punished for TeamKilling. (" +
                                    (inttostr(sprite_system.GetSprite(who).player->tkwarnings)) +
                                    '/' + (inttostr(CVar::sv_warnings_tk)) + ')',
                                  0, 255, msgtype_pub);
        }
        if (sprite_system.GetSprite(who).player->tkwarnings > (CVar::sv_warnings_tk - 1))
        {
          kickplayer(who, true, kick_console, 3600 * 15, "Team Killing");
        }
      }
    }
#endif

    if (who != num)
    {
      if (CVar::sv_gamemode == gamestyle_deathmatch)
      {
        sprite_system.GetSprite(who).player->kills += 1;

        // mulitkill count
#ifdef SERVER
        sprite_system.GetSprite(who).multikilltime = multikillinterval;
        sprite_system.GetSprite(who).multikills += 1;
#endif
      }
      if (CVar::sv_gamemode == gamestyle_pointmatch)
      {
        // add point for kill
        i = 1;

        // add another point for holding the flag
        if ((sprite_system.GetSprite(who).holdedthing > 0) &&
            (sprite_system.GetSprite(who).holdedthing < max_things + 1))
        {
          if (things[sprite_system.GetSprite(who).holdedthing].style == object_pointmatch_flag)
          {
            i = i * 2;
          }
        }

            // add points for multikill
#ifdef SERVER
        if (sprite_system.GetSprite(who).multikilltime > 0)
        {
          if (sprite_system.GetSprite(who).multikills == 2)
          {
            i = i * 2;
          }
          if (sprite_system.GetSprite(who).multikills == 3)
          {
            i = i * 4;
          }
          if (sprite_system.GetSprite(who).multikills == 4)
          {
            i = i * 8;
          }
          if (sprite_system.GetSprite(who).multikills == 5)
          {
            i = i * 16;
          }
          if (sprite_system.GetSprite(who).multikills > 5)
          {
            i = i * 32;
          }
        }
#endif

        sprite_system.GetSprite(who).player->kills += i;

        // mulitkill count
#ifdef SERVER
        sprite_system.GetSprite(who).multikilltime = multikillinterval;
        sprite_system.GetSprite(who).multikills += 1;
#endif
      }
      if (CVar::sv_gamemode == gamestyle_teammatch)
      {
        if (isnotinsameteam(sprite_system.GetSprite(who)))
        {
          sprite_system.GetSprite(who).player->kills += 1;
          auto t = sprite_system.GetSprite(who).player->team;
          GS::GetGame().SetTeamScore(t, GS::GetGame().GetTeamScore(t) + 1);
#ifdef SERVER
          // mulitkill count
          sprite_system.GetSprite(who).multikilltime = multikillinterval;
          sprite_system.GetSprite(who).multikills += 1;
#endif
        }
      }
      if (CVar::sv_gamemode == gamestyle_ctf)
      {
        if (isnotinsameteam(sprite_system.GetSprite(who)))
        {
          sprite_system.GetSprite(who).player->kills += 1;
#ifdef SERVER
          // mulitkill count
          sprite_system.GetSprite(who).multikilltime = multikillinterval;
          sprite_system.GetSprite(who).multikills += 1;
#endif
        }
      }
      if (CVar::sv_gamemode == gamestyle_inf)
      {
        if (isnotinsameteam(sprite_system.GetSprite(who)))
        {
          sprite_system.GetSprite(who).player->kills += 1;
#ifdef SERVER
          // mulitkill count
          sprite_system.GetSprite(who).multikilltime = multikillinterval;
          sprite_system.GetSprite(who).multikills += 1;
#endif
        }
      }
      if (CVar::sv_gamemode == gamestyle_htf)
      {
        if (isnotinsameteam(sprite_system.GetSprite(who)))
        {
          sprite_system.GetSprite(who).player->kills += 1;
#ifdef SERVER
          // mulitkill count
          sprite_system.GetSprite(who).multikilltime = multikillinterval;
          sprite_system.GetSprite(who).multikills += 1;
#endif
        }
      }
      if (CVar::sv_gamemode == gamestyle_rambo)
      {
        if (what > 0)
        {
          i = bullet[what].ownerweapon;
        }
        else
        {
          i = -1;
        }
        if ((i == bow_num) ||                             // Shooter is Rambo
            (i == bow2_num) || (weapon.num == bow_num) || // Shootee is Rambo
            (weapon.num == bow2_num))
        {
          sprite_system.GetSprite(who).player->kills += 1;
#ifdef SERVER
          // mulitkill count
          sprite_system.GetSprite(who).multikilltime = multikillinterval;
          sprite_system.GetSprite(who).multikills += 1;
#endif
        }
        else
        {
          // Punish for killing non-Rambos when someone is Rambo
          for (i = 1; i <= max_players; i++)
          {
            if ((sprite_system.GetSprite(i).weapon.num == bow_num) ||
                (sprite_system.GetSprite(i).weapon.num == bow2_num))
            {
              if (sprite_system.GetSprite(who).player->kills > 0)
              {
                sprite_system.GetSprite(who).player->kills -= 1;
                break;
              }
            }
          }
        }
      }
    }

    if (idlerandom == 7)
    {
      if (weapon.num == noweapon_num)
      {
        how = brutal_death;
      }
    }

    bodyanimation.currframe = 0;

    // console message about kill
    // game log
    if (what > 0)
    {
      if constexpr (Config::IsServer(M))
      {
        auto &b = bullet[what];
        s = weaponnamebynum(b.ownerweapon, guns);
        if (b.ownerweapon == 0)
        {
          s = "USSOCOM";
        }
        if (b.style == bullet_style_fragnade)
        {
          s = "Grenade";
        }
        if (b.style == bullet_style_cluster)
        {
          s = "Cluster Grenades";
        }
        if (b.style == bullet_style_punch)
        {
          s = guns[noweapon].name;
        }
        if (b.style == bullet_style_m2)
        {
          s = "Stationary gun";
        }
      }
      else
      {
        // if Bullet[What].OwnerWeapon = noweapon_num then S := 'Selfkill';
        s = weaponnamebynum(what, guns);
        if (what == 222)
        {
          s = "Grenade";
        }
        if (what == 210)
        {
          s = "Clusters";
        }
        if (what == 211)
        {
          s = guns[knife].name;
        }
        if (what == 212)
        {
          s = guns[chainsaw].name;
        }
        if (what == 224)
        {
          s = guns[law].name;
        }
        if (what == 225)
        {
          s = "Stationary gun";
        }
        if (what == 205)
        {
          s = guns[flamer].name;
        }
        if (what == 207)
        {
          s = guns[bow].name;
        }
        if (what == 208)
        {
          s = guns[bow2].name;
        }
        if (what == 206)
        {
          s = guns[noweapon].name;
        }
        if (what == 250)
        {
          s = "Selfkill";
        }
      }
    }
    else
    {
      s = "Selfkill";
    }

#ifndef SERVER
    if ((who != num) && (sprite_system.IsPlayerSprite(who)))
    {
      for (auto &w : wepstats)
      {
        if (w.name == s)
        {
          w.kills = w.kills + 1;
          if (where == 12) /*head*/
          {
            w.headshots = w.headshots + 1;
          }
        }
      }
    }
#endif

#ifdef SERVER
    // console message for kills
    if ((CVar::sv_echokills) && !(sprite_system.GetSprite(who).player->name == player->name))
    {
      GS::GetMainConsole().console(std::string("(") +
                                     inttostr(sprite_system.GetSprite(who).player->team) + ") " +
                                     sprite_system.GetSprite(who).player->name + " killed (" +
                                     inttostr(player->team) + ") " + player->name + " with " + s,
                                   0);
    }
#endif

#ifdef SCRIPT
    // COMMENT: SpriteSystem::Get().GetSprite(Num).Num = Num?
    scrptdispatcher.onplayerkill(SpriteSystem::Get().GetSprite(who).num, num, (std::uint8_t)(what));
#endif

    // console message about kill
    // game log
#ifdef SERVER
    {
      if (CVar::log_enable)
      {
        NotImplemented("No date");
#if 0
                s2 = formatdatetime("yy/mm/dd", get_date());
                s2 = s2 + ' ' + formatdatetime("hh:nn:ss", get_time());
#endif
        GS::GetKillLogFile().Log(std::string("--- ") + s2, false);
        GS::GetKillLogFile().Log(sprite_system.GetSprite(who).player->name, false);
        GS::GetKillLogFile().Log(player->name, false);
        GS::GetKillLogFile().Log(s, false);
      }

      // Bot Chat
      if (CVar::bots_chat)
      {
        if (player->controlmethod == bot)
        {
          if (Random(brain.chatfreq / 2) == 0)
          {
            serversendstringmessage((brain.chatdead), all_players, num, msgtype_pub);
          }
        }
        if ((who != num) && (sprite_system.GetSprite(who).player->controlmethod == bot))
        {
          if (Random(sprite_system.GetSprite(who).brain.chatfreq / 3) == 0)
          {
            serversendstringmessage((sprite_system.GetSprite(who).brain.chatkill), all_players, who,
                                    msgtype_pub);
          }
        }
      }

      k = weapon.hitmultiply;

      lastweaponhm = weapon.hitmultiply;
      lastweaponstyle = weapon.bulletstyle;
      lastweaponspeed = weapon.speed;
      lastweaponfire = weapon.fireinterval;
      lastweaponreload = weapon.reloadtime;

      i = dropweapon();
      weapon.hitmultiply = k;

      if ((i > 0) && (weapon.num != flamer_num) && (weapon.num != noweapon_num))
      {
        things[i].skeleton.forces[2] = impact;
      }

      freecontrols();
    }
#endif // SERVER

#ifndef SERVER
    if (((sprite_system.IsPlayerSprite(who)) || (sprite_system.IsPlayerSprite(num))) &&
        (what > 0) && GS::GetGame().ispointonscreen(skeleton.pos[9]))
    {
      if ((sprite_system.IsPlayerSprite(who)) && (sprite_system.IsPlayerSprite(num)))
      {
        ;
      }
      else
      {
        screencounter = 5;
        capscreen = 4;
      }
    }
#endif
  }

#ifdef SERVER
  if (what > 0)
  {
    extern float shotdistanceServer;
    extern float shotlifeServer;
    extern float shotricochetServer;
    auto &b = bullet[what];
    if (((what == 1) && (where == 1)) || (b.style == bullet_style_flame) || deadmeat)
    {
      ;
    }
    else
    {
      a = vec2subtract(GetBulletParts().pos[what], b.initial);
      shotdistanceServer = (float)(vec2length(a)) / 14;
      shotricochetServer = b.ricochetcount;
      shotlifeServer = (float)((GS::GetGame().GetMainTickCounter() - b.startuptime)) / 60;
    }
  }

  LogTraceG("TSprite.Die 2");
#endif

#ifndef SERVER
  if (what > 0)
  {
    if ((where == 12) && ((bullet[what].ownerweapon == ruger77_num)))
    {
      how = headchop_death;
    }
  }
#endif
  switch (how)
  {
  case normal_death: {
#ifndef SERVER
    auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);
    // the sound of death...
    if (!deadmeat)
    {
      playsound(SfxEffect::death + Random(3), spritePartsPos);
    }
#endif
  }
  break;

  case headchop_death: {
#ifndef SERVER
    if (deadmeat)
#endif
    {
      if (where == 12)
      {
        skeleton.constraints[20].active = false;
      }
      if (where == 3)
      {
        skeleton.constraints[2].active = false;
      }
      if (where == 4)
      {
        skeleton.constraints[4].active = false;
      }
    }

#ifndef SERVER
    if (what > 0)
    {
      if (!deadmeat && (where == 12) &&
          ((bullet[what].ownerweapon == barrett_num) || (bullet[what].ownerweapon == ruger77_num)))
      {
        Randomize();
        if (Random(100) > 50)
        {
          skeleton.constraints[20].active = true; // Keep head attached to corpse
          for (i = 0; i <= 50; i++)
          {
            a.x = skeleton.pos[9].x + (cos(deg2rad((float)(360) / 50 * i)) * 2);
            a.y = skeleton.pos[9].y + (sin(deg2rad((float)(360) / 50 * i)) * 2);
            Randomize();
            // FIXME: Causes range check error
            // RandSeed := RandSeed * i;
            b.x = (cos(deg2rad((float)(360) / 50 * i)) * randomrange(1, 3));
            b.y = (sin(deg2rad((float)(360) / 50 * i)) * randomrange(1, 3));
            createspark(a, b, iif(i < 25, randomrange(4, 5), 5), num, 100 - Random(20));
          }
        }

        if (bullet[what].ownerweapon == barrett_num)
        {
          // corpse explode
          playsound(SfxEffect::bryzg, skeleton.pos[12]);
        }

        if (sprite_system.IsPlayerSprite(who))
        {
          playsound(SfxEffect::boomheadshot);
        }
      }
    }

    // siup leb!
    if (!deadmeat)
    {
      playsound(SfxEffect::headchop, skeleton.pos[12]);
    }
#endif
  }
  break;

  case brutal_death: {
#ifndef SERVER
    if (deadmeat)
#endif
    {
      skeleton.constraints[2].active = false;
      skeleton.constraints[4].active = false;
      skeleton.constraints[20].active = false;
      skeleton.constraints[21].active = false;
      skeleton.constraints[23].active = false;
    }

#ifndef SERVER
    // play bryzg sound!
    playsound(SfxEffect::bryzg, skeleton.pos[12]);
#endif
  }
  break;
  } // case

#ifndef SERVER
  if (deadmeat)
#endif
  {
    if (sprite_system.GetSprite(who).bonusstyle == bonus_berserker)
    {
      skeleton.constraints[2].active = false;
      skeleton.constraints[4].active = false;
      skeleton.constraints[20].active = false;
      skeleton.constraints[21].active = false;
      skeleton.constraints[23].active = false;

#ifndef SERVER
      playsound(SfxEffect::killberserk, skeleton.pos[12]);
#endif
    }
  }

#ifndef SERVER
  if (!deadmeat && (what > 0))
  {
    if (bullet[what].ownerweapon == flamer_num)
    {
      playsound(SfxEffect::burn, skeleton.pos[12]);
    }
  }
#endif

  if (!deadmeat && (hascigar == 10))
  {
#ifndef SERVER
    createspark(skeleton.pos[12], impact, 34, num, 245);
#endif
    hascigar = 0;
  }

  // Survival Mode
  if (CVar::sv_survivalmode)
  {
    if (!deadmeat)
    {
      if ((CVar::sv_gamemode == gamestyle_deathmatch) || (CVar::sv_gamemode == gamestyle_rambo))
      {
        auto alivenum = 0;

        for (auto &sprite : sprite_system.GetActiveSprites())
        {
          if (!sprite.deadmeat && sprite.isnotspectator())
          {
            alivenum += 1;
          }
        }

        alivenum -= 1;
        GS::GetGame().SetAlivenum(alivenum);

        if (alivenum < 2)
        {
          for (auto &sprite : sprite_system.GetActiveSprites())
          {
            sprite.respawncounter = survival_respawntime;
          }

          GS::GetGame().SetSurvivalendround(true);

#ifndef SERVER
          for (auto &sprite : sprite_system.GetActiveSprites())
          {
            if (!sprite.deadmeat and (num != sprite.num) && // not the current player
                sprite.isnotspectator())
            {
              auto &spritePartsPos = sprite_system.GetSpritePartsPos(sprite.num);
              playsound(SfxEffect::roar, spritePartsPos);
            }
          }
#endif
        }

        {
          auto show_log = true;
          if constexpr(!Config::IsServer(M))
          {
            show_log = !TSpriteSystem<Sprite<M>>::Get().GetPlayerSprite().deadmeat;
          }

          if (show_log)
          {
            GS::GetMainConsole().console(
              _("Players left:") + ' ' +
                (inttostr(alivenum)),
              game_message_color);
          }
        }
      }

      if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf) ||
          (CVar::sv_gamemode == gamestyle_htf) || (CVar::sv_gamemode == gamestyle_teammatch))
      {

        GS::GetGame().CalculateTeamAliveNum(player->team);
#ifndef SERVER
        if (sprite_system.IsPlayerSpriteValid())
        {
          if (isinsameteam(sprite_system.GetPlayerSprite()))
          {
            if (!sprite_system.GetPlayerSprite().deadmeat)
            {
              GS::GetMainConsole().console(_("Players left on your team:") + ' ' +
                                             (inttostr(GS::GetGame().GetTeamAliveNum(
                                               sprite_system.GetPlayerSprite().player->team))),
                                           game_message_color);
            }
          }
        }
#endif
      }
    }
  }

#ifdef SERVER
  // Fire on from bullet
  if (what > 0)
  {
    if (bullet[what].style == bullet_style_fragnade)
    {
      if (Random(12) == 0)
      {
        onfire = 4;
      }
    }

    if (bullet[what].style == bullet_style_m79)
    {
      if (Random(8) == 0)
      {
        onfire = 2;
      }
    }

    if (bullet[what].style == bullet_style_flame)
    {
      onfire = 1;
    }

    if (bullet[what].style == bullet_style_flamearrow)
    {
      if (Random(4) == 0)
      {
        onfire = 1;
      }
    }

    if (bullet[what].style == bullet_style_cluster)
    {
      if (Random(3) == 0)
      {
        onfire = 3;
      }
    }
  }
#endif

  for (i = 1; i <= max_things; i++)
  {
    if (things[i].holdingsprite == num)
    {
      if (things[i].style < object_ussocom)
      {
        things[i].holdingsprite = 0;
        holdedthing = 0;
#ifndef SERVER
        if ((things[i].style == object_alpha_flag) || (things[i].style == object_bravo_flag))
        {
          GS::GetMainConsole().console(
            wideformat(_("{} dropped the {} Flag"), player->name,
                       iif(player->team == team_alpha, _("Blue"), _("Red"))),
            iif(player->team == team_alpha, bravo_message_color, alpha_message_color));

          if (isinsameteam(sprite_system.GetPlayerSprite()))
          {
            if (things[i].style == object_alpha_flag)
            { // Alpha
              bigmessage(wideformat(_("{} Flag dropped!"), _("Red")), capturemessagewait,
                         capture_message_color);
            }
            else if (things[i].style == object_bravo_flag)
            { // Bravo
              bigmessage(wideformat(_("{} Flag dropped!"), _("Blue")), capturemessagewait,
                         capture_message_color);
            }

            playsound(SfxEffect::infilt_point);
          }
        }
#endif
#ifdef SCRIPT
        scrptdispatcher.onflagdrop(num, things[i].style, false);
#endif
      }
    }

    if (things[i].owner == num)
    {
      things[i].owner = 255;
    }

    if (stat == num)
    {
      stat = 0;
      things[i].statictype = true;
    }
  }

  // send net info, so the death is smooth
#ifdef SERVER
  if (!deadmeat)
  {
    serverspritedeath(num, who, what, where);
  }

  LogTraceG("TSprite.Die 3");
#endif

#ifndef SERVER
  stopsound(reloadsoundchannel);
#endif
  auto &weaponsel = GS::GetGame().GetWeaponsel();

  // BREAD
  bool bread = false;
  if constexpr (Config::IsServer(M))
  {
    bread = !CVar::sv_advancemode;
  }
  else
  {
    bread = CVar::sv_advancemode;
  }
  if (bread)
  {
    auto &weaponSystem = GlobalSystems<M>::GetWeaponSystem();
    
#ifdef SERVER
    if (!deadmeat && (num != who))
#else
    if (!deadmeat)
#endif
    {
      i = CVar::sv_advancemode_amount;

#ifndef SERVER
      if ((num != who) && (isnotinsameteam(sprite_system.GetSprite(who)) or issolo()))
#endif
      {
        if ((sprite_system.GetSprite(who).player->kills % i) == 0)
        {
          j = 0;
          for (i = 1; i <= primary_weapons; i++)
          {
            if ((weaponsel[who][i] == 0) && (weaponSystem.IsEnabled((i))))
            {
              j = 1;
            }
          }

          if (j == 1)
          {
            do
            {
              j = Random(primary_weapons) + 1;
            } while ((weaponsel[who][j] != 0) || !(weaponSystem.IsEnabled(j)));
            weaponsel[who][j] = 1;
          }
        }
      }

      i = CVar::sv_advancemode_amount;

      if ((player->deaths % i) == 0)
      {
        j = 0;
        for (i = 1; i <= primary_weapons; i++)
        {
          if (weaponsel[num][i] == 1)
          {
            j = 1;
          }
        }

        if (j == 1)
        {
          do
          {
            j = Random(primary_weapons) + 1;
          } while (!(weaponsel[num][j] == 1));
          weaponsel[num][j] = 0;
        }
      }

#ifndef SERVER
      if ((sprite_system.IsPlayerSprite(num)) || (sprite_system.IsPlayerSprite(who)))
      {
        for (i = 1; i <= primary_weapons; i++)
        {
          if (weaponSystem.IsEnabled(i))
          {
            limbomenu->button[i - 1].active = (bool)(weaponsel[mysprite][i]);
          }
        }
      }
#endif
    }
  }

  deadmeat = true;
#ifdef SERVER
  holdedthing = 0;
#endif
  alpha = 255;
  vest = 0;
  bonusstyle = bonus_none;
  bonustime = 0;
  if ((deadtime > 0) && (onfire == 0))
  {
    deadtime = deadtime / 2;
  }
  else
  {
    deadtime = 0;
  }

  auto &spriteVelocity = sprite_system.GetVelocity(num);

  spriteVelocity.x = 0;
  spriteVelocity.y = 0;
  sprite_system.GetSprite(who).brain.pissedoff = 0;

  // sort the players frag list
  GS::GetGame().sortplayers();
}

template <Config::Module M>
auto Sprite<M>::dropweapon() -> std::int32_t
{
  std::int32_t result;
  result = -1;

#ifdef SERVER
  LogTraceG("TSprite.DropWeapon");
#endif

  weaponscleaned = false;
  // drop weapon
#ifdef SERVER
  if (weapon.num == colt_num)
  {
    result = creatething(skeleton.pos[16], num, object_ussocom, 255);
  }
  else if (weapon.num == eagle_num)
  {
    result = creatething(skeleton.pos[16], num, object_desert_eagle, 255);
  }
  else if (weapon.num == mp5_num)
  {
    result = creatething(skeleton.pos[16], num, object_hk_mp5, 255);
  }
  else if (weapon.num == ak74_num)
  {
    result = creatething(skeleton.pos[16], num, object_ak74, 255);
  }
  else if (weapon.num == steyraug_num)
  {
    result = creatething(skeleton.pos[16], num, object_steyr_aug, 255);
  }
  else if (weapon.num == spas12_num)
  {
    result = creatething(skeleton.pos[16], num, object_spas12, 255);
  }
  else if (weapon.num == ruger77_num)
  {
    result = creatething(skeleton.pos[16], num, object_ruger77, 255);
  }
  else if (weapon.num == m79_num)
  {
    result = creatething(skeleton.pos[16], num, object_m79, 255);
  }
  else if (weapon.num == barrett_num)
  {
    result = creatething(skeleton.pos[16], num, object_barret_m82a1, 255);
  }
  else if (weapon.num == m249_num)
  {
    result = creatething(skeleton.pos[16], num, object_minimi, 255);
  }
  else if (weapon.num == minigun_num)
  {
    result = creatething(skeleton.pos[16], num, object_minigun, 255);
  }
  else if (weapon.num == knife_num)
  {
    result = creatething(skeleton.pos[16], num, object_combat_knife, 255);
  }
  else if (weapon.num == chainsaw_num)
  {
    result = creatething(skeleton.pos[16], num, object_chainsaw, 255);
  }
  else if (weapon.num == law_num)
  {
    result = creatething(skeleton.pos[16], num, object_law, 255);
  }

  if (CVar::sv_gamemode == gamestyle_rambo)
  {
    if ((weapon.num == bow_num) || (weapon.num == bow2_num))
    {
      result = creatething(skeleton.pos[16], num, object_rambo_bow, 255);
    }
  }

  auto &things = GS::GetThingSystem().GetThings();
  if (result > 0)
  {
    things[result].ammocount = weapon.ammocount;
  }

    // This should be called before weapon is actually applied
    // so that sprite still holds old values
#ifdef SCRIPT
  if (result > 0)
  {
    // event must be before actual weapon apply.
    // script might've called ForceWeapon, which we should check.
    // if it did, we don't apply snapshot weapon's as they were already applied
    // by force weapon.
    forceweaponcalled = false;
    scrptdispatcher.onweaponchange(num, noweapon_num, secondaryweapon.num, guns[noweapon].ammocount,
                                   secondaryweapon.ammocount);
  }

  if (!forceweaponcalled)
#endif
    applyweaponbynum(noweapon_num, 1);
#endif
  return result;
}

template <Config::Module M>
void Sprite<M>::legsapplyanimation(const AnimationType anim, std::int32_t curr)
{
  LogTraceG("TSprite.LegsApplyAnimation");

  if ((legsanimation.id == AnimationType::Prone) || (legsanimation.id == AnimationType::ProneMove))
  {
    return;
  }

  if (anim != legsanimation.id)
  {
    legsanimation = AnimationSystem::Get().GetAnimation(anim);
    legsanimation.currframe = curr;
  }
}

template <Config::Module M>
void Sprite<M>::bodyapplyanimation(const AnimationType anim, std::int32_t curr)
{
  ZoneScopedN("ApplybodyAnimation");
  LogTraceG("TSprite.BodyApplyAnimation");

#ifndef SERVER
  if (anim == AnimationType::Stand)
  {
    if (wasreloading)
    {
      bodyapplyanimation(AnimationType::Reload, 1);
      wasreloading = false;
      return;
    }
  }
#endif

  if (anim != bodyanimation.id)
  {
    bodyanimation = AnimationSystem::Get().GetAnimation(anim);
    bodyanimation.currframe = curr;
  }
}

template <Config::Module M>
void Sprite<M>::moveskeleton(float x1, float y1, bool fromzero)
{
  std::int32_t i;

  ZoneScopedN("Sprite::MoveSkeleton");
  if (!fromzero)
  {
    for (i = 1; i <= num_particles; i++)
    {
      if (skeleton.active[i])
      {
        skeleton.pos[i].x = skeleton.pos[i].x + x1;
        skeleton.pos[i].y = skeleton.pos[i].y + y1;
        skeleton.oldpos[i] = skeleton.pos[i];
      }
    }
  }

  if (fromzero)
  {
    for (i = 1; i <= num_particles; i++)
    {
      if (skeleton.active[i])
      {
        skeleton.pos[i].x = x1;
        skeleton.pos[i].y = y1;
        skeleton.oldpos[i] = skeleton.pos[i];
      }
    }
  }
}

template <Config::Module M>
auto Sprite<M>::checkradiusmapcollision(float x, float y, bool hascollided) -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t k;
  std::int32_t z;
  std::int32_t b = 0;
  tvector2 spos;
  tvector2 pos;
  tvector2 perp;
  tvector2 step;
  tvector2 norm;
  tvector2 p1;
  tvector2 p2;
  tvector2 p3;
  float d = 0.0;
  std::int32_t detacc;
  bool teamcol;

  LogTraceG("TSprite.CheckRadiusMapCollision");
  auto &map = GS::GetGame().GetMap();

  bool result = false;
  spos.x = x;
  spos.y = y - 3;

  auto &spriteVelocity = sprite_system.GetVelocity(num);
  auto &spriteForces = sprite_system.GetForces(num);

  // make step
  detacc = trunc(vec2length(spriteVelocity));
  if (detacc == 0)
  {
    detacc = 1;
  }
  vec2scale(step, spriteVelocity, (float)(1) / detacc);

  // make steps for accurate collision detection
  for (z = 0; z <= detacc - 1; z++)
  {
    spos.x = spos.x + step.x;
    spos.y = spos.y + step.y;

    // iterate through maps sector polygons
    const auto sector = map.GetSector(spos);
    if (sector.IsValid())
    {
      for (const auto &w : sector.GetPolys())
      {
        const auto polytype = w.Type;

        teamcol = teamcollides(polytype, player->team, false);

        if (((holdedthing == 0) && (polytype == poly_type_only_flaggers)) ||
            ((holdedthing != 0) && (polytype == poly_type_not_flaggers)))
        {
          teamcol = false;
        }
        if (teamcol && (polytype != poly_type_doesnt) && (polytype != poly_type_only_bullets))
        {
          for (k = 1; k <= 3; k++)
          {
            norm = map.perp[w.Index][k];
            vec2scale(norm, norm, -sprite_col_radius);

            pos = vec2add(spos, norm);

            if (map.pointinpolyedges(pos.x, pos.y, w.Index))
            {
              if (bgstate.backgroundtest(w))
              {
                continue;
              }

              if (!hascollided)
              {
                handlespecialpolytypes(polytype, pos);
              }

              perp = map.closestperpendicular(w.Index, spos, d, b);

              const auto &poly = map.polys[w.Index];

              switch (b)
              {
              case 1: {
                p1.x = poly.vertices[0].x;
                p1.y = poly.vertices[0].y;
                p2.x = poly.vertices[1].x;
                p2.y = poly.vertices[1].y;
              }
              break;
              case 2: {
                p1.x = poly.vertices[1].x;
                p1.y = poly.vertices[1].y;
                p2.x = poly.vertices[2].x;
                p2.y = poly.vertices[2].y;
              }
              break;
              case 3: {
                p1.x = poly.vertices[2].x;
                p1.y = poly.vertices[2].y;
                p2.x = poly.vertices[0].x;
                p2.y = poly.vertices[0].y;
              }
              break;
              }

              p3 = pos;
              d = pointlinedistance(p1, p2, p3);
              vec2scale(perp, perp, d);

              auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);
              spritePartsPos = sprite_system.GetSpritePartsOldPos(num);
              spriteVelocity = vec2subtract(spriteForces, perp);

              result = true;
              return result;
            } // PointInPolyEdges
          }
        }
      } // for j
    }
  } // n
  return result;
}

template <Config::Module M>
auto Sprite<M>::checkmapcollision(float x, float y, std::int32_t area) -> bool
{
  ZoneScopedN("CheckMapCollision");
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t k = 0;
  tvector2 spos;
  tvector2 pos;
  tvector2 perp;
  tvector2 step;
  float d = 0.0;
  bool teamcol;

#ifdef SERVER
  LogTraceG("TSprite.CheckMapCollision");
#endif
  auto &map = GS::GetGame().GetMap();

  bool result = false;
  spos.x = x;
  spos.y = y;

  auto &spriteVelocity = sprite_system.GetVelocity(num);
  auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);
  auto &spriteForces = sprite_system.GetForces(num);

  pos.x = spos.x + spriteVelocity.x;
  pos.y = spos.y + spriteVelocity.y;

  // iterate through maps sector polygons
  const auto sector = map.GetSector(pos);
  if (sector.IsValid())
  {
    bgstate.backgroundtestbigpolycenter(pos);

    for (const auto &w : sector.GetPolys())
    {
      const auto &polytype = w.Type;

      teamcol = teamcollides(polytype, player->team, false);

      if (((polytype != poly_type_doesnt) && (polytype != poly_type_only_bullets) && teamcol &&
           (polytype != poly_type_only_flaggers) && (polytype != poly_type_not_flaggers)) ||
          ((holdedthing != 0) && (polytype == poly_type_only_flaggers)) ||
          ((holdedthing == 0) && (polytype == poly_type_not_flaggers)))
      {
        if (Polymap::PointInPoly(pos, w))
        {
          if (bgstate.backgroundtest(w))
          {
            continue;
          }

#ifdef SERVER
          player->standingpolytype = polytype;
#endif

          handlespecialpolytypes(polytype, pos);

#ifndef SERVER
          if ((fabs(spriteVelocity.y) > 2.2) && (fabs(spriteVelocity.y) < 3.4) &&
              (polytype != poly_type_bouncy))
          {
            playsound(SfxEffect::fall, spritePartsPos);
          }
#endif

          if (fabs(spriteVelocity.y) > 3.5)
          {
#ifndef SERVER
            playsound(SfxEffect::fall_hard, spritePartsPos);
#endif

            // Hit ground
            if (CVar::sv_realisticmode)
            {
              if ((spriteVelocity.y > 3.5) && (polytype != poly_type_bouncy))
              {
                healthhit(spriteVelocity.y * 5, num, 12, -1, spos);
#ifndef SERVER
                playsound(SfxEffect::fall, spritePartsPos);
#endif
              }
            }
          }

#ifndef SERVER
          // Run
          if (((legsanimation.id == AnimationType::Run) ||
               (legsanimation.id == AnimationType::RunBack)) &&
              ((legsanimation.currframe == 16) || (legsanimation.currframe == 32)))
          {
            if (CVar::r_maxsparks > (max_sparks - 10))
            {
              if (fabs(spriteVelocity.x) > 1.0)
              {
                spos.x = (float)(spriteVelocity.x) / 4;
                spos.y = -0.8;
                vec2scale(spos, spos, 0.4 + (float)(Random(4)) / 10);
                createspark(pos, spos, 1, num, 70);
              }
            }

            if (CVar::r_maxsparks > (max_sparks - 10))
            {
              if ((((direction == 1) && (spriteVelocity.x < 0.01)) ||
                   ((direction == -1) && (spriteVelocity.x > 0.01))) &&
                  (legsanimation.id == AnimationType::Run))
              {
                spos.x = (float)(spriteVelocity.x) / 4;
                spos.y = -1.3;
                vec2scale(spos, spos, 0.4 + (float)(Random(4)) / 10);
                createspark(pos, spos, 1, num, 70);
              }
            }

            if (map.steps == 0)
            {
              playsound(SfxEffect::step + Random(4), spritePartsPos);
            }
            if (map.steps == 1)
            {
              playsound(SfxEffect::step5 + Random(4), spritePartsPos);
            }

            if (map.weather == 1)
            {
              playsound(SfxEffect::water_step, spritePartsPos);
            }
          }

          // Crouch
          if (((legsanimation.id == AnimationType::CrouchRun) ||
               (legsanimation.id == AnimationType::CrouchRunBack)) &&
              ((legsanimation.currframe == 15) || (legsanimation.currframe == 1)) &&
              (legsanimation.count == 1))
          {
            if (Random(2) == 0)
            {
              playsound(SfxEffect::crouch_move, spritePartsPos);
            }
            else if (Random(2) == 0)
            {
              playsound(SfxEffect::crouch_movel, spritePartsPos);
            }
          }

          // Prone
          if ((legsanimation.id == AnimationType::ProneMove) && (legsanimation.currframe == 8) &&
              (legsanimation.count == 1))
          {
            playsound(SfxEffect::prone_move, spritePartsPos);
          }

          if ((fabs(spriteVelocity.x) > 2.4) && (legsanimation.id != AnimationType::Run) &&
              (legsanimation.id != AnimationType::RunBack) && (Random(4) == 0))
          {
            spos.x = (float)(spriteVelocity.x) / 4;
            spos.y = -0.9;
            vec2scale(spos, spos, 0.4 + (float)(Random(4)) / 10);
            createspark(pos, spos, 1, num, 70);
          }
#endif

          perp = map.closestperpendicular(w.Index, pos, d, k);
          step = perp;

          vec2normalize(perp, perp);
          vec2scale(perp, perp, d);

          d = vec2length(spriteVelocity);
          if (vec2length(perp) > d)
          {
            vec2normalize(perp, perp);
            vec2scale(perp, perp, d);
          }

          if ((area == 0) ||
              ((area == 1) && ((spriteVelocity.y < 0) || (spriteVelocity.x > slidelimit) ||
                               (spriteVelocity.x < -slidelimit))))
          {
            sprite_system.SetSpritePartsOldPos(num, spritePartsPos);
            spritePartsPos = vec2subtract(spritePartsPos, perp);
            if (polytype == poly_type_bouncy) // bouncy polygon
            {
              vec2normalize(perp, perp);
              vec2scale(perp, perp, map.bounciness[w.Index] * d);
#ifndef SERVER
              if (vec2length(perp) > 1)
              {
                playsound(SfxEffect::bounce, spritePartsPos);
              }
#endif
            }
            spriteVelocity = vec2subtract(spriteVelocity, perp);
          }

          if (area == 0)
          {
            if ((legsanimation.id == AnimationType::Stand) ||
                (legsanimation.id == AnimationType::Crouch) ||
                (legsanimation.id == AnimationType::Prone) ||
                (legsanimation.id == AnimationType::ProneMove) ||
                (legsanimation.id == AnimationType::GetUp) ||
                (legsanimation.id == AnimationType::Fall) ||
                (legsanimation.id == AnimationType::Mercy) ||
                (legsanimation.id == AnimationType::Mercy2) ||
                (legsanimation.id == AnimationType::Own))
            {
              if ((spriteVelocity.x < slidelimit) && (spriteVelocity.x > -slidelimit) &&
                  (step.y > slidelimit))
              {
                const auto &spritePartsOldPos = sprite_system.GetSpritePartsOldPos(num);
                spritePartsPos = spritePartsOldPos;
                spriteForces.y = spriteForces.y - grav;
              }
              else
              {
#ifndef SERVER
                if (CVar::r_maxsparks > (max_sparks - 10))
                {
                  if (Random(15) == 0)
                  {
                    spos.x = spriteVelocity.x * 3;
                    spos.y = -spriteVelocity.y * 2;
                    vec2scale(spos, spos, 0.4 + (float)(Random(4)) / 10);
                    createspark(pos, spos, 1, num, 70);
                  }
                }
#endif
              }

              if ((step.y > slidelimit) && (polytype != poly_type_ice) &&
                  (polytype != poly_type_bouncy))
              {
                if ((legsanimation.id == AnimationType::Stand) ||
                    (legsanimation.id == AnimationType::Fall) ||
                    (legsanimation.id == AnimationType::Crouch))
                {
                  spriteVelocity.x = spriteVelocity.x * standsurfacecoefx;
                  spriteVelocity.y = spriteVelocity.y * standsurfacecoefy;
                  spriteForces.x = spriteForces.x - spriteVelocity.x;
                }
                else if (legsanimation.id == AnimationType::Prone)
                {
                  if (legsanimation.currframe > 24)
                  {
                    if (!(control.down && (control.left || control.right)))
                    {
                      spriteVelocity.x = spriteVelocity.x * standsurfacecoefx;
                      spriteVelocity.y = spriteVelocity.y * standsurfacecoefy;
                      spriteForces.x = spriteForces.x - spriteVelocity.x;
                    }
                  }
                  else
                  {
                    spriteVelocity.x = spriteVelocity.x * surfacecoefx;
                    spriteVelocity.y = spriteVelocity.y * surfacecoefy;
                  }
                }
                else if (legsanimation.id == AnimationType::GetUp)
                {
                  spriteVelocity.x = spriteVelocity.x * surfacecoefx;
                  spriteVelocity.y = spriteVelocity.y * surfacecoefy;
                }
                else if (legsanimation.id == AnimationType::ProneMove)
                {
                  spriteVelocity.x = spriteVelocity.x * standsurfacecoefx;
                  spriteVelocity.y = spriteVelocity.y * standsurfacecoefy;
                }
              }
            }
            else
            {
              if ((legsanimation.id == AnimationType::CrouchRun) ||
                  (legsanimation.id == AnimationType::CrouchRunBack))
              {
                spriteVelocity.x = spriteVelocity.x * crouchmovesurfacecoefx;
                spriteVelocity.y = spriteVelocity.y * crouchmovesurfacecoefy;
              }
              else
              {
                spriteVelocity.x = spriteVelocity.x * surfacecoefx;
                spriteVelocity.y = spriteVelocity.y * surfacecoefy;
              }
            }
          }
          result = true;
          return result;
        }
      }
    }
  }
  return result;
}

template <Config::Module M>
auto Sprite<M>::checkmapverticescollision(float x, float y, float r, bool hascollided) -> bool
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  tvector2 pos;
  tvector2 dir;
  tvector2 vert;
  float d;
  bool teamcol;

#ifdef SERVER
  LogTraceG("TSprite.CheckMapVerticesCollision");
#endif
  auto &map = GS::GetGame().GetMap();

  bool result = false;
  pos.x = x;
  pos.y = y;

  // iterate through maps sector polygons
  const auto sector = map.GetSector(pos);
  if (sector.IsValid())
  {
    for (const auto &w : sector.GetPolys())
    {
      const auto &polytype = w.Type;

      teamcol = teamcollides(polytype, player->team, false);

      if (((polytype != poly_type_doesnt) && (polytype != poly_type_only_bullets) && teamcol &&
           (polytype != poly_type_only_flaggers) && (polytype != poly_type_not_flaggers)) ||
          ((holdedthing != 0) && (polytype == poly_type_only_flaggers)) ||
          ((holdedthing == 0) && (polytype == poly_type_not_flaggers)))
      {
        for (i = 0; i < 3; i++)
        {
          vert.x = map.polys[w.Index].vertices[i].x;
          vert.y = map.polys[w.Index].vertices[i].y;
          d = distance(vert, pos);
          if (d < r) // collision
          {
            if (bgstate.backgroundtest(w))
            {
              continue;
            }

            if (!hascollided)
            {
              handlespecialpolytypes(polytype, pos);
            }

            dir = vec2subtract(pos, vert);
            vec2normalize(dir, dir);
            auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);
            spritePartsPos = vec2add(spritePartsPos, dir);

            result = true;
            return result;
          } // D < R
        }   // i
      }     // if (PolyType...)
    }       // j
  }
  return result;
}

template <Config::Module M>
auto Sprite<M>::checkskeletonmapcollision(std::int32_t i, float x, float y) -> bool
{
  ZoneScopedN("CheckSkeletonMapCollision");
  std::int32_t b = 0;
  tvector2 pos;
  tvector2 perp;
#ifndef SERVER
  tvector2 a;
#endif
  float d = 0.0;
  bool teamcol;

#ifdef SERVER
  LogTraceG("TSprite.CheckSkeletonMapCollision");
#endif
  auto &map = GS::GetGame().GetMap();

  bool result = false;
  pos.x = x - 1;
  pos.y = y + 4;

  // iterate through map polygons
  const auto sector = map.GetSector(pos);
  if (sector.IsValid())
  {
    bgstate.backgroundtestbigpolycenter(pos);

    for (const auto &w : sector.GetPolys())
    {
      const auto &polytype = w.Type;
      teamcol = teamcollides(polytype, player->team, false);

      if (((polytype != poly_type_doesnt) && (polytype != poly_type_only_bullets) && teamcol &&
           (polytype != poly_type_only_flaggers) && (polytype != poly_type_not_flaggers)) ||
          ((holdedthing != 0) && (polytype == poly_type_only_flaggers)) ||
          ((holdedthing == 0) && (polytype == poly_type_not_flaggers)))
      {
        if (map.pointinpolyedges(pos.x, pos.y, w.Index))
        {
          if (bgstate.backgroundtest(w))
          {
            continue;
          }

          perp = map.closestperpendicular(w.Index, pos, d, b);

          vec2normalize(perp, perp);
          vec2scale(perp, perp, d);

          skeleton.pos[i] = skeleton.oldpos[i];
          skeleton.pos[i] = vec2subtract(skeleton.pos[i], perp);

#ifndef SERVER
          a = vec2subtract(skeleton.pos[i], skeleton.oldpos[i]);

          if ((fabs(a.y) > 0.8) && (deadcollidecount < 13))
          {
            playsound(SfxEffect::bodyfall, skeleton.pos[i]);
          }

          if ((fabs(a.y) > 2.1) && (deadcollidecount < 4))
          {
            playsound(SfxEffect::bonecrack, skeleton.pos[i]);
          }
#endif

          deadcollidecount += 1;

          result = true;
        }
      }
    }
  }

  if (result)
  {
    pos.x = x;
    pos.y = y + 1;

    // iterate through map polygons
    const auto sector = map.GetSector(pos);

    if (sector.IsValid())
    {
      bgstate.backgroundtestbigpolycenter(pos);

      for (const auto &w : sector.GetPolys())
      {
        const auto &polytype = w.Type;
        if ((polytype != poly_type_doesnt) && (polytype != poly_type_only_bullets))
        {
          if (map.pointinpolyedges(pos.x, pos.y, w.Index))
          {
            if (bgstate.backgroundtest(w))
            {
              continue;
            }

            perp = map.closestperpendicular(w.Index, pos, d, b);

            vec2normalize(perp, perp);
            vec2scale(perp, perp, d);

            skeleton.pos[i] = skeleton.oldpos[i];
            skeleton.pos[i] = vec2subtract(skeleton.pos[i], perp);

            result = true;
          }
        }
      }
    }
  }
  return result;
}

template <Config::Module M>
void Sprite<M>::handlespecialpolytypes(std::int32_t polytype, const tvector2 &pos)
{
  auto &sprite_system = SpriteSystem::Get();
  tvector2 a;
  tvector2 b;
  auto &spriteVelocity = sprite_system.GetVelocity(num);
#ifndef SERVER
  auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);
#endif
  auto &guns = GS::GetWeaponSystem().GetGuns();
  auto &things = GS::GetThingSystem().GetThings();

  switch (polytype)
  {
  case poly_type_deadly: {
#ifdef SERVER
    healthhit(50 + GetHealth(), num, 12, -1, spriteVelocity);
#endif
  }
  break;
  case poly_type_bloody_deadly: {
#ifdef SERVER
    healthhit(450 + GetHealth(), num, 12, -1, spriteVelocity);
#endif
  }
  break;
  case poly_type_hurts:
  case poly_type_lava: // hurts
  {
    if (!deadmeat)
    {
      if (Random(10) == 0)
      {
#ifdef SERVER
        Health = Health - 5;
#else

        if (polytype == poly_type_hurts)
        {
          playsound(SfxEffect::arg, spritePartsPos);
        }
        else if (polytype == poly_type_lava)
        {
          playsound(SfxEffect::lava, spritePartsPos);
        }
#endif
      }
#ifdef SERVER
      if (GetHealth() < 1)
      {
        healthhit(10, num, 12, -1, spriteVelocity);
      }
#endif
    }

    // lava
    if (Random(3) == 0)
    {
      if (polytype == poly_type_lava)
      {
        a = pos;
        a.y = a.y - 3.0;
#ifndef SERVER
        createspark(a, vector2(0, -1.3), 36, num, 40);
#endif

        if (Random(3) == 0)
        {
          b.x = -spriteVelocity.x;
          b.y = -spriteVelocity.y;
          createbullet(a, b, flamer_num, num, 255, guns[flamer].hitmultiply, false, true);
        }
      }
    }
  }
  break;
  case poly_type_regenerates: {
    if (Health < GS::GetGame().GetStarthealth())
    {
      if (GS::GetGame().GetMainTickCounter() % 12 == 0)
      {
#ifdef SERVER
        healthhit(-2, num, 12, -1, spriteVelocity);
#else
        playsound(SfxEffect::regenerate, spritePartsPos);
#endif
      }
    }
  }
  break;
  case poly_type_explodes: {
    if (!deadmeat)
    {
      a = pos;
      a.y = a.y - 3.0;
      b.x = 0;
      b.y = 0;
#ifndef SERVER
      createspark(a, vector2(0, -1.3), 36, num, 40);
#else
      servercreatebullet(a, b, m79_num, num, 255, guns[m79].hitmultiply, true);
      healthhit(4000, num, 12, -1, spriteVelocity);
      Health = -600;
#endif
    }
  }
  break;
  case poly_type_hurts_flaggers: {
    if (!deadmeat && (holdedthing > 0) && (things[holdedthing].style < object_ussocom))
    {
      if (Random(10) == 0)
      {
#ifdef SERVER
        Health = Health - 10;
#else
        playsound(SfxEffect::arg, spritePartsPos);
#endif
      }
    }
#ifdef SERVER
    if (Health < 1)
    {
      healthhit(10, num, 12, -1, spriteVelocity);
    }
#endif
  }
  break;
  }
}

template <Config::Module M>
auto BackgroundState<M>::backgroundtest(const PolyMapSector::Poly &poly) -> bool
{
  bool result;
  result = false;

  const auto &polytype = poly.Type;

  if ((polytype == poly_type_background) && (backgroundstatus == background_transition))
  {
    backgroundtestresult = true;
    backgroundpoly = poly.Index;
    backgroundstatus = background_transition;

    result = true;
  }
  else if (polytype == poly_type_background_transition)
  {
    backgroundtestresult = true;
    if (backgroundstatus == background_normal)
    {
      backgroundstatus = background_transition;
    }

    result = true;
  }
  return result;
}

template <Config::Module M>
void BackgroundState<M>::backgroundtestbigpolycenter(const tvector2 &pos)
{
  auto &map = GS::GetGame().GetMap();
  if (backgroundstatus == background_transition)
  {
    if (backgroundpoly == background_poly_unknown)
    {
      backgroundpoly = backgroundfindcurrentpoly(pos);
      if (backgroundpoly != background_poly_none)
      {
        backgroundtestresult = true;
      }
    }
    else if ((backgroundpoly != background_poly_none) and
             (Polymap::pointinpoly(pos, map.polys[backgroundpoly])))
    {
      backgroundtestresult = true;
    }
  }
}

template <Config::Module M>
auto BackgroundState<M>::backgroundfindcurrentpoly(const tvector2 &pos) -> std::int16_t
{
  auto &map = GS::GetGame().GetMap();
  std::int32_t i;

  std::int16_t result;
  for (i = 1; i <= map.backpolycount; i++)
  {
    if (Polymap::pointinpoly(pos, *map.backpolys[i]))
    {
      result = i;
      return result;
    }
  }
  result = background_poly_none;
  return result;
}

template <Config::Module M>
void BackgroundState<M>::backgroundtestprepare()
{
  backgroundtestresult = false;
}

template <Config::Module M>
void BackgroundState<M>::backgroundtestreset()
{
  if (!backgroundtestresult)
  {
    backgroundstatus = background_normal;
    backgroundpoly = background_poly_none;
  }
}

template <Config::Module M>
void Sprite<M>::applyweaponbynum(std::uint8_t wnum, std::uint8_t gun, std::int32_t ammo,
                                 bool restoreprimarystate)
{
  std::int32_t weaponindex;

#ifdef SERVER
  LogTraceG("TSprite.ApplyWeaponByNum");

  if (player->knifewarnings > 0)
  {
    player->knifewarnings -= 1;
  }
#endif
  auto &guns = GS::GetWeaponSystem().GetGuns();

  if (restoreprimarystate && (gun == 2))
  {
    SetSecondWeapon(weapon);
  }
  else
  {
    weaponindex = weaponnumtoindex(wnum, guns);
    if (gun == 1)
    {
      SetFirstWeapon(guns[weaponindex]);
    }
    else
    {
      SetSecondWeapon(guns[weaponindex]);
    }
  }

  if (ammo > -1)
  {
    weapon.ammocount = ammo;
  }

#ifndef SERVER
  weapon.startuptimecount = weapon.startuptime;
#endif

#ifdef SERVER
  if (weapon.num == knife_num)
  {
    knifecan[num] = true;
  }
#endif

  if (wnum != noweapon_num)
  {
    lastweaponhm = weapon.hitmultiply;
    lastweaponstyle = weapon.bulletstyle;
    lastweaponspeed = weapon.speed;
    lastweaponfire = weapon.fireinterval;
    lastweaponreload = weapon.reloadtime;
  }

#ifdef SERVER
  LogTraceG("TSprite.ApplyWeaponByNum 2");
#endif
}

template <Config::Module M>
void Sprite<M>::healthhit(float amount, std::int32_t who, std::int32_t where, std::int32_t what,
                          const tvector2 &impact)
{
  auto &sprite_system = SpriteSystem::Get();
  tvector2 t;
  float hm;
#ifndef SERVER
  std::int32_t j;
  std::string s;
#endif

#ifdef SERVER
  LogTraceG("TSprite.HealthHit");
#endif
  // Friendly Fire
  if ((!CVar::sv_friendlyfire) && isnotsolo() && isinsameteam(sprite_system.GetSprite(who)) &&
      (num != who))
  {
    return;
  }

#ifdef SERVER
  if (sprite_system.GetSprite(who).isspectator() &&
      (sprite_system.GetSprite(who).player->controlmethod == human))
  {
    return;
  }
#endif

  if (bonusstyle == bonus_flamegod)
  {
    return;
  }

  // no health hit if someone is rambo
  if (CVar::sv_gamemode == gamestyle_rambo)
  {
    if (num != who)
    {
      for (auto &sprite : sprite_system.GetActiveSprites())
      {
        if ((who != sprite.num) && (num != sprite.num))
        {
          if ((sprite.weapon.num == bow_num) || (sprite.weapon.num == bow2_num))
          {
            return;
          }
        }
      }
    }
  }

  hm = amount;

  if (vest > 0)
  {
    hm = round(0.33 * amount);
    vest = vest - hm;
    hm = round(0.25 * amount);
  }

  bool enable_berserker = who != num;
  if constexpr (Config::IsServer(M))
  {
    enable_berserker = true;
  }
  if (sprite_system.GetSprite(who).bonusstyle == bonus_berserker && enable_berserker)
  {
    hm = 4 * amount;
  }

#ifdef SCRIPT
  if (!this->deadmeat)
    hm = scrptdispatcher.onplayerdamage(this->num, who, hm, (std::uint8_t)(what));
#endif

  Health = Health - hm;

#ifndef SERVER
  auto &guns = GS::GetWeaponSystem().GetGuns();
  auto &bullet = GS::GetBulletSystem().GetBullets();
  if ((what > 0) && (this->num != mysprite))
  {
    switch (bullet[what].style)
    {
    case bullet_style_fragnade:
      s = "Grenade";
      break;
    case bullet_style_cluster:
      s = "Clusters";
      break;
    case bullet_style_m2:
      s = "Stationary gun";
      break;
    default:
      s = weaponnamebynum(bullet[what].ownerweapon, guns);
    }
  }
  else
  {
    s = "Selfkill";
  }

  if (!this->deadmeat && (what > 0))
  {
    // Check to prevent one AoE explosion counting as multiple hits on one bullet
    if ((who != this->num) && (!bullet[what].hashit) && (sprite_system.IsPlayerSprite(who)))
    {
      for (j = 1; j <= 20; j++)
      {
        if (wepstats[j].name == s)
        {
          wepstats[j].hits = wepstats[j].hits + 1;
          bullet[what].hashit = true;
        }
      }
    }
  }
#endif

  // helmet fall off
  if ((Health < helmetfallhealth) && (wearhelmet == 1) && (where == 12) &&
      (weapon.num != bow_num) && (weapon.num != bow2_num) && (player->headcap > 0))
  {
    wearhelmet = 0;
#ifndef SERVER
    auto &spriteVelocity = sprite_system.GetVelocity(num);
    createspark(skeleton.pos[12], spriteVelocity, 6, num, 198);
    playsound(SfxEffect::headchop, skeleton.pos[where]);
#endif
  }

  // safety precautions
  if (Health < (brutaldeathhealth - 1))
  {
    Health = brutaldeathhealth;
  }
  if (Health > GS::GetGame().GetStarthealth())
  {
    Health = GS::GetGame().GetStarthealth();
  }

  // death!
  t = impact;
  if ((Health < 1) && (Health > headchopdeathhealth))
  {
    die(normal_death, who, where, what, t);
  }
  else if ((Health < headchopdeathhealth + 1) && (Health > brutaldeathhealth))
  {
    die(headchop_death, who, where, what, t);
  }
  else if (Health < brutaldeathhealth + 1)
  {
    die(brutal_death, who, where, what, t);
  }

  brain.targetnum = who;

  // Bot Chat
#ifdef SERVER
  if (CVar::bots_chat)
  {
    if ((GetHealth() < hurt_health) && (player->controlmethod == bot))
    {
      if (Random(10 * brain.chatfreq) == 0)
      {
        serversendstringmessage((brain.chatlowhealth), all_players, num, msgtype_pub);
      }
    }
  }
#endif
}

template <Config::Module M>
void Sprite<M>::freecontrols()
{
#ifdef SERVER
  LogTraceG("TSprite.FreeControls");
#endif

  control.left = false;
  control.right = false;
  control.up = false;
  control.down = false;
  control.fire = false;
  control.jetpack = false;
  control.thrownade = false;
  control.changeweapon = false;
  control.throwweapon = false;
  control.reload = false;
  control.prone = false;
#ifndef SERVER
  control.mousedist = 150;
#endif
  control.flagthrow = false;
}

template <Config::Module M>
void Sprite<M>::checkoutofbounds()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t bound;

#ifdef SERVER
  LogTraceG("TSprite.CheckOutOfBounds");
#endif
  auto &map = GS::GetGame().GetMap();

  if (GS::GetGame().GetSurvivalEndRound())
  {
    return;
  }

  bound = map.sectorsnum * map.GetSectorsDivision() - 50;
  auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);

  if ((fabs(spritePartsPos.x) > bound) || (fabs(spritePartsPos.y) > bound))
  {
#ifndef SERVER
    randomizestart(spritePartsPos, player->team);
#endif
    respawn();
  }
}

template <Config::Module M>
void Sprite<M>::checkskeletonoutofbounds()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  std::int32_t bound;
  struct tvector2 *skeletonpos;

  LogTraceG("TSprite.CheckSkeletonOutOfBounds");
  auto &map = GS::GetGame().GetMap();

  if (GS::GetGame().GetSurvivalEndRound())
  {
    return;
  }

  bound = map.sectorsnum * map.GetSectorsDivision() - 50;
  auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);

  for (i = 1; i <= 20; i++)
  {
    skeletonpos = &skeleton.pos[i];

    if ((fabs(skeletonpos->x) > bound) || (fabs(skeletonpos->y) > bound))
    {
      if constexpr (Config::IsClient(M))
      {
        randomizestart(spritePartsPos, player->team);
      }
      respawn();
      break;
    }
  }
}

template <Config::Module M>
void Sprite<M>::respawn()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t j;
#ifdef SERVER
  std::int32_t k;
  std::int32_t weaponindex;
  std::int16_t favweaponindex;
#endif
  std::int32_t secwep;
  bool deadmeatbeforerespawn;
  bool survivalcheckendround;
  auto &map = GS::GetGame().GetMap();

  auto &guns = GS::GetWeaponSystem().GetGuns();
  auto &things = GS::GetThingSystem().GetThings();

  LogTraceG("TSprite.Respawn");
  if (CVar::sv_survivalmode_clearweapons)
  {
    if (GS::GetGame().GetSurvivalEndRound() && !weaponscleaned)
    {
      for (j = 1; j <= max_things; j++)
      {
        if (things[j].active &&
            ((isInRange((std::int32_t)things[j].style, object_ussocom, object_minigun)) ||
             (isInRange((std::int32_t)things[j].style, object_combat_knife, object_law))))
        {
          things[j].kill();
        }
      }
      weaponscleaned = true;
    }
  }

  if (isspectator())
  {
    return;
  }

  auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);
  auto &spriteForces = sprite_system.GetForces(num);
  auto &weaponsel = GS::GetGame().GetWeaponsel();

#ifndef SERVER
  if ((player->name.empty()) or (player->demoplayer))
  {
    return;
  }

  if (sprite_system.IsPlayerSprite(num))
  {
    playsound(SfxEffect::wermusic, spritePartsPos);
  }
#endif

#ifdef SERVER
  randomizestart(spritePartsPos, player->team);
#endif

#ifdef SCRIPT
  spritePartsPos = scrptdispatcher.onbeforeplayerrespawn(num);
#endif

  deadmeatbeforerespawn = deadmeat;
  deadmeat = false;
  halfdead = false;
  Health = GS::GetGame().GetStarthealth();
  wearhelmet = 1;

  if (player->headcap == 0)
  {
    wearhelmet = 0;
  }
  skeleton.constraints = AnimationSystem::Get().GetSkeleton(Gostek).constraints;
  auto &spriteVelocity = sprite_system.GetVelocity(num);
  spriteVelocity.x = 0;
  spriteVelocity.y = 0;
  spriteForces.x = 0;
  spriteForces.y = 0;
  jetscount = map.startjet;
  jetscountprev = map.startjet;
  ceasefirecounter = GS::GetGame().GetCeasefiretime();
  if (CVar::sv_advancemode)
  {
    ceasefirecounter = ceasefirecounter * 3;
  }
  brain.pissedoff = 0;
  brain.gothing = false;
  vest = 0;
  bonusstyle = bonus_none;
  bonustime = 0;
  multikills = 0;
  multikilltime = 0;
  SetThirdWeapon(guns[fraggrenade]);
  tertiaryweapon.ammocount = CVar::sv_maxgrenades / 2;
  hascigar = 0;
  canmercy = true;
  idletime = default_idletime;
  idlerandom = -1;

#ifndef SERVER
  if (sprite_system.IsPlayerSprite(num))
  {
    camerafollowsprite = mysprite;
    grenadeeffecttimer = 0;
    hitspraycounter = 0;
  }
#endif

  bodyanimation = AnimationSystem::Get().GetAnimation(AnimationType::Stand);
  legsanimation = AnimationSystem::Get().GetAnimation(AnimationType::Stand);
  position = pos_stand;
  onfire = 0;
  deadcollidecount = 0;
  brain.currentwaypoint = 0;
  respawncounter = 0;
  player->camera = num;
  onground = false;
  ongroundlastframe = false;
  ongroundpermanent = false;

  bgstate.backgroundstatus = background_transition;
  bgstate.backgroundpoly = background_poly_unknown;

#ifdef SERVER
  bullettime[num] = GS::GetGame().GetMainTickCounter() - 10;
  grenadetime[num] = GS::GetGame().GetMainTickCounter() - 10;
  knifecan[num] = true;
#endif

  if ((holdedthing > 0) && (holdedthing < max_things + 1))
  {
    if (things[holdedthing].style != object_parachute)
    {
      things[holdedthing].respawn();
    }
    else
    {
      things[holdedthing].kill();
    }
  }

  holdedthing = 0;

#ifndef SERVER
  if (selweapon > 0)
  {
    if (weaponsel[num][selweapon] == 0)
    {
      selweapon = 0;
    }
  }
#endif

  SetFirstWeapon(guns[noweapon]);
  auto &weaponSystem = GS::GetWeaponSystem();

  if (selweapon > 0)
  {
    bool is_weapon_enabled = (weaponSystem.IsEnabled(selweapon)) && (weaponsel[num][selweapon] == 1);
    if constexpr(Config::IsServer(M))
    {
      is_weapon_enabled = true;
    }
    if (is_weapon_enabled)
    {
      applyweaponbynum(selweapon, 1);
#ifndef SERVER
      if (sprite_system.IsPlayerSprite(num))
      {
        clientspritesnapshot();
      }
#endif
    }
  }

  secwep = player->secwep + 1;

  if ((secwep >= 1) && (secwep <= secondary_weapons) &&
      (weaponSystem.IsEnabled(primary_weapons + secwep)) &&
      (weaponsel[num][primary_weapons + secwep] == 1))
  {
    SetSecondWeapon(guns[primary_weapons + secwep]);
  }
  else
  {
    SetSecondWeapon(guns[noweapon]);
  }

  bool advanced_mode = CVar::sv_advancemode;
  if constexpr(Config::IsClient(M))
  {
    advanced_mode = true;
  }

  if (advanced_mode)
  {
    if ((selweapon > 0) && (!weaponSystem.IsEnabled(selweapon) || (weaponsel[num][selweapon] == 0)))
    {
      SetFirstWeapon(secondaryweapon);
      SetSecondWeapon(guns[noweapon]);
    }
  }

#ifdef SERVER
  if (player->controlmethod == bot)
  {
    brain.currentwaypoint = 0;

    if ((CVar::sv_gamemode == gamestyle_ctf) || (CVar::sv_gamemode == gamestyle_inf) ||
        (CVar::sv_gamemode == gamestyle_htf))
    {
      brain.pathnum = player->team;
    }

    LogTraceG("TSprite.Respawn 2");

    // randomize bot weapon
    if ((brain.favweapon != noweapon_num) && (brain.favweapon != knife_num) &&
        (brain.favweapon != chainsaw_num) && (brain.favweapon != law_num) && !dummy)
    {
      if (((weaponSystem.IsEnabled(1)) && (weaponsel[num][1] == 1)) ||
          ((weaponSystem.IsEnabled(2)) && (weaponsel[num][2] == 1)) ||
          ((weaponSystem.IsEnabled(3)) && (weaponsel[num][3] == 1)) ||
          ((weaponSystem.IsEnabled(4)) && (weaponsel[num][4] == 1)) ||
          ((weaponSystem.IsEnabled(5)) && (weaponsel[num][5] == 1)) ||
          ((weaponSystem.IsEnabled(6)) && (weaponsel[num][6] == 1)) ||
          ((weaponSystem.IsEnabled(7)) && (weaponsel[num][7] == 1)) ||
          ((weaponSystem.IsEnabled(8)) && (weaponsel[num][8] == 1)) ||
          ((weaponSystem.IsEnabled(9)) && (weaponsel[num][9] == 1)) ||
          ((weaponSystem.IsEnabled(10)) && (weaponsel[num][10] == 1)))
      {
        do
        {
          if (Random(2) == 0)
          {
            applyweaponbynum(brain.favweapon, 1);
          }
          else
          {
            k = Random(9) + 1;
            SetFirstWeapon(guns[k]);
          }

          if ((weaponSystem.GetWeaponsInGame() < 6) && (weaponSystem.IsEnabled(minigun)) &&
              (weaponsel[num][minigun] == 1))
          {
            SetFirstWeapon(guns[minigun]);
          }

          if (CVar::sv_advancemode)
          {
            for (j = 1; j <= primary_weapons; j++)
            {
              if (weaponsel[num][j] == 1)
              {
                break;
              }
            }
            applyweaponbynum(j, 1);
          }
        } while (!((weaponSystem.IsEnabled(weapon.num)) or (CVar::sv_advancemode)));
      }
    }

    if ((!weaponSystem.IsEnabled(1)) && (!weaponSystem.IsEnabled(2)) &&
        (!weaponSystem.IsEnabled(3)) && (!weaponSystem.IsEnabled(4)) &&
        (!weaponSystem.IsEnabled(5)) && (!weaponSystem.IsEnabled(6)) &&
        (!weaponSystem.IsEnabled(7)) && (!weaponSystem.IsEnabled(8)) &&
        (!weaponSystem.IsEnabled(9)) && (!weaponSystem.IsEnabled(10)) && (weaponsel[num][1] == 0) &&
        (weaponsel[num][2] == 0) && (weaponsel[num][3] == 0) && (weaponsel[num][4] == 0) &&
        (weaponsel[num][5] == 0) && (weaponsel[num][6] == 0) && (weaponsel[num][7] == 0) &&
        (weaponsel[num][8] == 0) && (weaponsel[num][9] == 0) && (weaponsel[num][10] == 0))
    {
      SetFirstWeapon(guns[noweapon]);
    }

    favweaponindex = weaponnumtoindex(brain.favweapon, guns);
    if ((brain.favweapon == noweapon_num) or issecondaryweaponindex(favweaponindex) or dummy)
    {
      SetFirstWeapon(guns[favweaponindex]);
      SetSecondWeapon(guns[noweapon]);
    }

    if (brain.use != 255)
    {
      if (brain.use == 1)
      {
        idletime = 0;
        idlerandom = 1;
      }
      if (brain.use == 2)
      {
        idletime = 0;
        idlerandom = 0;
      }
    }

    // Disarm bot if the primary weapon isn't allowed and selectable
    weaponindex = weapon.num;
    if ((weaponindex >= 1) && (weaponindex <= primary_weapons))
    {
      if ((!weaponSystem.IsEnabled(weaponindex)) || (weaponsel[num][weaponindex] == 0))
      {
        SetFirstWeapon(guns[noweapon]);
      }
    }
  }
#endif

  if (weaponSystem.GetWeaponsInGame() == 0)
  {
    SetFirstWeapon(guns[noweapon]);
  }

  parachute(spritePartsPos);

#ifdef SCRIPT
  scrptdispatcher.onafterplayerrespawn(num);
#endif

  // clear push wait list
  {
    ZoneScopedN("CleanNextPush");
    for (j = 0; j <= max_pushtick; j++)
    {
      nextpush[j].x = 0;
      nextpush[j].y = 0;
    }
  }

#ifndef SERVER
  // Spawn sound
  if (!sprite_system.IsPlayerSprite(num))
  {
    playsound(SfxEffect::spawn, spritePartsPos);
  }

  // spawn spark
  createspark(spritePartsPos, spriteVelocity, 25, num, 33);
#endif

  freecontrols();

  legsapplyanimation(AnimationType::Stand, 1);
  bodyapplyanimation(AnimationType::Stand, 1);

  if (canrespawn(deadmeatbeforerespawn))
  {
    if (GS::GetGame().GetSurvivalEndRound())
    {
      survivalcheckendround = false;
      for (auto &sprite : sprite_system.GetActiveSprites())
      {
        if (sprite.player->team != team_spectator)
        {
          if (sprite.deadmeat)
          {
            survivalcheckendround = true;
            break;
          }
        }
      }
      GS::GetGame().SetSurvivalendround(survivalcheckendround);
    }
  }
  else
  {
    // CheckSkeletonOutOfBounds would trigger infinitely
    // Respawn if this is not done
    for (j = 1; j <= 20; j++)
    {
      skeleton.pos[j].x = spritePartsPos.x;
      skeleton.pos[j].y = spritePartsPos.y;
      skeleton.oldpos[j] = skeleton.pos[j];
    }
    // TODO: Fix this shouldn't change wepstats
    die(normal_death, num, 1, -1, skeleton.pos[12]);
    player->deaths -= 1;
  }
}

template <Config::Module M>
void Sprite<M>::parachute(tvector2 &a)
{
  tvector2 b;
  std::int32_t n;
  std::int32_t i;
  float d = 0.0;
  auto &map = GS::GetGame().GetMap();
  auto &things = GS::GetThingSystem().GetThings();

  LogTraceG("Parachute");
  if (holdedthing > 0)
  {
    return;
  }
  if (isspectator())
  {
    return;
  }

  for (i = 1; i <= max_things; i++)
  {
    if (things[i].holdingsprite == num)
    {
      things[i].holdingsprite = 0;
      things[i].kill();
    }
  }

  b = a;
  b.y = b.y + para_distance;
  if (!map.raycast(a, b, d, para_distance + 50, true, false, false, false, player->team))
  {
    if (d > para_distance - 10)
    {
      a.y = a.y + 70;
      n = creatething(a, num, object_parachute, 255);
      things[n].holdingsprite = num;
#ifndef SERVER
      things[n].color = player->shirtcolor;
#endif
      holdedthing = n;
    }
  }
}

template <Config::Module M>
void Sprite<M>::changeteam_ServerVariant(std::int32_t team, bool adminchange, std::uint8_t jointype)
{
  [[maybe_unused]] auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  tvector2 a;
#ifdef SERVER
  std::vector<std::int32_t> teamscount{6};
#endif
#ifdef SCRIPT
  std::uint8_t spriteoldteam;
#endif

  if (team > team_spectator)
  {
    return;
  }

  auto &things = GS::GetThingSystem().GetThings();

  if (active)
  {
#ifdef SERVER
    for (i = team_none; i <= team_spectator; i++)
    {
      teamscount[i] = 0;
    }

    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
      if (sprite.isnotspectator())
      {
        teamscount[sprite.player->team] = teamscount[sprite.player->team] + 1;
      }
    }

    // Check for uneven teams
    if ((CVar::sv_balanceteams) && (!adminchange))
    {
      if (isspectator() && (team == findlowestteam(teamscount)))
      {
        ;
      }
      else if ((teamscount[team] >= teamscount[player->team]) && (team < team_spectator))
      {
        const char *teams[] = {"Alpha", "Bravo", "Charlie", "Delta"};
        writeconsole(num, std::string(choose(team - 1, teams)) + " team is full", rgb(0, 0, 255));
        return;
      }
    }

    if ((teamscount[team_spectator] >= CVar::sv_maxspectators) && (team == team_spectator) &&
        (!adminchange))
    {
      writeconsole(num, "Spectators are full", rgb(0, 0, 255));
      return;
    }

    if ((CVar::sv_gamemode != gamestyle_teammatch) &&
        ((team == team_charlie) || (team == team_delta)) && (!adminchange))
    {
      return;
    }

#ifdef SCRIPT
    spriteoldteam = player->team;
    team = scrptdispatcher.onbeforejointeam(num, team, spriteoldteam);
    if ((team < team_none) || (team > team_spectator))
      return;
#endif
#endif

    dropweapon();

    player->team = team;
#ifdef SERVER
    player->applyshirtcolorfromteam();
#endif
    auto ret = createsprite(a, num, player);
    SoldatAssert(ret == num);
    if (holdedthing > 0)
    {
      if (things[holdedthing].style < object_ussocom)
      {
        things[holdedthing].respawn();
        holdedthing = 0;
      }
    }

    for (i = 1; i <= max_things; i++)
    {
      if (things[i].holdingsprite == num)
      {
        things[i].respawn();
      }
    }
    respawn();

#ifdef SERVER
    bullettime[num] = GS::GetGame().GetMainTickCounter() - 10;
    grenadetime[num] = GS::GetGame().GetMainTickCounter() - 10;
    knifecan[num] = true;

    if (!player->demoplayer)
    {
      serversendnewplayerinfo(num, jointype);
    }
    GS::GetGame().sortplayers();
    LogDebugG("Player switched teams");
#endif

    switch (player->team)
    {
#ifdef SERVER
    case team_none:
      GS::GetMainConsole().console(player->name + " has joined the game.", enter_message_color);
      break;
    case team_alpha:
      GS::GetMainConsole().console(player->name + " has joined alpha team.", alphaj_message_color);
      break;
    case team_bravo:
      GS::GetMainConsole().console(player->name + " has joined bravo team.", bravoj_message_color);
      break;
    case team_charlie:
      GS::GetMainConsole().console(player->name + " has joined charlie team.", charliej_message_color);
      break;
    case team_delta:
      GS::GetMainConsole().console(player->name + " has joined delta team.", deltaj_message_color);
      break;
    case team_spectator:
      GS::GetMainConsole().console(player->name + " has joined spectators.", deltaj_message_color);
      break;
#else
    case team_none:
      GS::GetMainConsole().console(wideformat(_("%s has joined the game."), player->name),
                                   enter_message_color);
      break;
    case team_alpha:
      GS::GetMainConsole().console(wideformat(_("%s has joined alpha team"), player->name),
                                   alphaj_message_color);
      break;
    case team_bravo:
      GS::GetMainConsole().console(wideformat(_("%s has joined bravo team"), player->name),
                                   bravoj_message_color);
      break;
    case team_charlie:
      GS::GetMainConsole().console(wideformat(_("%s has joined charlie team"), player->name),
                                   charliej_message_color);
      break;
    case team_delta:
      GS::GetMainConsole().console(wideformat(_("%s has joined delta team"), player->name),
                                   deltaj_message_color);
      break;
#endif
    }

    // prevent players from joining alive midround in survival mode
    if ((CVar::sv_survivalmode) && (player->team != team_spectator))
    {
      // TODO: Fix this shouldn't change wepstats
      healthhit(4000, num, 1, 1, a);
      player->deaths -= 1;
    }
#ifndef SERVER
    if (sprite_system.IsPlayerSprite(num))
    {
      if (player->team == team_spectator)
      {
        camerafollowsprite = 0;
        camerafollowsprite = getcameratarget();
        gamemenushow(limbomenu, false);
      }
      else
      {
        camerafollowsprite = mysprite;
      }
    }
#endif

    // Check if map change is in progress
#ifdef SERVER
    if ((GS::GetGame().GetMapchangecounter() > -60) &&
        (GS::GetGame().GetMapchangecounter() < 99999999))
    {
      servermapchange(num);
    }
#endif

#ifdef SCRIPT
    scrptdispatcher.onjointeam(num, team, spriteoldteam, false);
#endif
  }
}

// SPRITE FIRE
template <Config::Module M>
void Sprite<M>::fire()
{
  auto &sprite_system = SpriteSystem::Get();
  tvector2 a;
  tvector2 b;
  tvector2 d;
  tvector2 m;
  tvector2 aimdirection;
  std::int32_t i;
  std::int32_t bn;
  float inaccuracy;
  float maxdeviation;
  tvector2 collisiontestperpendicular;
  tvector2 bnorm;
#ifndef SERVER
  tvector2 muzzlesmokevector;
  tvector2 c;
  float rc;
  bool col;
#endif
  auto &map = GS::GetGame().GetMap();

  bn = 0;
  inaccuracy = 0;
#ifdef SERVER
  LogTraceG("SpriteFire");
#endif

  // Create a normalized directional vector
  if ((weapon.bulletstyle == bullet_style_knife) || (bodyanimation.id == AnimationType::Mercy) ||
      (bodyanimation.id == AnimationType::Mercy2))
  {
    aimdirection = gethandsaimdirection();
  }
  else
  {
    aimdirection = getcursoraimdirection();
  }

  b = aimdirection;

  a.x = skeleton.pos[15].x - (b.x * 4);
  a.y = skeleton.pos[15].y - (b.y * 4) - 2;

#ifndef SERVER
  // TODO(skoskav): Make bink and self-bink sprite-specific so bots can also use it
  if (sprite_system.IsPlayerSprite(num))
  {
    // Bink && self-bink
    if (hitspraycounter > 0)
    {
      inaccuracy = inaccuracy + hitspraycounter * 0.01;
    }
  }
#endif

  // Moveacc
  inaccuracy = inaccuracy + getmoveacc();

  // Bullet spread
  if ((weapon.num != eagle_num) && (weapon.num != spas12_num) &&
      (weapon.bulletstyle != bullet_style_shotgun))
  {
    if (weapon.bulletspread > 0)
    {
      if ((legsanimation.id == AnimationType::ProneMove) ||
          ((legsanimation.id == AnimationType::Prone) && (legsanimation.currframe > 23)))
      {
        inaccuracy = inaccuracy + weapon.bulletspread / 1.625;
      }
      else if ((legsanimation.id == AnimationType::CrouchRun) ||
               (legsanimation.id == AnimationType::CrouchRunBack) ||
               ((legsanimation.id == AnimationType::Crouch) && (legsanimation.currframe > 13)))
      {
        inaccuracy = inaccuracy + weapon.bulletspread / 1.3;
      }
      else
      {
        inaccuracy = inaccuracy + weapon.bulletspread;
      }
    }
  }

  // FIXME(skoskav): Inaccuracy decreased due to altered way of acquiring the directional vector.
  // This should be solved more elegantly.
  inaccuracy = inaccuracy * 0.25;

  if (inaccuracy > max_inaccuracy)
  {
    inaccuracy = max_inaccuracy;
  }

  // Calculate the maximum bullet deviation between 0 and MAX_INACCURACY.
  // The scaling is modeled after Sin(x) where x = 0 -> Pi/2 to gracefully reach
  // the maximum. Then multiply by a float between -1.0 and 1.0.
  maxdeviation = max_inaccuracy * sin((inaccuracy / max_inaccuracy) * (pi / 2));
  d.x = (float)(Random() * 2 - 1) * maxdeviation;
  d.y = (float)(Random() * 2 - 1) * maxdeviation;

  // Add inaccuracies to directional vector and re-normalize
  b = vec2add(b, d);
  vec2normalize(b, b);

  // Multiply with the weapon speed
  vec2scale(b, b, weapon.speed);

  auto &spriteVelocity = sprite_system.GetVelocity(num);
#ifndef SERVER
  auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);
#endif // SERVER

  // Add some of the player's velocity to the bullet
  vec2scale(m, spriteVelocity, weapon.inheritedvelocity);
  b = vec2add(b, m);

  // Check for immediate collision (could just be head in polygon), if so then
  // offset the bullet origin downward slightly
  if (map.collisiontest(a, collisiontestperpendicular))
  {
    a.y = a.y + 2.5;
  }

  if (((weapon.num != eagle_num) && (weapon.num != spas12_num) && (weapon.num != flamer_num) &&
       (weapon.num != noweapon_num) && (weapon.num != knife_num) && (weapon.num != chainsaw_num) &&
       (weapon.num != law_num)) ||
      (bodyanimation.id == AnimationType::Mercy) || (bodyanimation.id == AnimationType::Mercy2))
  {
    bn = createbullet(a, b, weapon.num, num, 255, weapon.hitmultiply, true, false);
  }

#ifdef SERVER
  LogTraceG("SpriteFire 10");
#endif

  if (weapon.num == eagle_num) // Eagles
  {
    bulletcount += 1;
    NotImplemented();
#if 0
        randseed = bulletcount;
#endif

    d.x = b.x + (Random() * 2 - 1) * weapon.bulletspread;
    d.y = b.y + (Random() * 2 - 1) * weapon.bulletspread;

    bn = createbullet(a, d, weapon.num, num, 255, weapon.hitmultiply, true, false, bulletcount);

    d.x = b.x + (Random() * 2 - 1) * weapon.bulletspread;
    d.y = b.y + (Random() * 2 - 1) * weapon.bulletspread;

    vec2normalize(bnorm, b);
    a.x = a.x - sign(b.x) * fabs(bnorm.y) * 3.0;
    a.y = a.y + sign(b.y) * fabs(bnorm.x) * 3.0;

    createbullet(a, d, weapon.num, num, 255, weapon.hitmultiply, false, false);
  }

  if (weapon.bulletstyle == bullet_style_shotgun) // Shotgun
  {
    bulletcount += 1;
    NotImplemented();
#if 0
        randseed = bulletcount;
#endif

    d.x = b.x + (Random() * 2 - 1) * weapon.bulletspread;
    d.y = b.y + (Random() * 2 - 1) * weapon.bulletspread;

    bn = createbullet(a, d, weapon.num, num, 255, weapon.hitmultiply, true, false, bulletcount);

    for (i = 0; i <= 4; i++) // Remaining 5 pellets
    {
      d.x = b.x + (Random() * 2 - 1) * weapon.bulletspread;
      d.y = b.y + (Random() * 2 - 1) * weapon.bulletspread;
      createbullet(a, d, weapon.num, num, 255, weapon.hitmultiply, false, false);
    }

    d.x = b.x * 0.0412;
    d.y = b.y * 0.041;
    spriteVelocity = vec2subtract(spriteVelocity, d);
  }

  LogTraceG("SpriteFire 11");

  if (weapon.num == minigun_num) // Minigun
  {
    if (control.jetpack && (jetscount > 0))
    {
      d.x = b.x * 0.0012;
      d.y = b.y * 0.0009;
    }
    else
    {
      d.x = b.x * 0.0082;
      d.y = b.y * 0.0078;
    }

    if (holdedthing > 0)
    {
      d.x = d.x * 0.5;
      d.y = d.y * 0.7;
    }
    d.x = d.x * 0.6;

    spriteVelocity = vec2subtract(spriteVelocity, d);
  }

  if (weapon.num == flamer_num) // Flamer
  {
    a.x = a.x + b.x * 2;
    a.y = a.y + b.y * 2;
    bn = createbullet(a, b, weapon.num, num, 255, weapon.hitmultiply, true, false);
#ifndef SERVER
    playsound(SfxEffect::flamer, spritePartsPos, gattlingsoundchannel);
#endif
  }

  if (weapon.num == chainsaw_num) // Chainsaw
  {
    a.x = a.x + b.x * 2;
    a.y = a.y + b.y * 2;
    bn = createbullet(a, b, weapon.num, num, 255, weapon.hitmultiply, true, false);
  }

  if (weapon.num == law_num)
  { // LAW
    if ((onground || ongroundpermanent || ongroundforlaw) &&
        (((legsanimation.id == AnimationType::Crouch) && (legsanimation.currframe > 13)) ||
         (legsanimation.id == AnimationType::CrouchRun) ||
         (legsanimation.id == AnimationType::CrouchRunBack) ||
         ((legsanimation.id == AnimationType::Prone) && (legsanimation.currframe > 23))))
    {
      bn = createbullet(a, b, weapon.num, num, 255, weapon.hitmultiply, true, false);
    }
    else
    {
      return;
    }
  }

  auto &bullet = GS::GetBulletSystem().GetBullets();
  // Mercy animation
  if ((bodyanimation.id == AnimationType::Mercy) || (bodyanimation.id == AnimationType::Mercy2))
  {
    if ((bn > 0) && (bn < max_bullets + 1))
    {
      auto &b = bullet[bn];
      if (b.active)
      {
        a = GetBulletParts().velocity[bn];
        vec2normalize(GetBulletParts().velocity[bn], GetBulletParts().velocity[bn]);
        vec2scale(GetBulletParts().velocity[bn], GetBulletParts().velocity[bn], 70);
        b.hit(2);
        b.hit(9);
        // couple more - not sure why
        b.hit(2);
        b.hit(9);
        b.hit(2);
        b.hit(9);
        b.hitbody = b.owner;
        GetBulletParts().velocity[bn] = a;
      }
    }
  }

  // Shouldn't we dec on server too?
#ifndef SERVER
  if (weapon.ammocount > 0)
  {
    weapon.ammocount -= 1;
  }
#endif

  if (weapon.num == spas12_num)
  {
    canautoreloadspas = false;
  }

  weapon.fireintervalprev = weapon.fireinterval;
  weapon.fireintervalcount = weapon.fireinterval;

  fired = weapon.firestyle;

#ifndef SERVER
  // Spent bullet shell vectors
  c.x = spriteVelocity.x + direction * aimdirection.y * (Random(0) * 0.5 + 0.8);
  c.y = spriteVelocity.y - direction * aimdirection.x * (Random(0) * 0.5 + 0.8);
  a.x = skeleton.pos[15].x + 2 - direction * 0.015 * b.x;
  a.y = skeleton.pos[15].y - 2 - direction * 0.015 * b.y;

  col = map.collisiontest(a, b);

  if (CVar::r_maxsparks < (max_sparks - 10))
  {
    if (Random(2) == 0)
    {
      col = true;
    }
  }
#endif

#ifdef SERVER
  LogTraceG("SpriteFire 13");
#endif

  // play fire sound
  if (weapon.num == ak74_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::ak74_fire, spritePartsPos);
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position == pos_stand) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::SmallRecoil, 1);
    }
    if (position == pos_crouch)
    {
      if (bodyanimation.id == AnimationType::HandSupAim)
      {
        bodyapplyanimation(AnimationType::HandSupRecoil, 1);
      }
      else
      {
        bodyapplyanimation(AnimationType::AimRecoil, 1);
      }
    }
#ifndef SERVER
    if (!col)
    {
      createspark(a, c, 68, num, 255); // shell
    }
#endif
  }
  if (weapon.num == m249_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::m249_fire, spritePartsPos);
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position == pos_stand) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::SmallRecoil, 1);
    }
    if (position == pos_crouch)
    {
      if (bodyanimation.id == AnimationType::HandSupAim)
      {
        bodyapplyanimation(AnimationType::HandSupRecoil, 1);
      }
      else
      {
        bodyapplyanimation(AnimationType::AimRecoil, 1);
      }
    }
#ifndef SERVER
    if (!col)
    {
      createspark(a, c, 72, num, 255); // shell
    }
#endif
  }
  if (weapon.num == ruger77_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::ruger77_fire, spritePartsPos);
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position == pos_stand) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::Recoil, 1);
    }
    if (position == pos_crouch)
    {
      if (bodyanimation.id == AnimationType::HandSupAim)
      {
        bodyapplyanimation(AnimationType::HandSupRecoil, 1);
      }
      else
      {
        bodyapplyanimation(AnimationType::AimRecoil, 1);
      }
    }
#ifndef SERVER
    if (!col)
    {
      createspark(a, c, 70, num, 255); // shell
    }
#endif
  }
  if (weapon.num == mp5_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::mp5_fire, spritePartsPos);
    }
    a.x = skeleton.pos[15].x + 2 - 0.2 * b.x;
    a.y = skeleton.pos[15].y - 2 - 0.2 * b.y;
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position == pos_stand) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::SmallRecoil, 1);
    }
    if (position == pos_crouch)
    {
      if (bodyanimation.id == AnimationType::HandSupAim)
      {
        bodyapplyanimation(AnimationType::HandSupRecoil, 1);
      }
      else
      {
        bodyapplyanimation(AnimationType::AimRecoil, 1);
      }
    }
#ifndef SERVER
    if (!col)
    {
      createspark(a, c, 67, num, 255); // shell
    }
#endif
  }
  if (weapon.num == spas12_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::spas12_fire, spritePartsPos);
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position != pos_prone) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::Shotgun, 1);
    }

    // make sure firing interrupts reloading when prone
    if ((position == pos_prone) && (bodyanimation.id == AnimationType::Reload))
    {
      bodyanimation.currframe = bodyanimation.numframes;
    }
  }
  if (weapon.num == m79_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::m79_fire, spritePartsPos);
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position != pos_prone) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::SmallRecoil, 1);
    }
  }
  if (weapon.num == eagle_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::deserteagle_fire, spritePartsPos);
    }
    a.x = skeleton.pos[15].x + 3 - 0.17 * b.x;
    a.y = skeleton.pos[15].y - 2 - 0.15 * b.y;
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position == pos_stand) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::SmallRecoil, 1);
    }
    if (position == pos_crouch)
    {
      if (bodyanimation.id == AnimationType::HandSupAim)
      {
        bodyapplyanimation(AnimationType::HandSupRecoil, 1);
      }
      else
      {
        bodyapplyanimation(AimRecoil, 1);
      }
    }
#ifndef SERVER
    if (!col)
    {
      createspark(a, c, 66, num, 255); // shell
    }
    if (!col)
    {
      a.x = skeleton.pos[15].x - 3 - 0.25 * b.x;
      a.y = skeleton.pos[15].y - 3 - 0.3 * b.y;
      c.x = spriteVelocity.x + direction * aimdirection.y * (Random(0) * 0.5 + 0.8);
      c.y = spriteVelocity.y - direction * aimdirection.x * (Random(0) * 0.5 + 0.8);
      createspark(a, c, 66, num, 255); // shell
    }
#endif
  }
  if (weapon.num == steyraug_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::steyraug_fire, spritePartsPos);
    }
    if (!col)
    {
      createspark(a, c, 69, num, 255); // shell
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position == pos_stand) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::SmallRecoil, 1);
    }
    if (position == pos_crouch)
    {
      if (bodyanimation.id == AnimationType::HandSupAim)
      {
        bodyapplyanimation(AnimationType::HandSupRecoil, 1);
      }
      else
      {
        bodyapplyanimation(AnimationType::AimRecoil, 1);
      }
    }
  }
  if (weapon.num == barrett_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::barretm82_fire, spritePartsPos);
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (bodyanimation.id != AnimationType::GetUp) &&
        (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::Barret, 1);
    }
#ifndef SERVER
    if (!col)
    {
      createspark(a, c, 71, num, 255); // shell
    }
#endif
  }
  if (weapon.num == minigun_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::minigun_fire, spritePartsPos);
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position == pos_stand) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::SmallRecoil, 2);
    }
#ifndef SERVER
    if (!col)
    {
      createspark(a, c, 73, num, 255); // shell
    }
#endif
  }
  if (weapon.num == colt_num)
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::colt1911_fire, spritePartsPos);
    }
    a.x = skeleton.pos[15].x + 2 - 0.2 * b.x;
    a.y = skeleton.pos[15].y - 2 - 0.2 * b.y;
    if (!col)
    {
      createspark(a, c, 65, num, 255); // shell
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position == pos_stand) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::SmallRecoil, 1);
    }
    if (position == pos_crouch)
    {
      if (bodyanimation.id == AnimationType::HandSupAim)
      {
        bodyapplyanimation(AnimationType::HandSupRecoil, 1);
      }
      else
      {
        bodyapplyanimation(AnimationType::AimRecoil, 1);
      }
    }
  }
  if ((weapon.num == bow_num) || (weapon.num == bow2_num))
  {
#ifndef SERVER
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::bow_fire, spritePartsPos);
    }
#endif
    if ((bodyanimation.id != AnimationType::Throw) && (position == pos_stand) &&
        (bodyanimation.id != AnimationType::GetUp) && (bodyanimation.id != AnimationType::Melee))
    {
      bodyapplyanimation(AnimationType::SmallRecoil, 1);
    }
    if (position == pos_crouch)
    {
      if (bodyanimation.id == AnimationType::HandSupAim)
      {
        bodyapplyanimation(AnimationType::HandSupRecoil, 1);
      }
      else
      {
        bodyapplyanimation(AnimationType::AimRecoil, 1);
      }
    }
  }
#ifndef SERVER
  if (weapon.num == law_num)
  {
    if (bonusstyle != bonus_predator)
    {
      playsound(SfxEffect::law, spritePartsPos);
    }
  }

  // smoke from muzzle
  muzzlesmokevector = b;
  vec2scale(muzzlesmokevector, muzzlesmokevector, 0.5);
  a = vec2add(a, muzzlesmokevector);
  vec2scale(muzzlesmokevector, muzzlesmokevector, 0.2);
  createspark(a, muzzlesmokevector, 35, num, 10);
#endif

  if (burstcount < 255)
  {
    burstcount += 1;
  }

    // TODO(skoskav): Make bink and self-bink sprite-specific so bots can also use it
#ifndef SERVER
  if (sprite_system.IsPlayerSprite(num))
  {
    // Increase self-bink for next shot
    if (weapon.bink < 0)
    {
      if ((legsanimation.id == AnimationType::Crouch) ||
          (legsanimation.id == AnimationType::CrouchRun) ||
          (legsanimation.id == AnimationType::CrouchRunBack) ||
          (legsanimation.id == AnimationType::Prone) ||
          (legsanimation.id == AnimationType::ProneMove))
      {
        hitspraycounter = calculatebink(hitspraycounter, round((float)(-weapon.bink) / 2));
      }
      else
      {
        hitspraycounter = calculatebink(hitspraycounter, -weapon.bink);
      }
    }
  }

  // Screen shake
  if (((sprite_system.IsPlayerSpriteValid()) && (camerafollowsprite != 0)) and
      ((sprite_system.IsPlayerSprite(num)) || CVar::cl_screenshake))
  {
    if (weapon.num != chainsaw_num)
    {
      if (GS::GetGame().pointvisible(spritePartsPos.x, spritePartsPos.y, camerafollowsprite))
      {
        if ((weapon.num == m249_num) || (weapon.num == spas12_num) || (weapon.num == barrett_num) ||
            (weapon.num == minigun_num))
        {
          camerax = camerax - 3 + Random(7);
          cameray = cameray - 3 + Random(7);
        }
        else
        {
          camerax = camerax - 1 + Random(3);
          cameray = cameray - 1 + Random(3);
        }
      }
    }
  }

  // Recoil!
  if (sprite_system.IsPlayerSprite(num))
  {
    rc = (float)(burstcount) / 10.f;
    rc = rc * (float)weapon.recoil;

    // less recoil on crouch
    if (onground)
    {
      if ((legsanimation.id == AnimationType::Crouch) && (legsanimation.currframe > 13))
      {
        rc = rc / 2;
      }

      if ((legsanimation.id == AnimationType::Prone) && (legsanimation.currframe > 23))
      {
        rc = rc / 3;
      }
    }

    if (rc > 0)
    {
      rc = sin(degtorad(((float)(weapon.speed * weapon.fireinterval) / 364) * rc));

      calculaterecoil(gamewidthhalf - camerax + spritePartsPos.x,
                      gameheighthalf - cameray + spritePartsPos.y, mx, my, -(pi / 1) * rc);
    }
  }
#endif
}

template <Config::Module M>
void Sprite<M>::throwflag()
{
  auto &sprite_system = SpriteSystem::Get();
  std::int32_t i;
  std::int32_t j;
  tvector2 b;
  float d = 0.0;
  tvector2 cursordirection;
  tvector2 boffset;
  tvector2 bperp;
  tvector2 lookpoint1;
  tvector2 lookpoint2;
  tvector2 lookpoint3;
  tvector2 newposdiff;
  tvector2 futurepoint1;
  tvector2 futurepoint2;
  tvector2 futurepoint3;
  tvector2 futurepoint4;
  auto &map = GS::GetGame().GetMap();
  auto &spriteVelocity = sprite_system.GetVelocity(num);
  auto &things = GS::GetThingSystem().GetThings();

  if ((bodyanimation.id != AnimationType::Roll) && (bodyanimation.id != AnimationType::RollBack))
  {
    if (control.flagthrow)
    {
      if (holdedthing > 0)
      {
        for (i = 1; i <= max_things; i++)
        {
          auto &thing = things[i];
          if (thing.holdingsprite == num)
          {
            if (thing.style < 4)
            {
              // Create start velocity vector
              cursordirection = getcursoraimdirection();
              vec2scale(cursordirection, cursordirection, flagthrow_power);

              // FIXME: Offset it away from the player so it isn't instantly
              // re-grabbed, it makes it look like lag though
              vec2scale(boffset, cursordirection, 5);

              // Add velocity
              b = vec2add(cursordirection, spriteVelocity);

              // Don't throw if the flag would collide in the upcoming frame
              newposdiff = vec2add(boffset, b);
              lookpoint1 = vec2add(thing.skeleton.pos[1], newposdiff);

              futurepoint1 = vec2add(lookpoint1, vector2(-10, -8));
              futurepoint2 = vec2add(lookpoint1, vector2(10, -8));
              futurepoint3 = vec2add(lookpoint1, vector2(-10, 8));
              futurepoint4 = vec2add(lookpoint1, vector2(10, 8));

              lookpoint1 = vec2add(thing.skeleton.pos[2], newposdiff);
              lookpoint2 = vec2add(thing.skeleton.pos[3], newposdiff);
              lookpoint3 = vec2add(thing.skeleton.pos[4], newposdiff);

              if (!map.raycast(skeleton.pos[15], lookpoint1, d, 200, false, true, false) and
                  !map.raycast(skeleton.pos[15], lookpoint2, d, 200, false, true, false) and
                  !map.raycast(skeleton.pos[15], lookpoint3, d, 200, false, true, false) and
                  !map.collisiontest(futurepoint1, bperp, true) and
                  !map.collisiontest(futurepoint2, bperp, true) and
                  !map.collisiontest(futurepoint3, bperp, true) and
                  !map.collisiontest(futurepoint4, bperp, true))
              {
                for (j = 1; j <= 4; j++)
                {
                  // Apply offset from flagger
                  thing.skeleton.pos[j] = vec2add(thing.skeleton.pos[j], boffset);

                  // Apply velocities
                  thing.skeleton.pos[j] = vec2add(thing.skeleton.pos[j], b);
                  thing.skeleton.oldpos[j] = vec2subtract(thing.skeleton.pos[j], b);
                }

                // Add some spin for visual effect
                bperp = vector2(-b.y, b.x);
                vec2normalize(bperp, bperp);
                vec2scale(bperp, bperp, direction);
                thing.skeleton.pos[1] = vec2subtract(thing.skeleton.pos[1], bperp);
                thing.skeleton.pos[2] = vec2add(thing.skeleton.pos[2], bperp);

                // Release the flag
                thing.holdingsprite = 0;
                holdedthing = 0;
                flaggrabcooldown = second / 4;

#ifdef SCRIPT
                scrptdispatcher.onflagdrop(num, thing.style, true);
#endif

                thing.bgstate.backgroundstatus = background_transition;
                thing.bgstate.backgroundpoly = background_poly_unknown;

                thing.statictype = false;
#ifdef SERVER
                serverthingmustsnapshot(i);
#endif
              }
            }
          }
        }
      }
    }
  }
}

template <Config::Module M>
void Sprite<M>::throwgrenade()
{
  auto &sprite_system = SpriteSystem::Get();
  tvector2 a;
  tvector2 b;
  tvector2 c;
  tvector2 e;
  float f = 0.0;
  float grenadearcsize;
  float grenadearcx;
  float grenadearcy;
  tvector2 playervelocity;
  auto &map = GS::GetGame().GetMap();
  auto &spriteVelocity = sprite_system.GetVelocity(num);
  auto &guns = GS::GetWeaponSystem().GetGuns();

  // Start throw animation
  if (!control.thrownade)
  {
    grenadecanthrow = true;
  }

  if (grenadecanthrow && control.thrownade && (bodyanimation.id != AnimationType::Roll) &&
      (bodyanimation.id != AnimationType::RollBack))
  {
    bodyapplyanimation(AnimationType::Throw, 1);
#ifndef SERVER
    setsoundpaused(reloadsoundchannel, true);
#endif
  }

#ifndef SERVER
  // Pull pin
  if ((bodyanimation.id == AnimationType::Throw) && (bodyanimation.currframe == 15) &&
      (tertiaryweapon.ammocount > 0) && (ceasefirecounter < 0))
  {
    b = gethandsaimdirection();
    vec2scale(b, b, (float)(bodyanimation.currframe) / guns[fraggrenade].speed);
    if (bodyanimation.currframe < 24)
    {
      vec2scale(b, b, 0.65);
    }
    b = vec2add(b, spriteVelocity);
    a.x = skeleton.pos[15].x + b.x * 3;
    a.y = skeleton.pos[15].y - 2 + b.y * 3;
    if (!map.collisiontest(a, c))
    {
      b = gethandsaimdirection();
      b.x = b.x * 0.5;
      b.y = b.y + 0.4;
      createspark(a, b, 30, num, 255); // Pin
      playsound(SfxEffect::grenade_pullout, a);
    }
  }
#endif

  auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);

  if ((bodyanimation.id == AnimationType::Throw) &&
      (!control.thrownade || (bodyanimation.currframe == 36)))
  {
    // Grenade throw
    if ((bodyanimation.currframe > 14) && (bodyanimation.currframe < 37) &&
        (tertiaryweapon.ammocount > 0) && (ceasefirecounter < 0))
    {
      b = getcursoraimdirection();

      // Add a few degrees of arc to the throw. The arc approaches zero as you aim up or down
      grenadearcsize = (float)(sign(b.x)) / 8 * (1 - fabs(b.y));
      grenadearcx = sin(b.y * pi / 2) * grenadearcsize;
      grenadearcy = sin(b.x * pi / 2) * grenadearcsize;
      b.x = b.x + grenadearcx;
      b.y = b.y - grenadearcy;
      vec2normalize(b, b);

      vec2scale(b, b, (float)(bodyanimation.currframe) / guns[fraggrenade].speed);
      if (bodyanimation.currframe < 24)
      {
        vec2scale(b, b, 0.65);
      }

      vec2scale(playervelocity, spriteVelocity, guns[fraggrenade].inheritedvelocity);

      b = vec2add(b, playervelocity);
      a.x = skeleton.pos[15].x + b.x * 3;
      a.y = skeleton.pos[15].y - 2 + (b.y * 3);
      e.x = spritePartsPos.x;
      e.y = spritePartsPos.y - 12;
      if (!map.collisiontest(a, c) and
          !map.raycast(e, a, f, 50, false, false, true, false, player->team))
      {
        createbullet(a, b, tertiaryweapon.num, num, 255, guns[fraggrenade].hitmultiply, true,
                     false);

        bool is_current_sprite = false;
          
        if constexpr(Config::IsClient(M))
        {
          is_current_sprite = ((player->controlmethod == human) && (TSpriteSystem<Sprite<M>>::Get().IsPlayerSprite(num)));
        }

        if (is_current_sprite || (player->controlmethod == bot))
        {
          tertiaryweapon.ammocount -= 1;
        }

#ifndef SERVER
        if ((sprite_system.IsPlayerSprite(num)) && (guns[fraggrenade].bink < 0))
        {
          hitspraycounter = calculatebink(hitspraycounter, -guns[fraggrenade].bink);
      }

        playsound(SfxEffect::grenade_throw, a);
#endif
      }
    }

    if (control.thrownade)
    {
      grenadecanthrow = false;
    }

    if (weapon.ammocount == 0)
    {
      if (weapon.reloadtimecount > weapon.clipouttime)
      {
        bodyapplyanimation(AnimationType::ClipOut, 1);
      }
      if (weapon.reloadtimecount < weapon.clipouttime)
      {
        bodyapplyanimation(AnimationType::ClipIn, 1);
      }
      if ((weapon.reloadtimecount < weapon.clipintime) && (weapon.reloadtimecount > 0))
      {
        bodyapplyanimation(AnimationType::SlideBack, 1);
      }
#ifndef SERVER
      setsoundpaused(reloadsoundchannel, false);
#endif
    }
  }
}

template <Config::Module M>
auto Sprite<M>::getmoveacc() -> float
{
  float moveacc;

  float result;
  result = 0;

  // No moveacc for bots on harder difficulties
  bool bots_difficulty_low = true;
  if constexpr (Config::IsServer(M))
  {
    bots_difficulty_low = CVar::bots_difficulty < 50;
  }
  if (player->controlmethod == bot && bots_difficulty_low)
  {
    moveacc = 0;
  }
  else
  {
    moveacc = weapon.movementacc;
  }

  if (moveacc > 0)
  {
    if ((control.jetpack && (jetscount > 0)) || (legsanimation.id == AnimationType::Jump) ||
        (legsanimation.id == AnimationType::JumpSide) || (legsanimation.id == AnimationType::Run) ||
        (legsanimation.id == AnimationType::RunBack) || (legsanimation.id == AnimationType::Roll) ||
        (legsanimation.id == AnimationType::RollBack))
    {
      result = moveacc * 7;
    }
    else if ((!ongroundpermanent && (legsanimation.id != AnimationType::Prone) &&
              (legsanimation.id != AnimationType::ProneMove) &&
              (legsanimation.id != AnimationType::Crouch) &&
              (legsanimation.id != AnimationType::CrouchRun) &&
              (legsanimation.id != AnimationType::CrouchRunBack)) ||
             (legsanimation.id == AnimationType::GetUp) ||
             ((legsanimation.id == AnimationType::Prone) &&
              (legsanimation.currframe < legsanimation.numframes)))
    {
      result = moveacc * 3;
    }
  }
  return result;
}

template <Config::Module M>
auto Sprite<M>::getcursoraimdirection() -> tvector2
{
  tvector2 mouseaim;

  mouseaim.x = control.mouseaimx;
  mouseaim.y = control.mouseaimy;

  tvector2 aimdirection = vec2subtract(mouseaim, skeleton.pos[15]);
  vec2normalize(aimdirection, aimdirection);

  return aimdirection;
}

template <Config::Module M>
auto Sprite<M>::gethandsaimdirection() -> tvector2
{
  tvector2 aimdirection = vec2subtract(skeleton.pos[15], skeleton.pos[16]);
  vec2normalize(aimdirection, aimdirection);

  return aimdirection;
}

template <Config::Module M>
auto Sprite<M>::issolo() -> bool
{
  return player->team == team_none;
}

template <Config::Module M>
auto Sprite<M>::isnotsolo() -> bool
{
  return player->team != team_none;
}

template <Config::Module M>
auto Sprite<M>::isinteam() -> bool
{
  bool result;
  switch (player->team)
  {
  case team_alpha:
  case team_bravo:
  case team_charlie:
  case team_delta:
    result = true;
    break;
  default:
    result = false;
  }
  return result;
}

template <Config::Module M>
auto Sprite<M>::isspectator() -> bool
{
  return player->team == team_spectator;
}

template <Config::Module M>
auto Sprite<M>::isinsameteam(const Sprite &otherplayer) -> bool
{
  return player->team == otherplayer.player->team;
}

template <Config::Module M>
auto Sprite<M>::isnotinsameteam(const Sprite &otherplayer) -> bool
{
  return player->team != otherplayer.player->team;
}

template <Config::Module M>
auto Sprite<M>::canrespawn(bool deadmeatbeforerespawn) -> bool
{
  bool result;
  result = (CVar::sv_survivalmode == false) or (GS::GetGame().GetSurvivalEndRound()) or
           (!deadmeatbeforerespawn);
  return result;
}

template <Config::Module M>
void Sprite<M>::SetHealth(float health)
{
  Health = health;
}

template <Config::Module M>
auto Sprite<M>::GetHealth() -> float
{
  // LogDebugG("Health {}", Health);
  return Health;
}

template <Config::Module M>
void Sprite<M>::SetFirstWeapon(const tgun &gun)
{
  new (&weapon) tgun(gun);
}

template <Config::Module M>
void Sprite<M>::SetSecondWeapon(const tgun &gun)
{
  new (&secondaryweapon) tgun(gun);
}

template <Config::Module M>
void Sprite<M>::SetThirdWeapon(const tgun &gun)
{
  new (&tertiaryweapon) tgun(gun);
}

template <Config::Module M>
void Sprite<M>::CopyOldSpritePos()
{
  ZoneScopedN("CopyOldSpritePos");
  // Ping Impr
  auto &sprite_system = SpriteSystem::Get();
  for (auto i = max_oldpos; i >= 1; i--)
  {
    oldspritepos[i] = oldspritepos[i - 1];
  }

  auto &spritePartsPos = sprite_system.GetSpritePartsPos(num);

  oldspritepos[0] = spritePartsPos;
}

template <Config::Module M>
auto Sprite<M>::GetOldSpritePos(int32_t idx) -> tvector2
{
  auto i = max_oldpos - 1;
  if (idx < max_oldpos)
  {
    i = idx;
  }
  return oldspritepos[i];
}

template class Sprite<Config::GetModule()>;
template bool teamcollides(PolygonType polytype, std::int32_t team, const bool bullet);
template class BackgroundState<>;

#include <doctest.h>

class SpritesFixture
{
public:
  SpritesFixture()
  {
    GS::Init();
    AnimationSystem::Get().LoadAnimObjects("");
  }
  ~SpritesFixture()
  {
    GS::Deinit();
  }
  SpritesFixture(const SpritesFixture &) = delete;
};

TEST_CASE_FIXTURE(SpritesFixture, "CreateSprite")
{
  tvector2 spos; // out
  std::uint8_t spriteId = 255;
  auto player = std::make_shared<tplayer>();
  auto retSpriteId = createsprite(spos, spriteId, player);
  CHECK(retSpriteId == 1);
}

TEST_CASE_FIXTURE(SpritesFixture, "CreateSpriteCheckDefaultValues")
{
  auto &sprite_system = SpriteSystem::Get();
  tvector2 spos; // out
  std::uint8_t spriteId = 255;
  auto player = std::make_shared<tplayer>();
  {
    // make sprite dirty
    spriteId = createsprite(spos, spriteId, player);
    auto &sprite = sprite_system.GetSprite(spriteId);

    CHECK(spriteId == 1);
    sprite.active = false;
    sprite.player = nullptr;
    sprite.deadmeat = true;
  }
  auto retSpriteId = createsprite(spos, spriteId, player);
  const auto &sprite = sprite_system.GetSprite(retSpriteId);

  CHECK(retSpriteId == 1);
  CHECK(sprite.style == tsprite::Style::Default);
  CHECK(sprite.active == true);
  CHECK(sprite.deadmeat == false);
  CHECK(sprite.respawncounter == 0);
  CHECK(sprite.alpha == 255);
  CHECK(sprite.brain.pissedoff == 0);
  CHECK(sprite.vest == 0);
  CHECK(sprite.bonusstyle == bonus_none);
  CHECK(sprite.bonustime == 0);
  CHECK(sprite.multikills == 0);
  CHECK(sprite.multikilltime == 0);
  CHECK(sprite.hascigar == 0);
  CHECK(sprite.idletime == default_idletime);
  CHECK(sprite.idlerandom == -1);
  CHECK(sprite.position == pos_stand);
  // CHECK(sprite.bodyanimation == anim.GetAnimation(AnimationType::Stand));
  // CHECK(sprite.legsanimation == anim.GetAnimation(AnimationType::Stand));
  CHECK(sprite.onfire == 0);
  CHECK(sprite.holdedthing == 0);
  CHECK(sprite.selweapon == 0);
  CHECK(sprite.stat == 0);
}
