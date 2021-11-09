#pragma once

#include <cstdint>

// DK_TODO soldat_version_chars is dummy lenght
constexpr std::int32_t soldat_version_chars = 6;

constexpr char soldat_version[] = "1.8.0";

//  SOLDAT_VERSION = {$INCLUDE Version.txt};
// constexpr std::int32_t soldat_version_chars = length(soldat_version);
//  SOLDAT_VERSION_LONG = {$INCLUDE %BUILD_ID%};
#ifdef SERVER
const char dedversion[] = "2.9.0";
const char coreversion[] = "2.5";
#else
constexpr char dedversion[] = "GAME";
#endif
constexpr std::int32_t demo_version = 0; // version number for soldat demo files
