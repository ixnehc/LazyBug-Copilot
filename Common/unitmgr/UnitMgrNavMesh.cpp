
#include "stdh.h"

#include "commondefines/general_stl.h"
#include "UnitMgrNavMesh.h"

#include "timer/profiler.h"

//////////////////////////////////////////////////////////////////////////
//CUnitMgrNavMesh


BOOL CUnitMgrNavMesh::Create(navData *nd)
{
	if (!nd)
		return FALSE;
	_nd=nd;

	_query=dtAllocNavMeshQuery();
	_query->init(_nd->getNavMesh(),200);

	i_math::recti rcMap;
	navMesh *nmesh=(navMesh *)_nd->getNavMesh();
	rcMap.Left()=nmesh->getLeft();
	rcMap.Right()=nmesh->getRight();
	rcMap.Top()=nmesh->getTop();
	rcMap.Bottom()=nmesh->getBottom();
	rcMap*=nmesh->getTileLen();

	_csites.Build(2,0.1f,0.05f);

	CUnitMgr::Create(rcMap);

	return TRUE;
}

void CUnitMgrNavMesh::Destroy()
{
	CUnitMgr::Destroy();
	if (_query)
		dtFreeNavMeshQuery(_query);
	_csites.Clear();
	Zero();
}



inline bool dtPolyHitTest(dtMeshTile *tile,dtPoly *poly,float x,float z)
{
	float *s,*e;
	for (int i=0;i<poly->vertCount;i++)
	{
		s=&tile->verts[poly->verts[i]*3];
		if (i+1>=poly->vertCount)
			e=&tile->verts[poly->verts[0]*3];
		else
			e=&tile->verts[poly->verts[i+1]*3];

		float d=(x-s[0])*(e[2]-s[2])-(z-s[2])*(e[0]-s[0]);
		if (d<0.0f)
			return false;
	}
	return true;
}

inline bool findPolyFast(dtMeshBase*nmesh,navCells *ncells,float x,float z,dtQueryFilter *filter,dtPolyRef &ref)
{
	int c;
	short xTile,yTile;
	WORD *ids=ncells->findPolys(x,z,c,xTile,yTile);

	dtMeshTile *tile;

	if (c>0)
	{
		if (nmesh->getTilesAt(xTile,yTile,&tile,1)>0)
		{
			for (int i=0;i<c;i++)
			{
				if(ids[i]<tile->header->polyCount)
				{
					dtPoly *poly= tile->polys + ids[i];

					if (!filter->passFilter(ids[i],tile,poly))
						continue;

					if (dtPolyHitTest(tile,poly,x,z))
					{
						((short *)(&ref))[3]=xTile;
						((short *)(&ref))[2]=yTile;
						((DWORD*)(&ref))[0]=ids[i];
						return true;
					}
				}
			}
		}
	}

	return false;
}

BOOL CUnitMgrNavMesh::IsWalkable(UnitFindPathType tpFindPath,i_math::vector2df &pos)
{
	if (((DWORD)tpFindPath)>=UnitFindPathType_Max)
		return false;
	dtPolyRef ref;
	if (findPolyFast(_nd->getNavMesh(),_nd->getCells(),pos.x,pos.y,&_filters[(int)tpFindPath],ref))
		return TRUE;
	return FALSE;
}

BOOL CUnitMgrNavMesh::CheckWalkableArea(UnitFindPathType tpFindPath,i_math::vector2df &pos,float radius)
{
	dtPolyRef ref;
	if (!findPolyFast(_nd->getNavMesh(),_nd->getCells(),pos.x,pos.y,&_filters[(int)tpFindPath],ref))
		return FALSE;

	i_math::vector3df center(pos.x,0.0f,pos.y);

	float t;
	i_math::vector3df t1,t2;
	if (!dtStatusSucceed(_query->findDistanceToWall(ref,(float*)&center,radius,&_filters[(int)tpFindPath],&t,(float*)&t1,(float*)&t2)))
		return FALSE;

	if (t<radius)
		return FALSE;
	return TRUE;
}


