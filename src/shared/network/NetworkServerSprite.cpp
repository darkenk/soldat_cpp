// automatically converted

#include "NetworkServerSprite.hpp"
#include "../../server/Server.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "NetworkUtils.hpp"
#include <steam/isteamnetworkingmessages.h>

std::array<std::array<tmsg_serverspritedelta_movement, max_sprites>, max_sprites> oldmovementmsg;
std::array<std::array<tmsg_serverspritedelta_mouseaim, max_sprites>, max_sprites> oldmouseaimmsg;
std::array<std::array<tmsg_serverspritedelta_weapons, max_sprites>, max_sprites> oldweaponsmsg;
std::array<std::array<tmsg_serverspritedelta_helmet, max_sprites>, max_sprites> oldhelmetmsg;
std::array<tmsg_serverspritesnapshot, max_sprites> oldspritesnapshotmsg;
std::array<std::int32_t, max_sprites> time_spritesnapshot;
std::array<std::int32_t, max_sprites> time_spritesnapshot_mov;

// SERVER SNAPSHOT
void serverspritesnapshot(std::uint8_t r)
{
    tmsg_serverspritesnapshot servermsg;
    std::int32_t i, j;
    tvector2 b;

    // SERVER SPRITES SNAPSHOT
    for (i = 1; i <= max_sprites; i++)
        if (sprite[i].active && !sprite[i].deadmeat && sprite[i].isnotspectator())
        { // active player/sprite
            servermsg.header.id = msgid_serverspritesnapshot;
            // assign sprite values to ServerMsg
            servermsg.pos = spriteparts.pos[i];
            servermsg.velocity = spriteparts.velocity[i];
            servermsg.num = sprite[i].num;
            servermsg.health = sprite[i].health;
            servermsg.position = sprite[i].position;
            servermsg.serverticks = servertickcounter;

            encodekeys(sprite[i], servermsg.keys16);

            servermsg.mouseaimy = sprite[i].control.mouseaimy;
            servermsg.mouseaimx = sprite[i].control.mouseaimx;

            servermsg.look = 0;
            if (sprite[i].wearhelmet == 0)
                servermsg.look = servermsg.look | B1;
            if (sprite[i].hascigar == 5)
                servermsg.look = servermsg.look | B2;
            if (sprite[i].hascigar == 10)
                servermsg.look = servermsg.look | B3;
            if (sprite[i].wearhelmet == 2)
                servermsg.look = servermsg.look | B4;

            servermsg.weaponnum = sprite[i].weapon.num;
            servermsg.secondaryweaponnum = sprite[i].secondaryweapon.num;
            servermsg.ammocount = sprite[i].weapon.ammocount;
            servermsg.grenadecount = sprite[i].tertiaryweapon.ammocount;
            if (sprite[i].vest < 0)
                sprite[i].vest = 0;
            if (sprite[i].vest > defaultvest)
                sprite[i].vest = defaultvest;
            servermsg.vest = sprite[i].vest;

            b = vec2subtract(servermsg.velocity, oldspritesnapshotmsg[i].velocity);

            if (((vec2length(b) > veldelta)) ||
                (maintickcounter - time_spritesnapshot_mov[i] > 30) ||
                (maintickcounter - time_spritesnapshot[i] > 30) ||
                (servermsg.health != oldspritesnapshotmsg[i].health) ||
                (servermsg.position != oldspritesnapshotmsg[i].position) ||
                (servermsg.keys16 != oldspritesnapshotmsg[i].keys16) ||
                (servermsg.weaponnum != oldspritesnapshotmsg[i].weaponnum) ||
                (servermsg.secondaryweaponnum != oldspritesnapshotmsg[i].secondaryweaponnum) ||
                (servermsg.ammocount != oldspritesnapshotmsg[i].ammocount) ||
                (servermsg.grenadecount != oldspritesnapshotmsg[i].grenadecount) ||
                (servermsg.vest != oldspritesnapshotmsg[i].vest))
            {
                // send to all
                if (r == netw)
                    for (j = 1; j <= max_players; j++)
                        if ((sprite[j].active) && (sprite[j].player->controlmethod == human))
                            udp->senddata(&servermsg, sizeof(servermsg), sprite[j].player->peer,
                                          k_nSteamNetworkingSend_Unreliable);
            }
            if (r == netw)
                oldspritesnapshotmsg[i] = servermsg;
        } // sprite[i]
}

