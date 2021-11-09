#pragma once

#include "shared/mechanics/Sprites.hpp"
#include "shared/misc/TIniFile.hpp"

void rendergostek(tsprite &soldier);
void loadgostekdata(const TIniFile::Entries &data);
void applygostekconstraints();
