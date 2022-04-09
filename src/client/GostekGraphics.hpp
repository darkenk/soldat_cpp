#pragma once

#include "common/misc/TIniFile.hpp"

template <Config::Module M>
class Sprite;

void rendergostek(Sprite<Config::GetModule()> &soldier);
void loadgostekdata(const TIniFile::Entries &data);
void applygostekconstraints();
