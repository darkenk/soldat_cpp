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
    return;

  auto spritesnap = pmsg_serverspritesnapshot(netmessage->packet);
  auto &things = GS::GetThingSystem().GetThings();

  // assign received sprite info to sprite
  std::int32_t i = spritesnap->num;

  if ((i < 1) || (i > max_sprites))
    return;
  if (!SpriteSystem::Get().GetSprite(i).active)
    return;

  clienttickcount = spritesnap->serverticks;
  lastheartbeatcounter = spritesnap->serverticks;

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(i);
  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(i);

  // CLIENT RESPAWN
  if (SpriteSystem::Get().GetSprite(i).deadmeat)
  {
    SpriteSystem::Get().SetSpritePartsOldPos(i, spritePartsPos);
    spritePartsPos = spritesnap->pos;

    spriteVelocity = spritesnap->velocity;
    SpriteSystem::Get().GetSprite(i).respawn();
    SpriteSystem::Get().GetSprite(i).olddeadmeat = SpriteSystem::Get().GetSprite(i).deadmeat;
    spritePartsPos = spritesnap->pos;
  }

  SpriteSystem::Get().GetSprite(i).deadmeat = false;

  if (i != mysprite)
  {
    if (SpriteSystem::Get().GetSprite(i).GetHealth() == spritesnap->health)
    {
      SpriteSystem::Get().SetSpritePartsOldPos(i, spritePartsPos);
      spritePartsPos = spritesnap->pos;
      spriteVelocity = spritesnap->velocity;
    }

    SpriteSystem::Get().GetSprite(i).control.mouseaimy = spritesnap->mouseaimy;
    SpriteSystem::Get().GetSprite(i).control.mouseaimx = spritesnap->mouseaimx;

    decodekeys(SpriteSystem::Get().GetSprite(i), spritesnap->keys16);

    if (SpriteSystem::Get().GetSprite(i).weapon.num != spritesnap->weaponnum)
      SpriteSystem::Get().GetSprite(i).applyweaponbynum(spritesnap->weaponnum, 1);
    if (SpriteSystem::Get().GetSprite(i).secondaryweapon.num != spritesnap->secondaryweaponnum)
      SpriteSystem::Get().GetSprite(i).applyweaponbynum(spritesnap->secondaryweaponnum, 2);
    SpriteSystem::Get().GetSprite(i).weapon.ammocount = spritesnap->ammocount;

    if (SpriteSystem::Get().GetSprite(i).weapon.num == knife_num)
      SpriteSystem::Get().GetSprite(i).player->secwep = 1;
    if (SpriteSystem::Get().GetSprite(i).weapon.num == chainsaw_num)
      SpriteSystem::Get().GetSprite(i).player->secwep = 2;
    if (SpriteSystem::Get().GetSprite(i).weapon.num == law_num)
      SpriteSystem::Get().GetSprite(i).player->secwep = 3;

    // Toggle prone if it was activated or deactivated
    SpriteSystem::Get().GetSprite(i).control.prone =
      (spritesnap->position == pos_prone) ^
      (SpriteSystem::Get().GetSprite(i).position == pos_prone);
  }

  // kill the bow
  if ((SpriteSystem::Get().GetSprite(i).weapon.num == bow_num) ||
      (SpriteSystem::Get().GetSprite(i).weapon.num == bow2_num))
    for (std::int32_t j = 1; j <= max_things; j++)
      if ((things[j].active) && (things[j].style == object_rambo_bow))
      {
        gamethingtarget = 0;
        things[j].kill();
      }

  SpriteSystem::Get().GetSprite(i).wearhelmet = 1;
  if ((spritesnap->look & B1) == B1)
    SpriteSystem::Get().GetSprite(i).wearhelmet = 0;
  if ((spritesnap->look & B4) == B4)
    SpriteSystem::Get().GetSprite(i).wearhelmet = 2;
  if ((SpriteSystem::Get().GetSprite(i).bodyanimation.id != AnimationType::Cigar) &&
      (SpriteSystem::Get().GetSprite(i).bodyanimation.id != AnimationType::Smoke) &&
      !((SpriteSystem::Get().GetSprite(i).idlerandom == 1) &&
        (SpriteSystem::Get().GetSprite(i).bodyanimation.id == AnimationType::Stand)))
  {
    SpriteSystem::Get().GetSprite(i).hascigar = 0;
    if ((spritesnap->look & B2) == B2)
      SpriteSystem::Get().GetSprite(i).hascigar = 5;
    if ((spritesnap->look & B3) == B3)
      SpriteSystem::Get().GetSprite(i).hascigar = 10;
  }

  SpriteSystem::Get().GetSprite(i).tertiaryweapon.ammocount = spritesnap->grenadecount;

  // LogDebugG("sprite: {} grenade {}", i, spritesnap->grenadecount);

  SpriteSystem::Get().GetSprite(i).SetHealth(spritesnap->health);
  SpriteSystem::Get().GetSprite(i).vest = spritesnap->vest;
  if (SpriteSystem::Get().GetSprite(i).vest > defaultvest)
    SpriteSystem::Get().GetSprite(i).vest = defaultvest;

  if (i == mysprite)
  {
    if (!targetmode)
    {
      camerafollowsprite = mysprite;
      SpriteSystem::Get().GetSprite(i).player->camera = mysprite;
    }
  }
}

