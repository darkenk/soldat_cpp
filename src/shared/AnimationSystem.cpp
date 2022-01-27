// automatically converted
#include "AnimationSystem.hpp"
#include "LogFile.hpp"
#include "common/Anims.hpp"
#include "common/Parts.hpp"
#include "mechanics/SpriteSystem.hpp"
#include "physfs.h"
#include "shared/mechanics/Bullets.hpp"
#include "shared/mechanics/Sparks.hpp"

// TODO: add file missing checks
// TODO: translate filenames into english
template <Config::Module M>
void TAnimationSystem<M>::LoadAnimObjects(const std::string &moddir)
{
    Animations.clear();
    addlinetologfile(GetGameLog(), std::string("Loading Animations. ") + moddir,
                     GetGameLogFilename());
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

    addlinetologfile(GetGameLog(), "Loading objects.", GetGameLogFilename());

    SpriteSystem::Get().ResetSpriteParts();

    Skeletons.clear();

    // Gostek
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/gostek.po", scale);
    Skeletons.back().timestep = 1;
    Skeletons.back().gravity = 1.06 * grav;
    Skeletons.back().vdamping = 0.997;

    // Box
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/kit.po", 2.15);
    Skeletons.back().timestep = 1;

    // Flag
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/flag.po", 4.0);

    // Para
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/para.po", 5.0);
    // Stat
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/stat.po", 4.0);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 1.0);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 1.1);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 1.8);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 2.2);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 2.8);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 3.6);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 3.7);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 3.9);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 4.3);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 5.0);
    Skeletons.emplace_back();
    Skeletons.back().loadpoobject("objects/karabin.po", 5.5);

    GetBulletParts().destroy();
    GetBulletParts().timestep = 1;
    GetBulletParts().gravity = grav * 2.25;
    GetBulletParts().edamping = 0.99;

    if constexpr (Config::GetModule() == Config::CLIENT_MODULE)
    {
        GetSparkParts().destroy();
        GetSparkParts().timestep = 1;
        GetSparkParts().gravity = grav / 1.4;
        GetSparkParts().edamping = 0.998;
    }
}

template <Config::Module M>
tanimation &TAnimationSystem<M>::GetAnimation(AnimationType type)
{
    return Animations[type];
}

template <Config::Module M>
const particlesystem &TAnimationSystem<M>::GetSkeleton(const SkeletonType type) const
{
    return Skeletons[type];
}

template class TAnimationSystem<>;
