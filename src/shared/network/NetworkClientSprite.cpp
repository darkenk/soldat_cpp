// automatically converted

#include "NetworkClientSprite.hpp"
#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/Sound.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../GameStrings.hpp"
#include "NetworkClient.hpp"
#include "NetworkUtils.hpp"
#include "common/Console.hpp"
#include "common/Logging.hpp"
#include "common/gfx.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/misc/GlobalSystems.hpp"

namespace
{
tmsg_clientspritesnapshot oldclientsnapshotmsg;
tmsg_clientspritesnapshot_mov oldclientsnapshotmovmsg;
} // namespace

void clienthandleserverspritesnapshot(NetworkContext *netmessage)
{

  if (!verifypacket(sizeof(tmsg_serverspritesnapshot), netmessage->size,
                    msgid_serverspritesnapshot))
  {
    return;
  }

  auto *spritesnap = pmsg_serverspritesnapshot(netmessage->packet);
  auto &things = GS::GetThingSystem().GetThings();

  // assign received sprite info to sprite
  const std::int32_t i = spritesnap->num;

  if ((i < 1) || (i > max_sprites))
  {
    return;
  }
  auto& sprite = SpriteSystem::Get().GetSprite(i);
  if (!sprite.active)
  {
    return;
  }

  clienttickcount = spritesnap->serverticks;
  lastheartbeatcounter = spritesnap->serverticks;

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(i);
  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(i);

  // CLIENT RESPAWN
  if (sprite.deadmeat)
  {
    SpriteSystem::Get().SetSpritePartsOldPos(i, spritePartsPos);
    spritePartsPos = spritesnap->pos;

    spriteVelocity = spritesnap->velocity;
    sprite.respawn();
    sprite.olddeadmeat = sprite.deadmeat;
    spritePartsPos = spritesnap->pos;
  }

  sprite.deadmeat = false;

  if (i != mysprite)
  {
    if (sprite.GetHealth() == spritesnap->health)
    {
      SpriteSystem::Get().SetSpritePartsOldPos(i, spritePartsPos);
      spritePartsPos = spritesnap->pos;
      spriteVelocity = spritesnap->velocity;
    }

    sprite.control.mouseaimy = spritesnap->mouseaimy;
    sprite.control.mouseaimx = spritesnap->mouseaimx;

    decodekeys(sprite, spritesnap->keys16);

    if (sprite.weapon.num != spritesnap->weaponnum)
    {
      sprite.applyweaponbynum(spritesnap->weaponnum, 1);
    }
    if (sprite.secondaryweapon.num != spritesnap->secondaryweaponnum)
    {
      sprite.applyweaponbynum(spritesnap->secondaryweaponnum, 2);
    }
    sprite.weapon.ammocount = spritesnap->ammocount;

    if (sprite.weapon.num == knife_num)
    {
      sprite.player->secwep = 1;
    }
    if (sprite.weapon.num == chainsaw_num)
    {
      sprite.player->secwep = 2;
    }
    if (sprite.weapon.num == law_num)
    {
      sprite.player->secwep = 3;
    }

    // Toggle prone if it was activated or deactivated
    sprite.control.prone =
      (spritesnap->position == pos_prone) ^
      (sprite.position == pos_prone);
  }

  // kill the bow
  if ((sprite.weapon.num == bow_num) || (sprite.weapon.num == bow2_num))
  {
    for (std::int32_t j = 1; j <= max_things; j++)
    {
      if ((things[j].active) && (things[j].style == object_rambo_bow))
      {
        gamethingtarget = 0;
        things[j].kill();
      }
    }
  }

  sprite.wearhelmet = 1;
  if ((spritesnap->look & B1) == B1)
  {
    sprite.wearhelmet = 0;
  }
  if ((spritesnap->look & B4) == B4)
  {
    sprite.wearhelmet = 2;
  }
  if ((sprite.bodyanimation.id != AnimationType::Cigar) &&
      (sprite.bodyanimation.id != AnimationType::Smoke) &&
      ((sprite.idlerandom != 1) || (sprite.bodyanimation.id != AnimationType::Stand)))
  {
    sprite.hascigar = 0;
    if ((spritesnap->look & B2) == B2)
    {
      sprite.hascigar = 5;
    }
    if ((spritesnap->look & B3) == B3)
    {
      sprite.hascigar = 10;
    }
  }

  sprite.tertiaryweapon.ammocount = spritesnap->grenadecount;

  // LogDebugG("sprite: {} grenade {}", i, spritesnap->grenadecount);

  sprite.SetHealth(spritesnap->health);
  sprite.vest = spritesnap->vest;
  if (sprite.vest > defaultvest)
  {
    sprite.vest = defaultvest;
  }

  if (i == mysprite)
  {
    if (!targetmode)
    {
      camerafollowsprite = mysprite;
      sprite.player->camera = mysprite;
    }
  }
}

