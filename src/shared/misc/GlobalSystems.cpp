#include "GlobalSystems.hpp"
#include "common/FileUtility.hpp"
#include "shared/AnimationSystem.hpp"
#include "common/LogFile.hpp"
#include "common/Console.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/SpriteSystem.hpp"
#include <memory>
#ifdef SERVER
#include "server/Server.hpp"
#endif // SERVER

template <Config::Module M>
GlobalSystems<M>::GlobalSystems()
{
  if constexpr (Config::IsServer(M))
  {
    FileUtilityObject = std::make_unique<FileUtility>("/server");
  }
  else
  {
    FileUtilityObject = std::make_unique<FileUtility>("/client");
  }
  MainConsoleObject = std::make_unique<ConsoleType>();
  ConsoleLogFileObject = std::make_unique<LogFile>(*FileUtilityObject);
  if constexpr(Config::IsServer(M))
  {
    KillLogFileObject = std::make_unique<LogFile>(*FileUtilityObject);
  }

  SpriteSystem::Init();
  AnimationSystem::Init();
  ThingSystemObject = std::make_unique<ThingSystem>();
  BulletSystemObject = std::make_unique<BulletSystem>();
  WeaponSystemObject = std::make_unique<WeaponSystem>();
  DemoRecorder = std::make_unique<tdemorecorder<M>>();
  Game<M>::Init();
}

template <Config::Module M>
GlobalSystems<M>::~GlobalSystems()
{
  Game<M>::Deinit();
  DemoRecorder.reset();
  WeaponSystemObject.reset();
  BulletSystemObject.reset();
  ThingSystemObject.reset();
  AnimationSystem::Deinit();
  SpriteSystem::Deinit();
  MainConsoleObject.reset();
  KillLogFileObject.reset();
  ConsoleLogFileObject.reset();
  FileUtilityObject.reset();
}

template class GlobalSystems<Config::GetModule()>;
