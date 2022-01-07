#pragma once
#include "Weapons.hpp"

class WeaponSystem
{
  public:
    WeaponSystem();

    void EnableAllWeapons();
    void EnableWeapon(std::uint32_t weaponId, bool enable = true) {
        WeaponActive[weaponId] = enable ? 1 : 0;
    }
    std::uint32_t CountEnabledPrimaryWeapons() const;
    std::uint32_t CountEnabledSecondaryWeapons() const;
    bool IsEnabled(std::uint32_t weaponId) const { return WeaponActive[weaponId] == 1; }

  private:
    WeaponSystem(const WeaponSystem&) = delete;
    PascalArray<std::uint8_t, 1, main_weapons> WeaponActive;
};