void clienthandleserverspritesnapshot_major(NetworkContext *netmessage)
{

  if (!verifypacket(sizeof(tmsg_serverspritesnapshot_major), netmessage->size,
                    msgid_serverspritesnapshot_major))
    return;

  auto spritesnapmajor =
    pmsg_serverspritesnapshot_major(netmessage->packet);

  // assign received sprite info to sprite
  std::int32_t i = spritesnapmajor->num;

  if ((i < 1) || (i > max_sprites))
    return;

  if (!SpriteSystem::Get().GetSprite(i).active)
  {
    LogWarn("network", "[ClientSprite] Warning: Received snapshot for inactive player {}", i);
    return;
  }

  clienttickcount = spritesnapmajor->serverticks;
  lastheartbeatcounter = spritesnapmajor->serverticks;

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(i);
  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(i);

  // CLIENT RESPAWN
  if (SpriteSystem::Get().GetSprite(i).deadmeat)
  {
    SpriteSystem::Get().SetSpritePartsOldPos(i, spritePartsPos);
    spritePartsPos = spritesnapmajor->pos;
    spriteVelocity = spritesnapmajor->velocity;
    SpriteSystem::Get().GetSprite(i).respawn();
    SpriteSystem::Get().GetSprite(i).olddeadmeat = SpriteSystem::Get().GetSprite(i).deadmeat;
    spritePartsPos = spritesnapmajor->pos;
  }

  SpriteSystem::Get().GetSprite(i).deadmeat = false;

  if (i != mysprite)
  {
    if (SpriteSystem::Get().GetSprite(i).GetHealth() == spritesnapmajor->health)
    {
      SpriteSystem::Get().SetSpritePartsOldPos(i, spritePartsPos);
      spritePartsPos = spritesnapmajor->pos;
      spriteVelocity = spritesnapmajor->velocity;
    }

    SpriteSystem::Get().GetSprite(i).control.mouseaimy = spritesnapmajor->mouseaimy;
    SpriteSystem::Get().GetSprite(i).control.mouseaimx = spritesnapmajor->mouseaimx;

    decodekeys(SpriteSystem::Get().GetSprite(i), spritesnapmajor->keys16);

    // Toggle prone if it was activated or deactivated
    SpriteSystem::Get().GetSprite(i).control.prone =
      (spritesnapmajor->position == pos_prone) ^
      (SpriteSystem::Get().GetSprite(i).position == pos_prone);
  }

  // kill the bow
  auto &things = GS::GetThingSystem().GetThings();
  if ((SpriteSystem::Get().GetSprite(i).weapon.num == bow_num) ||
      (SpriteSystem::Get().GetSprite(i).weapon.num == bow2_num))
    for (std::int32_t j = 1; j <= max_things; j++)
      if ((things[j].active) & (things[j].style == object_rambo_bow))
      {
        gamethingtarget = 0;
        things[j].kill();
      }

  SpriteSystem::Get().GetSprite(i).SetHealth(spritesnapmajor->health);

  if (i == mysprite)
  {
    if (!targetmode)
    {
      camerafollowsprite = mysprite;
      SpriteSystem::Get().GetSprite(i).player->camera = mysprite;
    }
  }
}