void clienthandleserverspritesnapshot_major(NetworkContext *netmessage)
{

  if (!verifypacket(sizeof(tmsg_serverspritesnapshot_major), netmessage->size,
                    msgid_serverspritesnapshot_major))
  {
    return;
  }

  auto *spritesnapmajor = pmsg_serverspritesnapshot_major(netmessage->packet);

  // assign received sprite info to sprite
  std::int32_t i = spritesnapmajor->num;

  if ((i < 1) || (i > max_sprites))
  {
    return;
  }

  auto& sprite = SpriteSystem::Get().GetSprite(i);
  if (!sprite.active)
  {
    LogWarn("network", "[ClientSprite] Warning: Received snapshot for inactive player {}", i);
    return;
  }

  clienttickcount = spritesnapmajor->serverticks;
  lastheartbeatcounter = spritesnapmajor->serverticks;

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(i);
  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(i);

  // CLIENT RESPAWN
  if (sprite.deadmeat)
  {
    SpriteSystem::Get().SetSpritePartsOldPos(i, spritePartsPos);
    spritePartsPos = spritesnapmajor->pos;
    spriteVelocity = spritesnapmajor->velocity;
    sprite.respawn();
    sprite.olddeadmeat = sprite.deadmeat;
    spritePartsPos = spritesnapmajor->pos;
  }

  sprite.deadmeat = false;

  if (i != mysprite)
  {
    if (sprite.GetHealth() == spritesnapmajor->health)
    {
      SpriteSystem::Get().SetSpritePartsOldPos(i, spritePartsPos);
      spritePartsPos = spritesnapmajor->pos;
      spriteVelocity = spritesnapmajor->velocity;
    }

    sprite.control.mouseaimy = spritesnapmajor->mouseaimy;
    sprite.control.mouseaimx = spritesnapmajor->mouseaimx;

    decodekeys(sprite, spritesnapmajor->keys16);

    // Toggle prone if it was activated or deactivated
    sprite.control.prone =
      (spritesnapmajor->position == pos_prone) ^
      (sprite.position == pos_prone);
  }

  // kill the bow
  auto &things = GS::GetThingSystem().GetThings();
  if ((sprite.weapon.num == bow_num) || (sprite.weapon.num == bow2_num))
  {
    for (std::int32_t j = 1; j <= max_things; j++)
    {
      if ((things[j].active) && (things[j].style == object_rambo_bow))
      {
        gamethingtarget = 0;
        things[j].kill();
      }
    }
  }

  sprite.SetHealth(spritesnapmajor->health);

  if (i == mysprite)
  {
    if (!targetmode)
    {
      camerafollowsprite = mysprite;
      sprite.player->camera = mysprite;
    }
  }
}

void clienthandleserverskeletonsnapshot(NetworkContext *netmessage)
{

  if (!verifypacket(sizeof(tmsg_serverskeletonsnapshot), netmessage->size,
                    msgid_serverskeletonsnapshot))
  {
    return;
  }

  auto *skeletonsnap = pmsg_serverskeletonsnapshot(netmessage->packet);

  // assign received Skeleton info to skeleton
  std::int32_t i = skeletonsnap->num;

  if ((i < 1) || (i > max_sprites))
  {
    return;
  }
  auto& sprite = SpriteSystem::Get().GetSprite(i);
  if (!sprite.active)
  {
    return;
  }

  auto &guns = GS::GetWeaponSystem().GetGuns();

  sprite.deadmeat = true;
  sprite.respawncounter = skeletonsnap->respawncounter;
  sprite.SetFirstWeapon(guns[noweapon]);
}

