/********************************************************************
	created:	1:3:2009   14:04
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	defines&interfaces for Asset Body Map
*********************************************************************/
#pragma once

#include "PhysicsSystem/IPhysicsSystemDefines.h"



typedef DWORD BodyID;
#define BodyID_Null NULL

#define TRIFLAG_WALKABLE 1//可走
#define TRIFLAG_TRRN 2
#define TRIFLAG_ABYSS 4//深渊
#define TRIFLAG_SPRITE 8//小块的不可走的区域(不会在outline map上标出)
#define TRIFLAG_FLYABLE 16//可容飞行物体飞过
#define TRIFLAG_WALKABLEADV 32 //高级可走,需要特殊的权限可走
#define TRIFLAG_SWITCHABLE 64 //可切换Walkable/Unwalkable
#define TRIFLAG_FORCEWALKABLE 128 //强制可走


typedef DWORD BodyFlags;
#define BodyFlag_Default (1)
#define BodyFlag_Ground (2)
#define BodyFlag_Moving (4)


struct MoppMesh;


enum BodyUpdateType
{
	BodyUpdate_Static,
	BodyUpdate_Dynamic,
	BodyUpdate_Keyframed,
};

struct CustomBodyRayCast
{
	virtual BOOL CastRay(i_math::line3df &line,float radiusLine,float &distSQ)=0;
};

class IDummies;
class IMopp;
struct BodyShape
{
	enum Type
	{
		None,
		Dummies,
		Mopp,
		Custom,
	};
	BodyShape()
	{
		tp=None;
		dummies_=NULL;
		mopp_=NULL;
		raycastCustom=NULL;
	}

	void SetDummies(IDummies *p)
	{
		tp=Dummies;
		dummies_=p;
	}
	void SetMopp(IMopp *p)
	{
		tp=Mopp;
		mopp_=p;
	}
	void SetCustomRayCast(CustomBodyRayCast *raycast)
	{
		tp=Custom;
		raycastCustom=raycast;
	}

	Type tp;
	IDummies *dummies_;
	IMopp *mopp_;
	CustomBodyRayCast *raycastCustom;


};

class IDummies;
class IAnimNode;
struct BodyParam
{
// 	BodyParam()
// 	{
// 		flag=BodyFlag_None;
// 		dummies=NULL;
// 
// 		dummiesPick=NULL;
// 
// 		anBase=NULL;
// 		matBase=NULL;
// 	}
// 	BodyFlag flag;
// 
// 	//物理模拟用的形状
// 	IDummies *dummies;
// 	MoppMesh*mopp;
// 	//Pick用的形状
// 	IDummies *dummiesPick;
// 
// 	//位置
// 	IAnimNode *anBase;
// 	i_math::matrix43f *matBase;
};

class ITrrnMap;
struct TrrnBodyParam
{
	TrrnBodyParam()
	{
		trrnmap=NULL;
		id=0;
	};
	i_math::recti rc;//地表的总大小,(注意是整张地图的大小,不是当前载入的大小),单位为米
	ITrrnMap *trrnmap;

	BodyID id;
};







class ITrrnBody
{
public:
	INTERFACE_REFCOUNT;
};


struct DummyInfo;
struct BrushBodyParam
{
	BrushBodyParam()
	{
		matBase=NULL;
		id=0;
		flagTri=0;
	}

	BodyShape shape;

	i_math::matrix43f *matBase;

	BodyID id;
	BYTE flagTri;//TRIFLAG_XXXX value

};

class IBrushBody
{
public:
//	INTERFACE_REFCOUNT;

	virtual void Destroy()=0;

};

struct DummyInfo;
struct RigidBodyParam
{
	RigidBodyParam()
	{
		typeUpdate=BodyUpdate_Static;
		bZonable=TRUE;
		bSimulate=TRUE;

		matBase=NULL;
		anBase=NULL;

		prop=NULL;
	}
	BodyUpdateType typeUpdate;
	BOOL bZonable;
	BOOL bSimulate;//注意,如果typeUpdate为BodyUpdate_Dynamic,bSimulate必须为TRUE

