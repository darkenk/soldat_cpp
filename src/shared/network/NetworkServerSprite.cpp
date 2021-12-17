// automatically converted

#include "NetworkServerSprite.hpp"
#include "../../server/Server.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "NetworkUtils.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include <steam/isteamnetworkingmessages.h>

// clang-format off
#include "shared/misc/GlobalVariableStorage.cpp"
// clang-format on

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
        if (SpriteSystem::Get().GetSprite(i).active && !SpriteSystem::Get().GetSprite(i).deadmeat &&
            SpriteSystem::Get().GetSprite(i).isnotspectator())
        { // active player/sprite
            servermsg.header.id = msgid_serverspritesnapshot;
            // assign sprite values to ServerMsg
            servermsg.pos = spriteparts.pos[i];
            servermsg.velocity = spriteparts.velocity[i];
            servermsg.num = SpriteSystem::Get().GetSprite(i).num;
            servermsg.health = SpriteSystem::Get().GetSprite(i).GetHealth();
            servermsg.position = SpriteSystem::Get().GetSprite(i).position;
            servermsg.serverticks = servertickcounter;

            encodekeys(SpriteSystem::Get().GetSprite(i), servermsg.keys16);

            servermsg.mouseaimy = SpriteSystem::Get().GetSprite(i).control.mouseaimy;
            servermsg.mouseaimx = SpriteSystem::Get().GetSprite(i).control.mouseaimx;

            servermsg.look = 0;
            if (SpriteSystem::Get().GetSprite(i).wearhelmet == 0)
                servermsg.look = servermsg.look | B1;
            if (SpriteSystem::Get().GetSprite(i).hascigar == 5)
                servermsg.look = servermsg.look | B2;
            if (SpriteSystem::Get().GetSprite(i).hascigar == 10)
                servermsg.look = servermsg.look | B3;
            if (SpriteSystem::Get().GetSprite(i).wearhelmet == 2)
                servermsg.look = servermsg.look | B4;

            servermsg.weaponnum = SpriteSystem::Get().GetSprite(i).weapon.num;
            servermsg.secondaryweaponnum = SpriteSystem::Get().GetSprite(i).secondaryweapon.num;
            servermsg.ammocount = SpriteSystem::Get().GetSprite(i).weapon.ammocount;
            servermsg.grenadecount = SpriteSystem::Get().GetSprite(i).tertiaryweapon.ammocount;
            if (SpriteSystem::Get().GetSprite(i).vest < 0)
                SpriteSystem::Get().GetSprite(i).vest = 0;
            if (SpriteSystem::Get().GetSprite(i).vest > defaultvest)
                SpriteSystem::Get().GetSprite(i).vest = defaultvest;
            servermsg.vest = SpriteSystem::Get().GetSprite(i).vest;

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
                        if ((SpriteSystem::Get().GetSprite(j).active) &&
                            (SpriteSystem::Get().GetSprite(j).player->controlmethod == human))
                            GetServerNetwork()->senddata(
                                &servermsg, sizeof(servermsg),
                                SpriteSystem::Get().GetSprite(j).player->peer,
                                k_nSteamNetworkingSend_Unreliable);
            }
            if (r == netw)
                oldspritesnapshotmsg[i] = servermsg;
        } // SpriteSystem::Get().GetSprite(i)
}

// SERVER SNAPSHOT MAJOR
void serverspritesnapshotmajor(std::uint8_t r)
{
    tmsg_serverspritesnapshot_major servermsg;
    std::int32_t i, j;
    tvector2 b;

    // SERVER SPRITES SNAPSHOT
    for (i = 1; i <= max_sprites; i++)
        if (SpriteSystem::Get().GetSprite(i).active && !SpriteSystem::Get().GetSprite(i).deadmeat &&
            SpriteSystem::Get().GetSprite(i).isnotspectator())
        { // active player/sprite
            servermsg.header.id = msgid_serverspritesnapshot_major;
            // assign sprite values to ServerMsg
            servermsg.pos = spriteparts.pos[i];
            servermsg.velocity = spriteparts.velocity[i];
            servermsg.num = SpriteSystem::Get().GetSprite(i).num;
            servermsg.health = SpriteSystem::Get().GetSprite(i).GetHealth();
            servermsg.position = SpriteSystem::Get().GetSprite(i).position;
            servermsg.serverticks = servertickcounter;

            encodekeys(SpriteSystem::Get().GetSprite(i), servermsg.keys16);

            servermsg.mouseaimy = SpriteSystem::Get().GetSprite(i).control.mouseaimy;
            servermsg.mouseaimx = SpriteSystem::Get().GetSprite(i).control.mouseaimx;

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
                        if ((SpriteSystem::Get().GetSprite(j).active) &&
                            (SpriteSystem::Get().GetSprite(j).player->controlmethod == human))
                            GetServerNetwork()->senddata(
                                &servermsg, sizeof(servermsg),
                                SpriteSystem::Get().GetSprite(j).player->peer,
                                k_nSteamNetworkingSend_Unreliable);
            }

            if (r == netw)
            {
                oldspritesnapshotmsg[i].keys16 = servermsg.keys16;
                oldspritesnapshotmsg[i].position = servermsg.position;
                oldspritesnapshotmsg[i].pos = servermsg.pos;
                oldspritesnapshotmsg[i].velocity = servermsg.velocity;
            }
        } // SpriteSystem::Get().GetSprite(i)
}