void clientspritesnapshot()
{
  tmsg_clientspritesnapshot clientmsg;

  clientmsg.header.id = msgid_clientspritesnapshot;

  clientmsg.ammocount = SpriteSystem::Get().GetSprite(mysprite).weapon.ammocount;
  clientmsg.secondaryammocount = SpriteSystem::Get().GetSprite(mysprite).secondaryweapon.ammocount;
  clientmsg.weaponnum = SpriteSystem::Get().GetSprite(mysprite).weapon.num;
  clientmsg.secondaryweaponnum = SpriteSystem::Get().GetSprite(mysprite).secondaryweapon.num;
  clientmsg.position = SpriteSystem::Get().GetSprite(mysprite).position;

  if ((clientmsg.ammocount == oldclientsnapshotmsg.ammocount) &&
      (clientmsg.weaponnum == oldclientsnapshotmsg.weaponnum) &&
      (clientmsg.secondaryweaponnum == oldclientsnapshotmsg.secondaryweaponnum) &&
      (clientmsg.position == oldclientsnapshotmsg.position))
  {
    return;
  }

  oldclientsnapshotmsg = clientmsg;

  GetNetwork()->SendData(&clientmsg, sizeof(clientmsg), false);
}

// CLIENT SPRITE SNAPSHOT MOV
void clientspritesnapshotmov()
{
  tmsg_clientspritesnapshot_mov clientmsg;

  clientmsg.header.id = msgid_clientspritesnapshot_mov;

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(mysprite);
  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(mysprite);

  clientmsg.pos = spritePartsPos;
  clientmsg.velocity = spriteVelocity;
  clientmsg.mouseaimx = SpriteSystem::Get().GetSprite(mysprite).control.mouseaimx;
  clientmsg.mouseaimy = SpriteSystem::Get().GetSprite(mysprite).control.mouseaimy;

  encodekeys(SpriteSystem::Get().GetSprite(mysprite), clientmsg.keys16);

  if (SpriteSystem::Get().GetSprite(mysprite).dontdrop)
  {
    clientmsg.keys16 = clientmsg.keys16 & ~B9;
  }

  tvector2 posdiff = vec2subtract(clientmsg.pos, oldclientsnapshotmovmsg.pos);
  tvector2 veldiff = vec2subtract(clientmsg.velocity, oldclientsnapshotmovmsg.velocity);

  if ((vec2length(posdiff) > posdelta) || (vec2length(veldiff) > veldelta) ||
      (clientmsg.keys16 != oldclientsnapshotmovmsg.keys16) || ((clientmsg.keys16 & B6) == B6) ||
      (((SpriteSystem::Get().GetSprite(mysprite).weapon.fireinterval > fireinterval_net) ||
        (SpriteSystem::Get().GetSprite(mysprite).weapon.ammocount <= 0) ||
        (round(mx) != oldclientsnapshotmovmsg.mouseaimx) ||
        (round(my) != oldclientsnapshotmovmsg.mouseaimy)) &&
       ((fabs(mx - oldclientsnapshotmovmsg.mouseaimx) >= mouseaimdelta) ||
        (fabs(my - oldclientsnapshotmovmsg.mouseaimy) >= mouseaimdelta))))
  {
    oldclientsnapshotmovmsg = clientmsg;
    oldclientsnapshotmovmsg.mouseaimx = round(mx);
    oldclientsnapshotmovmsg.mouseaimy = round(my);

    GetNetwork()->SendData(&clientmsg, sizeof(clientmsg), false);
  }
}

// CLIENT SPRITE SNAPSHOT DEAD
void clientspritesnapshotdead()
{
  tmsg_clientspritesnapshot_dead clientmsg;

  clientmsg.header.id = msgid_clientspritesnapshot_dead;
  clientmsg.camerafocus = camerafollowsprite;

  GetNetwork()->SendData(&clientmsg, sizeof(clientmsg), false);
}

static auto sConvertKillBulletToGFX(const std::uint8_t killbullet) -> std::int32_t
{
  switch (killbullet)
  {
  case 0:
    return GFX::INTERFACE_GUNS_SOCOM;
  case 1:
    return GFX::INTERFACE_GUNS_DEAGLES;
  case 2:
    return GFX::INTERFACE_GUNS_MP5;
  case 3:
    return GFX::INTERFACE_GUNS_AK74;
  case 4:
    return GFX::INTERFACE_GUNS_STEYR;
  case 5:
    return GFX::INTERFACE_GUNS_SPAS;
  case 6:
    return GFX::INTERFACE_GUNS_RUGER;
  case 7:
    return GFX::INTERFACE_GUNS_M79;
  case 8:
    return GFX::INTERFACE_GUNS_BARRETT;
  case 9:
    return GFX::INTERFACE_GUNS_MINIMI;
  case 10:
    return GFX::INTERFACE_GUNS_MINIGUN;
  case 205:
    return GFX::INTERFACE_GUNS_FLAMER;
  case 206:
    return GFX::INTERFACE_GUNS_FIST;
  case 207:
    return GFX::INTERFACE_GUNS_BOW;
  case 208:
    return GFX::INTERFACE_GUNS_BOW;
  case 210:
    return GFX::INTERFACE_CLUSTER_NADE;
  case 211:
    return GFX::INTERFACE_GUNS_KNIFE;
  case 212:
    return GFX::INTERFACE_GUNS_CHAINSAW;
  case 222:
    return GFX::INTERFACE_NADE;
  case 224:
    return GFX::INTERFACE_GUNS_LAW;
  case 225:
    return GFX::INTERFACE_GUNS_M2;
  default:
    return -255;
  }
}

