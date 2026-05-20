#pragma once

#include "../math/pos2d.h"
#include <vector>


extern void TileByLine(float xSrc,float ySrc,float xTarget,float yTarget,float tilelength,std::vector<i_math::pos2di>&queueTiles);

//xys should be a 6 float buffer,with 3 2D-coordinate(format:x1,y1,x2,y2,x3,y3)
extern void TileByTriangle(float *xys,float tilelength,std::vector<i_math::pos2di>&queueTiles);