BOOL CUnitMgrNavMesh::StaticObstacleTest(UnitFindPathType tpFindPath,i_math::vector2df &posSrc0,i_math::vector2df &posTarget)
{
	if (((DWORD)tpFindPath)>=UnitFindPathType_Max)
		return false;

	BOOL bRet=TRUE;
	i_math::vector2df posSrc=posSrc0;
	dtPolyRef ref;
//	ProfilerStart_Recent(StaticObstacleTest);
	if (_ClipIn(tpFindPath,posSrc.x,posSrc.y,ref))
	{
		i_math::vector3df start(posSrc.x,0.0f,posSrc.y);
		i_math::vector3df end(posTarget.x,0.0f,posTarget.y);
		float t;
		if (dtStatusSucceed(_query->raycast(ref,(float*)&start,(float*)&end,&_filters[(int)tpFindPath],&t,NULL,NULL,NULL,0)))
		{
			if (t>=1.0f)
				bRet=FALSE;
		}
	}
//	ProfilerEnd();

	return bRet;
}

BOOL CUnitMgrNavMesh::_ClipIn(UnitFindPathType tpFindPath,float &x,float &y,dtPolyRef &ref)
{
	if (findPolyFast(_nd->getNavMesh(),_nd->getCells(),x,y,&_filters[(int)tpFindPath],ref))
		return TRUE;

	static const i_math::vector3df ext0(2.0f,1000.0f,2.0f);
	i_math::vector3df start(x,0.0f,y);
	i_math::vector3df vNearest;
	if (dtStatusSucceed(_query->findNearestPoly((float*)&start,(float*)&ext0,&_filters[(int)tpFindPath],&ref,(float*)&vNearest)))
	{
		x=vNearest.x;
		y=vNearest.z;
		return TRUE;
	}

	return FALSE;
 }

BOOL CUnitMgrNavMesh::StaticRayCast(UnitFindPathType tpFindPath,i_math::vector2df &posSrc0,i_math::vector2df &posTarget,i_math::vector2df &posHit)
{
	if (((DWORD)tpFindPath)>=UnitFindPathType_Max)
		return false;

	i_math::vector2df posSrc=posSrc0;
	dtPolyRef ref;

	if (!_ClipIn(tpFindPath,posSrc.x,posSrc.y,ref))
		return false;

	i_math::vector3df start(posSrc.x,0.0f,posSrc.y);
	i_math::vector3df end(posTarget.x,0.0f,posTarget.y);
	float t;
	if (dtStatusSucceed(_query->raycast(ref,(float*)&start,(float*)&end,&_filters[(int)tpFindPath],&t,NULL,NULL,NULL,0)))
	{
		if (t>=1.0f)
			return FALSE;

		posHit=posSrc+(posTarget-posSrc)*t;
		return TRUE;
	}
	return FALSE;
}

BOOL CUnitMgrNavMesh::MoveTo(UnitFindPathType tpFindPath,i_math::vector2df &pos,i_math::vector2df &dir)
{
	i_math::vector2df posHit;
	if (StaticRayCast(tpFindPath,pos,pos+dir,posHit))
	{
		pos=posHit;
		return TRUE;
	}
	pos=pos+dir;
	return FALSE;
}


BOOL CUnitMgrNavMesh::FindCorridor(UnitFindPathType tpFindPath,i_math::vector2df &start0,i_math::vector2df& end0,std::vector<dtPolyRef>&path)
{
	if (((DWORD)tpFindPath)>=UnitFindPathType_Max)
		return false;

	dtQueryFilter *filter=&_filters[(int)tpFindPath];

	static const i_math::vector3df ext0(1.0f,1000.0f,1.0f);
	static const i_math::vector3df ext1(10.0f,1000.0f,10.0f);
	dtPolyRef refStart,refEnd;
	i_math::vector3df start,end;

	if (!findPolyFast(_nd->getNavMesh(),_nd->getCells(),start0.x,start0.y,filter,refStart))
	{
		i_math::vector3df v,vNearest;
		v.set(start0.x,0.0f,start0.y);

		if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext0,filter,&refStart,(float*)&vNearest)))
		{
			if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext1,filter,&refStart,(float*)&vNearest)))
				return FALSE;
		}
		start.set(vNearest.x,0.0f,vNearest.z);
	}
	else
		start.set(start0.x,0.0f,start0.y);

	BOOL bNeedFinalAdjust=FALSE;
	if (!findPolyFast(_nd->getNavMesh(),_nd->getCells(),end0.x,end0.y,filter,refEnd))
	{
		i_math::vector3df v,vNearest;
		v.set(end0.x,0.0f,end0.y);

		if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext0,filter,&refEnd,(float*)&vNearest)))
		{
			if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext1,filter,&refEnd,(float*)&vNearest)))
				return FALSE;
		}
		end.set(vNearest.x,0.0f,vNearest.z);
		if (!(i_math::equals(end0.x,vNearest.x)&&i_math::equals(end0.y,vNearest.z)))
			bNeedFinalAdjust=TRUE;
	}
	else
		end.set(end0.x,0.0f,end0.y);

	int nRefPath;
	if (dtStatusFailed(_query->findPath(refStart,refEnd,(float*)&start,(float*)&end,filter,_refPath,&nRefPath,ARRAY_SIZE(_refPath),false)))
		return FALSE;

	path.resize(nRefPath);
	for (int i=0;i<nRefPath;i++)
		path[i]=_refPath[i];

	return TRUE;

}