void serverspritesnapshotmajorfloat(std::uint8_t who, std::uint8_t r)
{
    tmsg_serverspritesnapshot_major servermsg;
    std::int32_t i;

    servermsg.header.id = msgid_serverspritesnapshot_major;
    // assign sprite values to ServerMsg
    servermsg.pos = spriteparts.pos[who];
    servermsg.velocity = spriteparts.velocity[who];
    servermsg.num = SpriteSystem::Get().GetSprite(who).num;
    servermsg.health = SpriteSystem::Get().GetSprite(who).GetHealth();
    servermsg.position = SpriteSystem::Get().GetSprite(who).position;
    servermsg.serverticks = servertickcounter;

    encodekeys(SpriteSystem::Get().GetSprite(who), servermsg.keys16);

    servermsg.mouseaimy = SpriteSystem::Get().GetSprite(who).control.mouseaimy;
    servermsg.mouseaimx = SpriteSystem::Get().GetSprite(who).control.mouseaimx;

    // send to all
    if (r == netw)
    {
        for (i = 1; i <= max_players; i++)
            if ((SpriteSystem::Get().GetSprite(i).active) &&
                (SpriteSystem::Get().GetSprite(i).player->controlmethod == human))
            {
                GetServerNetwork()->senddata(&servermsg, sizeof(servermsg),
                                             SpriteSystem::Get().GetSprite(i).player->peer,
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
        if (SpriteSystem::Get().GetSprite(i).active && SpriteSystem::Get().GetSprite(i).deadmeat &&
            SpriteSystem::Get().GetSprite(i).isnotspectator())
        { // active player/sprite
            skeletonmsg.header.id = msgid_serverskeletonsnapshot;
            // assign sprite values to SkeletonMsg
            skeletonmsg.num = SpriteSystem::Get().GetSprite(i).num;
            if (SpriteSystem::Get().GetSprite(i).respawncounter > 0)
                skeletonmsg.respawncounter = SpriteSystem::Get().GetSprite(i).respawncounter;
            else
                skeletonmsg.respawncounter = 0;

            // send to all
            if (r == netw)
                for (j = 1; j <= max_players; j++)
                    if ((SpriteSystem::Get().GetSprite(j).active) &&
                        (SpriteSystem::Get().GetSprite(j).player->controlmethod == human))
                        GetServerNetwork()->senddata(&skeletonmsg, sizeof(skeletonmsg),
                                                     SpriteSystem::Get().GetSprite(j).player->peer,
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
    spritedeathmsg.respawncounter = SpriteSystem::Get().GetSprite(who).respawncounter;
    spritedeathmsg.health = SpriteSystem::Get().GetSprite(who).GetHealth();
    spritedeathmsg.onfire = SpriteSystem::Get().GetSprite(who).onfire;
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
        spritedeathmsg.pos[j].x = SpriteSystem::Get().GetSprite(who).skeleton.pos[j].x;
        spritedeathmsg.pos[j].y = SpriteSystem::Get().GetSprite(who).skeleton.pos[j].y;
        spritedeathmsg.oldpos[j].x = SpriteSystem::Get().GetSprite(who).skeleton.oldpos[j].x;
        spritedeathmsg.oldpos[j].y = SpriteSystem::Get().GetSprite(who).skeleton.oldpos[j].y;
    }

    spritedeathmsg.constraints = 0;
    if (!SpriteSystem::Get().GetSprite(who).skeleton.constraints[2].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B1;
    if (!SpriteSystem::Get().GetSprite(who).skeleton.constraints[4].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B2;
    if (!SpriteSystem::Get().GetSprite(who).skeleton.constraints[20].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B3;
    if (!SpriteSystem::Get().GetSprite(who).skeleton.constraints[21].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B4;
    if (!SpriteSystem::Get().GetSprite(who).skeleton.constraints[23].active)
        spritedeathmsg.constraints = spritedeathmsg.constraints | B5;

    for (j = 1; j <= max_players; j++)
        if ((SpriteSystem::Get().GetSprite(j).active) &&
            (SpriteSystem::Get().GetSprite(j).player->controlmethod == human))
            GetServerNetwork()->senddata(&spritedeathmsg, sizeof(spritedeathmsg),
                                         SpriteSystem::Get().GetSprite(j).player->peer,
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
    helmetmsg.wearhelmet = SpriteSystem::Get().GetSprite(i).wearhelmet;

    movementmsg.header.id = msgid_delta_movement;
    movementmsg.num = i;

    movementmsg.velocity = spriteparts.velocity[i];
    movementmsg.pos = spriteparts.pos[i];
    movementmsg.servertick = servertickcounter;

    encodekeys(SpriteSystem::Get().GetSprite(i), movementmsg.keys16);

    movementmsg.mouseaimy = SpriteSystem::Get().GetSprite(i).control.mouseaimy;
    movementmsg.mouseaimx = SpriteSystem::Get().GetSprite(i).control.mouseaimx;

    weaponsmsg.header.id = msgid_delta_weapons;
    weaponsmsg.num = i;
    weaponsmsg.weaponnum = SpriteSystem::Get().GetSprite(i).weapon.num;
    weaponsmsg.secondaryweaponnum = SpriteSystem::Get().GetSprite(i).secondaryweapon.num;
    weaponsmsg.ammocount = SpriteSystem::Get().GetSprite(i).weapon.ammocount;

    for (j = 1; j <= max_sprites; j++)
        if (SpriteSystem::Get().GetSprite(j).active &&
            (SpriteSystem::Get().GetSprite(j).player->controlmethod == human) && (j != i))
            if (pointvisible(spriteparts.pos[i].x, spriteparts.pos[i].y,
                             SpriteSystem::Get().GetSprite(j).player->camera) or
                (SpriteSystem::Get().GetSprite(j).isspectator() &&
                 (SpriteSystem::Get().GetSprite(j).player->port == 0))) // visible to sprite
            {
                a = vec2subtract(movementmsg.pos, oldmovementmsg[j][i].pos);
                b = vec2subtract(movementmsg.velocity, oldmovementmsg[j][i].velocity);
                if ((SpriteSystem::Get().GetSprite(i).player->controlmethod == human) ||
                    (((vec2length(a) > posdelta) || (vec2length(b) > veldelta)) &&
                     (movementmsg.keys16 != oldmovementmsg[j][i].keys16)))
                {
                    GetServerNetwork()->senddata(&movementmsg, sizeof(movementmsg),
                                                 SpriteSystem::Get().GetSprite(j).player->peer,
                                                 k_nSteamNetworkingSend_Unreliable);
                    oldmovementmsg[j][i] = movementmsg;
                }
            }

    for (j = 1; j <= max_sprites; j++)
        if (SpriteSystem::Get().GetSprite(j).active &&
            (SpriteSystem::Get().GetSprite(j).player->controlmethod == human) && (j != i))
            if ((weaponsmsg.weaponnum != oldweaponsmsg[j][i].weaponnum) ||
                (weaponsmsg.secondaryweaponnum != oldweaponsmsg[j][i].secondaryweaponnum))
                if (pointvisible(spriteparts.pos[i].x, spriteparts.pos[i].y,
                                 SpriteSystem::Get().GetSprite(j).player->camera) or
                    (SpriteSystem::Get().GetSprite(j).isspectator() &&
                     (SpriteSystem::Get().GetSprite(j).player->port == 0))) // visible to sprite
                {
                    GetServerNetwork()->senddata(&weaponsmsg, sizeof(weaponsmsg),
                                                 SpriteSystem::Get().GetSprite(j).player->peer,
                                                 k_nSteamNetworkingSend_Unreliable);
                    oldweaponsmsg[j][i] = weaponsmsg;
                }

    for (j = 1; j <= max_sprites; j++)
        if ((SpriteSystem::Get().GetSprite(j).active) &&
            (SpriteSystem::Get().GetSprite(j).player->controlmethod == human) && (j != i))
            if (helmetmsg.wearhelmet != oldhelmetmsg[j][i].wearhelmet)
            {
                GetServerNetwork()->senddata(&helmetmsg, sizeof(helmetmsg),
                                             SpriteSystem::Get().GetSprite(j).player->peer,
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
    mouseaimmsg.mouseaimy = SpriteSystem::Get().GetSprite(i).control.mouseaimy;
    mouseaimmsg.mouseaimx = SpriteSystem::Get().GetSprite(i).control.mouseaimx;

    for (j = 1; j <= max_sprites; j++)
        if ((SpriteSystem::Get().GetSprite(j).active) &&
            (SpriteSystem::Get().GetSprite(j).player->controlmethod == human) && (j != i))
        {
            GetServerNetwork()->senddata(&mouseaimmsg, sizeof(mouseaimmsg),
                                         SpriteSystem::Get().GetSprite(j).player->peer,
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

    if (SpriteSystem::Get().GetSprite(i).deadmeat)
        return;

    SpriteSystem::Get().GetSprite(i).player->camera = i;

#ifdef SCRIPT
    forceweaponcalled = false;
    if ((SpriteSystem::Get().GetSprite(i).weapon.num != clientmsg.weaponnum) ||
        (SpriteSystem::Get().GetSprite(i).secondaryweapon.num != clientmsg.secondaryweaponnum))
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
        if (SpriteSystem::Get().GetSprite(i).weapon.num != clientmsg->weaponnum)
        {
            SpriteSystem::Get().GetSprite(i).applyweaponbynum(clientmsg->weaponnum, 1,
                                                              clientmsg->ammocount);
        }
        if (SpriteSystem::Get().GetSprite(i).secondaryweapon.num != clientmsg->secondaryweaponnum)
        {
            SpriteSystem::Get().GetSprite(i).applyweaponbynum(clientmsg->secondaryweaponnum, 2,
                                                              clientmsg->secondaryammocount);
        }
#ifdef SCRIPT
    }
#endif

    if (SpriteSystem::Get().GetSprite(i).weapon.num == guns[colt].num)
        SpriteSystem::Get().GetSprite(i).player->secwep = 0;
    if (SpriteSystem::Get().GetSprite(i).weapon.num == guns[knife].num)
        SpriteSystem::Get().GetSprite(i).player->secwep = 1;
    if (SpriteSystem::Get().GetSprite(i).weapon.num == guns[chainsaw].num)
        SpriteSystem::Get().GetSprite(i).player->secwep = 2;
    if (SpriteSystem::Get().GetSprite(i).weapon.num == guns[law].num)
        SpriteSystem::Get().GetSprite(i).player->secwep = 3;

    SpriteSystem::Get().GetSprite(i).weapon.ammocount = clientmsg->ammocount;
    SpriteSystem::Get().GetSprite(i).secondaryweapon.ammocount = clientmsg->secondaryammocount;

    // Toggle prone if it was activated or deactivated
    SpriteSystem::Get().GetSprite(i).control.prone =
        (clientmsg->position == pos_prone) ^
        (SpriteSystem::Get().GetSprite(i).position == pos_prone);

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
    std::int32_t i;

    if (!verifypacket(sizeof(tmsg_clientspritesnapshot_mov), netmessage->m_cbSize,
                      msgid_clientspritesnapshot_mov))
        return;

    tmsg_clientspritesnapshot_mov &clientmovmsg =
        *pmsg_clientspritesnapshot_mov(netmessage->m_pData);
    tplayer &player = *reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    i = player.spritenum;

    messagesasecnum[i] += 1;

    if (SpriteSystem::Get().GetSprite(i).deadmeat)
        return;

    map.checkoutofbounds(clientmovmsg.pos.x, clientmovmsg.pos.y);
    map.checkoutofbounds(clientmovmsg.velocity.x, clientmovmsg.velocity.y);

    SpriteSystem::Get().GetSprite(i).player->camera = i;

    spriteparts.pos[i] = clientmovmsg.pos;
    spriteparts.velocity[i] = clientmovmsg.velocity;

    map.checkoutofbounds(clientmovmsg.mouseaimx, clientmovmsg.mouseaimy);

    SpriteSystem::Get().GetSprite(i).control.mouseaimx = clientmovmsg.mouseaimx;
    SpriteSystem::Get().GetSprite(i).control.mouseaimy = clientmovmsg.mouseaimy;

    decodekeys(SpriteSystem::Get().GetSprite(i), clientmovmsg.keys16);

    if (SpriteSystem::Get().GetSprite(i).control.throwweapon == false)
        SpriteSystem::Get().GetSprite(i).player->knifewarnings = 0;

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

    if (!SpriteSystem::Get().GetSprite(i).deadmeat)
        return;

    // assign received sprite info to sprite
    if (clientdeadmsg->camerafocus < max_sprites + 1)
        SpriteSystem::Get().GetSprite(i).player->camera = clientdeadmsg->camerafocus;
}
