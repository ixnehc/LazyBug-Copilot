
#pragma once

#include "WorldSystem/IObjMap.h"

#include "nav/navservice.h"

struct NavMeshBuildParams
{
	NavMeshBuildParams()
	{
		cellSize=0.1f;
		cellHeight=0.2f;

		agentHeight=2.0f;
		agentRadius=0.35f;
		agentMaxClimb=0.9f;
		agentMaxSlope=45.0f;

		regionMinSize=8;
		regionMergeSize=20;

		monotonePartitioning=true;

		edgeMaxLen=12;
		edgeMaxError=1.3f;
		vertsPerPoly=6;
		detailSampleDist=6;
		detailSampleMaxError=1;
	}
	float cellSize;	
	float cellHeight;
	float agentHeight;
	float agentRadius;
	float agentMaxClimb;
	float agentMaxSlope;
	float regionMinSize;
	float regionMergeSize;
	bool  monotonePartitioning;
	float edgeMaxLen;
	float edgeMaxError;
	float vertsPerPoly;
	float detailSampleDist;
	float detailSampleMaxError;
};

struct NavMeshBuildResult
{
	NavMeshBuildResult()
	{
		vtxsTris = NULL;
		areasTris=NULL;
		vtxsInnerEdge = NULL;
		vtxsOuterEdge = NULL;
		gateEdges = NULL;
		gateRegID = NULL;
		linkEdges = NULL;
		rx = ry = 0;
	}
	i_math::vector3df * vtxsTris;
	BYTE *areasTris;
	int nTris;
	i_math::vector3df * vtxsInnerEdge;
	int nInnerEdge;
	i_math::vector3df * vtxsOuterEdge;
	int nOuterEdge;
	i_math::vector3df * gateEdges;
	int * gateRegID;
	int nGates;
	i_math::vector3df * linkEdges;
	int nLinkEdges;
	float szKB;
	int rx,ry;
};

class IEntitySystem;
class IFile;
class INavMeshEditor :public IObjMapEditor
{
public:
	virtual BOOL Build(IEntitySystem *pES,i_math::recti &rcBlock) = 0;	//在一定区域内生成导航数据
	virtual BOOL SaveNavData(IFile *fl) = 0;							//导出导航信息
	virtual NavMeshBuildParams * GetParams() = 0;
	virtual void SetParams(NavMeshBuildParams * params) = 0;
	virtual void GetBuildResult(const HMapObj &hObj,NavMeshBuildResult * pResult) = 0;
};

class INavService
{
public:
	virtual navPath * CreatePath(i_math::vector3df &startPos,i_math::vector3df &endPos) = 0;
};