template<Config::Module M>
static void sFillPos(Sprite<M> &sprite, const tmsg_spritedeath *deathsnap)
{
  for (std::int32_t d = 0; d < 16; d++)
  {
    const auto &pos = deathsnap->pos[d];
    const auto &oldPos = deathsnap->oldpos[d];
    if ((round(pos.x) != 0) && (round(pos.y) != 0) && (round(oldPos.x) != 0) &&
        (round(oldPos.y) != 0))
    {
      sprite.skeleton.SetPos(d, pos);
      sprite.skeleton.SetOldPos(d, oldPos);

      if (d == 0)
      {
        sprite.skeleton.SetPos(16, pos);
        sprite.skeleton.SetOldPos(16, oldPos);
      }
      if (d == 1)
      {
        sprite.skeleton.SetPos(17, pos);
        sprite.skeleton.SetOldPos(17, oldPos);
      }
      if (d == 14)
      {
        sprite.skeleton.SetPos(18, pos);
        sprite.skeleton.SetOldPos(18, oldPos);
      }
      if (d == 15)
      {
        sprite.skeleton.SetPos(19, pos);
        sprite.skeleton.SetOldPos(19, oldPos);
      }
    }
  }
}

void clienthandlespritedeath(NetworkContext *netmessage)
{
  std::int32_t j;
  tvector2 b;

  auto &map = GS::GetGame().GetMap();

  if (!verifypacket(sizeof(tmsg_spritedeath), netmessage->size, msgid_spritedeath))
  {
    return;
  }

  auto *deathsnap = pmsg_spritedeath(netmessage->packet);

  const std::int32_t i = deathsnap->num;

  if ((i < 1) || (i > max_sprites))
  {
    return;
  }
  auto& sprite = SpriteSystem::Get().GetSprite(i);
  if (!sprite.active)
  {
    return;
  }

  sFillPos(sprite, deathsnap);

  b.x = 0;
  b.y = 0;
  sprite.SetHealth(deathsnap->health);

  // death!
  if ((sprite.GetHealth() < 1) && (sprite.GetHealth() > headchopdeathhealth))
  {
    sprite.die(normal_death, deathsnap->killer, deathsnap->where, deathsnap->killbullet, b);
  }
  else if ((sprite.GetHealth() < (headchopdeathhealth + 1)) &&
           (sprite.GetHealth() > brutaldeathhealth))
  {
    sprite.die(headchop_death, deathsnap->killer, deathsnap->where, deathsnap->killbullet, b);
  }
  else if (sprite.GetHealth() < (brutaldeathhealth + 1))
  {
    sprite.die(brutal_death, deathsnap->killer, deathsnap->where, deathsnap->killbullet, b);
  }

  sprite.skeleton.constraints[2].active = true;
  sprite.skeleton.constraints[4].active = true;
  sprite.skeleton.constraints[20].active = true;
  sprite.skeleton.constraints[21].active = true;
  sprite.skeleton.constraints[23].active = true;
  if ((deathsnap->constraints & B1) == B1)
  {
    sprite.skeleton.constraints[2].active = false;
  }
  if ((deathsnap->constraints & B2) == B2)
  {
    sprite.skeleton.constraints[4].active = false;
  }
  if ((deathsnap->constraints & B3) == B3)
  {
    sprite.skeleton.constraints[20].active = false;
  }
  if ((deathsnap->constraints & B4) == B4)
  {
    sprite.skeleton.constraints[21].active = false;
  }
  if ((deathsnap->constraints & B5) == B5)
  {
    sprite.skeleton.constraints[23].active = false;
  }

  auto &guns = GS::GetWeaponSystem().GetGuns();

  sprite.SetFirstWeapon(guns[noweapon]);
  sprite.respawncounter = deathsnap->respawncounter;
  sprite.onfire = deathsnap->onfire;

  // mulitkill count
  if (deathsnap->killer != i)
  {
    SpriteSystem::Get().GetSprite(deathsnap->killer).multikilltime = multikillinterval;
    SpriteSystem::Get().GetSprite(deathsnap->killer).multikills += 1;
  }

  if (i == mysprite)
  {
    bigmessage(
      wideformat(_("Killed by {}"), SpriteSystem::Get().GetSprite(deathsnap->killer).player->name),
      killmessagewait, die_message_color);
    if (!limbolock)
    {
      gamemenushow(limbomenu);
    }
    menutimer = menu_time;
    playsound(SfxEffect::playerdeath);
  }

  if (deathsnap->killer == mysprite)
  {
    bigmessage(wideformat(_("You killed {}"), sprite.player->name), killmessagewait,
               kill_message_color);

    if ((SpriteSystem::Get().GetSprite(deathsnap->killer).multikills > 1) &&
        (SpriteSystem::Get().GetSprite(deathsnap->killer).multikills < 18))
    {
      bigmessage(multikillmessage[SpriteSystem::Get().GetSprite(deathsnap->killer).multikills],
                 killmessagewait, kill_message_color);
    }
    if (SpriteSystem::Get().GetSprite(deathsnap->killer).multikills > 17)
    {
      bigmessage(multikillmessage[9], killmessagewait, kill_message_color);
    }

    if ((shotdistance > -1) && (deathsnap->killer != i))
    {
      shotdistanceshow = killmessagewait - 30;
      shotdistance = deathsnap->shotdistance;
      shotricochet = deathsnap->shotricochet;
      shotlife = deathsnap->shotlife;
    }
  }

  if ((deathsnap->killer == mysprite) && (i == mysprite))
  {
    bigmessage(_("You killed yourself"), killmessagewait, die_message_color);
  }

  // This k seems to go to the rendering code through KillConsole.NumMessage,
  // where it was used as IntTexture[11 + k] basically, so I'm replacing with
  // texture constants instead, without that 11 offset.
  // Similar code can be found in TSprite.Die (Sprites.pas)

  const std::int32_t k = sConvertKillBulletToGFX(deathsnap->killbullet);

  std::uint32_t col = 0;
  std::uint32_t col2 = 0;
  switch (SpriteSystem::Get().GetSprite(deathsnap->killer).player->team)
  {
  case team_none:
    col = killer_message_color;
    break;
  case team_alpha:
    col = alpha_k_message_color;
    break;
  case team_bravo:
    col = bravo_k_message_color;
    break;
  case team_charlie:
    col = charlie_k_message_color;
    break;
  case team_delta:
    col = delta_k_message_color;
    break;
  }
  switch (sprite.player->team)
  {
  case team_none:
    col2 = death_message_color;
    break;
  case team_alpha:
    col2 = alpha_d_message_color;
    break;
  case team_bravo:
    col2 = bravo_d_message_color;
    break;
  case team_charlie:
    col2 = charlie_d_message_color;
    break;
  case team_delta:
    col2 = delta_d_message_color;
    break;
  }

  if (deathsnap->killer != i)
  {
    GetKillConsole().ConsoleAdd(
      (SpriteSystem::Get().GetSprite(deathsnap->killer).player->name) + " (" +
        (inttostr(SpriteSystem::Get().GetSprite(deathsnap->killer).player->kills)) + ')',
      col, k);
    GetKillConsole().ConsoleAdd((sprite.player->name), col2, -255);
  }
  else
  {
    GetKillConsole().ConsoleAdd(
      (SpriteSystem::Get().GetSprite(deathsnap->killer).player->name) + " (" +
        (inttostr(SpriteSystem::Get().GetSprite(deathsnap->killer).player->kills)) + ')',
      spectator_d_message_color, k);
  }
  auto &bullet = GS::GetBulletSystem().GetBullets();

  // Explode - lag compensate
  if (deathsnap->killbullet == 7) // M79
  {
    for (j = max_bullets; j >= 1; j--)
    {
      auto &b = bullet[j];
      if ((b.active) && (b.owner == deathsnap->killer) && (b.style == guns[m79].bulletstyle))
      {
        GetBulletParts().oldpos[j] = sprite.skeleton.pos[8];
        GetBulletParts().pos[j] = sprite.skeleton.pos[8];
        b.hit(3);
        b.kill();
        break;
      }
    }
  }

  if (deathsnap->killbullet == 224) /*LAW*/
  {
    for (j = max_bullets; j >= 1; j--)
    {
      auto &b = bullet[j];
      if ((b.active) && (b.owner == deathsnap->killer) && (b.style == guns[law].bulletstyle))
      {
        GetBulletParts().oldpos[j] = sprite.skeleton.pos[8];
        GetBulletParts().pos[j] = sprite.skeleton.pos[8];
        b.hit(3);
        b.kill();
        break;
      }
    }
  }

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(i);
  if (deathsnap->killbullet == 222) /*grenade*/
  {
    float hm = 0.0;
    for (j = max_bullets; j >= 1; j--)
    {
      auto &b = bullet[j];
      if ((b.active) && (b.owner == deathsnap->killer) &&
          (b.style == guns[fraggrenade].bulletstyle))
      {
        map.raycast(GetBulletParts().pos[j], sprite.skeleton.pos[8], hm, 351);
        if (hm < after_explosion_radius)
        {
          GetBulletParts().oldpos[j] = spritePartsPos;
          GetBulletParts().pos[j] = spritePartsPos;
          b.hit(4);
          b.kill();
        }
      }
    }
  }
}

