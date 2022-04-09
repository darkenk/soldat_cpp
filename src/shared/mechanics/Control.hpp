#pragma once

template <Config::Module M>
class Sprite;

void controlsprite(Sprite<Config::GetModule()> &spritec);
