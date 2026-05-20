/********************************************************************
	created:	3:9:2008   14:29
	filename: 	d:\IxEngine\interfaces\PhysicsSystem\IPhysicsSystemDefines.h
	author:		chenxi
	
	purpose:	physics system defines
*********************************************************************/
#pragma once

#include "math/imath_all.h"
#include "math/range.h"
#include "math/line2d.h"
#include "fastdelegate/FastDelegate.h"


struct PhysWorldConfig
{
	PhysWorldConfig()
	{
		halfext=400;
		halfextInner=100;
	}

	int halfext;//整个世界的半径,以米为单位
	int halfextInner;//核心部分(进行physics simulation的区域)的半径,以米为单位
};



enum PhysRigidBodyType
{
	Fixed,
	Dynamic,
	KeyFramed,
};


struct MoppMesh
{
	MoppMesh()
	{
		memset(this,0,sizeof(*this));
	}
	float *vertices;
	DWORD nVertices;
	WORD *indices;
	DWORD nIndice;

	BYTE tpData;//0: Hk code, 1: Px data
	BYTE*data;
	DWORD szData;
};

struct ConvexHullParam
{
	ConvexHullParam()
	{
		memset(this, 0, sizeof(*this));
	}
	i_math::vector3df *vertices;
	DWORD nVertices;
};

struct ConvexHull
{
	ConvexHull()
	{
		Zero();
	}
	void Zero()
	{
		memset(this, 0, sizeof(*this));
	}
	DWORD nMaxVertices;
	i_math::vector3df *vertices;
	DWORD nVertices;
	DWORD nMaxIndices;
	WORD *indices;
	DWORD nIndices;
};

enum PhysCollideLayor
{
	CldLayor_None=0,
	CldLayor_Trrn,
	CldLayor_Ground,
	CldLayor_Brush,
	CldLayor_Model,
	CldLayor_Model_Zonable,
	CldLayor_WalkingBody_Zonable,
	CldLayor_FlyingBody_Zonable,
	CldLayor_Ragdoll,

	//这些collide layor用于hit test
	CldLayor_NotZonable,//这个layor与所有非zonable的layor都有collision
	CldLayor_StaticTest,//与所有静态物体碰撞(包括Trrn,Brush,Model)
	CldLayor_GroundTest,

	CldLayor_Max,
};


enum PhysCharBodyType
{
	PhysCharBody_None,
	PhysCharBody_Walking,
};

struct PhysCharBodyParam
{
	PhysCharBodyParam()
	{
		tp=PhysCharBody_None;
		radius=1.0f;
		height=2.0f;
		bEnableCollide=TRUE;
	}
	void Limit()
	{
		if (tp==PhysCharBody_Walking)
		{
			float hh=height/2.0f;
			radius=radius>hh?hh:radius;
		}
// 		if (tp==PhysCharBody_Flying)
// 			height=radius*2.0f;
	}
	PhysCharBodyType tp;
	BOOL bEnableCollide;//是否要进行精确的碰撞检测
	float radius;
	float height;
};

//CharBody的物理参数
#define HANG_INFINITE (100.0f)
struct PhysCharBodyState
{
	PhysCharBodyState()
	{
		sppt.bValid=FALSE;
		bQuat=FALSE;
		bJumping=FALSE;
		offSpeed=0.0f;
	}
	i_math::vector3df pos;
	i_math::vector3df speed;
	BOOL bQuat;
	i_math::vector3df euler;
	i_math::quatf rot;
	BOOL bJumping;
	float offSpeed;//这个值表示当前速度的方向相对于角色的当前方向的角度

	//Support State
	struct Support
	{
		BOOL bValid;//
		BOOL bSupport;//是否被地表支持
		float hang;//离地面的高度,可能为HANG_INFINITE
		i_math::vector3df nml;
	};
	Support sppt;
};


struct PhysRigidBodyProperties
{
	PhysRigidBodyProperties()
	{
		density = 5.0f;
		friction = 0.5f;
		dampingLinear = 0.0f;
		dampingAngular = 0.05f;
	}
	float density;
	float friction;
	float dampingLinear;
	float dampingAngular;
};



typedef fastdelegate::FastDelegate3<i_math::line3df &,i_math::vector3df &,i_math::vector3df *,BOOL> TerrainRayCollideHandler;


struct PhysTerrainParam
{
	PhysTerrainParam()
	{
		handlerRayCollide=NULL;
		halfextInner=0.0f;
	}
	TerrainRayCollideHandler handlerRayCollide;
	i_math::recti rc;//terrain的范围,单位为米,世界坐标,
	float halfextInner;//核心部分(进行physics simulation的区域)的半径,以米为单位,如果为0,使用缺省值
};


typedef void * BaffleHandle;


struct RagdollSwitchArg
{
	RagdollSwitchArg()
	{
		bVel=FALSE;
	}
	BOOL bVel;
	i_math::vector3df vel;
};
