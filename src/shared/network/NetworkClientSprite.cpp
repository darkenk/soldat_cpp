// automatically converted

#include "NetworkClientSprite.hpp"
#include "../../client/Client.hpp"
#include "../../client/ClientGame.hpp"
#include "../../client/GameMenus.hpp"
#include "../../client/Sound.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../GameStrings.hpp"
#include "../Logging.hpp"
#include "../gfx.hpp"
#include "NetworkUtils.hpp"

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

namespace
{
tmsg_clientspritesnapshot oldclientsnapshotmsg;
tmsg_clientspritesnapshot_mov oldclientsnapshotmovmsg;
} // namespace

void clienthandleserverspritesnapshot(SteamNetworkingMessage_t *netmessage)
{
    tmsg_serverspritesnapshot *spritesnap;
    std::int32_t i, j;

    if (!verifypacket(sizeof(tmsg_serverspritesnapshot), netmessage->m_cbSize,
                      msgid_serverspritesnapshot))
        return;

    spritesnap = pmsg_serverspritesnapshot(netmessage->m_pData);

    // assign received sprite info to sprite
    i = spritesnap->num;

    if ((i < 1) || (i > max_sprites))
        return;
    if (!sprite[i].active)
        return;

    clienttickcount = spritesnap->serverticks;
    lastheartbeatcounter = spritesnap->serverticks;

    // CLIENT RESPAWN
    if (sprite[i].deadmeat)
    {
        spriteparts.oldpos[i] = spriteparts.pos[i];
        spriteparts.pos[i] = spritesnap->pos;
        spriteparts.velocity[i] = spritesnap->velocity;
        sprite[i].respawn();
        sprite[i].olddeadmeat = sprite[i].deadmeat;
        spriteparts.pos[i] = spritesnap->pos;
    }

    sprite[i].deadmeat = false;

    if (i != mysprite)
    {
        if (sprite[i].health == spritesnap->health)
        {
            spriteparts.oldpos[i] = spriteparts.pos[i];
            spriteparts.pos[i] = spritesnap->pos;
            spriteparts.velocity[i] = spritesnap->velocity;
        }

        sprite[i].control.mouseaimy = spritesnap->mouseaimy;
        sprite[i].control.mouseaimx = spritesnap->mouseaimx;

        decodekeys(sprite[i], spritesnap->keys16);

        if (sprite[i].weapon.num != spritesnap->weaponnum)
            sprite[i].applyweaponbynum(spritesnap->weaponnum, 1);
        if (sprite[i].secondaryweapon.num != spritesnap->secondaryweaponnum)
            sprite[i].applyweaponbynum(spritesnap->secondaryweaponnum, 2);
        sprite[i].weapon.ammocount = spritesnap->ammocount;

        if (sprite[i].weapon.num == guns[knife].num)
            sprite[i].player->secwep = 1;
        if (sprite[i].weapon.num == guns[chainsaw].num)
            sprite[i].player->secwep = 2;
        if (sprite[i].weapon.num == guns[law].num)
            sprite[i].player->secwep = 3;

        // Toggle prone if it was activated or deactivated
        sprite[i].control.prone =
            (spritesnap->position == pos_prone) ^ (sprite[i].position == pos_prone);
    }

    // kill the bow
    if ((sprite[i].weapon.num == guns[bow].num) || (sprite[i].weapon.num == guns[bow2].num))
        for (j = 1; j <= max_things; j++)
            if ((thing[j].active) && (thing[j].style == object_rambo_bow))
            {
                gamethingtarget = 0;
                thing[j].kill();
            }

    sprite[i].wearhelmet = 1;
    if ((spritesnap->look & B1) == B1)
        sprite[i].wearhelmet = 0;
    if ((spritesnap->look & B4) == B4)
        sprite[i].wearhelmet = 2;
    if ((sprite[i].bodyanimation.id != cigar.id) && (sprite[i].bodyanimation.id != smoke.id) &&
        !((sprite[i].idlerandom == 1) && (sprite[i].bodyanimation.id == stand.id)))
    {
        sprite[i].hascigar = 0;
        if ((spritesnap->look & B2) == B2)
            sprite[i].hascigar = 5;
        if ((spritesnap->look & B3) == B3)
            sprite[i].hascigar = 10;
    }

    sprite[i].tertiaryweapon.ammocount = spritesnap->grenadecount;

    LogDebugG("sprite: {} grenade {}", i, spritesnap->grenadecount);

    sprite[i].health = spritesnap->health;
    sprite[i].vest = spritesnap->vest;
    if (sprite[i].vest > defaultvest)
        sprite[i].vest = defaultvest;

    if (i == mysprite)
    {
        if (!targetmode)
        {
            camerafollowsprite = mysprite;
            sprite[i].player->camera = mysprite;
        }
    }
}

