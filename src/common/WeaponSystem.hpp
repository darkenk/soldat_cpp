#pragma once
#include "Weapons.hpp"

class WeaponSystem
{
  public:
    WeaponSystem();

    void EnableAllWeapons();
    void EnableWeapon(std::uint32_t weaponId, bool enable = true) {
        if (WeaponActive[weaponId] == enable)
        {
            return;
        }
        WeaponActive[weaponId] = enable ? 1 : 0;
        WeaponsInGame += enable ? 1 : -1;
    }
    std::uint32_t CountEnabledPrimaryWeapons() const;
    std::uint32_t CountEnabledSecondaryWeapons() const;
    bool IsEnabled(std::uint32_t weaponId) const { return WeaponActive[weaponId] == 1; }

    std::uint32_t GetWeaponsInGame() const { return WeaponsInGame; }

    std::uint64_t GetLoadedWMChecksum() const
    {
        return LoadedWMChecksum;
    }
    void SetLoadedWMChecksum(std::uint64_t _LoadedWMChecksum)
    {
        LoadedWMChecksum = _LoadedWMChecksum;
    }

    std::uint64_t GetDefaultWMChecksum() const
    {
        return DefaultWMChecksum;
    }
    void SetDefaultWMChecksum(std::uint64_t _DefaultWMChecksum)
    {
        DefaultWMChecksum = _DefaultWMChecksum;
    }

  private:
    WeaponSystem(const WeaponSystem&) = delete;
    PascalArray<std::uint8_t, 1, main_weapons> WeaponActive;
    std::uint32_t WeaponsInGame = {0};
    std::uint64_t LoadedWMChecksum;
    std::uint64_t DefaultWMChecksum;
};
