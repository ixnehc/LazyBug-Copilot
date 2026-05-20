
#pragma once

#include <vector>

#include "../math/vector3d.h"



void GenScatteringDir(std::vector<i_math::vector3df>&dirs,DWORD nSeg,
					  i_math::vector3df &dirBase,float angleRange);
