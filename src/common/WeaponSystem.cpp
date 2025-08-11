#include "WeaponSystem.hpp"

#include <algorithm>

WeaponSystem::WeaponSystem()
{
  std::ranges::fill(WeaponActive, 0);
}

void WeaponSystem::EnableAllWeapons()
{
  // Implement TODO enabled weapons in cvar system
  for (auto i = 1; i <= main_weapons; i++)
  {
    WeaponActive[i] = 1;
  }
  WeaponsInGame = main_weapons;
}

auto WeaponSystem::CountEnabledPrimaryWeapons() const -> uint32_t
{
  auto pricount = 0;
  for (auto i = 1; i <= primary_weapons; i++)
  {
    pricount += WeaponActive[i];
  }
  return pricount;
}

auto WeaponSystem::CountEnabledSecondaryWeapons() const -> uint32_t
{
  auto seccount = 0;
  for (auto i = 1; i <= secondary_weapons; i++)
  {
    seccount += WeaponActive[i + primary_weapons];
  }
  return seccount;
}
