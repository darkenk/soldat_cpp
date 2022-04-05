// automatically converted

#define __autoupdater_implementation__

#include "soldat_cpp/soldat/shared/AutoUpdater.cpp.h"

void startautoupdater()
{
  std::int32_t i;

  updaterprocess = tprocess.create(nullptr);
  //  try
#ifdef WINDOWS
  updaterprocess.executable = extractfilepath(paramstr(0)) + "solupd.exe";
#else
  updaterprocess.executable = extractfilepath(paramstr(0)) + "solupd";
#endif
  updaterprocess.currentdirectory = extractfilepath(paramstr(0));
  updaterprocess.parameters.add("--waitpid");
  updaterprocess.parameters.add(inttostr(system.getprocessid));
  for (i = 1; i <= getenvironmentvariablecount; i++)
    updaterprocess.environment.add(getenvironmentstring(i));
  updaterprocess.options = updaterprocess.options + set::of(pousepipes, eos);

  updaterprocess.execute;
  //  except
  //    on E: Exception do
  //    begin
  //    end;
  //  end;
}
