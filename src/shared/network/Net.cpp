#include "Net.hpp"
#include "../Constants.hpp"
#include "common/misc/SoldatConfig.hpp" // IWYU pragma: keep
#include "common/Logging.hpp"
#include "common/misc/PortUtilsSoldat.hpp"
#include "shared/Demo.hpp"
#include "shared/Game.hpp"
#include <array>
#include <numeric>
#include <string>
#include <vector>
#ifdef SERVER
#include "../../server/Server.hpp"
#include "shared/misc/GlobalSystems.hpp"
#endif

#ifdef SERVER

void TServerPlayer::applyshirtcolorfromteam()
{
  if ((CVar::sv_teamcolors) and GS::GetGame().isteamgame())
  {
    switch (team)
    {
    case 1:
      shirtcolor = 0xFFD20F05;
      break;
    case 2:
      shirtcolor = 0xFF151FD9;
      break;
    case 3:
      shirtcolor = 0xFFD2D205;
      break;
    case 4:
      shirtcolor = 0xFF05D205;
      break;
    }
  }
}

#endif
