// automatically converted
#include "AnimationSystem.hpp"

#include <array>
#include <cstdint>
#include <string>

#include "Anims.hpp"
#include "Constants.hpp"
#include "FileUtility.hpp"
#include "LogFile.hpp"
#include "Parts.hpp"
#include "misc/TFileStream.hpp"

FAnimationSystem::FAnimationSystem(FLogFile& InLogFile, FFileUtility& InFileUtility)
	: LogFile(InLogFile), FileUtility(InFileUtility)
{
}

// TODO(vscode): add file missing checks
// TODO(vscode): translate filenames into english
void FAnimationSystem::LoadAnimObjects(const std::string& moddir)
{
	Animations.clear();
	LogFile.Log(std::string("Loading Animations. ") + moddir);
	struct FAnimData
	{
		AnimationType AnimType = AnimationType::Last;
		const char* Filename = nullptr;
		bool Loop = false;
		std::int32_t Speed = 1;
	};

	constexpr std::array<FAnimData, AnimationType::Last> kAnimData = {
		FAnimData{ AnimationType::Stand, "anims/stoi.poa", true, 3 },
		FAnimData{ AnimationType::Run, "anims/biega.poa", true, 1 },
		FAnimData{ AnimationType::RunBack, "anims/biegatyl.poa", true },
		FAnimData{ AnimationType::Jump, "anims/skok.poa" },
		FAnimData{ AnimationType::JumpSide, "anims/skokwbok.poa" },
		FAnimData{ AnimationType::Fall, "anims/spada.poa" },
		FAnimData{ AnimationType::Crouch, "anims/kuca.poa" },
		FAnimData{ AnimationType::CrouchRun, "anims/kucaidzie.poa", true, 2 },
		FAnimData{ AnimationType::Reload, "anims/laduje.poa", false, 2 },
		FAnimData{ AnimationType::Throw, "anims/rzuca.poa", false, 1 },
		FAnimData{ AnimationType::Recoil, "anims/odrzut.poa" },
		FAnimData{ AnimationType::SmallRecoil, "anims/odrzut2.poa" },
		FAnimData{ AnimationType::Shotgun, "anims/shotgun.poa" },
		FAnimData{ AnimationType::ClipOut, "anims/clipout.poa", false, 3 },
		FAnimData{ AnimationType::ClipIn, "anims/clipin.poa", false, 3 },
		FAnimData{ AnimationType::SlideBack, "anims/slideback.poa", true, 2 },
		FAnimData{ AnimationType::Change, "anims/change.poa", false },
		FAnimData{ AnimationType::ThrowWeapon, "anims/wyrzuca.poa", false },
		FAnimData{ AnimationType::WeaponNone, "anims/bezbroni.poa", false, 3 },
		FAnimData{ AnimationType::Punch, "anims/bije.poa", false },
		FAnimData{ AnimationType::ReloadBow, "anims/strzala.poa" },
		FAnimData{ AnimationType::Barret, "anims/barret.poa", false, 9 },
		FAnimData{ AnimationType::Roll, "anims/skokdolobrot.poa", false, 1 },
		FAnimData{ AnimationType::RollBack, "anims/skokdolobrottyl.poa", false, 1 },
		FAnimData{ AnimationType::CrouchRunBack, "anims/kucaidzietyl.poa", true, 2 },
		FAnimData{ AnimationType::Cigar, "anims/cigar.poa", false, 3 },
		FAnimData{ AnimationType::Match, "anims/match.poa", false, 3 },
		FAnimData{ AnimationType::Smoke, "anims/smoke.poa", false, 4 },
		FAnimData{ AnimationType::Wipe, "anims/wipe.poa", false, 4 },
		FAnimData{ AnimationType::Groin, "anims/krocze.poa", false, 2 },
		FAnimData{ AnimationType::Piss, "anims/szcza.poa", false, 8 },
		FAnimData{ AnimationType::Mercy, "anims/samo.poa", false, 3 },
		FAnimData{ AnimationType::Mercy2, "anims/samo2.poa", false, 3 },
		FAnimData{ AnimationType::TakeOff, "anims/takeoff.poa", false, 2 },
		FAnimData{ AnimationType::Prone, "anims/lezy.poa", false, 1 },
		FAnimData{ AnimationType::Victory, "anims/cieszy.poa", false, 3 },
		FAnimData{ AnimationType::Aim, "anims/celuje.poa", false, 2 },
		FAnimData{ AnimationType::HandSupAim, "anims/gora.poa", false, 2 },
		FAnimData{ AnimationType::ProneMove, "anims/lezyidzie.poa", true, 2 },
		FAnimData{ AnimationType::GetUp, "anims/wstaje.poa", false, 1 },
		FAnimData{ AnimationType::AimRecoil, "anims/celujeodrzut.poa", false, 1 },
		FAnimData{ AnimationType::HandSupRecoil, "anims/goraodrzut.poa", false, 1 },
		FAnimData{ AnimationType::Melee, "anims/kolba.poa", false, 1 },
		FAnimData{ AnimationType::Own, "anims/rucha.poa", false, 3 },
	};

	// Anims load

	for (const auto& Anim : kAnimData)
	{
		auto& a = Animations.emplace_back();
		a.id = Anim.AnimType;
		auto Stream = ReadAsFileStream(FileUtility, moddir + Anim.Filename);
		if (Stream)
		{
			a.loadfromfile(*Stream);
		}
		a.loop = Anim.Loop;
		a.speed = Anim.Speed;
	}

	LogFile.Log("Loading objects.");

	Skeletons.clear();

	// Gostek
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/gostek.po", Constants::SCALE);
	Skeletons.back().timestep = 1;
	Skeletons.back().gravity = 1.06 * Constants::GRAV;
	Skeletons.back().vdamping = 0.997;

	// Box
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/kit.po", 2.15);
	Skeletons.back().timestep = 1;

	// Flag
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/flag.po", 4.0);

	// Para
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/para.po", 5.0);
	// Stat
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/stat.po", 4.0);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 1.0);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 1.1);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 1.8);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 2.2);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 2.8);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 3.6);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 3.7);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 3.9);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 4.3);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 5.0);
	Skeletons.emplace_back();
	Skeletons.back().loadpoobject(FileUtility, "objects/karabin.po", 5.5);
}

auto FAnimationSystem::GetAnimation(AnimationType type) -> tanimation&
{
	return Animations[type];
}

auto FAnimationSystem::GetSkeleton(const SkeletonType InType) const -> const particlesystem&
{
	return Skeletons[InType];
}