void clienthandleserverspritesnapshot_major(SteamNetworkingMessage_t *netmessage)
{
    tmsg_serverspritesnapshot_major *spritesnapmajor;
    std::int32_t i, j;

    if (!verifypacket(sizeof(tmsg_serverspritesnapshot_major), netmessage->m_cbSize,
                      msgid_serverspritesnapshot_major))
        return;

    spritesnapmajor = pmsg_serverspritesnapshot_major(netmessage->m_pData);

    // assign received sprite info to sprite
    i = spritesnapmajor->num;

    if ((i < 1) || (i > max_sprites))
        return;

    if (!sprite[i].active)
    {
        LogWarn("network", "[ClientSprite] Warning: Received snapshot for inactive player {}", i);
        return;
    }

    clienttickcount = spritesnapmajor->serverticks;
    lastheartbeatcounter = spritesnapmajor->serverticks;

    // CLIENT RESPAWN
    if (sprite[i].deadmeat)
    {
        spriteparts.oldpos[i] = spriteparts.pos[i];
        spriteparts.pos[i] = spritesnapmajor->pos;
        spriteparts.velocity[i] = spritesnapmajor->velocity;
        sprite[i].respawn();
        sprite[i].olddeadmeat = sprite[i].deadmeat;
        spriteparts.pos[i] = spritesnapmajor->pos;
    }

    sprite[i].deadmeat = false;

    if (i != mysprite)
    {
        if (sprite[i].health == spritesnapmajor->health)
        {
            spriteparts.oldpos[i] = spriteparts.pos[i];
            spriteparts.pos[i] = spritesnapmajor->pos;
            spriteparts.velocity[i] = spritesnapmajor->velocity;
        }

        sprite[i].control.mouseaimy = spritesnapmajor->mouseaimy;
        sprite[i].control.mouseaimx = spritesnapmajor->mouseaimx;

        decodekeys(sprite[i], spritesnapmajor->keys16);

        // Toggle prone if it was activated or deactivated
        sprite[i].control.prone =
            (spritesnapmajor->position == pos_prone) ^ (sprite[i].position == pos_prone);
    }

    // kill the bow
    if ((sprite[i].weapon.num == guns[bow].num) || (sprite[i].weapon.num == guns[bow2].num))
        for (j = 1; j <= max_things; j++)
            if ((thing[j].active) & (thing[j].style == object_rambo_bow))
            {
                gamethingtarget = 0;
                thing[j].kill();
            }

    sprite[i].health = spritesnapmajor->health;

    if (i == mysprite)
    {
        if (!targetmode)
        {
            camerafollowsprite = mysprite;
            sprite[i].player->camera = mysprite;
        }
    }
}

void clienthandleserverskeletonsnapshot(SteamNetworkingMessage_t *netmessage)
{
    tmsg_serverskeletonsnapshot *skeletonsnap;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_serverskeletonsnapshot), netmessage->m_cbSize,
                      msgid_serverskeletonsnapshot))
        return;

    skeletonsnap = pmsg_serverskeletonsnapshot(netmessage->m_pData);

    // assign received Skeleton info to skeleton
    i = skeletonsnap->num;

    if ((i < 1) || (i > max_sprites))
        return;
    if (!sprite[i].active)
        return;

    sprite[i].deadmeat = true;
    sprite[i].respawncounter = skeletonsnap->respawncounter;
    sprite[i].weapon = guns[noweapon];
}

