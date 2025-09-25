#pragma once
#include <memory>
#include <type_traits>
#include <utility>

#include "common/misc/SoldatConfig.hpp" // IWYU pragma: keep
#include "common/FileUtility.hpp"
#include "common/WeaponSystem.hpp"
#include "common/misc/GlobalSubsystem.hpp"
#include "common/misc/PortUtils.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/BulletSystem.hpp"
#include "shared/mechanics/ThingSystem.hpp"

class FFileUtility;
class FConsoleMain;
class FConsoleServer;
class FLogFile;
class WeaponSystem;
class FAnimationSystem;

template <Config::Module M>
class FGlobalSystems final : public FGlobalSubsystem<FGlobalSystems<M>>
{
public:
	FGlobalSystems(const FGlobalSystems&) = delete;
	FGlobalSystems(FGlobalSystems&&) = delete;
	FGlobalSystems& operator=(const FGlobalSystems&) = delete;
	FGlobalSystems& operator=(FGlobalSystems&&) = delete;
	using TConsoleType = typename std::conditional_t<Config::IsClient(M), FConsoleMain, FConsoleServer>;

	static WeaponSystem& GetWeaponSystem() { return *FGlobalSystems::Get().WeaponSystemObject; }

	static Game<M>& GetGame() { return Game<M>::Get(); }

	static tdemorecorder<M>& GetDemoRecorder() { return *FGlobalSystems::Get().DemoRecorder; }

	static BulletSystem& GetBulletSystem() { return *FGlobalSystems::Get().BulletSystemObject; }

	static ThingSystem& GetThingSystem() { return *FGlobalSystems::Get().ThingSystemObject; }

	static TConsoleType& GetMainConsole() { return *FGlobalSystems::Get().MainConsoleObject; }

	static FAnimationSystem& GetAnimationSystem() { return *FGlobalSystems::Get().AnimationSystemObject; }

	static FFileUtility& GetFileSystem() { return *FGlobalSystems::Get().FileUtilityObject; }

	static FLogFile& GetConsoleLogFile() { return *FGlobalSystems::Get().LogFileObject; }

	static FLogFile& GetKillLogFile()
		requires(Config::IsServer(M))
	{
		return *FGlobalSystems::Get().KillLogFileObject;
	}

protected:
	FGlobalSystems();
	~FGlobalSystems();

private:
	friend std::default_delete<FGlobalSystems>;
	friend FGlobalSubsystem<FGlobalSystems<M>>;
	std::unique_ptr<WeaponSystem> WeaponSystemObject;
	std::unique_ptr<tdemorecorder<M>> DemoRecorder;
	std::unique_ptr<BulletSystem> BulletSystemObject;
	std::unique_ptr<ThingSystem> ThingSystemObject;
	std::shared_ptr<FFileUtility> FileUtilityObject;
	std::shared_ptr<TConsoleType> MainConsoleObject;
	std::shared_ptr<FLogFile> LogFileObject;
	std::shared_ptr<FLogFile> KillLogFileObject;
	std::shared_ptr<FAnimationSystem> AnimationSystemObject;
};

using GS = FGlobalSystems<Config::GetModule()>;