// SERVER SNAPSHOT MAJOR
void serverspritesnapshotmajor(std::uint8_t r)
{
    tmsg_serverspritesnapshot_major servermsg;
    std::int32_t i, j;
    tvector2 b;

    // SERVER SPRITES SNAPSHOT
    for (i = 1; i <= max_sprites; i++)
        if (sprite[i].active && !sprite[i].deadmeat && sprite[i].isnotspectator())
        { // active player/sprite
            servermsg.header.id = msgid_serverspritesnapshot_major;
            // assign sprite values to ServerMsg
            servermsg.pos = spriteparts.pos[i];
            servermsg.velocity = spriteparts.velocity[i];
            servermsg.num = sprite[i].num;
            servermsg.health = sprite[i].health;
            servermsg.position = sprite[i].position;
            servermsg.serverticks = servertickcounter;

            encodekeys(sprite[i], servermsg.keys16);

            servermsg.mouseaimy = sprite[i].control.mouseaimy;
            servermsg.mouseaimx = sprite[i].control.mouseaimx;

            b = vec2subtract(servermsg.velocity, oldspritesnapshotmsg[i].velocity);

            if (((vec2length(b) > veldelta)) ||
                ((maintickcounter - time_spritesnapshot_mov[i]) > 30) ||
                (servermsg.position != oldspritesnapshotmsg[i].position) ||
                (servermsg.health != oldspritesnapshotmsg[i].health) ||
                (servermsg.keys16 != oldspritesnapshotmsg[i].keys16))
            {
                // send to all
                if (r == netw)
                    for (j = 1; j <= max_players; j++)
                        if ((sprite[j].active) && (sprite[j].player->controlmethod == human))
                            udp->senddata(&servermsg, sizeof(servermsg), sprite[j].player->peer,
                                          k_nSteamNetworkingSend_Unreliable);
            }

            if (r == netw)
            {
                oldspritesnapshotmsg[i].keys16 = servermsg.keys16;
                oldspritesnapshotmsg[i].position = servermsg.position;
                oldspritesnapshotmsg[i].pos = servermsg.pos;
                oldspritesnapshotmsg[i].velocity = servermsg.velocity;
            }
        } // sprite[i]
}

void serverspritesnapshotmajorfloat(std::uint8_t who, std::uint8_t r)
{
    tmsg_serverspritesnapshot_major servermsg;
    std::int32_t i;

    servermsg.header.id = msgid_serverspritesnapshot_major;
    // assign sprite values to ServerMsg
    servermsg.pos = spriteparts.pos[who];
    servermsg.velocity = spriteparts.velocity[who];
    servermsg.num = sprite[who].num;
    servermsg.health = sprite[who].health;
    servermsg.position = sprite[who].position;
    servermsg.serverticks = servertickcounter;

    encodekeys(sprite[who], servermsg.keys16);

    servermsg.mouseaimy = sprite[who].control.mouseaimy;
    servermsg.mouseaimx = sprite[who].control.mouseaimx;

    // send to all
    if (r == netw)
    {
        for (i = 1; i <= max_players; i++)
            if ((sprite[i].active) && (sprite[i].player->controlmethod == human))
            {
                udp->senddata(&servermsg, sizeof(servermsg), sprite[i].player->peer,
                              k_nSteamNetworkingSend_Unreliable);
            }
    }

    if (r == netw)
    {
        oldspritesnapshotmsg[who].keys16 = servermsg.keys16;
        oldspritesnapshotmsg[who].position = servermsg.position;
        oldspritesnapshotmsg[who].pos = servermsg.pos;
        oldspritesnapshotmsg[who].velocity = servermsg.velocity;
    }
}

// SERVER SKELETON SNAPSHOT
void serverskeletonsnapshot(std::uint8_t r)
{
    tmsg_serverskeletonsnapshot skeletonmsg;
    std::int32_t i, j;

    for (i = 1; i <= max_sprites; i++)
        if (sprite[i].active && sprite[i].deadmeat && sprite[i].isnotspectator())
        { // active player/sprite
            skeletonmsg.header.id = msgid_serverskeletonsnapshot;
            // assign sprite values to SkeletonMsg
            skeletonmsg.num = sprite[i].num;
            if (sprite[i].respawncounter > 0)
                skeletonmsg.respawncounter = sprite[i].respawncounter;
            else
                skeletonmsg.respawncounter = 0;

            // send to all
            if (r == netw)
                for (j = 1; j <= max_players; j++)
                    if ((sprite[j].active) && (sprite[j].player->controlmethod == human))
                        udp->senddata(&skeletonmsg, sizeof(skeletonmsg), sprite[j].player->peer,
                                      k_nSteamNetworkingSend_Unreliable);
        }
}