void clientspritesnapshot()
{
    tmsg_clientspritesnapshot clientmsg;

    clientmsg.header.id = msgid_clientspritesnapshot;

    clientmsg.ammocount = sprite[mysprite].weapon.ammocount;
    clientmsg.secondaryammocount = sprite[mysprite].secondaryweapon.ammocount;
    clientmsg.weaponnum = sprite[mysprite].weapon.num;
    clientmsg.secondaryweaponnum = sprite[mysprite].secondaryweapon.num;
    clientmsg.position = sprite[mysprite].position;

    if ((clientmsg.ammocount == oldclientsnapshotmsg.ammocount) &&
        (clientmsg.weaponnum == oldclientsnapshotmsg.weaponnum) &&
        (clientmsg.secondaryweaponnum == oldclientsnapshotmsg.secondaryweaponnum) &&
        (clientmsg.position == oldclientsnapshotmsg.position))
        return;

    oldclientsnapshotmsg = clientmsg;

    GetNetwork()->senddata(&clientmsg, sizeof(clientmsg), k_nSteamNetworkingSend_Unreliable);
}

// CLIENT SPRITE SNAPSHOT MOV
void clientspritesnapshotmov()
{
    tmsg_clientspritesnapshot_mov clientmsg;
    tvector2 posdiff, veldiff;

    clientmsg.header.id = msgid_clientspritesnapshot_mov;

    clientmsg.pos = spriteparts.pos[mysprite];
    clientmsg.velocity = spriteparts.velocity[mysprite];
    clientmsg.mouseaimx = sprite[mysprite].control.mouseaimx;
    clientmsg.mouseaimy = sprite[mysprite].control.mouseaimy;

    encodekeys(sprite[mysprite], clientmsg.keys16);

    if (sprite[mysprite].dontdrop)
        clientmsg.keys16 = clientmsg.keys16 & ~B9;

    posdiff = vec2subtract(clientmsg.pos, oldclientsnapshotmovmsg.pos);
    veldiff = vec2subtract(clientmsg.velocity, oldclientsnapshotmovmsg.velocity);

    if ((vec2length(posdiff) > posdelta) || (vec2length(veldiff) > veldelta) ||
        (clientmsg.keys16 != oldclientsnapshotmovmsg.keys16) || ((clientmsg.keys16 & B6) == B6) ||
        !(((sprite[mysprite].weapon.fireinterval <= fireinterval_net) &&
           (sprite[mysprite].weapon.ammocount > 0) &&
           (round(mx) == oldclientsnapshotmovmsg.mouseaimx) &&
           (round(my) == oldclientsnapshotmovmsg.mouseaimy)) ||
          ((fabs(mx - oldclientsnapshotmovmsg.mouseaimx) < mouseaimdelta) &&
           (fabs(my - oldclientsnapshotmovmsg.mouseaimy) < mouseaimdelta))))
    {
        oldclientsnapshotmovmsg = clientmsg;
        oldclientsnapshotmovmsg.mouseaimx = round(mx);
        oldclientsnapshotmovmsg.mouseaimy = round(my);

        GetNetwork()->senddata(&clientmsg, sizeof(clientmsg), k_nSteamNetworkingSend_Unreliable);
    }
}

// CLIENT SPRITE SNAPSHOT DEAD
void clientspritesnapshotdead()
{
    tmsg_clientspritesnapshot_dead clientmsg;

    clientmsg.header.id = msgid_clientspritesnapshot_dead;
    clientmsg.camerafocus = camerafollowsprite;

    GetNetwork()->senddata(&clientmsg, sizeof(clientmsg), k_nSteamNetworkingSend_Unreliable);
}

