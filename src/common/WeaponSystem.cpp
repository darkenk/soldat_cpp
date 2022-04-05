#include "WeaponSystem.hpp"

WeaponSystem::WeaponSystem()
{
  std::fill(std::begin(WeaponActive), std::end(WeaponActive), 0);
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

uint32_t WeaponSystem::CountEnabledPrimaryWeapons() const
{
  auto pricount = 0;
  for (auto i = 1; i <= primary_weapons; i++)
  {
    pricount += WeaponActive[i];
  }
  return pricount;
}

uint32_t WeaponSystem::CountEnabledSecondaryWeapons() const
{
  auto seccount = 0;
  for (auto i = 1; i <= secondary_weapons; i++)
  {
    seccount += WeaponActive[i + primary_weapons];
  }
  return seccount;
}
