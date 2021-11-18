// automatically converted
#include "Anims.hpp"

#include "LogFile.hpp"
#include "Logging.hpp"
#include "Parts.hpp"
#include "PhysFSExt.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"
#include "physfs.h"

// clang-format off
#include "misc/GlobalVariableStorage.cpp"
// clang-format on

void tanimation::doanimation()
{
    count += 1;
    if (count == speed)
    {
        count = 0;
        currframe += 1;
        if (currframe > numframes)
        {
            if (loop)
            {
                currframe = 1;
            }
            else
            {
                currframe = numframes;
            }
        }
    }
}

void tanimation::loadfromfile(const std::string &filename)
{
    PHYSFS_File *f;
    std::string r1;
    std::string r2;
    std::string r3;
    std::string r4;
    std::int32_t p;

    if (!PHYSFS_exists((pchar)(filename)))
        return;

    numframes = 1;

    // default settings
    loop = false;
    speed = 1;
    count = 0;

    f = PHYSFS_openRead((pchar)(filename));

    PhysFS_ReadLn(f, r1);
    while (r1 != "ENDFILE")
    {
        if (r1 == "NEXTFRAME")
        {
            if (numframes == max_frames_index)
            {
                LogWarnG("Corrupted frame index: {}", filename);
                break;
            }

            numframes += 1;
        }
        else
        {
            PhysFS_ReadLn(f, r2); // X
            PhysFS_ReadLn(f, r3); // Y
            PhysFS_ReadLn(f, r4); // Z

            p = strtointdef(r1, 0);
            if ((p >= 1) && (p <= max_pos_index))
            {
                // TODO: check if this is correct
                frames[numframes].pos[p].x = -scale * strtofloat(r2) / 1.1;
                frames[numframes].pos[p].y = -scale * strtofloat(r4);
            }
            else
            {
                LogWarnG("Corrupted Index ({}): {}", r1, filename);
            }
        }

        PhysFS_ReadLn(f, r1);
    }
    PHYSFS_close(f);
    currframe = 1;
}

std::int32_t tanimation::checksum()
{
    float chk;
    std::int32_t i, j;

    chk = 0.5;

    for (i = 1; i <= numframes; i++)
        for (j = 1; j <= 20; j++)
        {
            chk = chk + frames[i].pos[j].x;
            chk = chk + frames[i].pos[j].y;
            chk = chk + frames[i].pos[j].z;
        }

    return trunc(chk);
}

