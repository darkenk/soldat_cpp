#define BOOST_DI_CFG_DIAGNOSTICS_LEVEL 2
#include "GlobalSystems.hpp"

#include <memory>
#include <boost/di.hpp>
#include <boost/di/extension/scopes/shared.hpp>
#include <boost/di/extension/injections/named_parameters.hpp>

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
#include "shared/Cvar.hpp"
#ifdef SERVER
	#include "server/Server.hpp"
#endif // SERVER

using namespace std::literals::string_view_literals;
using namespace boost;
using namespace std::literals;

namespace
{
	template <Config::Module M>
	constexpr auto MakeInjector()
	{
		auto MainConsoleInjector = []
		{
			return di::make_injector<di::extension::shared_config>(
				di::bind<std::int32_t>().named("NewMessageWait"_s).to(150),
				di::bind<std::int32_t>().named("CountMax"_s).to(Config::IsServer(M) ? 7 : CVar::ui_console_length),
				di::bind<std::int32_t>().named("ScrollTickMax"_s).to(150),
				di::bind<bool>().named("WriteToFile"_s).to(true),
				di::bind<typename FGlobalSystems<M>::TConsoleType>.in(di::extension::shared));
		};

		return di::make_injector<di::extension::shared_config>(di::bind<std::string_view>()
																   .named("FileRootPrefix"_s)
																   .to(Config::IsServer(M) ? "/server"sv : "/client"sv)
																   .in(di::extension::shared),
			di::bind<FFileUtility>.in(di::extension::shared),
			di::bind<FLogFile>.in(di::extension::shared),
			di::bind<FAnimationSystem>.in(di::extension::shared),
			MainConsoleInjector());
	}
} // namespace

template <Config::Module M>
FGlobalSystems<M>::FGlobalSystems()
{
	auto Injector = MakeInjector<M>();
	FileUtilityObject = Injector.template create<std::shared_ptr<FFileUtility>>();
	LogFileObject = Injector.template create<std::shared_ptr<FLogFile>>();
	AnimationSystemObject = Injector.template create<std::shared_ptr<FAnimationSystem>>();
	MainConsoleObject = Injector.template create<std::shared_ptr<TConsoleType>>();
	if constexpr (Config::IsServer(M))
	{
		KillLogFileObject = std::make_unique<FLogFile>(*FileUtilityObject);
	}

	SpriteSystem::Init();
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
	SpriteSystem::Deinit();
	MainConsoleObject.reset();
	KillLogFileObject.reset();
	AnimationSystemObject.reset();
	LogFileObject.reset();
	FileUtilityObject.reset();
}

template class FGlobalSystems<Config::GetModule()>;