BOOL CUnitMgrNavMesh::FindPath(UnitFindPathType tpFindPath,i_math::vector2df &start0,i_math::vector2df&end0,float toleranceEnd,std::vector<i_math::vector2df>&path,BOOL &bEscape)
{
	if (((DWORD)tpFindPath)>=UnitFindPathType_Max)
		return false;

	dtQueryFilter *filter=&_filters[(int)tpFindPath];

	static const i_math::vector3df ext0(1.0f,1000.0f,1.0f);
	static const i_math::vector3df ext1(10.0f,1000.0f,10.0f);
	dtPolyRef refStart,refEnd;
	i_math::vector3df start,end;
	bEscape=FALSE;

	if (!findPolyFast(_nd->getNavMesh(),_nd->getCells(),start0.x,start0.y,filter,refStart))
	{
		i_math::vector3df v,vNearest;
		v.set(start0.x,0.0f,start0.y);

		if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext0,filter,&refStart,(float*)&vNearest)))
		{
			if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext1,filter,&refStart,(float*)&vNearest)))
				return FALSE;
		}
		start.set(vNearest.x,0.0f,vNearest.z);
		if (!(i_math::equals(start0.x,vNearest.x)&&i_math::equals(start0.y,vNearest.z)))
			bEscape=TRUE;
	}
	else
		start.set(start0.x,0.0f,start0.y);

	BOOL bNeedFinalAdjust=FALSE;
	if (!findPolyFast(_nd->getNavMesh(),_nd->getCells(),end0.x,end0.y,filter,refEnd))
	{
		i_math::vector3df v,vNearest;
		v.set(end0.x,0.0f,end0.y);

		if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext0,filter,&refEnd,(float*)&vNearest)))
		{
			if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext1,filter,&refEnd,(float*)&vNearest)))
				return FALSE;
		}
		end.set(vNearest.x,0.0f,vNearest.z);
		if (!(i_math::equals(end0.x,vNearest.x)&&i_math::equals(end0.y,vNearest.z)))
			bNeedFinalAdjust=TRUE;
	}
	else
		end.set(end0.x,0.0f,end0.y);

	int nRefPath;
	if (dtStatusFailed(_query->findPath(refStart,refEnd,(float*)&start,(float*)&end,filter,_refPath,&nRefPath,ARRAY_SIZE(_refPath),false)))
		return FALSE;

	int nPath;
	if (dtStatusFailed(_query->findStraightPath((float*)&start,(float*)&end,_refPath,nRefPath,(float*)_path,NULL,NULL,&nPath,ARRAY_SIZE(_path))))
		return FALSE;

	if (bNeedFinalAdjust)
	{
		if (nPath>=2)
		{
			int idxDirectReach=-1;
			i_math::vector2df endDirectReach;
			float dist2Tolerance=toleranceEnd*toleranceEnd;

			for (int i=nPath-2;i>=0;i--)
			{
				i_math::vector2df ptHit;
				i_math::vector2df ptTest(_path[i].x,_path[i].z);
				if (end0.getDistanceSQFrom(ptTest)<=dist2Tolerance)
				{//这个路点已经足够接近了
					if (i>0)
					{
						idxDirectReach=i-1;
						endDirectReach=ptTest;
						continue;
					}
				}
				if (StaticRayCast(tpFindPath,ptTest,end0,ptHit))
				{
					//找到一个有效的ptHit
					if (end0.getDistanceSQFrom(ptHit)<=dist2Tolerance)
					{
						idxDirectReach=i;
						endDirectReach=ptHit;
						break;
					}
				}
			}

			if (idxDirectReach>=0)
			{
				_path[idxDirectReach+1].set(endDirectReach.x,0.0f,endDirectReach.y);
				nPath=idxDirectReach+2;
			}
		}
	}

	if (!bEscape)
	{
		path.resize(nPath);
		for (int i=0;i<nPath;i++)
			path[i].set(_path[i].x,_path[i].z);
		if (nPath==1)
			path.push_back(path[path.size()-1]);
	}
	else
	{
		path.resize(nPath+1);
		path[0]=start0;
		for (int i=0;i<nPath;i++)
			path[i+1].set(_path[i].x,_path[i].z);
	}


	return TRUE;
}