void clienthandlespritedeath(SteamNetworkingMessage_t *netmessage)
{
    tmsg_spritedeath *deathsnap;
    std::int32_t i, d, j, k;
    tvector2 b;
    std::uint32_t col, col2;
    float hm = 0.0;

    if (!verifypacket(sizeof(tmsg_spritedeath), netmessage->m_cbSize, msgid_spritedeath))
        return;

    deathsnap = pmsg_spritedeath(netmessage->m_pData);

    i = deathsnap->num;

    if ((i < 1) || (i > max_sprites))
        return;
    if (!sprite[i].active)
        return;

    for (d = 1; d <= 16; d++)
    {
        [[deprecated("dminus 1")]] auto dminus1 = d - 1;
        if ((round(deathsnap->pos[dminus1].x) != 0) && (round(deathsnap->pos[dminus1].y) != 0) &&
            (round(deathsnap->oldpos[dminus1].x) != 0) &&
            (round(deathsnap->oldpos[dminus1].y) != 0))
        {
            sprite[i].skeleton.pos[d].x = deathsnap->pos[dminus1].x;
            sprite[i].skeleton.pos[d].y = deathsnap->pos[dminus1].y;
            sprite[i].skeleton.oldpos[d].x = deathsnap->oldpos[dminus1].x;
            sprite[i].skeleton.oldpos[d].y = deathsnap->oldpos[dminus1].y;

            if (d == 1)
            {
                sprite[i].skeleton.pos[17].x = deathsnap->pos[dminus1].x;
                sprite[i].skeleton.pos[17].y = deathsnap->pos[dminus1].y;
                sprite[i].skeleton.oldpos[17].x = deathsnap->oldpos[dminus1].x;
                sprite[i].skeleton.oldpos[17].y = deathsnap->oldpos[dminus1].y;
            }
            if (d == 2)
            {
                sprite[i].skeleton.pos[18].x = deathsnap->pos[dminus1].x;
                sprite[i].skeleton.pos[18].y = deathsnap->pos[dminus1].y;
                sprite[i].skeleton.oldpos[18].x = deathsnap->oldpos[dminus1].x;
                sprite[i].skeleton.oldpos[18].y = deathsnap->oldpos[dminus1].y;
            }
            if (d == 15)
            {
                sprite[i].skeleton.pos[19].x = deathsnap->pos[dminus1].x;
                sprite[i].skeleton.pos[19].y = deathsnap->pos[dminus1].y;
                sprite[i].skeleton.oldpos[19].x = deathsnap->oldpos[dminus1].x;
                sprite[i].skeleton.oldpos[19].y = deathsnap->oldpos[dminus1].y;
            }
            if (d == 16)
            {
                sprite[i].skeleton.pos[20].x = deathsnap->pos[dminus1].x;
                sprite[i].skeleton.pos[20].y = deathsnap->pos[dminus1].y;
                sprite[i].skeleton.oldpos[20].x = deathsnap->oldpos[dminus1].x;
                sprite[i].skeleton.oldpos[20].y = deathsnap->oldpos[dminus1].y;
            }
        }
    }

    b.x = 0;
    b.y = 0;
    sprite[i].health = deathsnap->health;

    // death!
    if ((sprite[i].health < 1) && (sprite[i].health > headchopdeathhealth))
        sprite[i].die(normal_death, deathsnap->killer, deathsnap->where, deathsnap->killbullet, b);
    else if ((sprite[i].health < (headchopdeathhealth + 1)) &&
             (sprite[i].health > brutaldeathhealth))
        sprite[i].die(headchop_death, deathsnap->killer, deathsnap->where, deathsnap->killbullet,
                      b);
    else if (sprite[i].health < (brutaldeathhealth + 1))
        sprite[i].die(brutal_death, deathsnap->killer, deathsnap->where, deathsnap->killbullet, b);

    sprite[i].skeleton.constraints[2].active = true;
    sprite[i].skeleton.constraints[4].active = true;
    sprite[i].skeleton.constraints[20].active = true;
    sprite[i].skeleton.constraints[21].active = true;
    sprite[i].skeleton.constraints[23].active = true;
    if ((deathsnap->constraints & B1) == B1)
        sprite[i].skeleton.constraints[2].active = false;
    if ((deathsnap->constraints & B2) == B2)
        sprite[i].skeleton.constraints[4].active = false;
    if ((deathsnap->constraints & B3) == B3)
        sprite[i].skeleton.constraints[20].active = false;
    if ((deathsnap->constraints & B4) == B4)
        sprite[i].skeleton.constraints[21].active = false;
    if ((deathsnap->constraints & B5) == B5)
        sprite[i].skeleton.constraints[23].active = false;

    sprite[i].weapon = guns[noweapon];
    sprite[i].respawncounter = deathsnap->respawncounter;
    sprite[i].onfire = deathsnap->onfire;

    // mulitkill count
    if (deathsnap->killer != i)
    {
        sprite[deathsnap->killer].multikilltime = multikillinterval;
        sprite[deathsnap->killer].multikills += 1;
    }

    if (i == mysprite)
    {
        bigmessage(wideformat(_("Killed by {}"), sprite[deathsnap->killer].player->name),
                   killmessagewait, die_message_color);
        if (!limbolock)
            gamemenushow(limbomenu);
        menutimer = menu_time;
        playsound(sfx_playerdeath);
    }

    if (deathsnap->killer == mysprite)
    {
        bigmessage(wideformat(_("You killed {}"), sprite[i].player->name), killmessagewait,
                   kill_message_color);

        if ((sprite[deathsnap->killer].multikills > 1) &&
            (sprite[deathsnap->killer].multikills < 18))
            bigmessage(multikillmessage[sprite[deathsnap->killer].multikills], killmessagewait,
                       kill_message_color);
        if (sprite[deathsnap->killer].multikills > 17)
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

    switch (deathsnap->killbullet)
    {
    case 0:
        k = GFX::INTERFACE_GUNS_SOCOM;
        break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
        k = GFX::INTERFACE_GUNS_DEAGLES + deathsnap->killbullet - 1;
        break;
    case 205:
        k = GFX::INTERFACE_GUNS_FLAMER;
        break;
    case 206:
        k = GFX::INTERFACE_GUNS_FIST;
        break;
    case 207:
        k = GFX::INTERFACE_GUNS_BOW;
        break;
    case 208:
        k = GFX::INTERFACE_GUNS_BOW;
        break;
    case 210:
        k = GFX::INTERFACE_CLUSTER_NADE;
        break;
    case 211:
        k = GFX::INTERFACE_GUNS_KNIFE;
        break;
    case 212:
        k = GFX::INTERFACE_GUNS_CHAINSAW;
        break;
    case 222:
        k = GFX::INTERFACE_NADE;
        break;
    case 224:
        k = GFX::INTERFACE_GUNS_LAW;
        break;
    case 225:
        k = GFX::INTERFACE_GUNS_M2;
        break;
    default:
        k = -255;
    }

    col = 0;
    col2 = 0;
    switch (sprite[deathsnap->killer].player->team)
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
    switch (sprite[i].player->team)
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
        GetKillConsole().consolenum((sprite[deathsnap->killer].player->name) + " (" +
                                        (inttostr(sprite[deathsnap->killer].player->kills)) + ')',
                                    col, k);
        GetKillConsole().consolenum((sprite[i].player->name), col2, -255);
    }
    else
    {
        GetKillConsole().consolenum((sprite[deathsnap->killer].player->name) + " (" +
                                        (inttostr(sprite[deathsnap->killer].player->kills)) + ')',
                                    spectator_d_message_color, k);
    }

    // Explode - lag compensate
    if (deathsnap->killbullet == 7) // M79
        for (j = max_bullets; j >= 1; j--)
            if ((bullet[j].active) && (bullet[j].owner == deathsnap->killer) &&
                (bullet[j].style == guns[m79].bulletstyle))
            {
                bulletparts.oldpos[j] = sprite[i].skeleton.pos[8];
                bulletparts.pos[j] = sprite[i].skeleton.pos[8];
                bullet[j].hit(3);
                bullet[j].kill();
                break;
            }

    if (deathsnap->killbullet == 224) /*LAW*/
        for (j = max_bullets; j >= 1; j--)
            if ((bullet[j].active) && (bullet[j].owner == deathsnap->killer) &&
                (bullet[j].style == guns[law].bulletstyle))
            {
                bulletparts.oldpos[j] = sprite[i].skeleton.pos[8];
                bulletparts.pos[j] = sprite[i].skeleton.pos[8];
                bullet[j].hit(3);
                bullet[j].kill();
                break;
            }

    if (deathsnap->killbullet == 222) /*grenade*/
        for (j = max_bullets; j >= 1; j--)
            if ((bullet[j].active) && (bullet[j].owner == deathsnap->killer) &&
                (bullet[j].style == guns[fraggrenade].bulletstyle))
            {
                map.raycast(bulletparts.pos[j], sprite[i].skeleton.pos[8], hm, 351);
                if (hm < after_explosion_radius)
                {
                    bulletparts.oldpos[j] = spriteparts.pos[i];
                    bulletparts.pos[j] = spriteparts.pos[i];
                    bullet[j].hit(4);
                    bullet[j].kill();
                }
            }
}