void clienthandleserverskeletonsnapshot(NetworkContext *netmessage)
{

  if (!verifypacket(sizeof(tmsg_serverskeletonsnapshot), netmessage->size,
                    msgid_serverskeletonsnapshot))
    return;

  auto skeletonsnap = pmsg_serverskeletonsnapshot(netmessage->packet);

  // assign received Skeleton info to skeleton
  std::int32_t i = skeletonsnap->num;

  if ((i < 1) || (i > max_sprites))
    return;
  if (!SpriteSystem::Get().GetSprite(i).active)
    return;

  auto &guns = GS::GetWeaponSystem().GetGuns();

  SpriteSystem::Get().GetSprite(i).deadmeat = true;
  SpriteSystem::Get().GetSprite(i).respawncounter = skeletonsnap->respawncounter;
  SpriteSystem::Get().GetSprite(i).SetFirstWeapon(guns[noweapon]);
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
    return;

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
    clientmsg.keys16 = clientmsg.keys16 & ~B9;

  tvector2 posdiff = vec2subtract(clientmsg.pos, oldclientsnapshotmovmsg.pos);
  tvector2 veldiff = vec2subtract(clientmsg.velocity, oldclientsnapshotmovmsg.velocity);

  if ((vec2length(posdiff) > posdelta) || (vec2length(veldiff) > veldelta) ||
      (clientmsg.keys16 != oldclientsnapshotmovmsg.keys16) || ((clientmsg.keys16 & B6) == B6) ||
      !(((SpriteSystem::Get().GetSprite(mysprite).weapon.fireinterval <= fireinterval_net) &&
         (SpriteSystem::Get().GetSprite(mysprite).weapon.ammocount > 0) &&
         (round(mx) == oldclientsnapshotmovmsg.mouseaimx) &&
         (round(my) == oldclientsnapshotmovmsg.mouseaimy)) ||
        ((fabs(mx - oldclientsnapshotmovmsg.mouseaimx) < mouseaimdelta) &&
         (fabs(my - oldclientsnapshotmovmsg.mouseaimy) < mouseaimdelta))))
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

static std::int32_t sConvertKillBulletToGFX(const std::uint8_t killbullet)
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
    return  GFX::INTERFACE_GUNS_BOW;
  case 208:
    return  GFX::INTERFACE_GUNS_BOW;
  case 210:
    return  GFX::INTERFACE_CLUSTER_NADE;
  case 211:
    return  GFX::INTERFACE_GUNS_KNIFE;
  case 212:
    return  GFX::INTERFACE_GUNS_CHAINSAW;
  case 222:
    return  GFX::INTERFACE_NADE;
  case 224:
    return  GFX::INTERFACE_GUNS_LAW;
  case 225:
    return  GFX::INTERFACE_GUNS_M2;
  default:
    return -255;
  }
}

