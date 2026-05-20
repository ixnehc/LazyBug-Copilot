#pragma once

#include "class/class.h"

#include "UnitMgr.h"

#include "nav/navdata.h"
#include "nav/detour/DetourNavMeshQuery.h"

#include "circum/CircumSites.h"

class dtNavMeshQuery;
class CUnitMgrNavMesh:public CUnitMgr
{
public:
	CUnitMgrNavMesh()
	{
		Zero();
	}
	void Zero()
	{
		_query=NULL;
		_nd=NULL;
		_filters[UnitFindPath_Walkable].m_tp=dtQueryFilter::FindWalkable;
		_filters[UnitFindPath_AdvWalkable].m_tp=dtQueryFilter::FindAdvWalkable;
		_filterSwitchable.m_tp=dtQueryFilter::FindSwitchable;
	}

	BOOL Create(navData *nd);
	void Destroy();

	navData *GetNavData()	{		return _nd;	}

	//返回有无Hit
	BOOL StaticRayCast(UnitFindPathType tpFindPath,i_math::vector2df &posSrc,i_math::vector2df &posTarget,i_math::vector2df &posHit);

	//返回有无Hit
	BOOL MoveTo(UnitFindPathType tpFindPath,i_math::vector2df &pos,i_math::vector2df &dir);

	BOOL ToClosestWalkable(UnitFindPathType tpFindPath,i_math::vector2df &pos);//将pos移到最近的walkable处
	BOOL ToClosestWalkable(UnitFindPathType tpFindPath,i_math::vector3df &pos);//将pos移到最近的walkable处

	//注意:返回的路径包含起始点及终止点
	BOOL FindCorridor(UnitFindPathType tpFindPath,i_math::vector2df &start,i_math::vector2df& end,std::vector<dtPolyRef>&path);

	virtual BOOL FindPath(UnitFindPathType tpFindPath,i_math::vector2df &start,i_math::vector2df& end,float toleranceEnd,std::vector<i_math::vector2df>&path,BOOL &bEscape);
	virtual BOOL FindPathOnUnwalkable(UnitFindPathType tpFindPath,i_math::vector2df &start,i_math::vector2df& end,std::vector<i_math::vector2df>&path);

	//返回两点间是否有静态障碍
	virtual BOOL StaticObstacleTest(UnitFindPathType tpFindPath,i_math::vector2df &posSrc,i_math::vector2df &posTarget);

	BOOL IsReachable(UnitFindPathType tpFindPath,i_math::vector2df &posSrc,i_math::vector2df &posTarget);
	virtual BOOL IsWalkable(UnitFindPathType tpFindPath,i_math::vector2df &pos);

	virtual BOOL CheckWalkableArea(UnitFindPathType tpFindPath,i_math::vector2df &pos,float radius);

	virtual void SwitchWalkable(BOOL bOn,i_math::vector2df &pos,float radius=20.0f);

protected:

	BOOL _ClipIn(UnitFindPathType tpFindPath,float &x,float &y,dtPolyRef &ref);

	CCircumSites _csites;

	navData *_nd;
	dtNavMeshQuery *_query;
	dtQueryFilter _filters[UnitFindPathType_Max];
	dtQueryFilter _filterSwitchable;

	dtPolyRef _refPath[128];
	i_math::vector3df _path[128];



};
