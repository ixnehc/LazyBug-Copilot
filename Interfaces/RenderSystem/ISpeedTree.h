
#pragma once

#include "IResource.h"
#include "IVertexBuffer.h"


class ITexture;
struct SptMaps
{
	ITexture *trunk;//树干的贴图
	ITexture *trunkNormal;//树干的法线贴图
	ITexture *composite;//组合贴图
	ITexture *compositeNormal;//组合贴图的法线贴图
	ITexture *shadowmap;//自阴影贴图
};


struct VBHandles;
struct VBBindArg;
struct SptBranch
{
	VBHandles vb;
	VBBindArg arg;

	float fAlphaTest;

};

struct SptFrond
{
	VBHandles vb;
	VBBindArg arg;


	float fAlphaTest;
};

struct SptLeaveCards
{
	BOOL bNeedMerge;//是否需要叶片的混合
	VBHandles vb;
	VBBindArg arg[2];

	float fAlphaTest[2];
	float fRockScale;
	float fRustleScale;

};
struct SptLeaveMeshs
{
	BOOL bNeedMerge;
	VBHandles vb;
	VBBindArg arg[2];
	float fAlphaTest[2];
};
struct SptBillboard
{
	float fFadeOut;
};

struct SptLod
{
	SptBranch branch;
	SptFrond frond;
	SptLeaveCards     leaveCards;
	SptLeaveMeshs     leaveMeshs;       
	SptBillboard billboard;
	BOOL HasLeaf(){	return leaveMeshs.vb.vb||leaveCards.vb.vb;}
	BOOL HasBranch(){	return branch.vb.vb!=NULL;}
	BOOL HasFrond(){	return frond.vb.vb!=NULL;}
	BOOL bPureBB;
	BOOL bHasBB;
	float fLodValue; /*0.0f --- 1.0f*/ 
};

struct SptWndCfg;



enum CollisionObjectType
{
	CO_BOX,
	CO_SPHERE,
	CO_CAPSULE,
};

struct TriSample;
class ISptTriSampleAdapter
{
public:
	// the pointer can't be retain for later using
 	virtual TriSample * BuildBranchTriSamples(DWORD &nSample,int lvl) = 0;
	virtual TriSample * BuildFrondTriSamples(DWORD &nSample,int lvl) = 0;
	virtual TriSample * BuildEnvSamples(DWORD &nSample,DWORD &w) = 0;
	virtual void Release() = 0;
	//call after BuildBranchTriSamples or BuildFrondTriSamples
	virtual i_math::vector3df * GetVtxPos(DWORD & nVtx) = 0;
};

enum SptColType
{
	SPTCT_LEAF = 0x1,
	SPTCT_BRANCH = 0x2,
	SPTCT_FROND = 0x4,
	SPTCT_ALL	= 0x7
};

class IDummies;
class ISpt:public IResource
{
	
public:
	virtual SptMaps &GetMaps()=0;
	//注意:fDist单位为米
	virtual SptLod &GetLodByDistance(float fDist)=0;

	virtual SptLod &GetLod(DWORD iLod) = 0;
	virtual DWORD GetNumberOfLod() = 0;	//number of lod ,when out of lod, the tree is appeared as a billboard.
	

	virtual DWORD GetWindCount()=0;
	virtual SptWndCfg &GetWind(DWORD idx)=0;	
	virtual i_math::aabbox3df & GetBoundingBox() = 0;
	// accurate collision detect.
	virtual int  GetNumberOfCollisionObjects() = 0;
	virtual void * GetCollisionObject(int idx ,CollisionObjectType & coType) = 0;
	virtual BOOL TestCollision(const i_math::line3df & rayHit,i_math::vector3df &pos,float scale,float rotY,i_math::vector3df &outIntersec,SptColType cTpye = SPTCT_ALL) = 0;
	virtual const i_math::spheref &GetLeafBoundSphere() = 0;

	virtual IDummies * GetCollisionDummies() = 0;
	
	//nPixel: 一米的长度上分布多少个象素
	virtual void GetMapSize(i_math::size2di &szBr,i_math::size2di &szFr,int lvl) = 0;
	
	virtual ISptTriSampleAdapter * GetTriSampleAdapter() = 0;

	virtual float GetMapScale(int lvl) const  = 0;

	virtual void GetLeafHookPoint(i_math::vector3df *& pos,float& r,DWORD &count) = 0;

};

class ISptMgr:public IResourceMgr
{
};

struct SptData;
class IDynSptMgr:public IResourceMgr
{
public:
	virtual ISpt *Create(SptData *data,const char * pathRes)=0;
};