void clienthandledelta_movement(SteamNetworkingMessage_t *netmessage)
{
    tmsg_serverspritedelta_movement *deltamov;
    std::int32_t i;
    // a: TVector2;

    if (!verifypacket(sizeof(tmsg_serverspritedelta_movement), netmessage->m_cbSize,
                      msgid_delta_movement))
        return;

    deltamov = pmsg_serverspritedelta_movement(netmessage->m_pData);

    // Older than Heartbeat Drop the Packet
    if (!demoplayer.active() && (deltamov->servertick < lastheartbeatcounter))
    {
        return;
    }
    i = deltamov->num;

    if ((i < 1) || (i > max_sprites))
        return;
    if (!sprite[i].active)
        return;

    // a := Vec2Subtract(SpriteParts.Pos[i], DeltaMov.Pos);

    spriteparts.pos[i] = deltamov->pos;
    spriteparts.velocity[i] = deltamov->velocity;

    sprite[i].control.mouseaimy = deltamov->mouseaimy;
    sprite[i].control.mouseaimx = deltamov->mouseaimx;

    decodekeys(sprite[i], deltamov->keys16);
}

void clienthandledelta_mouseaim(SteamNetworkingMessage_t *netmessage)
{
    std::int32_t i;
    tmsg_serverspritedelta_mouseaim *deltamouse;

    if (!verifypacket(sizeof(tmsg_serverspritedelta_mouseaim), netmessage->m_cbSize,
                      msgid_delta_mouseaim))
        return;

    deltamouse = pmsg_serverspritedelta_mouseaim(netmessage->m_pData);

    i = deltamouse->num;
    if ((i < 1) || (i > max_sprites))
        return;
    if (!sprite[i].active)
        return;

    sprite[i].control.mouseaimy = deltamouse->mouseaimy;
    sprite[i].control.mouseaimx = deltamouse->mouseaimx;

    if (sprite[i].position == pos_prone)
        sprite[i].bodyapplyanimation(prone, 1);
    else
        sprite[i].bodyapplyanimation(aim, 1);

    sprite[i].weapon.fireintervalprev = 0;
    sprite[i].weapon.fireintervalcount = 0;
}

