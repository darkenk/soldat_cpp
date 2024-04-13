#pragma once
#include "SoldatConfig.hpp"
#include "common/FileUtility.hpp"
#include "common/WeaponSystem.hpp"
#include "common/misc/GlobalSubsystem.hpp"
#include "common/misc/PortUtils.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/BulletSystem.hpp"
#include "shared/mechanics/ThingSystem.hpp"
#include "shared/Console.hpp"
#include <memory>

class FileUtility;

template <Config::Module M>
class GlobalSystems final : public GlobalSubsystem<GlobalSystems<M>>
{
public:
  static WeaponSystem &GetWeaponSystem()
  {
    return *GlobalSystems::Get().WeaponSystemObject;
  }

  static Game<M> &GetGame()
  {
    return Game<M>::Get();
  }

  static tdemorecorder<M> &GetDemoRecorder()
  {
    return *GlobalSystems::Get().DemoRecorder;
  }

  static BulletSystem &GetBulletSystem()
  {
    return *GlobalSystems::Get().BulletSystemObject;
  }

  static ThingSystem &GetThingSystem()
  {
    return *GlobalSystems::Get().ThingSystemObject;
  }

  static Console<M> &GetMainConsole()
  {
    return *GlobalSystems::Get().MainConsoleObject;
  }

  static void SetMainConsole(std::unique_ptr<Console<M>>&& console)
  {
    GlobalSystems::Get().MainConsoleObject = std::move(console);
  }


  static FileUtility &GetFileSystem()
  {
    return *GlobalSystems::Get().FileUtilityObject;
  }


protected:
  GlobalSystems();
  ~GlobalSystems();

private:
  friend std::default_delete<GlobalSystems>;
  friend GlobalSubsystem<GlobalSystems<M>>;
  std::unique_ptr<WeaponSystem> WeaponSystemObject;
  std::unique_ptr<tdemorecorder<M>> DemoRecorder;
  std::unique_ptr<BulletSystem> BulletSystemObject;
  std::unique_ptr<ThingSystem> ThingSystemObject;
  std::unique_ptr<FileUtility> FileUtilityObject;
  std::unique_ptr<Console<M>> MainConsoleObject;
};

using GS = GlobalSystems<Config::GetModule()>;
