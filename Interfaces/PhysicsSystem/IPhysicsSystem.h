/********************************************************************
	created:	3:9:2008   13:50
	filename: 	d:\IxEngine\interfaces\PhysicsSystem\IPhysicsSystem.h
	author:		chenxi
	
	purpose:	exposed physics system interfaces for the physics system user
*********************************************************************/
#pragma once
#include "IPhysicsSystemDefines.h"

struct ProfilerMgr;
struct LogHandler;

struct MoppMesh;

class IRenderSystem;
class IPhysWorld;
class IPhysRigidBody;
class IPhysRagdoll;
class IPhysicsSystem
{
public:
	virtual BOOL Init()=0;
	virtual BOOL UnInit()=0;

	virtual BOOL BeginThread()=0;
	virtual BOOL EndThread()=0;

	virtual BOOL BeginDebug() = 0;
	virtual BOOL EndDebug() = 0;

	virtual ProfilerMgr *GetProfilerMgr()=0;
	virtual void RegisterLogHandler(LogHandler &handler)=0;

	virtual IPhysWorld*CreateWorld(PhysWorldConfig &cfg)=0;

	virtual BOOL BuildMoppCode(MoppMesh&data)=0;
	virtual BOOL BuildConvexHull(ConvexHullParam &param,ConvexHull &result)=0;//result里返回的指针为临时指针

};

class IPhysRagdolls;
class IPhysTerrain;
struct ProfilerMgr;
class IPhysWorld
{
public:
	INTERFACE_REFCOUNT;

	virtual IPhysTerrain*CreateTerrain(PhysTerrainParam &param)=0;
	virtual IPhysRigidBody *CreateRigidBody()=0;

	virtual IPhysRagdolls* GetRagdolls()=0;

	//给定一个aabb,在物理世界里寻找一个可以落脚的地方(使这个aabb不与任何物体重叠)
	virtual BOOL FindFootPos(i_math::aabbox3df &aabb,i_math::vector3df &foot)=0;
	virtual void GroundHitTest(i_math::vector3df *pos,DWORD nPos,DWORD stride,i_math::rangef &rVer,BOOL *rets,i_math::vector3df *normal)=0;
	virtual BOOL RayHitTest(i_math::line3df &ray,PhysCollideLayor layor,i_math::vector3df &hitpos,i_math::vector3df *normal)=0;
	//目前参数layor只支持CldLayor_WalkingBody_Zonable和CldLayor_FlyingBody_Zonable
	virtual BOOL SphereHitTest(i_math::spheref &sph,i_math::vector3df &vTarget,PhysCollideLayor layor,i_math::vector3df &hitpos,i_math::vector3df *normal)=0;

	virtual BaffleHandle AddBaffle(i_math::line2df &line,BOOL bDoubleSided)=0;
	virtual void RemoveBaffle(BaffleHandle bfl)=0;

	virtual void BeginDebug()=0;
	virtual void EndDebug()=0;

	virtual void Locate(i_math::vector3df &center)=0;
	virtual void SetStep(float step)=0;
	virtual float GetStep()=0;
	virtual void Step()=0;


};

class IPhysTerrain
{
public:
	INTERFACE_REFCOUNT;

	//注意,同一个时刻只能有一个terrain attach到world里面,所以,AttachToWorld()会把原来attach的terrain
	//detach掉.
	virtual BOOL AttachToWorld()=0;
	virtual void DetachFromWorld()=0;
};

class IPhysRigidBody
{
public:
	INTERFACE_REFCOUNT;

	virtual void BeginBuild()=0;
	virtual void EndBuild()=0;
	virtual void Build_SetType(PhysRigidBodyType type)=0;
	virtual void Build_SetCollideLayor(PhysCollideLayor layor)=0;
	virtual void Build_SetXform(i_math::xformf&xfm)=0;//注意xfm里不能包含scale变换
	virtual BOOL Build_AddSphere(i_math::spheref &sph)=0;
	virtual BOOL Build_AddCapsule(i_math::capsulef&cap)=0;
	virtual BOOL Build_AddAABB(i_math::vector3df&halfext,i_math::matrix43f *mat=NULL)=0;//注意mat里不能包含scale变换
	virtual BOOL Build_AddMesh(MoppMesh *mopp,float scale)=0;
	virtual void Build_SetProp(PhysRigidBodyProperties &prop) = 0;

	virtual BOOL EnableDetectImpact(float velImpact)=0;//速度超过velImpact的撞击会被记录下来
	virtual BOOL FetchImpact()=0;

	virtual BOOL AddToWorld(BOOL bAlwaysSimulate=FALSE)=0;
	virtual void RemoveFromWorld()=0;

	virtual BOOL CalcPointVelocity(i_math::vector3df &pos,i_math::vector3df &vel)=0;//pos为世界坐标系的值

	virtual void SetXform(i_math::xformf &xfm)=0; //这个函数只对Keyframed的rigidbody有小,注意xfm里不能包含scale
	virtual void GetXform(i_math::xformf &xfm)=0; 

	virtual void GetPos(i_math::vector3df &pos)=0; 

	virtual void SetLinearVel(i_math::vector3df &vel)=0;
	virtual void SetAngularVel(i_math::vector3df &vel)=0;
	virtual void SetImpulse(i_math::vector3df &pos,i_math::vector3df &vel)=0;

};

class SkeletonInfo;
class IPhysRagdoll
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL Reset(i_math::xformf *xfms,i_math::xformf *xfms2,DWORD c,i_math::matrix43f &matBase,RagdollSwitchArg &arg,float dt)=0;
	virtual BOOL GetWorldXforms(i_math::xformf *xfms,DWORD c)=0;

	virtual void AddToWorld()=0;
	virtual void RemoveFromWorld()=0;

};

struct PhysRagdollData
{
	PhysRagdollData()
	{
		data=NULL;
		szData=0;
		infoSkl=NULL;
	}
	BYTE *data;
	DWORD szData;
	SkeletonInfo *infoSkl;
};


class IPhysRagdolls
{
public:
	virtual IPhysRagdoll *Obtain(const char *nm)=0;//返回的指针带一个引用计数
	virtual BOOL Register(const char *nm,PhysRagdollData &data)=0;
};