void serverspritedeath(std::int32_t who, std::int32_t killer, std::int32_t bulletnum,
                       std::int32_t where)
{
    tmsg_spritedeath spritedeathmsg;
    std::int32_t j;

    spritedeathmsg.header.id = msgid_spritedeath;
    // assign sprite values to SpriteDeathMsg
    spritedeathmsg.num = who;
    spritedeathmsg.killer = killer;
    spritedeathmsg.where = where;
    spritedeathmsg.respawncounter = sprite[who].respawncounter;
    spritedeathmsg.health = sprite[who].health;
    spritedeathmsg.onfire = sprite[who].onfire;
    spritedeathmsg.shotdistance = shotdistance;
    spritedeathmsg.shotlife = shotlife;
    spritedeathmsg.shotricochet = shotricochet;

    if (bulletnum == -1)
        spritedeathmsg.killbullet = 250;
    else
    {
        // if Bullet[BulletNum].OwnerWeapon = 0 then
        // SpriteDeathMsg.KillBullet := 250;
        spritedeathmsg.killbullet = bullet[bulletnum].ownerweapon;
        if (bullet[bulletnum].style == 2)
            spritedeathmsg.killbullet = 222;
        if (bullet[bulletnum].style == 10)
            spritedeathmsg.killbullet = 210;
        if (bullet[bulletnum].style == 5)
            spritedeathmsg.killbullet = 205;
        if (bullet[bulletnum].style == 7)
            spritedeathmsg.killbullet = 207;
        if (bullet[bulletnum].style == 8)
            spritedeathmsg.killbullet = 208;
        if (bullet[bulletnum].style == 6)
            spritedeathmsg.killbullet = 206;
        if (bullet[bulletnum].ownerweapon == guns[knife].num)
            spritedeathmsg.killbullet = 211;
        if (bullet[bulletnum].ownerweapon == guns[chainsaw].num)
            spritedeathmsg.killbullet = 212;
        if (bullet[bulletnum].style == 12)
            spritedeathmsg.killbullet = 224;
        if (bullet[bulletnum].style == 13)
            spritedeathmsg.killbullet = 211;
        if (bullet[bulletnum].style == 14)
            spritedeathmsg.killbullet = 225;
    }

    for (j = 1; j <= 16; j++)
    {
        spritedeathmsg.pos[j].x = sprite[who].skeleton.pos[j].x;
        spritedeathmsg.pos[j].y = sprite[who].skeleton.pos[j].y;
        spritedeathmsg.oldpos[j].x = sprite[who].skeleton.oldpos[j].x;
        spritedeathmsg.oldpos[j].y = sprite[who].skeleton.oldpos[j].y;
    }

    spritedeathmsg.constraints = 0;
    if (!sprite[who].skeleton.constraints[2].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B1;
    if (!sprite[who].skeleton.constraints[4].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B2;
    if (!sprite[who].skeleton.constraints[20].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B3;
    if (!sprite[who].skeleton.constraints[21].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B4;
    if (!sprite[who].skeleton.constraints[23].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B5;

    for (j = 1; j <= max_players; j++)
        if ((sprite[j].active) && (sprite[j].player->controlmethod == human))
            udp->senddata(&spritedeathmsg, sizeof(spritedeathmsg), sprite[j].player->peer,
                          k_nSteamNetworkingSend_Unreliable);
}

// SEND DELTAS OF SPRITE
void serverspritedeltas(std::uint8_t i)
{
    tmsg_serverspritedelta_movement movementmsg;
    tmsg_serverspritedelta_weapons weaponsmsg;
    tmsg_serverspritedelta_helmet helmetmsg;
    std::int32_t j;
    tvector2 a, b;

    helmetmsg.header.id = msgid_delta_helmet;
    helmetmsg.num = i;
    helmetmsg.wearhelmet = sprite[i].wearhelmet;

    movementmsg.header.id = msgid_delta_movement;
    movementmsg.num = i;

    movementmsg.velocity = spriteparts.velocity[i];
    movementmsg.pos = spriteparts.pos[i];
    movementmsg.servertick = servertickcounter;

    encodekeys(sprite[i], movementmsg.keys16);

    movementmsg.mouseaimy = sprite[i].control.mouseaimy;
    movementmsg.mouseaimx = sprite[i].control.mouseaimx;

    weaponsmsg.header.id = msgid_delta_weapons;
    weaponsmsg.num = i;
    weaponsmsg.weaponnum = sprite[i].weapon.num;
    weaponsmsg.secondaryweaponnum = sprite[i].secondaryweapon.num;
    weaponsmsg.ammocount = sprite[i].weapon.ammocount;

    for (j = 1; j <= max_sprites; j++)
        if (sprite[j].active && (sprite[j].player->controlmethod == human) && (j != i))
            if (pointvisible(spriteparts.pos[i].x, spriteparts.pos[i].y,
                             sprite[j].player->camera) or
                (sprite[j].isspectator() && (sprite[j].player->port == 0))) // visible to sprite
            {
                a = vec2subtract(movementmsg.pos, oldmovementmsg[j][i].pos);
                b = vec2subtract(movementmsg.velocity, oldmovementmsg[j][i].velocity);
                if ((sprite[i].player->controlmethod == human) ||
                    (((vec2length(a) > posdelta) || (vec2length(b) > veldelta)) &&
                     (movementmsg.keys16 != oldmovementmsg[j][i].keys16)))
                {
                    udp->senddata(&movementmsg, sizeof(movementmsg), sprite[j].player->peer,
                                  k_nSteamNetworkingSend_Unreliable);
                    oldmovementmsg[j][i] = movementmsg;
                }
            }

    for (j = 1; j <= max_sprites; j++)
        if (sprite[j].active && (sprite[j].player->controlmethod == human) && (j != i))
            if ((weaponsmsg.weaponnum != oldweaponsmsg[j][i].weaponnum) ||
                (weaponsmsg.secondaryweaponnum != oldweaponsmsg[j][i].secondaryweaponnum))
                if (pointvisible(spriteparts.pos[i].x, spriteparts.pos[i].y,
                                 sprite[j].player->camera) or
                    (sprite[j].isspectator() && (sprite[j].player->port == 0))) // visible to sprite
                {
                    udp->senddata(&weaponsmsg, sizeof(weaponsmsg), sprite[j].player->peer,
                                  k_nSteamNetworkingSend_Unreliable);
                    oldweaponsmsg[j][i] = weaponsmsg;
                }

    for (j = 1; j <= max_sprites; j++)
        if ((sprite[j].active) && (sprite[j].player->controlmethod == human) && (j != i))
            if (helmetmsg.wearhelmet != oldhelmetmsg[j][i].wearhelmet)
            {
                udp->senddata(&helmetmsg, sizeof(helmetmsg), sprite[j].player->peer,
                              k_nSteamNetworkingSend_Unreliable);
                oldhelmetmsg[j][i] = helmetmsg;
            }
}

void serverspritedeltasmouse(std::uint8_t i)
{
    tmsg_serverspritedelta_mouseaim mouseaimmsg;
    std::int32_t j;

    mouseaimmsg.header.id = msgid_delta_mouseaim;
    mouseaimmsg.num = i;
    mouseaimmsg.mouseaimy = sprite[i].control.mouseaimy;
    mouseaimmsg.mouseaimx = sprite[i].control.mouseaimx;

    for (j = 1; j <= max_sprites; j++)
        if ((sprite[j].active) && (sprite[j].player->controlmethod == human) && (j != i))
        {
            udp->senddata(&mouseaimmsg, sizeof(mouseaimmsg), sprite[j].player->peer,
                          k_nSteamNetworkingSend_Unreliable);
            oldmouseaimmsg[j][i] = mouseaimmsg;
        }
}

void serverhandleclientspritesnapshot(SteamNetworkingMessage_t *netmessage)
{
    pmsg_clientspritesnapshot clientmsg;
    tplayer *player;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_clientspritesnapshot), netmessage->m_cbSize,
                      msgid_clientspritesnapshot))
        return;
    clientmsg = pmsg_clientspritesnapshot(netmessage->m_pData);
    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    i = player->spritenum;

    messagesasecnum[i] += 1;

    if (sprite[i].deadmeat)
        return;

    sprite[i].player->camera = i;

#ifdef SCRIPT
    forceweaponcalled = false;
    if ((sprite[i].weapon.num != clientmsg.weaponnum) ||
        (sprite[i].secondaryweapon.num != clientmsg.secondaryweaponnum))
    {
        // event must be before actual weapon apply.
        // script might've called ForceWeapon, which we should check.
        // if it did, we don't apply snapshot weapon's as they were already applied
        // by force weapon.
        scrptdispatcher.onweaponchange(i, clientmsg.weaponnum, clientmsg.secondaryweaponnum,
                                       clientmsg.ammocount, clientmsg.secondaryammocount);
    }
    if (!forceweaponcalled)
    {
#endif
        if (sprite[i].weapon.num != clientmsg->weaponnum)
        {
            sprite[i].applyweaponbynum(clientmsg->weaponnum, 1, clientmsg->ammocount);
        }
        if (sprite[i].secondaryweapon.num != clientmsg->secondaryweaponnum)
        {
            sprite[i].applyweaponbynum(clientmsg->secondaryweaponnum, 2,
                                       clientmsg->secondaryammocount);
        }
#ifdef SCRIPT
    }
#endif

    if (sprite[i].weapon.num == guns[colt].num)
        sprite[i].player->secwep = 0;
    if (sprite[i].weapon.num == guns[knife].num)
        sprite[i].player->secwep = 1;
    if (sprite[i].weapon.num == guns[chainsaw].num)
        sprite[i].player->secwep = 2;
    if (sprite[i].weapon.num == guns[law].num)
        sprite[i].player->secwep = 3;

    sprite[i].weapon.ammocount = clientmsg->ammocount;
    sprite[i].secondaryweapon.ammocount = clientmsg->secondaryammocount;

    // Toggle prone if it was activated or deactivated
    sprite[i].control.prone =
        (clientmsg->position == pos_prone) ^ (sprite[i].position == pos_prone);

    if (checkweaponnotallowed(i))
    {
        kickplayer(i, true, kick_cheat, day, "Not allowed weapon");
        return;
    }

    serverspritedeltas(i);

    time_spritesnapshot[i] = maintickcounter;
}
void serverhandleclientspritesnapshot_mov(SteamNetworkingMessage_t *netmessage)
{
    tmsg_clientspritesnapshot_mov clientmovmsg;
    tplayer player;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_clientspritesnapshot_mov), netmessage->m_cbSize,
                      msgid_clientspritesnapshot_mov))
        return;

    NotImplemented(NITag::NETWORK);
