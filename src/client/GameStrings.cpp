// automatically converted
#include "GameStrings.hpp"
#include "shared/misc/PortUtils.hpp"
#include "shared/misc/PortUtilsSoldat.hpp"

#define ENABLE_STUBS 1

#if ENABLE_STUBS == 0
tmofile translationfile;
#endif

bool inittranslation(const std::string &filename)
{
#if ENABLE_STUBS
    NotImplemented(NITag::LOCALIZATION);
    return false;
#else
    tstream translationstream;

    bool inittranslation_result;
    result = false;
    translationstream = PhysFS_ReadAsStream((pchar)(filename));
    if (translationstream == nullptr)
    {
        return inittranslation_result;
    }
    //  try
    translationfile = tmofile.create(translationstream);
    result = true;
    //  except
    result = false;
    //  end;
    translationstream.free;
    return inittranslation_result;
#endif
}

std::wstring _(const std::wstring &inputtext)
{
#if ENABLE_STUBS
    NotImplemented(NITag::LOCALIZATION, __PRETTY_FUNCTION__);
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

std::string _(const std::string &inputtext)
{
#if ENABLE_STUBS
    NotImplemented(NITag::LOCALIZATION, "No translations only stubs available");
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
    NotImplemented(NITag::LOCALIZATION);
#else
    translationfile.free;
#endif
}
