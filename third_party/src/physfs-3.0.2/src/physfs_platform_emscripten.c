/*
 * Emscripten support routines for PhysicsFS.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 * This file is based on Ryan C. Gordon work.
 *
 */

#define __PHYSICSFS_INTERNAL__
#include "physfs_platforms.h"

#ifdef PHYSFS_PLATFORM_EMSCRIPTEN

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#if PHYSFS_NO_CDROM_SUPPORT
#elif PHYSFS_PLATFORM_LINUX
#  define PHYSFS_HAVE_MNTENT_H 1
#elif defined __CYGWIN__
#  define PHYSFS_HAVE_MNTENT_H 1
#elif PHYSFS_PLATFORM_SOLARIS
#  define PHYSFS_HAVE_SYS_MNTTAB_H 1
#elif PHYSFS_PLATFORM_BSD
#  define PHYSFS_HAVE_SYS_UCRED_H 1
#else
#  warning No CD-ROM support included. Either define your platform here,
#  warning  or define PHYSFS_NO_CDROM_SUPPORT=1 to confirm this is intentional.
#endif

#ifdef PHYSFS_HAVE_SYS_UCRED_H
#  ifdef PHYSFS_HAVE_MNTENT_H
#    undef PHYSFS_HAVE_MNTENT_H /* don't do both... */
#  endif
#  include <sys/mount.h>
#  include <sys/ucred.h>
#endif

#ifdef PHYSFS_HAVE_MNTENT_H
#include <mntent.h>
#endif

#ifdef PHYSFS_HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif

#ifdef PHYSFS_PLATFORM_FREEBSD
#include <sys/sysctl.h>
#endif


#include "physfs_internal.h"

int __PHYSFS_platformInit(void)
{
  return 1;  /* always succeed. */
} /* __PHYSFS_platformInit */


void __PHYSFS_platformDeinit(void)
{
  /* no-op */
} /* __PHYSFS_platformDeinit */


void __PHYSFS_platformDetectAvailableCDs(PHYSFS_StringCallback cb, void *data)
{
} /* __PHYSFS_platformDetectAvailableCDs */


/*
 * See where program (bin) resides in the $PATH specified by (envr).
 *  returns a copy of the first element in envr that contains it, or NULL
 *  if it doesn't exist or there were other problems. PHYSFS_SetError() is
 *  called if we have a problem.
 *
 * (envr) will be scribbled over, and you are expected to allocator.Free() the
 *  return value when you're done with it.
 */
static char *findBinaryInPath(const char *bin, char *envr)
{
  size_t alloc_size = 0;
  char *exe = NULL;
  char *start = envr;
  char *ptr;

  assert(bin != NULL);
  assert(envr != NULL);

  do
  {
    size_t size;
    size_t binlen;

    ptr = strchr(start, ':');  /* find next $PATH separator. */
    if (ptr)
      *ptr = '\0';

    binlen = strlen(bin);
    size = strlen(start) + binlen + 2;
    if (size >= alloc_size)
    {
      char *x = (char *) allocator.Realloc(exe, size);
      if (!x)
      {
        if (exe != NULL)
          allocator.Free(exe);
        BAIL(PHYSFS_ERR_OUT_OF_MEMORY, NULL);
      } /* if */

      alloc_size = size;
      exe = x;
    } /* if */

    /* build full binary path... */
    strcpy(exe, start);
    if ((exe[0] == '\0') || (exe[strlen(exe) - 1] != '/'))
      strcat(exe, "/");
    strcat(exe, bin);

    if (access(exe, X_OK) == 0)  /* Exists as executable? We're done. */
    {
      exe[(size - binlen) - 1] = '\0'; /* chop off filename, leave '/' */
      return exe;
    } /* if */

    start = ptr + 1;  /* start points to beginning of next element. */
  } while (ptr != NULL);

  if (exe != NULL)
    allocator.Free(exe);

  return NULL;  /* doesn't exist in path. */
} /* findBinaryInPath */


static char *readSymLink(const char *path)
{
  ssize_t len = 64;
  ssize_t rc = -1;
  char *retval = NULL;

  while (1)
  {
    char *ptr = (char *) allocator.Realloc(retval, (size_t) len);
    if (ptr == NULL)
      break;   /* out of memory. */
    retval = ptr;

    rc = readlink(path, retval, len);
    if (rc == -1)
      break;  /* not a symlink, i/o error, etc. */

    else if (rc < len)
    {
      retval[rc] = '\0';  /* readlink doesn't null-terminate. */
      return retval;  /* we're good to go. */
    } /* else if */

    len *= 2;  /* grow buffer, try again. */
  } /* while */

  if (retval != NULL)
    allocator.Free(retval);
  return NULL;
} /* readSymLink */


char *__PHYSFS_platformCalcBaseDir(const char *argv0)
{
  const char path[] = "/";
  char *retval = (char *) allocator.Malloc(sizeof(path) + 1);
  BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
  memcpy(retval, path, sizeof(path));
  retval[sizeof(path)] = '\0';
  return retval;
} /* __PHYSFS_platformCalcBaseDir */


char *__PHYSFS_platformCalcPrefDir(const char *org, const char *app)
{
  /*
   * We use XDG's base directory spec, even if you're not on Linux.
   *  This isn't strictly correct, but the results are relatively sane
   *  in any case.
   *
   * https://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html
   */
  const char *envr = getenv("XDG_DATA_HOME");
  const char *append = "/";
  char *retval = NULL;
  size_t len = 0;

  if (!envr)
  {
    /* You end up with "$HOME/.local/share/Game Name 2" */
    envr = __PHYSFS_getUserDir();
    BAIL_IF_ERRPASS(!envr, NULL);  /* oh well. */
    append = ".local/share/";
  } /* if */

  len = strlen(envr) + strlen(append) + strlen(app) + 2;
  retval = (char *) allocator.Malloc(len);
  BAIL_IF(!retval, PHYSFS_ERR_OUT_OF_MEMORY, NULL);
  snprintf(retval, len, "%s%s%s/", envr, append, app);
  return retval;
} /* __PHYSFS_platformCalcPrefDir */

#endif /* PHYSFS_PLATFORM_UNIX */

/* end of physfs_platform_emscripten.c ... */