#if 0
    clientmovmsg = pmsg_clientspritesnapshot_mov(netmessage->m_pData);
#endif
    player = *reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    i = player.spritenum;

    messagesasecnum[i] += 1;

    if (sprite[i].deadmeat)
        return;

    checkoutofbounds(clientmovmsg.pos.x, clientmovmsg.pos.y);
    checkoutofbounds(clientmovmsg.velocity.x, clientmovmsg.velocity.y);

    sprite[i].player->camera = i;

    spriteparts.pos[i] = clientmovmsg.pos;
    spriteparts.velocity[i] = clientmovmsg.velocity;

    checkoutofbounds(clientmovmsg.mouseaimx, clientmovmsg.mouseaimy);

    sprite[i].control.mouseaimx = clientmovmsg.mouseaimx;
    sprite[i].control.mouseaimy = clientmovmsg.mouseaimy;

    decodekeys(sprite[i], clientmovmsg.keys16);

    if (sprite[i].control.throwweapon == false)
        sprite[i].player->knifewarnings = 0;

    serverspritedeltas(i);

    time_spritesnapshot_mov[i] = maintickcounter;
}

void serverhandleclientspritesnapshot_dead(SteamNetworkingMessage_t *netmessage)
{
    pmsg_clientspritesnapshot_dead clientdeadmsg;
    tplayer *player;
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_clientspritesnapshot_dead), netmessage->m_cbSize,
                      msgid_clientspritesnapshot_dead))
        return;
    clientdeadmsg = pmsg_clientspritesnapshot_dead(netmessage->m_pData);
    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    i = player->spritenum;

    messagesasecnum[i] += 1;

    if (!sprite[i].deadmeat)
        return;

    // assign received sprite info to sprite
    if (clientdeadmsg->camerafocus < max_sprites + 1)
        sprite[i].player->camera = clientdeadmsg->camerafocus;
}
