
#pragma once

#include "../IAssetSystem.h"

#include "assetcomponents.h"

#include "math/matrix43.h"

#define ClassF_Asset 1

class CAsset;
inline void asset_verify(CAsset *c) {}

typedef WORD AssetClassUID;

#define DECLARE_ASSET_CLASS(clss,uid)														\
	_DECLARE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
		instance._flag|=ClassF_Asset;																	\
		instance._uid=uid;																						\
		{clss *p=NULL;asset_verify(p);}																	\
	_DECLARE_CLASS_END(clss)																			\
	typedef clss ThisType;

#define IMPLEMENT_ASSET_CLASS(clss) IMPLEMENT_CLASS(clss)

#define DECLARE_ASSET_DESC() virtual BOOL GetDesc(const char *nameStb,char *buf,DWORD szBuf);
#define BEGIN_ASSET_DESC(clss,s_)																	\
BOOL clss::GetDesc(const char *nameStb,char *buf_,DWORD szBuf_)			\
{																															\
	std::string s_;																									\
	std::string &rs_=s_;																							\
	__super::GetDesc(nameStb,buf_,szBuf_);														\
	s_=buf_;

#define END_ASSET_DESC()																				\
strcpy(buf_,rs_.c_str());																						\
return TRUE;																										\
}

#define ASSET_STUB_DESC(nmStb) if (strcmp(nameStb,#nmStb)==0)

#define ASSET_DESC() if (strcmp(nameStb,"")==0)


#define BEGIN_ASSET_HELP() virtual const char *GetHelp() { return 
#define END_ASSET_HELP() ;}

#define DEFINE_ASSET_NAME(cat,nm)														\
virtual const char *GetCategory()	{		return cat;	};									\
virtual const char *GetShowName()	{		return nm;	}






//////////////////////////////////////////////////////////////////////////
//Asset Validate macro

#define VALIDATE_ASSET(ast)													\
	if (!((CAsset*)ast)->IsAlive())													\
		SAFE_RELEASE(ast);

#define VALIDATE_ASSET_DEQUE(q)										\
{																									\
	std::deque<IAsset*>::iterator it=(q).begin();						\
	while(it!=(q).end())																	\
	{																								\
		if (!((CAsset*)(*it))->IsAlive())											\
		{																							\
			it=(q).erase(it);																\
			continue;																			\
		}																							\
		it++;																					\
	}																								\
}

//


typedef BOOL(CAsset::*HookFunc)(AstEvent &);

//Macro for register/unregister hook handler
#define _RegisterHook(eclass,func,prior)												\
{																												\
	fastdelegate::FastDelegate1<eclass&,BOOL> dlgt;						\
	dlgt.bind(this,&ThisType::func);														\
	_ss->eventer->RegisterHook(ID_##eclass,this,								\
														(HookHandler&)dlgt,prior);			\
}

#define _RegisterHook2(eclass,subid,func,prior)									\
{																												\
	fastdelegate::FastDelegate1<eclass&,BOOL> dlgt;						\
	dlgt.bind(this,&ThisType::func);																			\
	_ss->eventer->RegisterHook(ID_##eclass,subid,this,						\
														(HookHandler&)dlgt,prior);			\
}

#define _UnRegisterHook(eclass,func)													\
{																												\
	fastdelegate::FastDelegate1<eclass&,BOOL> dlgt;						\
	dlgt.bind(this,&ThisType::func);														\
	_ss->eventer->UnRegisterHook(ID_##eclass,this,							\
																(HookHandler&)dlgt);			\
}


class CDataPacket;
class CClass;
struct AssetZoneInfo;
class CRenderPass;
class CRagent;
struct Ratom;
struct Renv;
class CStubCore;
struct AstEventEntry;
typedef DWORD ClassID;
class CAsset:public IAsset
{
public:
	IMPLEMENT_REFCOUNT_OVERRIDE;
	void OnRelease();
	CAsset()
	{
		_ss=NULL;
		_bits=0;
	}
	virtual ~CAsset(){}
	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()	{		return NULL;	}
	virtual const char *GetCategory()	{		return "<未分类>";	}
	virtual const char *GetShowName()	{		return "";	}

	void SetSS(AssetSystemState *ss)	{		_ss=ss;	}
	AssetSystemState *GetSS()	{		return _ss;}
	void LuaDebugOutput(const char *type,const char *content,...);


	//interfaces
	virtual const char *GetClassName();
	virtual AssetFlag GetFlag()	{		return 0;	}
	virtual BOOL IsAlive()	{		return TestBit(AssetBit_Alive);	}
	virtual void SetBit(AssetBit bit)	{		_bits|=bit;	}
	virtual void ClearBit(AssetBit bit)	{		_bits&=~bit;		}
	virtual void ClearAllBits()	{		_bits=0;	}
	virtual BOOL TestBit(AssetBit bit)	{		return ((_bits&bit)!=0);	}
	virtual BOOL TestAllBit(AssetBit bit) {		return ((_bits&bit)==bit);	}
	virtual BOOL Create(AssetCreateArg &arg,AssetSystemState *ss);
	virtual void Destroy();//注意:这个Destroy()是给外部Asset的拥有者调的,如果Asset内部要删除自己,调用_DestroyMe()
	virtual void DeferredDestroy();
	virtual BOOL Save(CDataPacket &dp);
	virtual BOOL Load(CDataPacket &dp);

