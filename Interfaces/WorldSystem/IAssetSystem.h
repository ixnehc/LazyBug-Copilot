/********************************************************************
	created:	2008/1/16   12:33
	filename: 	e:\IxEngine\Interfaces\WorldSystem\IAssetSystem.h
	author:		cxi
	
	purpose:	exposed asset system interfaces for both the asset system user and the asset package
*********************************************************************/

#pragma once
#include "IAssetSystemDefines.h"

//IMPORTANT: the exposed interfaces functions could be devided into 2 categories:
//1. for asset system user
//2. for the asset package.You could only access these functions only when you are developping
//an asset package


class IAssetSystem;
class IAsset;
class IAssetLinker;
class IAssetClassLib;
class IAssetMap;
class IAssetRenderer;
class IAssetLocator;
class IAssetPhysResolver;

struct AssetSystemState;

class CDataPacket;
class CProgress;
struct Envelope;
struct CtrlOp;
class IAssetSystem
{
public:
	INTERFACE_REFCOUNT;
	virtual BOOL ResetConfig()=0;
	virtual AssetSystemState *GetSS()=0;
	//Note: the GetXXX functions will NOT increase refcount for the returned interface
	virtual IWorldSystem *GetWS()=0;
	virtual IAssetPackage *GetAPs()=0;
	virtual IMapFile *GetMapFile()=0;
	virtual IAssetRenderer *GetRenderer()=0;
	virtual IAssetEventer *GetEventer()=0;
	virtual IAssetShell *GetShell()=0;
	virtual IAssetBodyMap*GetBodyMap()=0;
	virtual IClient *GetClient()=0;

	virtual void SwitchEditMode(BOOL bEnable)=0;

	virtual void Pause()=0;
	virtual void Resume(DWORD nFrames)=0;

	virtual void Update(float fDelta,CtrlOp *ops,DWORD nOps)=0;//fDelta is in second
	virtual BOOL Locate(i_math::vector3df &center,CProgress *prg=NULL)=0;//center is a 3D position of the center of the 
																					//asset system,in meter
	virtual BOOL SaveToMap()=0;
	virtual BOOL ReloadMap(i_math::pos2di *blks,DWORD nBlks)=0;
	virtual BOOL ReloadAllMap()=0;
	virtual void UnLoadMap()=0;
	virtual void LoadMap(CProgress *prg)=0;

	virtual i_math::vector3df &GetCenter()=0;//get the center
	virtual void GarbageCollect()=0;	//for calling in editor mode,in running mode
															//the garbage collecting routine will be called automatically
															//by the asset system itself.

	//根据ray找到第一个相交的asset,注意这个函数不是非常快
	virtual IAsset *HitTest(i_math::line3df &ray)=0;
	virtual IAsset **VolumeHitTest(i_math::volumeCvxf &vol,DWORD &c)=0;
	virtual void CollectEnvelope(IAsset *ast,Envelope &evlp)=0;//得到一个asset的envelope

};

class ISceneBaker;
class CClass;
struct GStubBase;
struct GStubConn;
class CAssetCtrl;
struct AssetCreateArg;
struct AstEvent;
class IAnimNode;
class IAsset
{
public:
	INTERFACE_REFCOUNT;

	//General
	virtual CClass *GetClass()=0;
	virtual const char *GetClassName()=0;
	virtual AssetFlag GetFlag()=0;
	virtual const char *GetCategory()=0;
	virtual const char *GetShowName()=0;

	virtual AssetSystemState *GetSS()=0;

	virtual BOOL IsAlive()=0;
	virtual void SetBit(AssetBit bit)=0;
	virtual void ClearBit(AssetBit bit)=0;
	virtual void ClearAllBits()=0;
	virtual BOOL TestBit(AssetBit bit)=0;
	virtual BOOL TestAllBit(AssetBit bit)=0;

	virtual BOOL Create(AssetCreateArg &arg,AssetSystemState *ss)=0;
	virtual void Destroy()=0;
	virtual void DeferredDestroy()=0;//延后删除

	virtual BOOL Save(CDataPacket &dp)=0;
	virtual BOOL Load(CDataPacket &dp)=0;

	virtual BOOL GetAABB(i_math::aabbox3df &aabb)=0;
	virtual BOOL GetLocalAABB(i_math::aabbox3df &aabb)=0;//the aabb in the asset's local space

	virtual BOOL SendEvent(AstEvent &e)=0;//return whether handled


	//XForm Component
	virtual BOOL GetPos(i_math::vector3df &pos)=0;
	virtual BOOL GetXForm(i_math::matrix43f&mat)=0;
	virtual BOOL SetPos(i_math::vector3df &pos)=0;
	virtual BOOL SetXForm(i_math::matrix43f&mat,i_math::matrix43f *matLocal)=0;
	virtual BOOL GetBaseXform(AnimTick t,i_math::matrix43f &mat)=0;//如果这个asset的xform是绑在另一个xform上的话,返回那个xform


	//for baking
	virtual BOOL BakeCollect(ISceneBaker *baker,i_math::recti &rcCore,i_math::recti &rcBlk,float blocklen,BOOL bBakeTarget)=0;
	virtual BOOL BakeDispatch(ISceneBaker *baker,i_math::recti &rcCore,float blocklen)=0;
	
	virtual BOOL CollectPatches(i_math::recti rcBlk,float blocklen,
		std::vector<i_math::vector3df> &vertices,std::vector<WORD> &indices,DWORD &flags) = 0;

	//Stub component,
	virtual GStubBase *FindStub(const char *name)=0;
	virtual DWORD GetStubCount()=0;
	virtual GStubBase *GetStub(DWORD idx)=0;
	virtual void* GetStubOwner()=0;
	virtual GStubConn *FindConn(const char *name)=0;

	//tree link
	virtual BOOL SupportTreeLink()=0;
	virtual IAsset *GetParent()=0;
	virtual DWORD GetChildCount()=0;
	virtual IAsset *GetChild(DWORD idx)=0;
	virtual IAsset **GetChilds(DWORD &n)=0;
	virtual BOOL SetParent(IAsset *ast)=0;
	virtual BOOL AddChild(IAsset *ast)=0;
	virtual BOOL RemoveChild(IAsset *ast)=0;
	virtual BOOL ClearChilds()=0;
	virtual BOOL CheckDescendent(IAsset *ast)=0;//check whether ast is descendent of this asset

	//for AssetCtrl
	virtual CAssetCtrl *GetCtrl()=0;
	virtual void ClearCtrl()=0;

	//help信息
	virtual const char *GetHelp()=0;

	virtual const char *GetDebugLocation()=0;

	//UID
	virtual BOOL SupportUID()=0;
	virtual void SetUID(AssetUID uid)=0;
	virtual AssetUID GetUID()=0;


	//For test
	virtual void Test()=0;

};

struct GObjBase;



struct CtrlOp;
class CMouseCursor;


