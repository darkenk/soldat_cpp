#pragma once
#include "Bullets.hpp"


template<class Bullet = tbullet>
class TBulletSystem
{
  public:
    TBulletSystem();

    PascalArray<Bullet, 1, max_bullets> &GetBullets()
    {
        return Bullets;
    }

    void KillAll()
    {
        for (auto i = 1; i <= max_bullets; i++)
        {
            Bullets[i].kill();
        }
    }

  private:
    TBulletSystem(const TBulletSystem&) = delete;
    PascalArray<Bullet, 1, max_bullets> Bullets;
};

using BulletSystem = TBulletSystem<>;