	BodyShape shape;
	PhysRigidBodyProperties*prop;

	i_math::matrix43f *matBase;//当typeUpdate为Static或者Dynamic时使用
	IAnimNode *anBase;//当typeUpdate为Keyframed时使用

};


class IDtrPieces;
struct DtrBodyParam
{
	DtrBodyParam()
	{
		pieces=NULL;
		matBase=NULL;
		velImpactThreshold=-1.0f;
		posImpulse=NULL;
		velImpulse=NULL;
		bCenteredImpulse=FALSE;
		prop=NULL;
		spdAugluarImpulse=0.0f;
	}
	IDtrPieces *pieces;
	PhysRigidBodyProperties*prop;

	i_math::matrix43f *matBase;
	float velImpactThreshold;//速度多大的impact会被检测到,如果小于0,表示不需要侦测impact

	//初始的Impulse,参数都是世界空间的值
	BOOL bCenteredImpulse;
	i_math::vector3df *posImpulse;
	i_math::vector3df *velImpulse;
	float spdAugluarImpulse;
};

struct DtrBodyParam_NoSimulate
{
	DtrBodyParam_NoSimulate()
	{
		pieces=NULL;
		anBase=NULL;
	}
	IDtrPieces *pieces;
	IAnimNode *anBase;
};

class IRigidBody
{
public:
//	INTERFACE_REFCOUNT;
	virtual BodyID GetID()=0;

	virtual void Destroy()=0;
	virtual IAnimNode *GetBase()=0;

};

class IDtrBody
{
public:
	virtual BodyID GetID()=0;

	virtual DWORD GetPieceCount()=0;
	virtual BOOL GetPieceMat(DWORD iPiece,i_math::matrix43f &mat)=0;
	virtual BOOL FetchImpact(DWORD iPiece)=0;

	virtual BOOL IsStable()=0;

	virtual void ApplyVelocity(DWORD iPiece,IDtrBody *body)=0;//根据这个dtr body的iPiece个piece,来设置body里面所有pieces的速度

	virtual void DisablePiece(DWORD iPiece)=0;
	virtual void Deactivate()=0;
	virtual BOOL IsDeactivated()=0;

	virtual void Destroy()=0;
	virtual IAnimNode *GetBase()=0;
};


class IAnimNodeSkeleton;
struct CharBodyParam
{
	CharBodyParam()
	{
		dummies=NULL;
		anBase=NULL;
	}
	IAnimNode *anBase;
	IDummies *dummies;
	 
};


class ICharBody
{
public:
//	INTERFACE_REFCOUNT;
	virtual BodyID GetID()=0;

	virtual void Destroy()=0;
	virtual IAnimNodeSkeleton *GetSkl()=0;
	virtual BOOL SetBaseLink(IDummies *dummies,const char *name)=0;
	virtual BOOL Bind(IAnimNode *an)=0;
};

struct CustomBodyParam
{
	CustomBodyParam()
	{
		anBase=NULL;
		raycast=NULL;
	}
	IAnimNode *anBase;
	CustomBodyRayCast *raycast;
};

class ICustomBody
{
public:
	virtual BodyID GetID()=0;
	virtual void Destroy()=0;
};

struct SpacialTester;
struct PhantomParam
{
	PhantomParam()
	{
		tester=NULL;
		anBase=NULL;
	}
	SpacialTester *tester;
	IAnimNode *anBase;
};

struct PhantomEvent
{
	BodyID id;
	BOOL bEntering;// or leaving
};

class IPhantom
{
public:
//	INTERFACE_REFCOUNT;

	virtual void Destroy()=0;

	virtual PhantomEvent *FetchEvents(DWORD &c)=0;//返回临时指针
};

