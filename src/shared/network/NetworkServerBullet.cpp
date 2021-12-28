// automatically converted
#include "NetworkServerBullet.hpp"

#include "../../server/Server.hpp"
#include "../Game.hpp"
#include "../mechanics/Bullets.hpp"
#include "NetworkUtils.hpp"
#include "common/Calc.hpp"
#include "shared/mechanics/SpriteSystem.hpp"

//clang-format off
#include "../misc/GlobalVariableStorage.cpp"
// clang-format on

namespace
{
std::array<tmsg_bulletsnapshot, max_sprites> oldbulletsnapshotmsg;
}

void serverbulletsnapshot(std::uint8_t i, std::uint8_t tonum, bool forced)
{
    tmsg_bulletsnapshot bulletmsg;

    // SERVER BULLETS SNAPSHOT
    bulletmsg.header.id = msgid_bulletsnapshot;
    bulletmsg.owner = bullet[i].owner;
    bulletmsg.weaponnum = bullet[i].ownerweapon;
    bulletmsg.pos = bulletparts.pos[i];
    bulletmsg.velocity = bulletparts.velocity[i];
    bulletmsg.seed = bullet[i].seed;
    bulletmsg.forced = forced;

#ifdef SERVER
    if (!forced)
        if ((SpriteSystem::Get().GetSprite(bulletmsg.owner).weapon.ammocount > 0) &&
            (bullet[i].style != bullet_style_fragnade) &&
            (bullet[i].style != bullet_style_clusternade) &&
            (bullet[i].style != bullet_style_cluster))
            SpriteSystem::Get().GetSprite(bulletmsg.owner).weapon.ammocount -= 1;

    for (auto &sprite : SpriteSystem::Get().GetActiveSprites())
    {
        if ((sprite.player->controlmethod == human) && (sprite.num != bullet[i].owner))
            if ((tonum == 0) || (sprite.num == tonum))
                if (bulletcansend(bulletparts.pos[i].x, bulletparts.pos[i].y, sprite.player->camera,
                                  bulletparts.velocity[i].x) or
                    forced)
                {
                    GetServerNetwork()->senddata(&bulletmsg, sizeof(bulletmsg), sprite.player->peer,
                                                 k_nSteamNetworkingSend_Unreliable);
                }
    }
#else
    demorecorder.saverecord(bulletmsg, sizeof(bulletmsg));
#endif
}

