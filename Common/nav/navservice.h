
#pragma once

#include "detour/DetourNavMesh.h"

#include "routing/rtWorld.h"

#include "routing/rtQuery.h"

#include "detour/DetourNavMeshQuery.h"

#include "detour/DetourTileQuery.h"

#include "detour/DetourMath.h"

#include <deque>

#define default_maxSearchNodes	1600
#define default_maxSearchStep	256

struct navPathParams 
{
	navPathParams(void)
	{
		m_maxSearchNodes = default_maxSearchNodes;
		m_maxSearchStep = default_maxSearchStep;
		m_diableRouting = false;
	}
	int  m_maxSearchNodes;
	int  m_maxSearchStep;
	bool m_diableRouting;
};

class navPath
{
public:
	
	enum State
	{
		Disable,
		BeginNav,			// Gate 路径未初始化
		OnNav,				// 路径部分计算
		EndNav,				// 完成路径计算
	};
	
	struct FootPrint
	{
		FootPrint(){}
		FootPrint(dtVec3 &p,float d){m_pos = p;m_dist = d;}
		dtVec3 m_pos;
		float  m_dist;
	};
	
	navPath(void){m_navService = 0; m_idx = -1;m_refcount = 0;}
	//
	bool getPos(float dist,dtVec3 &pos);
	void reset(dtVec3 &start,dtVec3 &target);
	bool getVaildDist(float &dist);
	bool ensureValid(float dist);
	void discard(float dist);
	bool isCompleted();

	void setParams(navPathParams &param);

	void addRef();
	void release();
	
	void getPloygonPath(dtVec3 *& polyPt,int &num){polyPt = m_polygonPath.data(); num = m_polygonPath.size();}
	void getPolygonCenter(dtVec3 *&polyCt,int &num){polyCt = m_polygonCenter.data(); num = m_polygonCenter.size();}
	void getGatesPath(dtVec3 *&gatePt,int &num){ gatePt = m_gatePath.data(); num = m_gatePath.size();}
	const char * getMsg(){return m_msg.c_str();}

protected:

	void clear(){ m_gates.clear(); m_footprints.clear(); m_iGateCur = 0; m_polygonPath.clear(); m_polygonCenter.clear(); m_gatePath.clear(); m_msg.clear();}
	void append(dtVec3 *pos,int num);

	friend class navService;

private:
	dtVec3					m_start;
	dtVec3					m_target;
	std::vector<rtGateRef>	m_gates;
	std::vector<FootPrint>	m_footprints;
	State					m_state;
	int						m_iGateCur;
	int						m_idx;
	navService			  * m_navService;
	int						m_refcount;
	navPathParams			m_params;

	std::vector<dtVec3>		m_polygonPath;
	std::vector<dtVec3>		m_polygonCenter;
	std::vector<dtVec3>		m_gatePath;
	std::string				m_msg;
};

class navService
{
public:

	navService(void);
	~navService(void);

	void init(  dtMeshBase * mesh,rtWorld * world,
				int maxSearchNodes = default_maxSearchNodes,
				int maxSearchStep = default_maxSearchStep);

	void unInit(void);
	
	navPath *createPath();

protected:
	
	virtual dtMeshTile * getOffMeshTile(int rx,int ry);
	
	bool	getPos(int ia,float dist,dtVec3 &pos);
	void	reset(int ia,dtVec3 &start,dtVec3 &target);
	bool	isCompleted(int ia);
	bool	getVaildDist(int ia,float &dist);
	bool	ensureValid(int ia,float dist);
	void	discard(int ia,float dist);

	void	destroyPath(int ia);
	void	updatePos(int ia,float dist);

	bool	findKey(int &k0,int &k1,float &r,navPath *path,float dist);
	void	findGatePath(int ia);
	bool	findPolyPath(navPath *path,const dtVec3 &startPos,const dtVec3 &endPos,int maxPath,dtVec3 *&posPath,int &nPos);

	bool	isConInRegion(rtRegion * rg,const dtVec3 &startPos,const dtVec3 &endPos);

	void	calcPolyPath2Dist(int ia,float dist);
	
	friend class navPath;

private:

	dtMeshBase						* m_navMesh;
	rtWorld							* m_world;
	std::deque<navPath *>			m_Paths;				//路径代理
	std::vector<int>				m_idles;
	rtQuery							m_rtQuery;				//路由寻路
	dtNavMeshQuery					m_navQuery;				//多边形寻路
	dtQueryFilter					m_filter;	
	dtMeshTileQuery					m_navTileQuery;			//单个多边形导航
	dtPolyRef						m_startRef,m_endRef;
	dtPolyRef						* m_pathRef;
	dtPolyRef						* m_pathStraigthRef;
	dtVec3							* m_pathPoint;
	unsigned char					* m_pathFlags;
	int								m_maxSearchNodes;
	int								m_maxSearchStep;
	std::vector<rtGateRef>			m_tempSearchpath;
};