void clienthandlespritedeath(NetworkContext *netmessage)
{
  std::int32_t j;
  tvector2 b;

  auto &map = GS::GetGame().GetMap();

  if (!verifypacket(sizeof(tmsg_spritedeath), netmessage->size, msgid_spritedeath))
    return;

  auto deathsnap = pmsg_spritedeath(netmessage->packet);

  const std::int32_t i = deathsnap->num;

  if ((i < 1) || (i > max_sprites))
    return;
  if (!SpriteSystem::Get().GetSprite(i).active)
    return;

  auto &sprite = SpriteSystem::Get().GetSprite(i);

  for (std::int32_t d = 1; d <= 16; d++)
  {
    [[deprecated("dminus 1")]] auto dminus1 = d - 1;
    const auto &pos = deathsnap->pos[dminus1];
    const auto &oldPos = deathsnap->oldpos[dminus1];
    if ((round(pos.x) != 0) && (round(pos.y) != 0) && (round(oldPos.x) != 0) &&
        (round(oldPos.y) != 0))
    {
      sprite.skeleton.pos[d].x = pos.x;
      sprite.skeleton.pos[d].y = pos.y;
      sprite.skeleton.oldpos[d].x = oldPos.x;
      sprite.skeleton.oldpos[d].y = oldPos.y;

      if (d == 1)
      {
        sprite.skeleton.pos[17].x = pos.x;
        sprite.skeleton.pos[17].y = pos.y;
        sprite.skeleton.oldpos[17].x = oldPos.x;
        sprite.skeleton.oldpos[17].y = oldPos.y;
      }
      if (d == 2)
      {
        sprite.skeleton.pos[18].x = pos.x;
        sprite.skeleton.pos[18].y = pos.y;
        sprite.skeleton.oldpos[18].x = oldPos.x;
        sprite.skeleton.oldpos[18].y = oldPos.y;
      }
      if (d == 15)
      {
        sprite.skeleton.pos[19].x = pos.x;
        sprite.skeleton.pos[19].y = pos.y;
        sprite.skeleton.oldpos[19].x = oldPos.x;
        sprite.skeleton.oldpos[19].y = oldPos.y;
      }
      if (d == 16)
      {
        sprite.skeleton.pos[20].x = pos.x;
        sprite.skeleton.pos[20].y = pos.y;
        sprite.skeleton.oldpos[20].x = oldPos.x;
        sprite.skeleton.oldpos[20].y = oldPos.y;
      }
    }
  }

  b.x = 0;
  b.y = 0;
  sprite.SetHealth(deathsnap->health);

  // death!
  if ((sprite.GetHealth() < 1) && (sprite.GetHealth() > headchopdeathhealth))
    sprite.die(normal_death, deathsnap->killer, deathsnap->where, deathsnap->killbullet, b);
  else if ((sprite.GetHealth() < (headchopdeathhealth + 1)) &&
           (sprite.GetHealth() > brutaldeathhealth))
    sprite.die(headchop_death, deathsnap->killer, deathsnap->where, deathsnap->killbullet, b);
  else if (sprite.GetHealth() < (brutaldeathhealth + 1))
    sprite.die(brutal_death, deathsnap->killer, deathsnap->where, deathsnap->killbullet, b);

  sprite.skeleton.constraints[2].active = true;
  sprite.skeleton.constraints[4].active = true;
  sprite.skeleton.constraints[20].active = true;
  sprite.skeleton.constraints[21].active = true;
  sprite.skeleton.constraints[23].active = true;
  if ((deathsnap->constraints & B1) == B1)
    sprite.skeleton.constraints[2].active = false;
  if ((deathsnap->constraints & B2) == B2)
    sprite.skeleton.constraints[4].active = false;
  if ((deathsnap->constraints & B3) == B3)
    sprite.skeleton.constraints[20].active = false;
  if ((deathsnap->constraints & B4) == B4)
    sprite.skeleton.constraints[21].active = false;
  if ((deathsnap->constraints & B5) == B5)
    sprite.skeleton.constraints[23].active = false;

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
      gamemenushow(limbomenu);
    menutimer = menu_time;
    playsound(SfxEffect::playerdeath);
  }

  if (deathsnap->killer == mysprite)
  {
    bigmessage(wideformat(_("You killed {}"), sprite.player->name), killmessagewait,
               kill_message_color);

    if ((SpriteSystem::Get().GetSprite(deathsnap->killer).multikills > 1) &&
        (SpriteSystem::Get().GetSprite(deathsnap->killer).multikills < 18))
      bigmessage(multikillmessage[SpriteSystem::Get().GetSprite(deathsnap->killer).multikills],
                 killmessagewait, kill_message_color);
    if (SpriteSystem::Get().GetSprite(deathsnap->killer).multikills > 17)
      bigmessage(multikillmessage[9], killmessagewait, kill_message_color);

    if ((shotdistance > -1) && (deathsnap->killer != i))
    {
      shotdistanceshow = killmessagewait - 30;
      shotdistance = deathsnap->shotdistance;
      shotricochet = deathsnap->shotricochet;
      shotlife = deathsnap->shotlife;
    }
  }

  if ((deathsnap->killer == mysprite) && (i == mysprite))
    bigmessage(_("You killed yourself"), killmessagewait, die_message_color);

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
    return;

  auto deltamov = pmsg_serverspritedelta_movement(netmessage->packet);

  // Older than Heartbeat Drop the Packet
  if (!demoplayer.active() && (deltamov->servertick < lastheartbeatcounter))
  {
    return;
  }
  std::int32_t i = deltamov->num;

  if ((i < 1) || (i > max_sprites))
    return;
  if (!SpriteSystem::Get().GetSprite(i).active)
    return;

  auto &spritePartsPos = SpriteSystem::Get().GetSpritePartsPos(i);
  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(i);
  spritePartsPos = deltamov->pos;
  spriteVelocity = deltamov->velocity;

  SpriteSystem::Get().GetSprite(i).control.mouseaimy = deltamov->mouseaimy;
  SpriteSystem::Get().GetSprite(i).control.mouseaimx = deltamov->mouseaimx;

  decodekeys(SpriteSystem::Get().GetSprite(i), deltamov->keys16);
}

