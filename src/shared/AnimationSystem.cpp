// automatically converted
#include "AnimationSystem.hpp"
#include "common/Anims.hpp"

#include "LogFile.hpp"
#include "common/Parts.hpp"

#include "physfs.h"

// clang-format off
#include "misc/GlobalVariableStorage.cpp"
// clang-format on

// TODO: add file missing checks
// TODO: translate filenames into english
template <Config::Module M>
void TAnimationSystem<M>::LoadAnimObjects(const std::string &moddir)
{
    addlinetologfile(gamelog, std::string("Loading Animations. ") + moddir, consolelogfilename);
    struct AnimData
    {
        AnimationType AnimType;
        const char *Filename;
        bool Loop = false;
        std::int32_t Speed = 1;
    };

    constexpr std::array<AnimData, AnimationType::Last> animData = {
        AnimData{AnimationType::Stand, "anims/stoi.poa", true, 3},
        AnimData{AnimationType::Run, "anims/biega.poa", true, 1},
        AnimData{AnimationType::RunBack, "anims/biegatyl.poa", true},
        AnimData{AnimationType::Jump, "anims/skok.poa"},
        AnimData{AnimationType::JumpSide, "anims/skokwbok.poa"},
        AnimData{AnimationType::Fall, "anims/spada.poa"},
        AnimData{AnimationType::Crouch, "anims/kuca.poa"},
        AnimData{AnimationType::CrouchRun, "anims/kucaidzie.poa", true, 2},
        AnimData{AnimationType::Reload, "anims/laduje.poa", false, 2},
        AnimData{AnimationType::Throw, "anims/rzuca.poa", false, 1},
        AnimData{AnimationType::Recoil, "anims/odrzut.poa"},
        AnimData{AnimationType::SmallRecoil, "anims/odrzut2.poa"},
        AnimData{AnimationType::Shotgun, "anims/shotgun.poa"},
        AnimData{AnimationType::ClipOut, "anims/clipout.poa", false, 3},
        AnimData{AnimationType::ClipIn, "anims/clipin.poa", false, 3},
        AnimData{AnimationType::SlideBack, "anims/slideback.poa", true, 2},
        AnimData{AnimationType::Change, "anims/change.poa", false},
        AnimData{AnimationType::ThrowWeapon, "anims/wyrzuca.poa", false},
        AnimData{AnimationType::WeaponNone, "anims/bezbroni.poa", false, 3},
        AnimData{AnimationType::Punch, "anims/bije.poa", false},
        AnimData{AnimationType::ReloadBow, "anims/strzala.poa"},
        AnimData{AnimationType::Barret, "anims/barret.poa", false, 9},
        AnimData{AnimationType::Roll, "anims/skokdolobrot.poa", false, 1},
        AnimData{AnimationType::RollBack, "anims/skokdolobrottyl.poa", false, 1},
        AnimData{AnimationType::CrouchRunBack, "anims/kucaidzietyl.poa", true, 2},
        AnimData{AnimationType::Cigar, "anims/cigar.poa", false, 3},
        AnimData{AnimationType::Match, "anims/match.poa", false, 3},
        AnimData{AnimationType::Smoke, "anims/smoke.poa", false, 4},
        AnimData{AnimationType::Wipe, "anims/wipe.poa", false, 4},
        AnimData{AnimationType::Groin, "anims/krocze.poa", false, 2},
        AnimData{AnimationType::Piss, "anims/szcza.poa", false, 8},
        AnimData{AnimationType::Mercy, "anims/samo.poa", false, 3},
        AnimData{AnimationType::Mercy2, "anims/samo2.poa", false, 3},
        AnimData{AnimationType::TakeOff, "anims/takeoff.poa", false, 2},
        AnimData{AnimationType::Prone, "anims/lezy.poa", false, 1},
        AnimData{AnimationType::Victory, "anims/cieszy.poa", false, 3},
        AnimData{AnimationType::Aim, "anims/celuje.poa", false, 2},
        AnimData{AnimationType::HandSupAim, "anims/gora.poa", false, 2},
        AnimData{AnimationType::ProneMove, "anims/lezyidzie.poa", true, 2},
        AnimData{AnimationType::GetUp, "anims/wstaje.poa", false, 1},
        AnimData{AnimationType::AimRecoil, "anims/celujeodrzut.poa", false, 1},
        AnimData{AnimationType::HandSupRecoil, "anims/goraodrzut.poa", false, 1},
        AnimData{AnimationType::Melee, "anims/kolba.poa", false, 1},
        AnimData{AnimationType::Own, "anims/rucha.poa", false, 3},
    };

    // Anims load

    for (const auto &anim : animData)
    {
        auto &a = Animations.emplace_back();
        a.id = anim.AnimType;
        a.loadfromfile(moddir + anim.Filename);
        a.loop = anim.Loop;
        a.speed = anim.Speed;
    }

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

template <Config::Module M>
tanimation &TAnimationSystem<M>::GetAnimation(AnimationType type)
{
    return Animations[type];
}

template class TAnimationSystem<>;
