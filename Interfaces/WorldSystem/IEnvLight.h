/********************************************************************
	created:	5:11:2009   13:42
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	defines&interfaces for environment light
*********************************************************************/
#pragma once

#include "class/class.h"

#include "WorldSystem/IObjMap.h"

#define NUM_SAMPLES_PER_POSITION 9
#define NUM_DIR_PRE_POSITION	6

class IProbeCubeMapEditor;

//Ambient Cube
struct AmbCube
{
	i_math::vector3df cols[6];//六个值分别对应 -X,+X,-Y,+Y,-Z,+Z 六个方向
};

//Ambient Cube Detector
struct ACDetector
{
	DEFINE_CLASS(ACDetector);
	ACDetector()
	{
		bDirty=1;
		bNeedReset=1;
	}

	void ResetPos(i_math::vector3df &pos_)
	{
		pos=pos_;
		bNeedReset=1;
		bDirty=1;
	}

	void UpdatePos(i_math::vector3df &pos_)
	{
		float d= float((pos_-pos).getLengthSQ());
		if (d>=0.02f)
		{
			if (d>1.0f)
				bNeedReset=1;
			bDirty=1;
			pos=pos_;
		}
	}
	AmbCube ac;
	i_math::vector3df pos;
	DWORD bDirty:1;
	DWORD bNeedReset:1;
};

class IEnvLight
{
public:
	INTERFACE_REFCOUNT;
	//dlsm代表directional light shadow map
	//根据一个位置,计算在这个位置上,直射光被遮住的百分比,
	//返回值如果为1.0,表示完全被遮住,0.0表示未被遮住
	virtual float GetDlsmMask(i_math::vector3df &pos)=0;

	virtual void ClearDlsmCache()=0;

	//for ambient cube 
	virtual AmbCube *DetectAC(ACDetector *acd)=0;
	virtual AmbCube *GetDefaultAC()=0;

	virtual IProbeCubeMapEditor *GetEditor()=0;

	//测试代码
	virtual BOOL GetSHMap(i_math::vector3df *&pos,i_math::vector3df *&nor,float *& dist,DWORD &nSample,i_math::triangle3df *&tris,DWORD &nTris) = 0;
};

//typedef unsigned __int64 ElNodeHandle;
//#define ElNodeHandle_Null (0)


struct ProbeCubeInfo
{
	i_math::aabbox3df aabb;
	float density;				 // 采样点分布密度
	i_math::recti GetProjRect()
	{
		int x0 = int(aabb.MinEdge.x);
		int y0 = int(aabb.MinEdge.z);
		int x1 = int(aabb.MaxEdge.x+0.99999f);
		int y1 = int(aabb.MaxEdge.z+0.99999f);
		return i_math::recti(x0,y0,x1,y1); 
	}
};	

struct ProbeInfo
{
	BYTE   bBaked;
	short  d;
	int  idx;
	i_math::vector3df pos; 
};

struct BakeResult;
class IProbeCubeMapEditor :public IObjMapEditor
{
public:
	//Center:Probe cube center , radius: = w/2 = h/2
	virtual HMapObj AddProbeCube(i_math::aabbox3df &abb,float density) = 0; 
	virtual BOOL SetInfo(HMapObj &hCube,const ProbeCubeInfo & info) = 0;
	virtual BOOL GetInfo(const HMapObj &hCube,ProbeCubeInfo & info) const = 0;

	// for bake system
	virtual DWORD GetNumberOfSamples(const HMapObj &hCube) = 0;
	virtual const ProbeInfo *GetProbes(const HMapObj & hCube,DWORD * nVtx,const i_math::aabbox3df * abb = NULL,BOOL bSorted = FALSE) = 0;	//pointer will be invalid if any modify occur
	virtual BOOL SetResults(const HMapObj & hCube,BakeResult * pResults,int * pi,i_math::vector3df * pos,DWORD count) =0;
	virtual i_math::vector3df GetSampleDir(int idx) = 0; //idx [0,NUM_SAMPLES_PER_POSITION -1]
};