// TODO: add file missing checks
// TODO: translate filenames into english
void loadanimobjects(const std::string &moddir)
{
    addlinetologfile(gamelog, std::string("Loading Animations. ") + moddir, consolelogfilename);

    // Anims load

    // TODO: use english filenames
    // stand
    stand.loadfromfile(moddir + "anims/stoi.poa");
    stand.id = 0;
    stand.loop = true;
    stand.speed = 3;

    // run
    run.loadfromfile(moddir + "anims/biega.poa");
    run.id = 1;
    run.loop = true;

    // run back
    runback.loadfromfile(moddir + "anims/biegatyl.poa");
    runback.id = 2;
    runback.loop = true;

    // jump
    jump.loadfromfile(moddir + "anims/skok.poa");
    ;
    jump.id = 3;

    // jump to side
    jumpside.loadfromfile(moddir + "anims/skokwbok.poa");
    jumpside.id = 4;

    // fall
    fall.loadfromfile(moddir + "anims/spada.poa");
    fall.id = 5;

    // crouch
    crouch.loadfromfile(moddir + "anims/kuca.poa");
    crouch.id = 6;

    // crouch walk
    crouchrun.loadfromfile(moddir + "anims/kucaidzie.poa");
    crouchrun.id = 7;
    crouchrun.loop = true;
    crouchrun.speed = 2;

    // reloading
    reload.loadfromfile(moddir + "anims/laduje.poa");
    reload.id = 8;
    reload.speed = 2;

    // throw
    throw_.loadfromfile(moddir + "anims/rzuca.poa");
    throw_.id = 9;
    throw_.speed = 1;

    // throwback?
    recoil.loadfromfile(moddir + "anims/odrzut.poa");
    recoil.id = 10;

    // throwback2?
    smallrecoil.loadfromfile(moddir + "anims/odrzut2.poa");
    smallrecoil.id = 11;

    shotgun.loadfromfile(moddir + "anims/shotgun.poa");
    shotgun.id = 12;

    clipout.loadfromfile(moddir + "anims/clipout.poa");
    clipout.id = 13;
    clipout.speed = 3;

    clipin.loadfromfile(moddir + "anims/clipin.poa");
    clipin.id = 14;
    clipin.speed = 3;

    slideback.loadfromfile(moddir + "anims/slideback.poa");
    slideback.id = 15;
    slideback.speed = 2;
    slideback.loop = true;

    change.loadfromfile(moddir + "anims/change.poa");
    change.id = 16;
    change.loop = false;

    // throwout?
    throwweapon.loadfromfile(moddir + "anims/wyrzuca.poa");
    throwweapon.id = 17;
    throwweapon.loop = false;

    // without weapon
    weaponnone.loadfromfile(moddir + "anims/bezbroni.poa");
    weaponnone.id = 18;
    weaponnone.speed = 3;

    // punch
    punch.loadfromfile(moddir + "anims/bije.poa");
    punch.id = 19;
    punch.loop = false;

    // shoot?
    reloadbow.loadfromfile(moddir + "anims/strzala.poa");
    reloadbow.id = 20;

    barret.loadfromfile(moddir + "anims/barret.poa");
    barret.id = 21;
    barret.speed = 9;

    // jump?
    roll.loadfromfile(moddir + "anims/skokdolobrot.poa");
    roll.id = 22;
    roll.speed = 1;

    // jump? back
    rollback.loadfromfile(moddir + "anims/skokdolobrottyl.poa");
    rollback.id = 23;
    rollback.speed = 1;

    // crouch walk back
    crouchrunback.loadfromfile(moddir + "anims/kucaidzietyl.poa");
    crouchrunback.id = 24;
    crouchrunback.loop = true;
    crouchrunback.speed = 2;

    cigar.loadfromfile(moddir + "anims/cigar.poa");
    cigar.id = 25;
    cigar.speed = 3;

    match.loadfromfile(moddir + "anims/match.poa");
    match.id = 26;
    match.speed = 3;

    smoke.loadfromfile(moddir + "anims/smoke.poa");
    smoke.id = 27;
    smoke.speed = 4;

    wipe.loadfromfile(moddir + "anims/wipe.poa");
    wipe.id = 28;
    wipe.speed = 4;

    // ?
    groin.loadfromfile(moddir + "anims/krocze.poa");
    groin.id = 29;
    groin.speed = 2;

    // ?
    piss.loadfromfile(moddir + "anims/szcza.poa");
    piss.id = 30;
    piss.speed = 8;

    // ?
    mercy.loadfromfile(moddir + "anims/samo.poa");
    mercy.id = 31;
    mercy.speed = 3;

    // ?
    mercy2.loadfromfile(moddir + "anims/samo2.poa");
    mercy2.id = 32;
    mercy2.speed = 3;

    // take off hat
    takeoff.loadfromfile(moddir + "anims/takeoff.poa");
    takeoff.id = 33;
    takeoff.speed = 2;

    // lying
    prone.loadfromfile(moddir + "anims/lezy.poa");
    prone.id = 34;
    prone.speed = 1;

    // happy/victory
    victory.loadfromfile(moddir + "anims/cieszy.poa");
    victory.id = 35;
    victory.speed = 3;

    // aiming
    aim.loadfromfile(moddir + "anims/celuje.poa");
    aim.id = 36;
    aim.speed = 2;

    // top?
    handsupaim.loadfromfile(moddir + "anims/gora.poa");
    handsupaim.id = 37;
    handsupaim.speed = 2;

    // lying walk
    pronemove.loadfromfile(moddir + "anims/lezyidzie.poa");
    pronemove.id = 38;
    pronemove.loop = true;
    pronemove.speed = 2;

    // stand up
    getup.loadfromfile(moddir + "anims/wstaje.poa");
    getup.id = 39;
    getup.speed = 1;

    // aim throw?
    aimrecoil.loadfromfile(moddir + "anims/celujeodrzut.poa");
    aimrecoil.id = 40;
    aimrecoil.speed = 1;

    // top throw?
    handsuprecoil.loadfromfile(moddir + "anims/goraodrzut.poa");
    handsuprecoil.id = 41;
    handsuprecoil.speed = 1;

    // ?
    melee.loadfromfile(moddir + "anims/kolba.poa");
    melee.id = 42;
    melee.speed = 1;

    // ?
    own.loadfromfile(moddir + "anims/rucha.poa");
    own.id = 43;
    own.speed = 3;

    addlinetologfile(gamelog, "Loading objects.", consolelogfilename);

    spriteparts.destroy();
    spriteparts.timestep = 1;
    spriteparts.gravity = grav;
    spriteparts.edamping = 0.99;
    gostekskeleton.destroy();
    gostekskeleton.loadpoobject("objects/gostek.po", scale);
    gostekskeleton.timestep = 1;
    gostekskeleton.gravity = 1.06 * grav;
    gostekskeleton.vdamping = 0.997;

    boxskeleton.destroy();
    boxskeleton.loadpoobject("objects/kit.po", 2.15);
    boxskeleton.timestep = 1;

    bulletparts.destroy();
    bulletparts.timestep = 1;
    bulletparts.gravity = grav * 2.25;
    bulletparts.edamping = 0.99;

    sparkparts.destroy();
    sparkparts.timestep = 1;
    sparkparts.gravity = grav / 1.4;
    sparkparts.edamping = 0.998;

    flagskeleton.loadpoobject("objects/flag.po", 4.0);
    paraskeleton.loadpoobject("objects/para.po", 5.0);
    statskeleton.loadpoobject("objects/stat.po", 4.0);
    rifleskeleton10.loadpoobject("objects/karabin.po", 1.0);
    rifleskeleton11.loadpoobject("objects/karabin.po", 1.1);
    rifleskeleton18.loadpoobject("objects/karabin.po", 1.8);
    rifleskeleton22.loadpoobject("objects/karabin.po", 2.2);
    rifleskeleton28.loadpoobject("objects/karabin.po", 2.8);
    rifleskeleton36.loadpoobject("objects/karabin.po", 3.6);
    rifleskeleton37.loadpoobject("objects/karabin.po", 3.7);
    rifleskeleton39.loadpoobject("objects/karabin.po", 3.9);
    rifleskeleton43.loadpoobject("objects/karabin.po", 4.3);
    rifleskeleton50.loadpoobject("objects/karabin.po", 5.0);
    rifleskeleton55.loadpoobject("objects/karabin.po", 5.5);
}
