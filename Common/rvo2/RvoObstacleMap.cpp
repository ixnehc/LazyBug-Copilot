/*!
 * \file RvoObstacleMap.cpp
 *
 * \author chenxi
 * \date ÁùÔÂ 2018
 *
 * 
 */

#include "stdh.h"
#include "RvoObstacleMap.h"

#include "RvoObstacle.h"
#include "RvoUnit.h"

#include "rasterize/rasterize.h"

RvoObstacleMap::RvoObstacleMap()
{

}

RvoObstacleMap::~RvoObstacleMap()
{
}

void RvoObstacleMap::clear()
{
	for (size_t i = 0; i < _obstacles.size(); ++i) 
	{
		Safe_Class_Delete(_obstacles[i]);
	}
	for (size_t i = 0; i < _obstacles2.size(); ++i) 
	{
		Safe_Class_Delete(_obstacles2[i]);
	}

	_obstacles.clear();
	_obstacles2.clear();

	//Çå¿ÕUnit
	for (int i=0;i<_blocks.size();i++)
	{
		CUnitBlock *blk=_blocks[i];
		if (blk)
		{
			blk->_nUnits=0;
			for (int j=0;j<ARRAY_SIZE(blk->_tiles);j++)
				blk->_tiles[j].units=NULL;
		}
	}

	Destroy();

}


size_t RvoObstacleMap::addObstacle(const std::vector<RvoVec2> &vertices, bool isPolygon)
{
	if (vertices.size() < 2) 
	{
		return RVO::RVO_ERROR;
	}

	const size_t obstacleNo = _obstacles.size();

	for (size_t i = 0; i < vertices.size(); ++i) 
	{
		RvoObstacle *obstacle = Class_New2(RvoObstacle);
		obstacle->_point = vertices[i];

		if (i != 0) 
		{
			obstacle->_prevObstacle = _obstacles.back();
			obstacle->_prevObstacle->_nextObstacle = obstacle;
		}

		if (i == vertices.size() - 1) 
		{
			obstacle->_nextObstacle = _obstacles[obstacleNo];
			obstacle->_nextObstacle->_prevObstacle = obstacle;
		}

		obstacle->_unitDir = normalize(vertices[(i == vertices.size() - 1 ? 0 : i + 1)] - vertices[i]);

		if (vertices.size() == 2) 
		{
			obstacle->_isConvex = true;
		}
		else 
		{
			obstacle->_isConvex = (RVO::leftOf(vertices[(i == 0 ? vertices.size() - 1 : i - 1)], vertices[i], vertices[(i == vertices.size() - 1 ? 0 : i + 1)]) >= 0.0f);
		}

		if ((i != vertices.size() - 1) || isPolygon)
		{
			obstacle->_id = _obstacles.size();
			_obstacles.push_back(obstacle);
		}
		else
		{
			obstacle->_id = RVO::RVO_ERROR;
			_obstacles2.push_back(obstacle);
		}
	}

	return obstacleNo;
}


void RvoObstacleMap::build(CClassPool<RvoObstacleUnit> &unitsPool,std::vector<i_math::pos2di> &queueTilesWorking,std::vector<RvoObstacle *>&bufWorking)
{
	queueTilesWorking.clear();
	
	for (int i=0;i<_obstacles.size();i++)
	{
		RvoObstacle *cur=_obstacles[i];
		RvoObstacle *next=cur->_nextObstacle;

		if (cur&&next)
		{
			TileByLine(cur->_point.x_,cur->_point.y_,next->_point.x_,next->_point.y_,UNITMAP_TILE_LEN,queueTilesWorking);

			for (int i=0;i<queueTilesWorking.size();i++)
			{
				RvoObstacleUnit *unit=unitsPool.Alloc();
				unit->_pos.x=((float)queueTilesWorking[i].x)*UNITMAP_TILE_LEN+UNITMAP_TILE_LEN/2.0f;
				unit->_pos.y=((float)queueTilesWorking[i].y)*UNITMAP_TILE_LEN+UNITMAP_TILE_LEN/2.0f;

				unit->_obstacle=cur;

				AddUnit(unit);
			}
		}
	}

	relink(bufWorking);
}

void RvoObstacleMap::relink(std::vector<RvoObstacle *>&bufWorking)
{
	i_math::rectf rc;


	int nUnlinked=FALSE;
	for (int i=0;i<_obstacles.size();i++)
	{
		RvoObstacle *obstacle=_obstacles[i];

		if (!obstacle)
			continue;

		const float range=1.0f;
		if (obstacle->_nextObstacle)
		{
			if (obstacle->_nextObstacle->_id==RVO::RVO_ERROR)
			{
				RvoVec2 &pos=obstacle->_nextObstacle->_point;
				rc.set(pos.x_,pos.y_,pos.x_,pos.y_);
				rc.inflate(range);

				Enum(rc);

				DWORD c;
				RvoObstacleUnit **units=(RvoObstacleUnit **)GetEnums(c);

				bufWorking.clear();

				for (int i=0;i<c;i++)
				{
					RvoObstacleUnit *unit=units[i];

					if (unit->_obstacle->_bEnum)
						continue;

					if (unit->_obstacle->_id==RVO::RVO_ERROR)
						continue;

					unit->_obstacle->_bEnum=TRUE;
					bufWorking.push_back(unit->_obstacle); 
				}

				RvoObstacle *obstacleClosest=NULL;
				float dist2Min=10000000.0f;
				for (int i=0;i<bufWorking.size();i++)
				{
					RvoObstacle *obstacleTest=bufWorking[i];
					obstacleTest->_bEnum=FALSE;

					if (!obstacleTest->_prevObstacle)
						continue;
					if(obstacleTest->_prevObstacle->_id!=RVO::RVO_ERROR)
						continue;

					float dist2=absSq(obstacleTest->_point-pos);

					if (dist2<range*range)
					{
						if (dist2<dist2Min)
						{
							obstacleClosest=obstacleTest;
							dist2Min=dist2;
						}
					}
				}

				if (obstacleClosest)
				{
					obstacle->_nextObstacle=obstacleClosest;
					obstacleClosest->_prevObstacle=obstacle;

					if (obstacleClosest->_nextObstacle)
						obstacleClosest->_isConvex=(RVO::leftOf(obstacle->_point, obstacleClosest->_point,obstacleClosest->_nextObstacle->_point) >= 0.0f);
				}
				else
					nUnlinked++;
			}
		}
	}

// 	assert(nUnlinked<=0);
}


void RvoObstacleMap::collectNeighbors(RvoUnit *unitRvo,float r, std::vector<RvoObstacle *>&bufWorking)
{
	bufWorking.clear();

	i_math::vector2df pos=unitRvo->CUnitBase::GetPos();
	i_math::rectf rc;
	rc.set(pos.x,pos.y,pos.x,pos.y);
	rc.inflate(r);

	float r2=r*r;

	Enum(rc);

	DWORD c;
	RvoObstacleUnit **units=(RvoObstacleUnit **)GetEnums(c);

	for (int i=0;i<c;i++)
	{
		RvoObstacleUnit *unit=units[i];

		if (unit->_obstacle->_bEnum)
			continue;

		unit->_obstacle->_bEnum=TRUE;
		bufWorking.push_back(unit->_obstacle); 

		unitRvo->insertObstacleNeighbor(unit->_obstacle,r2);
	}

	for (int i=0;i<bufWorking.size();i++)
		bufWorking[i]->_bEnum=FALSE;

	bufWorking.clear();
}
