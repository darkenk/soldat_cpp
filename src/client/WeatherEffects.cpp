// automatically converted

#include "WeatherEffects.hpp"

#include "Client.hpp"
#include "Sound.hpp"
#include "shared/Cvar.hpp"
#include "shared/Game.hpp"
#include "shared/mechanics/Sprites.hpp"
#include "shared/misc/GlobalSystems.hpp"

// Backgorund animation and sound for rain
void makerain()
{
  std::int32_t i;
  tvector2 a;
  tvector2 b;
  std::int32_t modder;

  if (CVar::r_maxsparks < (max_sparks - 10))
  {
    modder = 34;
  }
  else
  {
    modder = 17;
  }

  if (GS::GetGame().GetMainTickCounter() % modder == 0)
  {
    a.x = camerax - gamewidthhalf - 128;

    b.x = 0;
    b.y = 12;

    for (i = 1; i <= 8; i++)
    {
      a.x = a.x + 128 - 50 + Random(90);
      a.y = cameray - gameheighthalf - 128 - 60 + Random(150);

      createspark(a, b, 38, 255, 60);
    }
  }

  playsound(SfxEffect::wind, channel_weather);
}

// Backgorund animation and sound for sandstorm
void makesandstorm()
{
  std::int32_t i;
  tvector2 a;
  tvector2 b;
  std::int32_t modder;

  if (CVar::r_maxsparks < (max_sparks - 10))
  {
    modder = 34;
  }
  else
  {
    modder = 17;
  }

  if (GS::GetGame().GetMainTickCounter() % modder == 0)
  {
    a.x = camerax - gamewidthhalf - 1.5 * 512;

    b.x = 10;
    b.y = 7;

    for (i = 1; i <= 8; i++)
    {
      a.x = a.x + 128 - 50 + Random(90);
      a.y = cameray - gameheighthalf - 256 - 60 + Random(150);

      createspark(a, b, 39, 255, 80);
    }
  }

  playsound(SfxEffect::wind, channel_weather);
}

// Backgorund animation and sound for snow
void makesnow()
{
  std::int32_t i;
  tvector2 a;
  tvector2 b;
  std::int32_t modder;

  if (CVar::r_maxsparks < (max_sparks - 10))
  {
    modder = 34;
  }
  else
  {
    modder = 17;
  }

  if (GS::GetGame().GetMainTickCounter() % modder == 0)
  {
    a.x = camerax - gamewidthhalf - 256;

    b.x = 1;
    b.y = 2;

    for (i = 1; i <= 8; i++)
    {
      a.x = a.x + 128 - 50 + Random(90);
      a.y = cameray - gameheighthalf - 60 + Random(150);

      createspark(a, b, 53, 255, 80);
    }
  }

  playsound(SfxEffect::wind, channel_weather);
}