void clienthandledelta_movement(NetworkContext *netmessage)
{
  // a: TVector2;

  if (!verifypacket(sizeof(tmsg_serverspritedelta_movement), netmessage->size,
                    msgid_delta_movement))
  {
    return;
  }

  auto *deltamov = pmsg_serverspritedelta_movement(netmessage->packet);

  // Older than Heartbeat Drop the Packet
  if (!demoplayer.active() && (deltamov->servertick < lastheartbeatcounter))
  {
    return;
  }
  std::int32_t i = deltamov->num;

  if ((i < 1) || (i > max_sprites))
  {
    return;
  }

  auto& sprite = SpriteSystem::Get().GetSprite(i);
  if (!sprite.active)
  {
    return;
  }

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(i);
  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(i);
  spritePartsPos = deltamov->pos;
  spriteVelocity = deltamov->velocity;

  sprite.control.mouseaimy = deltamov->mouseaimy;
  sprite.control.mouseaimx = deltamov->mouseaimx;

  decodekeys(sprite, deltamov->keys16);
}

void clienthandledelta_mouseaim(NetworkContext *netmessage)
{
  tmsg_serverspritedelta_mouseaim *deltamouse;

  if (!verifypacket(sizeof(tmsg_serverspritedelta_mouseaim), netmessage->size,
                    msgid_delta_mouseaim))
  {
    return;
  }

  deltamouse = pmsg_serverspritedelta_mouseaim(netmessage->packet);

  std::int32_t i = deltamouse->num;
  if ((i < 1) || (i > max_sprites))
  {
    return;
  }

  auto& sprite = SpriteSystem::Get().GetSprite(i);
  if (!sprite.active)
  {
    return;
  }

  sprite.control.mouseaimy = deltamouse->mouseaimy;
  sprite.control.mouseaimx = deltamouse->mouseaimx;

  if (sprite.position == pos_prone)
  {
    sprite.bodyapplyanimation(AnimationType::Prone, 1);
  }
  else
  {
    sprite.bodyapplyanimation(AnimationType::Aim, 1);
  }

  sprite.weapon.fireintervalprev = 0;
  sprite.weapon.fireintervalcount = 0;
}

