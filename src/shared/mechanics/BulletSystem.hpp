#pragma once
#include "Bullets.hpp"
#include "common/Constants.hpp"
#include "common/misc/PortUtilsSoldat.hpp"

template <class Bullet = tbullet>
class TBulletSystem
{
public:
  TBulletSystem();

  PascalArray<Bullet, 1, Constants::MAX_BULLETS> &GetBullets()
  {
    return Bullets;
  }

  void KillAll()
  {
    for (auto i = 1; i <= Constants::MAX_BULLETS; i++)
    {
      Bullets[i].kill();
    }
  }

private:
  TBulletSystem(const TBulletSystem &) = delete;
  PascalArray<Bullet, 1, Constants::MAX_BULLETS> Bullets;
};

using BulletSystem = TBulletSystem<>;
