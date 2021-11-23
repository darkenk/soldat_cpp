#pragma once

#include "common/misc/TIniFile.hpp"
#include "shared/mechanics/Sprites.hpp"

void rendergostek(tsprite &soldier);
void loadgostekdata(const TIniFile::Entries &data);
void applygostekconstraints();