void clienthandledelta_weapons(NetworkContext *netmessage)
{

  if (!verifypacket(sizeof(tmsg_serverspritedelta_weapons), netmessage->size, msgid_delta_weapons))
  {
    return;
  }

  std::int32_t i = pmsg_serverspritedelta_weapons(netmessage->packet)->num;

  if ((i < 1) || (i > max_sprites))
  {
    return;
  }
  auto& sprite = SpriteSystem::Get().GetSprite(i);
  if (!sprite.active)
  {
    return;
  }

  sprite.applyweaponbynum(
    pmsg_serverspritedelta_weapons(netmessage->packet)->weaponnum, 1);
  sprite.applyweaponbynum(
    pmsg_serverspritedelta_weapons(netmessage->packet)->secondaryweaponnum, 2);
  sprite.weapon.ammocount =
    pmsg_serverspritedelta_weapons(netmessage->packet)->ammocount;

  if ((i == mysprite) && !SpriteSystem::Get().GetSprite(mysprite).deadmeat)
  {
    clientspritesnapshot();
  }
}

void clienthandledelta_helmet(NetworkContext *netmessage)
{

  if (!verifypacket(sizeof(tmsg_serverspritedelta_helmet), netmessage->size, msgid_delta_helmet))
  {
    return;
  }

  auto *deltahelmet = pmsg_serverspritedelta_helmet(netmessage->packet);

  std::int32_t i = deltahelmet->num;

  if ((i < 1) || (i > max_sprites))
  {
    return;
  }
  auto& sprite = SpriteSystem::Get().GetSprite(i);
  if (!sprite.active)
  {
    return;
  }

  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(i);

  // helmet chop
  if (deltahelmet->wearhelmet == 0)
  {
    createspark(sprite.skeleton.pos[12], spriteVelocity, 6, i, 198);
    playsound(SfxEffect::headchop, sprite.skeleton.pos[12]);
  }

  sprite.wearhelmet = deltahelmet->wearhelmet;
}

