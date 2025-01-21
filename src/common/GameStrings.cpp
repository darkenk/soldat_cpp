// automatically converted
#include "GameStrings.hpp"
#include "misc/PortUtils.hpp"
#include "misc/PortUtilsSoldat.hpp"

#define ENABLE_STUBS 1

#if ENABLE_STUBS == 0
tmofile translationfile;
#endif

auto inittranslation(TStream *stream) -> bool
{
#if ENABLE_STUBS
  NotImplemented("localization");
  return false;
#else

  bool inittranslation_result;
  result = false;
  //  try
  translationfile = tmofile.create(stream);
  result = true;
  //  except
  result = false;
  //  end;
  translationstream.free;
  return inittranslation_result;
#endif
}

auto _(const std::wstring &inputtext) -> std::wstring
{
#if ENABLE_STUBS
  NotImplemented("localization", __PRETTY_FUNCTION__);
  return inputtext;
#else
  const std::wstring &translation;

  void __result;
  if (assigned(translationfile))
  {
    translation = const std::wstring & (translationfile.translate(string(inputtext)));
    if (translation == "")
      translation = inputtext;
  }
  else
    translation = inputtext;
  result = translation;
  return __result;
#endif
}

auto _(const std::string &inputtext) -> std::string
{
#if ENABLE_STUBS
  NotImplemented("localization", "No translations only stubs available");
  return inputtext;
#else

  std::string translation;

  void __result;
  if (assigned(translationfile))
  {
    translation = translationfile.translate(inputtext);
    if (translation == "")
      translation = inputtext;
  }
  else
    translation = inputtext;
  result = const std::wstring & (translation);
  return __result;
#endif
}

void deinittranslation()
{
#if ENABLE_STUBS
  NotImplemented("localization");
#else
  translationfile.free;
#endif
}