void clienthandledelta_weapons(SteamNetworkingMessage_t *netmessage)
{
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_serverspritedelta_weapons), netmessage->m_cbSize,
                      msgid_delta_weapons))
        return;

    i = pmsg_serverspritedelta_weapons(netmessage->m_pData)->num;

    if ((i < 1) || (i > max_sprites))
        return;
    if (!sprite[i].active)
        return;

    sprite[i].applyweaponbynum(pmsg_serverspritedelta_weapons(netmessage->m_pData)->weaponnum, 1);
    sprite[i].applyweaponbynum(
        pmsg_serverspritedelta_weapons(netmessage->m_pData)->secondaryweaponnum, 2);
    sprite[i].weapon.ammocount = pmsg_serverspritedelta_weapons(netmessage->m_pData)->ammocount;

    if ((i == mysprite) && !sprite[mysprite].deadmeat)
        clientspritesnapshot();
}

void clienthandledelta_helmet(SteamNetworkingMessage_t *netmessage)
{
    tmsg_serverspritedelta_helmet *deltahelmet;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_serverspritedelta_helmet), netmessage->m_cbSize,
                      msgid_delta_helmet))
        return;

    deltahelmet = pmsg_serverspritedelta_helmet(netmessage->m_pData);

    i = deltahelmet->num;

    if ((i < 1) || (i > max_sprites))
        return;
    if (!sprite[i].active)
        return;

    // helmet chop
    if (deltahelmet->wearhelmet == 0)
    {
        createspark(sprite[i].skeleton.pos[12], spriteparts.velocity[i], 6, i, 198);
        playsound(sfx_headchop, sprite[i].skeleton.pos[12]);
    }

    sprite[i].wearhelmet = deltahelmet->wearhelmet;
}

void clienthandleclientspritesnapshot_dead(SteamNetworkingMessage_t *netmessage)
{
    if (!verifypacket(sizeof(tmsg_clientspritesnapshot_dead), netmessage->m_cbSize,
                      msgid_clientspritesnapshot_dead))
        return;
    if (freecam == 0)
        camerafollowsprite = pmsg_clientspritesnapshot_dead(netmessage->m_pData)->camerafocus;
}
