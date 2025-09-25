#include "GlobalSystems.hpp"

#include <memory>

#include "common/AnimationSystem.hpp"
#include "common/Console.hpp"
#include "common/FileUtility.hpp"
#include "common/LogFile.hpp"
#include "common/WeaponSystem.hpp"
#include "common/misc/SoldatConfig.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/BulletSystem.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include "shared/mechanics/ThingSystem.hpp"
#ifdef SERVER
	#include "server/Server.hpp"
#endif // SERVER

template <Config::Module M>
FGlobalSystems<M>::FGlobalSystems()
{
	if constexpr (Config::IsServer(M))
	{
		FileUtilityObject = std::make_unique<FFileUtility>("/server");
	}
	else
	{
		FileUtilityObject = std::make_unique<FFileUtility>("/client");
	}
	MainConsoleObject = std::make_unique<TConsoleType>();
	ConsoleLogFileObject = std::make_unique<FLogFile>(*FileUtilityObject);
	if constexpr (Config::IsServer(M))
	{
		KillLogFileObject = std::make_unique<FLogFile>(*FileUtilityObject);
	}

	SpriteSystem::Init();
	AnimationSystemObject = std::make_unique<FAnimationSystem>(*ConsoleLogFileObject, *FileUtilityObject);
	ThingSystemObject = std::make_unique<ThingSystem>();
	BulletSystemObject = std::make_unique<BulletSystem>();
	WeaponSystemObject = std::make_unique<WeaponSystem>();
	DemoRecorder = std::make_unique<tdemorecorder<M>>();
	Game<M>::Init();
}

template <Config::Module M>
FGlobalSystems<M>::~FGlobalSystems()
{
	Game<M>::Deinit();
	DemoRecorder.reset();
	WeaponSystemObject.reset();
	BulletSystemObject.reset();
	ThingSystemObject.reset();
	AnimationSystemObject.reset();
	SpriteSystem::Deinit();
	MainConsoleObject.reset();
	KillLogFileObject.reset();
	ConsoleLogFileObject.reset();
	FileUtilityObject.reset();
}

template class FGlobalSystems<Config::GetModule()>;
