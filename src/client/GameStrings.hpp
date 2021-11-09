#pragma once

#include <string>

/*#include "GetText.h"*/
/*#include "Classes.h"*/
/*#include "PhysFS.h"*/

bool inittranslation(const std::string &filename);
std::wstring _(const std::wstring &inputtext);
std::string _(const std::string &inputtext);
void deinittranslation();