	virtual BOOL GetAABB(i_math::aabbox3df &aabb)	{		return FALSE;	}
	virtual BOOL GetLocalAABB(i_math::aabbox3df &aabb)	{		return FALSE;	}
	virtual BOOL GetSphere(i_math::spheref&sph)	{		return FALSE;	}

	//For Giant Asset
	virtual i_math::pos2di*GetGiantRef(DWORD &nBlk)	{		return NULL;	}


	//XForm Component
	virtual BOOL GetPos(i_math::vector3df &pos)	{		return FALSE;	}
	virtual BOOL GetXForm(i_math::matrix43f&mat)	{		return FALSE;	}
	virtual BOOL SetPos(i_math::vector3df &pos){		return FALSE;	}
	virtual BOOL SetXForm(i_math::matrix43f&mat,i_math::matrix43f *matLink){		return FALSE;	}
	virtual BOOL GetBaseXform(AnimTick t,i_math::matrix43f &mat)	{		return FALSE;	}

	//Stub component,
	virtual void DisableStub()	{		}
	virtual GStubBase *FindStub(const char *name)	{		return NULL;	}
	virtual DWORD GetStubCount()	{		return 0;	}
	virtual GStubBase *GetStub(DWORD idx)	{		return NULL;	}
	virtual void* GetStubOwner()	{		return this;	}
	virtual GStubConn *FindConn(const char *name)	{		return NULL;	}
	virtual int FindConnIdx(const char *name)	{		return -1;	}
	virtual GStubConn *GetConn(int idx)	{		return NULL;	}

	//event handling
	virtual BOOL SendEvent(AstEvent &e)	{		return FALSE;	}

	//tree link
	virtual BOOL SupportTreeLink()	{		return FALSE;	}
	virtual IAsset *GetParent()	{		return NULL;	}
	virtual DWORD GetChildCount()	{		return 0;	}
	virtual IAsset *GetChild(DWORD idx)	{		return NULL;	}
	virtual IAsset **GetChilds(DWORD &n)	{		n=0;		return NULL;	}
	virtual BOOL SetParent(IAsset *ast)	{		return FALSE;	}
	virtual BOOL AddChild(IAsset *ast)	{		return FALSE;	}
	virtual BOOL RemoveChild(IAsset *ast)	{		return FALSE;	}
	virtual BOOL ClearChilds()	{		return FALSE;	}
	virtual BOOL CheckDescendent(IAsset *ast){return FALSE;}

	//for AssetCtrl
	virtual CAssetCtrl *GetCtrl()	{		return NULL;	}
	virtual void ClearCtrl()	{		_ClearCtrl();	}

	//帮助信息(Help为一段固定的帮助信息,Desc为根据Asset的属性设置决定的描述字符串)
	virtual const char *GetHelp()	{		return "";	}
	//如果nameStb为"",表示要求得到整个asset的desc
	virtual BOOL GetDesc(const char *nameStb,char *buf,DWORD szBuf)	{		buf[0]=0;return FALSE;	}

	virtual const char *GetDebugLocation();

	//UID
	virtual BOOL SupportUID()	{		return FALSE;	}
	virtual void SetUID(DWORD uid)	{	}
	virtual DWORD GetUID()	{		return 0;	}


	//Rendering


	//For baking,rc is in block
	virtual BOOL BakeCollect(ISceneBaker *baker,i_math::recti &rcCore,i_math::recti &rc,float blocklen,BOOL bBakeTarget)	{		return FALSE;	}
	virtual BOOL BakeDispatch(ISceneBaker *baker,i_math::recti &rcCore,float blocklen)	{		return FALSE;	}

	virtual BOOL CollectPatches(i_math::recti rcBlk,float blocklen,std::vector<i_math::vector3df> &vertices,
									std::vector<WORD> &indices,DWORD &flags)
	{
		return FALSE;
	}

	//Overriding
	virtual BOOL OnCreate()		{return TRUE;		}
	virtual void OnPostCreate()	{	}//注意:OnPostCreate()会在asset的property设定,以及asset的Load()函数后调用
	virtual BOOL OnDestroy()		{return TRUE;		}
	virtual void OnClock()	{	}



	//For test
	virtual void Test();

protected:


	BOOL _Create(IAsset *parent);
	void _Destroy();


	//Helper functions
	BOOL _IsPostCreated()	{		return TestBit(AssetBit_PostCreated);	}
	CRagent *_GetRagent(int rat);
	void _DestroyMe()	{	_Destroy();	}
	

	//For components
	void _CreateComponent(IAsset *parent)
	{
		_CreateUID();
		_CreateCtrl(parent);
	}
	void _ClearComonent()
	{
		_ClearUID();
		_DestroyCtrl();
	}
	void _SaveComponent(CDataPacket &dp)
	{
		_SaveUID(dp);
	}
	void _LoadComponent(CDataPacket &dp)
	{
		_LoadUID(dp);
	}


	//component override

	virtual BOOL _CreateUID()	{return FALSE;}
	virtual void _ClearUID()	{}
	virtual void _SaveUID(CDataPacket &dp)	{	}
	virtual void _LoadUID(CDataPacket &dp){	}


	virtual BOOL _CreateCtrl(IAsset *parent)	{	return FALSE;}
	virtual void _DestroyCtrl()	{	}
	virtual void _ClearCtrl()	{	}

	virtual BOOL _HandleEvent(AstEvent &e)	{		return FALSE;	}

protected:
	AssetSystemState *_ss;
	AssetBit _bits;
private:

friend class CAssetLinker;
friend class CAssetEventer;
friend class CAssetSystem;
};