BOOL CUnitMgrNavMesh::IsReachable(UnitFindPathType tpFindPath,i_math::vector2df &start0,i_math::vector2df&end0)
{
	if (((DWORD)tpFindPath)>=UnitFindPathType_Max)
		return false;

	dtQueryFilter *filter=&_filters[(int)tpFindPath];

	static const i_math::vector3df ext0(1.0f,1000.0f,1.0f);
	static const i_math::vector3df ext1(10.0f,1000.0f,10.0f);
	dtPolyRef refStart,refEnd;
	i_math::vector3df start,end;

	if (!findPolyFast(_nd->getNavMesh(),_nd->getCells(),start0.x,start0.y,filter,refStart))
	{
		i_math::vector3df v,vNearest;
		v.set(start0.x,0.0f,start0.y);

		if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext0,filter,&refStart,(float*)&vNearest)))
		{
			if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext1,filter,&refStart,(float*)&vNearest)))
				return FALSE;
		}
		start.set(vNearest.x,0.0f,vNearest.z);
	}
	else
		start.set(start0.x,0.0f,start0.y);

	if (!findPolyFast(_nd->getNavMesh(),_nd->getCells(),end0.x,end0.y,filter,refEnd))
	{
		i_math::vector3df v,vNearest;
		v.set(end0.x,0.0f,end0.y);

		if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext0,filter,&refEnd,(float*)&vNearest)))
		{
			if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext1,filter,&refEnd,(float*)&vNearest)))
				return FALSE;
		}
		end.set(vNearest.x,0.0f,vNearest.z);
	}
	else
		end.set(end0.x,0.0f,end0.y);

	int nRefPath;
	dtStatus status=_query->findPath(refStart,refEnd,(float*)&start,(float*)&end,filter,_refPath,&nRefPath,ARRAY_SIZE(_refPath),false);
	if (dtStatusFailed(status))
		return FALSE;
	if (dtStatusDetail(status,DT_PARTIAL_RESULT))
		return FALSE;

	return TRUE;
}


BOOL CUnitMgrNavMesh::FindPathOnUnwalkable(UnitFindPathType tpFindPath,i_math::vector2df &start0,i_math::vector2df&end0,
										  std::vector<i_math::vector2df>&path)
{
	if (((DWORD)tpFindPath)>=UnitFindPathType_Max)
		return false;

	dtQueryFilter *filter=&_filters[(int)tpFindPath];

	static const i_math::vector3df ext0(1.0f,1000.0f,1.0f);
	static const i_math::vector3df ext1(10.0f,1000.0f,10.0f);
	dtPolyRef refStart,refEnd;
	i_math::vector3df start,end;
	BOOL bUnwalkableStart=FALSE;
	BOOL bEscapeStart=FALSE;
	BOOL bUnwalkableEnd=FALSE;
	BOOL bEscapeEnd=FALSE;

	if (!findPolyFast(_nd->getNavMesh(),_nd->getCells(),start0.x,start0.y,filter,refStart))
	{
		bEscapeStart=TRUE;
		i_math::vector3df v,vNearest;
		v.set(start0.x,0.0f,start0.y);

		if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext0,filter,&refStart,(float*)&vNearest)))
		{
			if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext1,filter,&refStart,(float*)&vNearest)))
				bEscapeStart=FALSE;
		}
		if (bEscapeStart)
		{
			start.set(vNearest.x,0.0f,vNearest.z);
			if (!(i_math::equals(start0.x,vNearest.x)&&i_math::equals(start0.y,vNearest.z)))
				bUnwalkableStart=TRUE;
		}
		else
			bUnwalkableStart=TRUE;
	}
	else
		start.set(start0.x,0.0f,start0.y);

	if (!findPolyFast(_nd->getNavMesh(),_nd->getCells(),end0.x,end0.y,filter,refEnd))
	{
		bEscapeEnd=TRUE;
		i_math::vector3df v,vNearest;
		v.set(end0.x,0.0f,end0.y);

		if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext0,filter,&refEnd,(float*)&vNearest)))
		{
			if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext1,filter,&refEnd,(float*)&vNearest)))
				bEscapeEnd=FALSE;
		}

		if (bEscapeEnd)
		{
			end.set(vNearest.x,0.0f,vNearest.z);
			if (!(i_math::equals(end0.x,vNearest.x)&&i_math::equals(end0.y,vNearest.z)))
				bUnwalkableEnd=TRUE;
		}
		else
			bUnwalkableEnd=TRUE;
	}
	else
		end.set(end0.x,0.0f,end0.y);

	if ((!bUnwalkableStart)&&(!bUnwalkableEnd))
	{
		//start 和 end都可走,不做什么
	}
	else
	{
		if (bUnwalkableStart&&bUnwalkableEnd)
		{
			//start 和 end都不可走,直接以它们的连线作为路径
			path.resize(2);
			path[0]=start0;
			path[1]=end0;
			return TRUE;
		}

		//Start和 end 只有一个不可走
		if (bUnwalkableStart&&(!bEscapeStart))//Start不可走但又无法找到Escape点
			return FALSE;
		if (bUnwalkableEnd&&(!bEscapeEnd))//End不可走但又无法找到Escape点
			return FALSE;
	}

	int nRefPath;
	if (dtStatusFailed(_query->findPath(refStart,refEnd,(float*)&start,(float*)&end,filter,_refPath,&nRefPath,ARRAY_SIZE(_refPath),false)))
		return FALSE;

	int nPath;
	if (dtStatusFailed(_query->findStraightPath((float*)&start,(float*)&end,_refPath,nRefPath,(float*)_path,NULL,NULL,&nPath,ARRAY_SIZE(_path))))
		return FALSE;

	if (!bUnwalkableStart)
	{
		path.resize(nPath);
		for (int i=0;i<nPath;i++)
			path[i].set(_path[i].x,_path[i].z);
	}
	else
	{
		path.resize(nPath+1);
		path[0]=start0;
		for (int i=0;i<nPath;i++)
			path[i+1].set(_path[i].x,_path[i].z);
	}

	if (bUnwalkableEnd)
		path.push_back(end0);

	return TRUE;
}