struct BodyRes
{
	BodyRes()
	{
		tp=None;
		path="";
	}
	enum Type
	{
		None,
		Dummies,
		Mopp,
	};
	Type tp;
	const char *path;
	i_math::matrix43f *mat;
};




class IAssetBodyMap
{
public:
	virtual BodyID GenBodyID()=0;

	virtual ITrrnBody* CreateTrrnBody(TrrnBodyParam &param)=0;
	virtual IBrushBody* CreateBrushBody(BrushBodyParam &param)=0;
	virtual IRigidBody* CreateRigidBody(RigidBodyParam &param)=0;
	virtual IDtrBody* CreateDtrBody(DtrBodyParam &param)=0;
	virtual IDtrBody* CreateDtrBody(DtrBodyParam_NoSimulate &param)=0;
	virtual ICharBody* CreateCharBody(CharBodyParam &param)=0;
	virtual ICustomBody* CreateCustomBody(CustomBodyParam &param)=0;
	virtual IPhantom *CreatePhantom(PhantomParam &param)=0;

	//如果三角形的有flagsAny里的任何一个标志,则返回这个三角形
	//如果flagsAny为0,返回所有三角形
	//flags 里返回TRIFLAG_XXXX
	virtual BOOL CollectTris(i_math::aabbox3df &aabb,i_math::triangle3df *&tris,BYTE *&flags,DWORD &count,BYTE flagsAny)=0;
	virtual BOOL CollectAllTris(i_math::aabbox3df &aabb,i_math::triangle3df *&tris,BYTE *&flags,DWORD &count)=0;

	//枚举在aabb范围内的所有zonable bodies
	virtual BodyID *EnumBodies(i_math::aabbox3df &aabb,DWORD &c)=0;

	//try to hit a zonable body,如果hit不到,返回BodyID_Null
	//radius表示line的粗细,目前只针对使用Dummies作为shape的body有效
	virtual BodyID BodyHitTest(i_math::line3df &line,float radius,i_math::vector3df *posHit,BOOL bIgnoreStatic,BodyFlags flags=BodyFlag_Ground|BodyFlag_Default)=0;
	virtual BOOL HitTest(i_math::line3df &line,float radius,BodyID &bodyHit,i_math::vector3df &posHit)=0;

	virtual BOOL GetHeight(float x,float z,float &h,BYTE *flagTri=NULL)=0;
	virtual BOOL GetGroundHeight(float x,float z,float &h)=0;
	virtual BOOL SphereHitTest(i_math::spheref &sph,i_math::vector3df &vTarget,i_math::vector3df &posHit,
					PhysCollideLayor layor=CldLayor_WalkingBody_Zonable)=0;
	virtual BOOL TrrnHitTest(i_math::line3df &line,i_math::vector3df &posHit)=0;
	virtual BOOL StaticHitTest(i_math::line3df &line,i_math::vector3df &posHit,i_math::vector3df *normalHit=NULL)=0;//只针对静态的物体进行碰撞检测
	virtual BOOL GroundHitTest(i_math::line3df &line,i_math::vector3df &posHit,i_math::vector3df *normalHit=NULL)=0;
	virtual BOOL WalkableHitTest(i_math::line3df &line,i_math::vector3df &posHit,i_math::vector3df *normalHit=NULL)=0;

	virtual void BatchGetGroundHeight_Begin(i_math::aabbox3df &aabb)=0;
	virtual BOOL BatchGetGroundHeight_Get(float x,float z,float &h)=0;
	virtual void BatchGetGroundHeight_End()=0;

	virtual BodyRes *EnumBodyRes(i_math::aabbox3df &aabb,DWORD &count)=0;

	virtual BOOL EnableBody(BodyID bodyid,BOOL bEnable)=0;
	virtual BOOL GetBodyEnabled(BodyID bodyid,BOOL &bEnable)=0;
	virtual BOOL SetBodyFlags(BodyID bodyid,BodyFlags flags)=0;
	virtual BOOL GetBodyFlags(BodyID bodyid,BodyFlags &flags)=0;

};
