#pragma once

#include <string>

/*#include "GetText.h"*/
/*#include "Classes.h"*/
/*#include "PhysFS.h"*/
#include <common/misc/TStream.hpp>

bool inittranslation(TStream *stream);
std::wstring _(const std::wstring &inputtext);
std::string _(const std::string &inputtext);
void deinittranslation();