#ifdef SERVER
void serverhandlebulletsnapshot(SteamNetworkingMessage_t *netmessage)
{
    tmsg_clientbulletsnapshot *bulletsnap;
    tplayer *player;
    std::int32_t p, d;
    float k;
    std::int32_t i;
    bool onstatgun;
    bool failedbulletcheck;
    std::int16_t weaponindex;
    std::uint8_t style;
    tvector2 a, b, bx;
    tvector2 bstraight;
    tvector2 bnorm;
    float bulletspread;

    if (!verifypacket(sizeof(tmsg_clientbulletsnapshot), netmessage->m_cbSize,
                      msgid_bulletsnapshot))
        return;

    bulletsnap = pmsg_clientbulletsnapshot(netmessage->m_pData);
    player = reinterpret_cast<tplayer *>(netmessage->m_nConnUserData);
    p = player->spritenum;

    messagesasecnum[p] += 1;

    SpriteSystem::Get().GetSprite(p).player->pingticksb =
        servertickcounter - bulletsnap->clientticks;
    if (SpriteSystem::Get().GetSprite(p).player->pingticksb < 0)
        SpriteSystem::Get().GetSprite(p).player->pingticksb = 0;
    if (SpriteSystem::Get().GetSprite(p).player->pingticksb > max_oldpos)
        SpriteSystem::Get().GetSprite(p).player->pingticksb = max_oldpos;

    weaponindex = weaponnumtoindex(bulletsnap->weaponnum, guns);
    if (weaponindex == -1)
        return;

    style = guns[weaponindex].bulletstyle;

    // Check for duplicated bullets
    // Using a ringbuffer of saved references of old ones
    failedbulletcheck = false;
    for (i = 0; i <= SpriteSystem::Get().GetSprite(p).bulletcheckamount; i++)
    {
        if (SpriteSystem::Get().GetSprite(p).bulletcheck[i] == bulletsnap->seed)
        {
            failedbulletcheck = true;
            break;
        }
    }
    if (failedbulletcheck)
    {
        // ignore duplicate packet
        return;
    }
    else
    {
        if (SpriteSystem::Get().GetSprite(p).bulletcheckindex > bulletcheckarraysize - 1)
        {
            SpriteSystem::Get().GetSprite(p).bulletcheckindex = 0;
        }
        if (SpriteSystem::Get().GetSprite(p).bulletcheckamount < bulletcheckarraysize - 1)
        {
            SpriteSystem::Get().GetSprite(p).bulletcheckamount += 1;
        }
        SpriteSystem::Get()
            .GetSprite(p)
            .bulletcheck[SpriteSystem::Get().GetSprite(p).bulletcheckindex] = bulletsnap->seed;
        SpriteSystem::Get().GetSprite(p).bulletcheckindex += 1;
    }

    // spec kill: spectators NEVER send bullet snapshots
    if (SpriteSystem::Get().GetSprite(p).isspectator())
        return;

    // Disable knife-cheat kick for now, until we have timestamped
    // packets, and will do a time-based comparison to sprite snapshot
    // with info about switching primary to secondary
    /*if (BulletSnap.Style <> SpriteSystem::Get().GetSprite(p).Weapon.BulletStyle) then
      if ((BulletSnap.Style = 13) and
        (SpriteSystem::Get().GetSprite(p).Weapon.BulletStyle <> 11) and
        (SpriteSystem::Get().GetSprite(p).Weapon.Num <> NoWeapon.Num)) then
      begin
        KickPlayer(p, True, KICK_CHEAT, DAY, 'Knife Cheat');
        Exit
      end;*/

    if ((style == bullet_style_thrownknife) && (weaponactive[knife] == 0))
    {
        if (kickplayer(p, true, kick_cheat, day, "Knife-Spawn Cheat"))
            return;
    }

    if (style == bullet_style_m2)
    {
        onstatgun = false;
        for (i = 0; i <= max_things; i++)
            if ((things[i].style == object_stationary_gun) /*Stat Gun*/ && (onstatgun == false))
                if (distance(SpriteSystem::Get().GetSprite(p).skeleton.pos[1].x,
                             SpriteSystem::Get().GetSprite(p).skeleton.pos[1].y,
                             things[i].skeleton.pos[1].x,
                             things[i].skeleton.pos[1].y) < stat_radius * 2)
                    onstatgun = true;
        if (!onstatgun)
        {
            // KickPlayer(p, True, KICK_CHEAT, DAY, 'StatGun Cheat');
            return;
        }
    }

    map.checkoutofbounds(bulletsnap->pos.x, bulletsnap->pos.y);

    if ((style < bullet_style_plain) || (style > bullet_style_m2))
        return;

    if ((style != bullet_style_fragnade) && (style != bullet_style_punch) &&
        (style != bullet_style_clusternade) && (style != bullet_style_cluster) &&
        (style != bullet_style_thrownknife) && (style != bullet_style_m2) &&
        (SpriteSystem::Get().GetSprite(p).lastweaponstyle != style))
        return;

    if ((oldbulletsnapshotmsg[p].weaponnum == bulletsnap->weaponnum) &&
        (oldbulletsnapshotmsg[p].pos.x == bulletsnap->pos.x) &&
        (oldbulletsnapshotmsg[p].pos.y == bulletsnap->pos.y) &&
        (oldbulletsnapshotmsg[p].velocity.x == bulletsnap->velocity.x) &&
        (oldbulletsnapshotmsg[p].velocity.y == bulletsnap->velocity.y))
        return;

    if ((style != bullet_style_fragnade) && (style != bullet_style_clusternade) &&
        (style != bullet_style_cluster) && (style != bullet_style_thrownknife) &&
        (style != bullet_style_m2))
        if (vec2length(bulletsnap->velocity) > SpriteSystem::Get().GetSprite(p).lastweaponspeed +
                                                   10 * guns[weaponindex].inheritedvelocity)
            return;

    a.x = SpriteSystem::Get().GetSprite(p).skeleton.pos[15].x - (bulletsnap->velocity.x / 1.33);
    a.y = SpriteSystem::Get().GetSprite(p).skeleton.pos[15].y - 2 - (bulletsnap->velocity.y / 1.33);
    b = vec2subtract(a, bulletsnap->pos);

    if ((style != bullet_style_fragnade) && (style != bullet_style_flame) &&
        (style != bullet_style_clusternade) && (style != bullet_style_cluster) &&
        (style != bullet_style_thrownknife) && (style != bullet_style_m2))
        if (vec2length(b) > 366)
            return;

    if (mapchangecounter == 999999999)
        return;

    if (bullettime[p] > maintickcounter)
        bullettime[p] = 0;

    if ((style != bullet_style_fragnade) && (style != bullet_style_punch) &&
        (style != bullet_style_clusternade) && (style != bullet_style_cluster) &&
        (style != bullet_style_thrownknife) && (style != bullet_style_m2))
    {
        if (SpriteSystem::Get().GetSprite(p).weapon.ammo > 1)
        {
            if ((maintickcounter - bullettime[p]) <
                ((SpriteSystem::Get().GetSprite(p).lastweaponfire) * 0.85))
            {
                bulletwarningcount[p] += 1;
            }
            else
            {
                bulletwarningcount[p] = 0;
            }
        }

        if (SpriteSystem::Get().GetSprite(p).weapon.ammo == 1)
        {
            if ((maintickcounter - bullettime[p]) <
                ((SpriteSystem::Get().GetSprite(p).lastweaponreload) * 0.9))
            {
                bulletwarningcount[p] += 1;
            }
            else
            {
                bulletwarningcount[p] = 0;
            }
        }

        if (bulletwarningcount[p] > 2)
            return;

        bullettime[p] = maintickcounter;
    }

    if (grenadetime[p] > maintickcounter)
        grenadetime[p] = 0;

    if ((style == bullet_style_fragnade) || (style == bullet_style_clusternade))
    {
        if (maintickcounter - grenadetime[p] < 6)
            return;

        grenadetime[p] = maintickcounter;
    }

    if (style == bullet_style_thrownknife)
    {
        if (CVar::sv_warnings_knifecheat == 69)
        {
            if ((!knifecan[p]) or
                (SpriteSystem::Get().GetSprite(p).weapon.bulletstyle != bullet_style_thrownknife) or
                (SpriteSystem::Get().GetSprite(p).weapon.bulletstyle != bullet_style_punch))
            {
                SpriteSystem::Get().GetSprite(p).player->knifewarnings += 1;
                if (SpriteSystem::Get().GetSprite(p).player->knifewarnings == 3)
                {
                    GetServerMainConsole().console(
                        std::string("** DETECTED KNIFE CHEATING FROM ") +
                            SpriteSystem::Get().GetSprite(p).player->name + " **",
                        server_message_color);
                    kickplayer(p, true, kick_cheat, day, "Knife Throw Cheat");
                }
            }
        }
        knifecan[p] = false;
    }

    a = bulletsnap->pos;
    b = bulletsnap->velocity;
    k = SpriteSystem::Get().GetSprite(p).lastweaponhm;

    if (style == bullet_style_punch)
        k = guns[noweapon].hitmultiply;

    if (style == bullet_style_cluster)
        if (SpriteSystem::Get().GetSprite(p).tertiaryweapon.ammocount == 0)
            return;

    if ((style == bullet_style_fragnade) || (style == bullet_style_clusternade))
    {
        k = guns[fraggrenade].hitmultiply;
        if (SpriteSystem::Get().GetSprite(p).tertiaryweapon.ammocount == 0)
            return;
        if (SpriteSystem::Get().GetSprite(p).tertiaryweapon.ammocount > 0)
            SpriteSystem::Get().GetSprite(p).tertiaryweapon.ammocount -= 1;
    }

    if (style == bullet_style_thrownknife)
    {
        k = guns[thrownknife].hitmultiply;
        SpriteSystem::Get().GetSprite(p).bodyapplyanimation(AnimationType::Stand, 1);
    }

    if (style == bullet_style_m2)
        k = guns[m2].hitmultiply;

    createbullet(a, b, bulletsnap->weaponnum, p, 255, k, true, true, bulletsnap->seed);

    bulletspread = guns[weaponindex].bulletspread;

    if (bulletsnap->weaponnum == guns[eagle].num) // Desert Eagle pellets
    {
        // Undo the bullet spread used on the first pellet that was sent in order to
        // get the "straight" bullet vector. Then re-apply the the same randomness
        NotImplemented(NITag::OTHER, "No randseed");
#if 0
        randseed = bulletsnap->seed;
#endif
        bstraight.x = b.x - (Random() * 2 - 1) * bulletspread;
        bstraight.y = b.y - (Random() * 2 - 1) * bulletspread;

        bx.x = bstraight.x + (Random() * 2 - 1) * bulletspread;
        bx.y = bstraight.y + (Random() * 2 - 1) * bulletspread;

        vec2normalize(bnorm, bstraight);
        a.x = a.x - sign(bstraight.x) * fabs(bnorm.y) * 3.0;
        a.y = a.y + sign(bstraight.y) * fabs(bnorm.x) * 3.0;

        createbullet(a, bx, bulletsnap->weaponnum, p, 255, k, false, true);
    }
    else if (style == bullet_style_shotgun) // SPAS-12 pellets
    {
        // Undo the bullet spread used on the first pellet that was sent in order to
        // get the "straight" bullet vector. Then re-apply the the same randomness
        NotImplemented(NITag::OTHER, "No randseed");
#if 0
        randseed = bulletsnap->seed;
#endif
        bstraight.x = b.x - (Random() * 2 - 1) * bulletspread;
        bstraight.y = b.y - (Random() * 2 - 1) * bulletspread;

        for (d = 0; d <= 4; d++) // Remaining 5 pellets
        {
            bx.x = bstraight.x + (Random() * 2 - 1) * bulletspread;
            bx.y = bstraight.y + (Random() * 2 - 1) * bulletspread;
            createbullet(a, bx, bulletsnap->weaponnum, p, 255, k, false, true);
        }
    }

    if ((style != bullet_style_fragnade) && (style != bullet_style_clusternade) &&
        (style != bullet_style_cluster) && (style != bullet_style_thrownknife) &&
        (style != bullet_style_m2))
    {
        SpriteSystem::Get().GetSprite(p).weapon.fireintervalprev = 1;
        SpriteSystem::Get().GetSprite(p).weapon.fireintervalcount = 1;
        SpriteSystem::Get().GetSprite(p).control.fire = true;
    }
}
#endif
