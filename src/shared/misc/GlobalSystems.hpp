#pragma once
#include "SoldatConfig.hpp"
#include "common/misc/PortUtils.hpp"
#include "common/WeaponSystem.hpp"
#include "common/misc/GlobalSubsystem.hpp"
#include "shared/Game.hpp"
#include "shared/Demo.hpp"

template<Config::Module M = Config::GetModule()>
class GlobalSystems final : public GlobalSubsystem<GlobalSystems<M>>
{
  public:
    static WeaponSystem& GetWeaponSystem()
    {
        return *GlobalSystems::Get().WeaponSystemObject;
    }

    static Game<M>& GetGame()
    {
        return Game<M>::Get();
    }

    static tdemorecorder<M>& GetDemoRecorder()
    {
        return *GlobalSystems::Get().DemoRecorder;
    }

  protected:
    GlobalSystems();
    ~GlobalSystems();

  private:
    friend std::default_delete<GlobalSystems>;
    friend GlobalSubsystem<GlobalSystems<M>>;
    std::unique_ptr<WeaponSystem> WeaponSystemObject;
    std::unique_ptr<tdemorecorder<M>> DemoRecorder;
};

using GS = GlobalSystems<>;