BOOL CUnitMgrNavMesh::ToClosestWalkable(UnitFindPathType tpFindPath,i_math::vector2df &pos)
{
	if (IsWalkable(tpFindPath,pos))
		return TRUE;

	if (((DWORD)tpFindPath)>=UnitFindPathType_Max)
		return false;

	dtQueryFilter *filter=&_filters[(int)tpFindPath];

	static const i_math::vector3df ext0(1.0f,1000.0f,1.0f);
	static const i_math::vector3df ext1(10.0f,1000.0f,10.0f);
	dtPolyRef ref;

	i_math::vector3df v,vNearest;
	v.set(pos.x,0.0f,pos.y);

	if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext0,filter,&ref,(float*)&vNearest)))
	{
		if (dtStatusFailed(_query->findNearestPoly((float*)&v,(float*)&ext1,filter,&ref,(float*)&vNearest)))
			return FALSE;
	}

	if (IsWalkable(tpFindPath,i_math::vector2df(vNearest.x,vNearest.z)))
	{
		pos.set(vNearest.x,vNearest.z);
		return TRUE;
	}

	pos.set(vNearest.x,vNearest.z);

	DWORD c;
	i_math::vector2df *buf=_csites.GetSites(c);

	for (int i=0;i<c;i++)
	{
		if (IsWalkable(tpFindPath,pos+buf[i]))
		{
			pos+=buf[i];
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CUnitMgrNavMesh::ToClosestWalkable(UnitFindPathType tpFindPath,i_math::vector3df &pos)
{
	i_math::vector2df pos2D=pos.getXZ();
	if (!ToClosestWalkable(tpFindPath,pos2D))
		return FALSE;
	pos.setXZ(pos2D);
	return TRUE;
}



void CUnitMgrNavMesh::SwitchWalkable(BOOL bOn,i_math::vector2df &pos,float radius)
{
	i_math::vector3df v;
	v.set(pos.x,0.0f,pos.y);
	static const i_math::vector3df ext0(1.0f,1000.0f,1.0f);

	dtPolyRef refStart;
	if (dtStatusFailed(_query->findNearestPoly((float*)&v, (float*)&ext0,&_filterSwitchable,&refStart,NULL)))
		return;

	int count;
	dtPolyRef buf[256];
	if (dtStatusSucceed(_query->findPolysAroundCircle(refStart,(float*)&v,radius,&_filterSwitchable,buf,NULL,NULL,&count,ARRAY_SIZE(buf))))
	{
		_query->switchWalkable(buf,count,bOn?true:false);
	}
}
