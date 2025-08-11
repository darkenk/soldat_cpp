#pragma once

#include <string>

#include "misc/TStream.hpp"

class TStream;

bool inittranslation(TStream *stream);
std::wstring _(const std::wstring &inputtext);
std::string _(const std::string &inputtext);
void deinittranslation();
