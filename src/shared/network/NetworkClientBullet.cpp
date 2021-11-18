// automatically converted
#include "NetworkClientBullet.hpp"

#include "../../client/Client.hpp"
#include "../Demo.hpp"
#include "../Game.hpp"
#include "../mechanics/Bullets.hpp"
#include "NetworkUtils.hpp"

std::array<tmsg_bulletsnapshot, max_sprites> oldbulletsnapshotmsg;

namespace
{
auto &bulletparts = InitGlobalVariable<particlesystem, "bulletparts">();
}

void clientsendbullet(std::uint8_t i)
{
    tmsg_clientbulletsnapshot bulletmsg;

    bulletmsg.header.id = msgid_bulletsnapshot;
    bulletmsg.weaponnum = bullet[i].ownerweapon;
    bulletmsg.pos = bulletparts.pos[i];
    bulletmsg.velocity = bulletparts.velocity[i];
    bulletmsg.clientticks = clienttickcount;
    bulletmsg.seed = bullet[i].seed;

    GetNetwork()->senddata(&bulletmsg, sizeof(bulletmsg), k_nSteamNetworkingSend_Unreliable);
    NotImplemented(NITag::NETWORK);
#if 0
    if (demorecorder.active())
        serverbulletsnapshot(i, 0, false);
#endif
}

void clienthandlebulletsnapshot(SteamNetworkingMessage_t *netmessage)
{
    tmsg_bulletsnapshot *bulletsnap;
    tvector2 a, b, bx;
    tvector2 bstraight;
    tvector2 bnorm;
    float hm;
    std::int32_t i, k, pa, c, d;
    std::int16_t weaponindex;
    std::uint8_t style;
    float bulletspread;

    if (!verifypacket(sizeof(tmsg_bulletsnapshot), netmessage->m_cbSize, msgid_bulletsnapshot))
        return;

    bulletsnap = pmsg_bulletsnapshot(netmessage->m_pData);

    bulletsnap->Dump();

    if ((bulletsnap->owner < 1) || (bulletsnap->owner > max_sprites))
        return;

    if (!bulletsnap->forced)
        if ((oldbulletsnapshotmsg[bulletsnap->owner].weaponnum == bulletsnap->weaponnum) &&
            (oldbulletsnapshotmsg[bulletsnap->owner].pos.x == bulletsnap->pos.x) &&
            (oldbulletsnapshotmsg[bulletsnap->owner].pos.y == bulletsnap->pos.y) &&
            (oldbulletsnapshotmsg[bulletsnap->owner].velocity.x == bulletsnap->velocity.x) &&
            (oldbulletsnapshotmsg[bulletsnap->owner].velocity.y == bulletsnap->velocity.y))
            return;

    weaponindex = weaponnumtoindex(bulletsnap->weaponnum);
    if (weaponindex == -1)
        return;

    style = guns[weaponindex].bulletstyle;

    a = bulletsnap->pos;
    b = bulletsnap->velocity;

    // FIXME (falcon): Also serialize HitMultiply for CreateBullet()
    // on the other side, how the hell it works now? (because it does)
    hm = sprite[bulletsnap->owner].weapon.hitmultiply;
    if (style == bullet_style_fragnade)
        hm = guns[fraggrenade].hitmultiply;

    i = createbullet(a, b, bulletsnap->weaponnum, bulletsnap->owner, 255, hm, false, true);

    bullet[i].ownerpingtick = sprite[bulletsnap->owner].player->pingticks + pingticksadd;
    pa = sprite[mysprite].player->pingticks + bullet[i].ownerpingtick;
    bullet[i].pingadd = pa;
    bullet[i].pingaddstart = pa;
    if (!bulletsnap->forced)
    {
        bulletspread = guns[weaponindex].bulletspread;

        if (bulletsnap->weaponnum == guns[eagle].num) // Desert Eagle pellets
        {
            // Undo the bullet spread used on the first pellet that was sent in order to
            // get the "straight" bullet vector. Then re-apply the the same randomness
            NotImplemented(NITag::NETWORK);
#if 0
            randseed = bulletsnap->seed;
#endif
            bstraight.x = b.x - (float)(Random() * 2 - 1) * bulletspread;
            bstraight.y = b.y - (float)(Random() * 2 - 1) * bulletspread;

            bx.x = bstraight.x + (float)(Random() * 2 - 1) * bulletspread;
            bx.y = bstraight.y + (float)(Random() * 2 - 1) * bulletspread;

            vec2normalize(bnorm, bstraight);
            a.x = a.x - sign(bstraight.x) * fabs(bnorm.y) * 3.0;
            a.y = a.y + sign(bstraight.y) * fabs(bnorm.x) * 3.0;

            k = createbullet(a, bx, bulletsnap->weaponnum, bulletsnap->owner, 255, i, false, true);

            if ((mysprite > 0) && (bulletsnap->owner > 0))
                for (c = 1; c <= pa; c++)
                    if (bullet[k].active)
                    {
                        bulletparts.doeulertimestepfor(k);
                        bullet[k].update();
                        if (!bullet[k].active)
                            break;
                    }
        }
        else if (style == bullet_style_shotgun) // SPAS-12 pellets
        {
            // Undo the bullet spread used on the first pellet that was sent in order to
            // get the "straight" bullet vector. Then re-apply the the same randomness
            NotImplemented(NITag::NETWORK);
#if 0
            randseed = bulletsnap->seed;
#endif
            bstraight.x = b.x - (Random() * 2 - 1) * bulletspread;
            bstraight.y = b.y - (Random() * 2 - 1) * bulletspread;

            for (d = 0; d <= 4; d++) // Remaining 5 pellets
            {
                bx.x = bstraight.x + (Random() * 2 - 1) * bulletspread;
                bx.y = bstraight.y + (Random() * 2 - 1) * bulletspread;
                k = createbullet(a, bx, bulletsnap->weaponnum, bulletsnap->owner, 255, hm, false,
                                 true);

                if ((mysprite > 0) && (bulletsnap->owner > 0))
                {
                    for (c = 1; c <= pa; c++)
                        if (bullet[k].active)
                        {
                            bulletparts.doeulertimestepfor(k);
                            bullet[k].update();
                            if (!bullet[k].active)
                                break;
                        }
                }
            }
        }

        if ((style != bullet_style_fragnade) && (style != bullet_style_clusternade) &&
            (style != bullet_style_cluster) && (style != bullet_style_thrownknife) &&
            (style != bullet_style_m2))
        {
            sprite[bulletsnap->owner].fire();
        }
    }

    if (bullet[i].active)
        if ((mysprite > 0) && (bulletsnap->owner > 0))
            for (c = 1; c <= pa; c++)
            {
                bulletparts.doeulertimestepfor(i);
                bullet[i].update();
                if (!bullet[i].active)
                    break;
            }

    // stat gun
    if (!bulletsnap->forced)
        if (style == bullet_style_m2)
            for (i = 1; i <= max_things; i++)
                if ((thing[i].active) && (thing[i].style == object_stationary_gun))
                    thing[i].checkstationaryguncollision(true);

    oldbulletsnapshotmsg[bulletsnap->owner] = *bulletsnap;
}
