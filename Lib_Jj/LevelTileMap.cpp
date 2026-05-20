/********************************************************************
	created:	2011/12/13
	file base:	LevelTileMap
	author:		cxi
	
	purpose:	Level Tile Map
*********************************************************************/
#include "stdh.h"

#include "LevelTileMap.h"


void CLevelTileMap::Create(i_math::recti &rcMap)
{
	_posStart.x=(float)rcMap.Left();
	_posStart.y=(float)rcMap.Top();

	_w=(int)(((float)rcMap.getWidth())/(float)LEVEL_AOVMAP_BLOCKLEN)+1;
	_h=(int)(((float)rcMap.getHeight())/(float)LEVEL_AOVMAP_BLOCKLEN)+1;

}