void clienthandledelta_mouseaim(NetworkContext *netmessage)
{
  tmsg_serverspritedelta_mouseaim *deltamouse;

  if (!verifypacket(sizeof(tmsg_serverspritedelta_mouseaim), netmessage->size,
                    msgid_delta_mouseaim))
    return;

  deltamouse = pmsg_serverspritedelta_mouseaim(netmessage->packet);

  std::int32_t i = deltamouse->num;
  if ((i < 1) || (i > max_sprites))
    return;
  if (!SpriteSystem::Get().GetSprite(i).active)
    return;

  SpriteSystem::Get().GetSprite(i).control.mouseaimy = deltamouse->mouseaimy;
  SpriteSystem::Get().GetSprite(i).control.mouseaimx = deltamouse->mouseaimx;

  if (SpriteSystem::Get().GetSprite(i).position == pos_prone)
    SpriteSystem::Get().GetSprite(i).bodyapplyanimation(AnimationType::Prone, 1);
  else
    SpriteSystem::Get().GetSprite(i).bodyapplyanimation(AnimationType::Aim, 1);

  SpriteSystem::Get().GetSprite(i).weapon.fireintervalprev = 0;
  SpriteSystem::Get().GetSprite(i).weapon.fireintervalcount = 0;
}

void clienthandledelta_weapons(NetworkContext *netmessage)
{

  if (!verifypacket(sizeof(tmsg_serverspritedelta_weapons), netmessage->size, msgid_delta_weapons))
    return;

  std::int32_t i = pmsg_serverspritedelta_weapons(netmessage->packet)->num;

  if ((i < 1) || (i > max_sprites))
    return;
  if (!SpriteSystem::Get().GetSprite(i).active)
    return;

  SpriteSystem::Get().GetSprite(i).applyweaponbynum(
    pmsg_serverspritedelta_weapons(netmessage->packet)->weaponnum, 1);
  SpriteSystem::Get().GetSprite(i).applyweaponbynum(
    pmsg_serverspritedelta_weapons(netmessage->packet)->secondaryweaponnum, 2);
  SpriteSystem::Get().GetSprite(i).weapon.ammocount =
    pmsg_serverspritedelta_weapons(netmessage->packet)->ammocount;

  if ((i == mysprite) && !SpriteSystem::Get().GetSprite(mysprite).deadmeat)
    clientspritesnapshot();
}

void clienthandledelta_helmet(NetworkContext *netmessage)
{

  if (!verifypacket(sizeof(tmsg_serverspritedelta_helmet), netmessage->size, msgid_delta_helmet))
    return;

  auto deltahelmet = pmsg_serverspritedelta_helmet(netmessage->packet);

  std::int32_t i = deltahelmet->num;

  if ((i < 1) || (i > max_sprites))
    return;
  if (!SpriteSystem::Get().GetSprite(i).active)
    return;

  auto &spriteVelocity = SpriteSystem::Get().GetVelocity(i);

  // helmet chop
  if (deltahelmet->wearhelmet == 0)
  {
    createspark(SpriteSystem::Get().GetSprite(i).skeleton.pos[12], spriteVelocity, 6, i, 198);
    playsound(SfxEffect::headchop, SpriteSystem::Get().GetSprite(i).skeleton.pos[12]);
  }

  SpriteSystem::Get().GetSprite(i).wearhelmet = deltahelmet->wearhelmet;
}

void clienthandleclientspritesnapshot_dead(NetworkContext *netmessage)
{
  if (!verifypacket(sizeof(tmsg_clientspritesnapshot_dead), netmessage->size,
                    msgid_clientspritesnapshot_dead))
    return;
  if (freecam == 0)
    camerafollowsprite = pmsg_clientspritesnapshot_dead(netmessage->packet)->camerafocus;
}

// TESTS
#include <doctest/doctest.h>

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
    deathsnap.killbullet = 999;
    const auto k = sConvertKillBulletToGFX(deathsnap.killbullet);
    CHECK(k == -255);
  }
}