#pragma once

#include <string>
#include <cstdint>
#include <vector>

#include "Anims.hpp"
#include "FileUtility.hpp"
#include "LogFile.hpp"
#include "Parts.hpp"

class particlesystem;
class Skeleton;
class FLogFile;

enum SkeletonType : std::int32_t
{
	Gostek,
	Box,
	Flag,
	Para,
	Stat,
	Rifle10,
	Rifle11,
	Rifle18,
	Rifle22,
	Rifle28,
	Rifle36,
	Rifle37,
	Rifle39,
	Rifle43,
	Rifle50,
	Rifle55
};

class FAnimationSystem
{
public:
	explicit FAnimationSystem(FLogFile& InLogFile, FFileUtility& InFileUtility);
	void LoadAnimObjects(const std::string& moddir);

	auto GetAnimation(AnimationType type) -> tanimation&;

	[[nodiscard]] auto GetSkeleton(SkeletonType type) const -> const particlesystem&;

private:
	FLogFile& LogFile;
	FFileUtility& FileUtility;
	std::vector<tanimation> Animations;
	std::vector<particlesystem> Skeletons;
};
