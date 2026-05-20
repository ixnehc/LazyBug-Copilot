
#pragma once

#include "unitmgr/UnitMap.h"

#include "mempool/classpool.h"

#include "Definitions.h"

class RvoObstacle;
class RvoObstacleUnit:public CUnitBase
{
public:
	RvoObstacleUnit()
	{
		_obstacle=NULL;
	}
	RvoObstacle * _obstacle;
};

class RvoUnit;
class RvoObstacleMap:public CUnitMap
{
public:
	RvoObstacleMap();

	~RvoObstacleMap();

	void clear();

	void collectNeighbors(RvoUnit *unit,float r, std::vector<RvoObstacle *>&bufWorking);

	RvoObstacle **getObstacles(DWORD &c)
	{
		c=_obstacles.size();
		return _obstacles.data();
	}

private:

	size_t addObstacle(const std::vector<RvoVec2> &vertices,bool isPolygon=true);

	void build(CClassPool<RvoObstacleUnit> &unitsPool,std::vector<i_math::pos2di> &queueTilesWorking,std::vector<RvoObstacle *>&bufWorking);

	void relink(std::vector<RvoObstacle *>&bufWorking);

	std::vector<RvoObstacle *> _obstacles;
	std::vector<RvoObstacle *> _obstacles2;

	friend class RvoSimulator;
	friend struct navMesh;
};