void clienthandleclientspritesnapshot_dead(NetworkContext *netmessage)
{
  if (!verifypacket(sizeof(tmsg_clientspritesnapshot_dead), netmessage->size,
                    msgid_clientspritesnapshot_dead))
  {
    return;
  }
  if (freecam == 0)
  {
    camerafollowsprite = pmsg_clientspritesnapshot_dead(netmessage->packet)->camerafocus;
  }
}

// TESTS
#include <doctest.h>

TEST_SUITE("NetworkClientSprite")
{
  TEST_CASE("Convert kill bullet to SOCOM")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 0;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_GUNS_SOCOM);
  }

  TEST_CASE("Convert kill bullet to DEAGLES")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 5;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_GUNS_DEAGLES + 4);
  }

  TEST_CASE("Convert kill bullet to FLAMER")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 205;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_GUNS_FLAMER);
  }

  TEST_CASE("Convert kill bullet to FIST")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 206;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_GUNS_FIST);
  }

  TEST_CASE("Convert kill bullet to BOW")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 207;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_GUNS_BOW);
  }

  TEST_CASE("Convert kill bullet to CLUSTER_NADE")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 210;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_CLUSTER_NADE);
  }

  TEST_CASE("Convert kill bullet to KNIFE")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 211;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_GUNS_KNIFE);
  }

  TEST_CASE("Convert kill bullet to CHAINSAW")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 212;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_GUNS_CHAINSAW);
  }

  TEST_CASE("Convert kill bullet to NADE")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 222;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_NADE);
  }

  TEST_CASE("Convert kill bullet to LAW")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 224;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_GUNS_LAW);
  }

  TEST_CASE("Convert kill bullet to M2")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 225;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == GFX::INTERFACE_GUNS_M2);
  }

  TEST_CASE("Convert kill bullet to unknown")
  {
    tmsg_spritedeath deathsnap;
    deathsnap.killbullet = 255;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == -255);
  }

  TEST_CASE("Fill sprite positions with valid data")
  {
    Sprite<Config::Module::CLIENT_MODULE> sprite{1};
    tmsg_spritedeath deathsnap;
    for (std::int32_t d = 0; d < 16; d++)
    {
      deathsnap.pos[d] = tvector2{static_cast<float>(d + 1), static_cast<float>(d + 1)};
      deathsnap.oldpos[d] = tvector2{static_cast<float>(d + 2), static_cast<float>(d + 2)};
    }

    sFillPos(sprite, &deathsnap);

    for (std::int32_t d = 1; d <= 16; d++)
    {
      CHECK(sprite.skeleton.pos[d].x == deathsnap.pos[d - 1].x);
      CHECK(sprite.skeleton.pos[d].y == deathsnap.pos[d - 1].y);
      CHECK(sprite.skeleton.oldpos[d].x == deathsnap.oldpos[d - 1].x);
      CHECK(sprite.skeleton.oldpos[d].y == deathsnap.oldpos[d - 1].y);
    }
  }

  TEST_CASE("Fill sprite positions with zero data")
  {
    Sprite<Config::Module::CLIENT_MODULE> sprite{1};
    tmsg_spritedeath deathsnap;
    for (std::int32_t d = 0; d < 16; d++)
    {
      deathsnap.pos[d] = tvector2{0.0f, 0.0f};
      deathsnap.oldpos[d] = tvector2{0.0f, 0.0f};
    }

    sFillPos(sprite, &deathsnap);

    for (std::int32_t d = 1; d <= 16; d++)
    {
      CHECK(sprite.skeleton.pos[d].x == 0.0f);
      CHECK(sprite.skeleton.pos[d].y == 0.0f);
      CHECK(sprite.skeleton.oldpos[d].x == 0.0f);
      CHECK(sprite.skeleton.oldpos[d].y == 0.0f);
    }
  }

  TEST_CASE("Fill sprite positions with mixed data")
  {
    Sprite<Config::Module::CLIENT_MODULE> sprite{1};
    tmsg_spritedeath deathsnap;
    for (std::int32_t d = 0; d < 16; d++)
    {
      deathsnap.pos[d] = tvector2{static_cast<float>(d % 2 + 2), static_cast<float>(d % 2 + 2)};
      deathsnap.oldpos[d] =
        tvector2{static_cast<float>((d + 1) % 2 + 2), static_cast<float>((d + 1) % 2 + 2)};
    }

    sFillPos(sprite, &deathsnap);

    for (std::int32_t d = 1; d <= 16; d++)
    {
      if (d % 2 == 1)
      {
        CHECK(sprite.skeleton.pos[d].x == 2.0f);
        CHECK(sprite.skeleton.pos[d].y == 2.0f);
        CHECK(sprite.skeleton.oldpos[d].x == 3.0f);
        CHECK(sprite.skeleton.oldpos[d].y == 3.0f);
      }
      else
      {
        CHECK(sprite.skeleton.pos[d].x == 3.0f);
        CHECK(sprite.skeleton.pos[d].y == 3.0f);
        CHECK(sprite.skeleton.oldpos[d].x == 2.0f);
        CHECK(sprite.skeleton.oldpos[d].y == 2.0f);
      }
    }
  }
  TEST_CASE("Fill sprite positions with valid data including special cases")
  {
    Sprite<Config::Module::CLIENT_MODULE> sprite{1};
    tmsg_spritedeath deathsnap;
    for (std::int32_t d = 0; d < 16; d++)
    {
      deathsnap.pos[d] = tvector2{static_cast<float>(d + 1), static_cast<float>(d + 1)};
      deathsnap.oldpos[d] = tvector2{static_cast<float>(d + 2), static_cast<float>(d + 2)};
    }

    sFillPos(sprite, &deathsnap);

    for (std::int32_t d = 1; d <= 16; d++)
    {
      CHECK(sprite.skeleton.pos[d].x == deathsnap.pos[d - 1].x);
      CHECK(sprite.skeleton.pos[d].y == deathsnap.pos[d - 1].y);
      CHECK(sprite.skeleton.oldpos[d].x == deathsnap.oldpos[d - 1].x);
      CHECK(sprite.skeleton.oldpos[d].y == deathsnap.oldpos[d - 1].y);
    }

    CHECK(sprite.skeleton.pos[17].x == deathsnap.pos[0].x);
    CHECK(sprite.skeleton.pos[17].y == deathsnap.pos[0].y);
    CHECK(sprite.skeleton.oldpos[17].x == deathsnap.oldpos[0].x);
    CHECK(sprite.skeleton.oldpos[17].y == deathsnap.oldpos[0].y);

    CHECK(sprite.skeleton.pos[18].x == deathsnap.pos[1].x);
    CHECK(sprite.skeleton.pos[18].y == deathsnap.pos[1].y);
    CHECK(sprite.skeleton.oldpos[18].x == deathsnap.oldpos[1].x);
    CHECK(sprite.skeleton.oldpos[18].y == deathsnap.oldpos[1].y);

    CHECK(sprite.skeleton.pos[19].x == deathsnap.pos[14].x);
    CHECK(sprite.skeleton.pos[19].y == deathsnap.pos[14].y);
    CHECK(sprite.skeleton.oldpos[19].x == deathsnap.oldpos[14].x);
    CHECK(sprite.skeleton.oldpos[19].y == deathsnap.oldpos[14].y);

    CHECK(sprite.skeleton.pos[20].x == deathsnap.pos[15].x);
    CHECK(sprite.skeleton.pos[20].y == deathsnap.pos[15].y);
    CHECK(sprite.skeleton.oldpos[20].x == deathsnap.oldpos[15].x);
    CHECK(sprite.skeleton.oldpos[20].y == deathsnap.oldpos[15].y);
  }
}