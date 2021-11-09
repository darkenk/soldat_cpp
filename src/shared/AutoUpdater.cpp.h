#pragma once


/*#include "Process.h"*/
/*#include "sysutils.h"*/


void startautoupdater();


#ifdef __autoupdater_implementation__
#undef EXTERN
#define EXTERN
#endif

extern tprocess updaterprocess;
#undef EXTERN
#define extern extern


