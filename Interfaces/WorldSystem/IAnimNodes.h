/********************************************************************
	created:	27:9:2008   10:12
	filename: 	d:\IxEngine\Interfaces\WorldSystem\IAnimNodes.h
	author:		chenxi
	
	purpose:	anim nodes
*********************************************************************/
#pragma once

#include "anim/animbase.h"

class IAnimTreeCtrl;
class IAnim;
class ISkeleton;
class IMatrice43;
class IDummies;
class IRagdollCtrl;

class IAnimNodeMat;
class IAnimNodeDtr;
class IAnimNodeMatFixed;
class IAnimNodeMatFixedExt;
class IAnimNodeMatOffset;
class IAnimNodeSkeleton;
class IAnimNodeMatrice43;
class IAnimNodeSkin;
class IAnimNodeSkinSite;
class IAnimNodeRagdoll;

class IAnimNodePosEuler;
class IAnimNodeEulerOverride;

class IAnimNodeUV;
class IAnimNodePath;
class IAnimNodeProxy;
class IAnimNodeSite;
class IAnimNodeNoRoll;

class IAnimNodeColorFixed;
class IAnimNodeColorSite;

class IAnimNodeSklLinks;
class IAnimNodeCamera;

class IAnimNodePathNav;

class IAnimNodeAnimTreeCtrl;

class IAnimNodeSwitcher;

class ITexture;
class ICamera;

class IAsset;


class CClass;

class CLocalTime
{
public:
	CLocalTime()
	{
		memset(this,0,sizeof(*this));
	}
	void Reset(AnimTick tAbs)
	{
		_t=_tLast=0;
		_tAbs=_tAbsLast=tAbs;
	}
	void Update(AnimTick tAbs)
	{
		if (tAbs==_tAbs)
			return;//没有增量
		AnimTick dt=tAbs-_tAbs;
		_tAbsLast=_tAbs;
		_tAbs=tAbs;

		_tLast=_t;
		_t+=(AnimTick)(((float)dt)*_rate);
	}
	AnimTick GetT()	{		return _t;	}
	AnimTick GetLastT()	{		return _tLast;	}
	AnimTick ToLocal(AnimTick tAbs)
	{
		if (_tAbs==_tAbsLast)
			return _t;
		if (tAbs<_tAbsLast)
			tAbs=_tAbsLast;
		return _tLast+(_t-_tLast)*(tAbs-_tAbsLast)/(_tAbs-_tAbsLast);
	}
	AnimTick ToGlobal(AnimTick tLocal)
	{
		if (_t==_tLast)
			return _tAbs;
		if (tLocal<_tLast)
			tLocal=_tLast;
		return _tAbsLast+(_tAbs-_tAbsLast)*(tLocal-_tLast)/(_t-_tLast);
	}
	void SetRate(float rate)
	{
		_rate=rate;
	}
protected:
	float _rate;
	AnimTick _tAbsLast,_tAbs;
	AnimTick _tLast,_t;

};




class ISklLinks;
class IRatomsBv;
class IMano;
class IAnimTreeCtrl;
class CAvtrStates;
class CLocalTime;
//注意:IAnimNode返回的都是指针,这些指针可能为空,如果为空,表示在这个时刻,这个AnimNode没有一个
//有意义的值
class IAnimNode
{
public:
	INTERFACE_REFCOUNT;
	virtual CClass *GetClass()=0;
	virtual BOOL IsAlive()	{		return TRUE;	}
	virtual IAnimNode *GetBase()	{		return NULL;	}
	virtual float *GetValue(AnimTick t)	{		return NULL;	}
	virtual i_math::matrix43f*GetMat(AnimTick t)	{		return NULL;	}
	virtual IMatrice43*GetSklMats(AnimTick t)	{		return NULL;	}
	virtual IMatrice43*GetSkinMats(AnimTick t)	{		return NULL;	}
	virtual i_math::aabbox3df *GetAabb(AnimTick t)	{		return NULL;	}
	virtual i_math::vector3df *GetPos(AnimTick t)	{		return NULL;	}
	virtual i_math::quatf*GetRot(AnimTick t)	{		return NULL;	}
	virtual float*GetEulerX(AnimTick t)	{		return NULL;	}
	virtual DWORD *GetColor(AnimTick t)	{		return NULL;	}
	virtual i_math::color4df *GetColorF(AnimTick t)	{		return NULL;	}
	virtual i_math::vector3df *GetColorMod_DL(AnimTick t)	{		return NULL;	}
	virtual i_math::vector3df *GetColorMod_Global(AnimTick t)	{		return NULL;	}
	virtual float *GetColorMod_SightRate(AnimTick t)	{		return NULL;	}
	virtual StringID *GetStringID(AnimTick t)	{		return NULL;	}
	virtual ITexture *GetTex(AnimTick t)	{		return NULL;	}
	virtual DWORD GetTeleportID(AnimTick t)	{		return 0;	}
	virtual IRatomsBv *GetRatomsBv()	{		return NULL;	}
	virtual ICamera*GetCamera(AnimTick t,BOOL &bLab)	{		bLab=FALSE;return NULL;	}//如果返回值为空,bLab为未定义的值
	virtual IMano*GetMano()	{		return NULL;	}
	virtual ISklLinks*GetSklLinks()	{		return NULL;	}
	virtual IAnimTreeCtrl*GetAnimTreeCtrl()	{		return NULL;	}
	virtual IAsset*GetOwner()	{		return NULL;	}
	virtual CAvtrStates *GetAvs()	{		return NULL;	}
	virtual CLocalTime *GetLocalTime()	{		return NULL;	}
	virtual BOOL IsFixed()	{		return FALSE;	}
	virtual BOOL IsSkl()	{		return FALSE;	}
	virtual BOOL IsStop(AnimTick t)	{	return FALSE;	}
	virtual BOOL CalkIKCtrls(IAnimTreeCtrl *ctrl,AnimTick t,float weight,i_math::matrix43f &matBase,i_math::xformf *xfmsLocal,DWORD nXfms)	{	return FALSE;	}
	//IMPORTANT:如果添加新的GetXXXX()函数,别忘了在IAnimNodeProxy里也添加对应的函数实现

};


#define IAnimNode_GetSklMats(an,t) (an)?(an)->GetSklMats(t):NULL
#define IAnimNode_GetSkinMats(an,t) (an)?(an)->GetSkinMats(t):NULL

inline i_math::matrix43f *IAnimNode_GetMat(IAnimNode *an,AnimTick t)
{
	if (!an)
		return NULL;
	return an->GetMat(t);
}

inline BOOL IAnimNode_GetMat(IAnimNode *an,AnimTick t,i_math::matrix43f &mat)
{
	if (!an)
		return FALSE;
	i_math::matrix43f *p=an->GetMat(t);
	if (!p)
		return FALSE;
	mat=*p;
	return TRUE;
}



class IAnimNodes
{
public:
	//注意:所有的CreateXXXX()返回的anim node全部带一个引用计数

	virtual IAnimNodeMat *CreateMat()=0;
	virtual IAnimNodeMatFixed *CreateMatFixed()=0;
	virtual IAnimNodeMatFixedExt *CreateMatFixedExt()=0;
	virtual IAnimNodeMatOffset *CreateMatOffset()=0;
	virtual IAnimNodeSkeleton *CreateSkeleton()=0;
	virtual IAnimNodeMatrice43 *CreateMatrice43()=0;
	virtual IAnimNodePathNav *CreatePathNav()=0;
	virtual IAnimNodeSkin*CreateSkin()=0;
	virtual IAnimNodeSkinSite*CreateSkinSite()=0;

	virtual IAnimNodeRagdoll*CreateRagdoll()=0;
	virtual IAnimNodeDtr *CreateDtr()=0;

	virtual IAnimNodePosEuler*CreatePosEuler()=0;

	virtual IAnimNodeEulerOverride *CreateEulerOverride()=0;

	virtual IAnimNodeUV*CreateUV()=0;
	virtual IAnimNodePath*CreatePath()=0;

	virtual IAnimNodeProxy*CreateProxy()=0;
	virtual IAnimNodeSite*CreateSite()=0;
	virtual IAnimNodeNoRoll*CreateNoRoll()=0;

	virtual IAnimNodeColorFixed *CreateColorFixed()=0;

	virtual IAnimNodeSklLinks *CreateSklLinks()=0;
	virtual IAnimNodeCamera*CreateCamera()=0;
	
	virtual IAnimNodeAnimTreeCtrl *CreateAnimTreeCtrl() = 0;

	virtual IAnimNodeSwitcher*CreateSwitcher() = 0;

};

class IAnimNode;
class IDummies;

class IAnimNodeProxy:public IAnimNode
{
public:
	IAnimNodeProxy()
	{
		_base=NULL;
	}
	~IAnimNodeProxy()
	{
		SAFE_RELEASE(_base);
	}
	virtual BOOL IsAlive()	{		return _base?_base->IsAlive():TRUE;	}
	virtual float *GetValue(AnimTick t)	{		return _base?_base->GetValue(t):NULL;	}
	virtual i_math::matrix43f*GetMat(AnimTick t)	{		return _base?_base->GetMat(t):NULL;	}
	virtual IMatrice43*GetSklMats(AnimTick t)	{		return _base?_base->GetSklMats(t):NULL;	}
	virtual IMatrice43*GetSkinMats(AnimTick t)	{		return _base?_base->GetSkinMats(t):NULL;	}
	virtual i_math::aabbox3df *GetAabb(AnimTick t)	{		return _base?_base->GetAabb(t):NULL;	}
	virtual i_math::vector3df *GetPos(AnimTick t)	{		return _base?_base->GetPos(t):NULL;	}
	virtual i_math::quatf*GetRot(AnimTick t)	{		return _base?_base->GetRot(t):NULL;	}
	virtual float*GetEulerX(AnimTick t)	{		return _base?_base->GetEulerX(t):NULL;	}
	virtual DWORD *GetColor(AnimTick t)	{		return _base?_base->GetColor(t):NULL;	}
	virtual i_math::color4df *GetColorF(AnimTick t)	{		return _base?_base->GetColorF(t):NULL;	}
	virtual i_math::vector3df *GetColorMod_DL(AnimTick t)	{		return _base?_base->GetColorMod_DL(t):NULL;	}
	virtual i_math::vector3df *GetColorMod_Global(AnimTick t)	{		return _base?_base->GetColorMod_Global(t):NULL;	}
	virtual float *GetColorMod_SightRate(AnimTick t)	{		return _base?_base->GetColorMod_SightRate(t):NULL;	}
	virtual ITexture *GetTex(AnimTick t)	{		return _base?_base->GetTex(t):NULL;	}
	virtual StringID *GetStringID(AnimTick t)	{		return _base?_base->GetStringID(t):NULL;	}
	virtual DWORD GetTeleportID(AnimTick t)	{		return _base?_base->GetTeleportID(t):0;	}
	virtual IRatomsBv *GetRatomsBv()	{		return _base?_base->GetRatomsBv():NULL;	}
	virtual ICamera*GetCamera(AnimTick t,BOOL &bLab)	{		return _base?_base->GetCamera(t,bLab):NULL;	}
	virtual IMano*GetMano()	{		return _base?_base->GetMano():NULL;	}
	virtual ISklLinks*GetSklLinks()	{		return _base?_base->GetSklLinks():NULL;	}
	virtual IAnimTreeCtrl*GetAnimTreeCtrl()	{		return _base?_base->GetAnimTreeCtrl():NULL;	}
	virtual IAsset*GetOwner()	{		return _base?_base->GetOwner():NULL;	}
	virtual CAvtrStates *GetAvs()	{		return _base?_base->GetAvs():NULL;	}
	virtual CLocalTime *GetLocalTime()	{		return _base?_base->GetLocalTime():NULL;	}
	virtual BOOL IsFixed()	{		return _base?_base->IsFixed():FALSE;	}
	virtual BOOL IsSkl()	{		return _base?_base->IsSkl():FALSE;	}
	virtual BOOL IsStop(AnimTick t)	{	return _base?_base->IsStop(t):FALSE;	}
	virtual BOOL CalkIKCtrls(IAnimTreeCtrl *ctrl,AnimTick t,float weight,i_math::matrix43f &matBase,i_math::xformf *xfmsLocal,DWORD nXfms)		{		return _base?_base->CalkIKCtrls(ctrl,t,weight,matBase,xfmsLocal,nXfms):FALSE;	}

	virtual void SetBase(IAnimNode *base)
	{
		SAFE_REPLACE(_base,base);
	}
	virtual IAnimNode *GetBase()	{		return _base;	}

protected:
	IAnimNode *_base;
};


class IAnimNodeMat:public IAnimNode
{
public:
	virtual void Reset(i_math::xformf &xfm,AnimTick t)=0;
	virtual void Add(i_math::xformf &xfm,AnimTick t)=0;

	virtual AnimTick GetFirstT()=0;
	virtual AnimTick GetSecondT()=0;

	virtual i_math::xformf &GetFirst()=0;
	virtual i_math::xformf &GetSecond()=0;
};

class IAnimNodeDtr:public IAnimNode
{
public:
	virtual void Reset(i_math::xformf *xfms,DWORD count,i_math::matrix43f &matBase,AnimTick t)=0;
	virtual void Reset(i_math::xformf *xfms,DWORD count,IAnimNode *anBase,AnimTick t)=0;
	virtual i_math::xformf *Add(AnimTick t)=0;

	virtual AnimTick GetFirstT()=0;
	virtual AnimTick GetSecondT()=0;

};


class IAnimNodeMatFixed:public IAnimNode
{
public:
	virtual void Set(i_math::matrix43f &mat)=0;
};

class IAnimNodeMatFixedExt:public IAnimNodeProxy
{
public:
	virtual void Set(i_math::matrix43f &mat)=0;
};


class IAnimNodeMatOffset:public IAnimNodeProxy
{
public:
	virtual void SetOffset(i_math::matrix43f &mat)=0;
	virtual void SetFixed(i_math::matrix43f &mat)=0;
};


class IAnimNodeEulerOverride:public IAnimNodeProxy
{
public:
	virtual void Reset(i_math::vector3df &euler,AnimTick t)=0;
	virtual void Add(i_math::vector3df &euler,AnimTick t)=0;
	virtual AnimTick GetFirstT()=0;
	virtual AnimTick GetSecondT()=0;
	virtual i_math::vector3df &GetFirst()=0;
	virtual i_math::vector3df &GetSecond()=0;

};

struct PosEuler
{
	i_math::vector3df pos;
	i_math::vector3df euler;
};

class IAnimNodePosEuler:public IAnimNode
{
public:
	virtual void Reset(PosEuler &pe,AnimTick t)=0;
	virtual void Add(PosEuler &pe,AnimTick t)=0;
	virtual AnimTick GetFirstT()=0;
	virtual AnimTick GetSecondT()=0;
	virtual PosEuler &GetFirst()=0;
	virtual PosEuler &GetSecond()=0;

	virtual void SetDbgName(const char *)=0;
};


class IAnimNodeSite:public IAnimNodeProxy
{
public:
	virtual void SetFixed(i_math::matrix43f &mat)=0;
	virtual void SetBase(IAnimNode *an,AnimTick t)=0;

	virtual BOOL GetBaseMat(AnimTick t,i_math::matrix43f &mat)=0;

	virtual i_math::matrix43f *GetLocalMat()=0;
	virtual void SetLocalMat(i_math::matrix43f *matLocal)=0;//注意,传入的matLocal指针将会被保留下来,所以要确保这个指针一直有效
	virtual IAnimNode *GetBase()=0;
	virtual BOOL IsAttaching()=0;
	virtual void SetForceAttach(BOOL bForceAttach)=0;//所谓Force Attach是指:如果没有Attach某个anim node上,则这个anim node不工作(GetMat()返回NULL)
	virtual BOOL GetForceAttach()=0;

};

#define IAnimNodeSite_GetBaseMat(__site,t,mat)	(__site)?((__site)->GetBaseMat(t,mat)):FALSE;


//一个用来从矩阵旋转的head/pitch/roll中去掉roll的过滤器
class IAnimNodeNoRoll:public IAnimNodeProxy
{
public:
};

class IAnimNodeSkeleton:public IAnimNodeProxy
{
public:
	virtual void SetLocalAabb(i_math::aabbox3df &aabb)=0;
	virtual void SetAnimTreeCtrl(IAnimTreeCtrl *ctrl)=0;
	virtual void SetBaseLink(IDummies *dummies,const char *name)=0;//设定骨骼上的一个dummy 位点连接在基点上
	virtual void SetBaseLink(IDummies *dummies,DWORD iDummy)=0;
	virtual BOOL CheckBaseLink(IDummies *dummies,const char *name)=0;//检查当前是不是指定的base link
	virtual BOOL CalcBaseLinkMat(AnimTick t,i_math::matrix43f &mat)=0;//计算一个矩阵,把局部空间的点转换到世界空间的点
};

class ISkeleton;
class IAnimNodeMatrice43:public IAnimNodeProxy
{
public:
	virtual void SetSkeleton(ISkeleton *skl)=0;
	virtual void Reset(i_math::xformf *xfms,DWORD count,AnimTick t)=0;
	virtual i_math::xformf *Add(AnimTick t)=0;
	virtual AnimTick GetFirstT()=0;
	virtual AnimTick GetSecondT()=0;
	virtual i_math::xformf *GetFirst()=0;
	virtual i_math::xformf *GetSecond()=0;
};

class IAnimNodeRagdoll:public IAnimNodeProxy
{
public:
	virtual void SetLocalAabb(i_math::aabbox3df &aabb)=0;
	virtual void SetRagdollCtrl(IRagdollCtrl*ctrl)=0;
	virtual IRagdollCtrl*GetRagdollCtrlCtrl()=0;
};

class IAnimNodeSkin:public IAnimNodeProxy
{
public:
	virtual void SetSkeleton(ISkeleton *skl)=0;
};

class IAnimNodeSkinSite:public IAnimNodeProxy
{
public:
	virtual void SetSiteInfo(i_math::vector3df &pos,i_math::vector3df &normal,i_math::vector3df&tangent,
											DWORD nWeights,BYTE *boneindices,float *weights)=0;
};

//代表骨骼上的一个dummy位点的动画
class IAnimNodeSkeletonDummy:public IAnimNodeProxy
{
public:
	virtual void SetDummy(IDummies *dummies,const char *name)=0;
};

class IAnimNodePathNav:public IAnimNodeProxy
{
public:
	virtual void SetAnimTreeCtrl(IAnimTreeCtrl *ctrl)=0;
	virtual void SetOwner(IAsset *owner)=0;
	virtual void Destroy()=0;//会释放引用计数 

};

class IAnimPlayer;

class IAnimNodeUV:public IAnimNode
{
public:
	virtual void SetAnimPlayer(IAnimPlayer*player)=0;//注意,调用过这个函数后,不能再调用IAnimPlayer::Reset(...)
};


class IAnimNodePath:public IAnimNodeProxy
{
public:
	virtual void SetAnimPlayer(IAnimPlayer*player)=0;//注意,调用过这个函数后,不能再调用IAnimPlayer::Reset(..)
	virtual void SetLocalBase(BOOL bLocalBase)=0;
	virtual BOOL GetLocalBase()=0;
	virtual float GetRatio(AnimTick t)=0;
};



class IAnimNodeTex:public IAnimNode
{
public:
	virtual void SetTex(ITexture *tex)=0;
	virtual ITexture *GetTex(AnimTick t)=0;
};

class IAnimNodeColorFixed:public IAnimNode
{
public:
	virtual void SetColor(DWORD col)=0;
};


class IAnimNodeSklLinks:public IAnimNodeProxy
{
public:
	virtual BOOL IsAlive()=0;
	virtual void SetRatomsBv(IRatomsBv *bv)=0;
	virtual void SetOwner(IAsset *owner)=0;
	virtual void SetAvs(CAvtrStates*avs)=0;
	virtual void Destroy()=0;//会释放引用计数 
	virtual void EnableLocalTime()=0;
	virtual void SetLocalTimeRate(float rate)=0;
	virtual void UpdateLocalTime()=0;
};

class IAnimNodeCamera:public IAnimNodeProxy
{
public:
	virtual void SetCamera(ICamera *cam,BOOL bLab)=0;
};

class IAnimNodeAnimTreeCtrl :public IAnimNode	//只包含一个AnimTreeCtrl
{
public:
	virtual void SetAnimTreeCtrl(IAnimTreeCtrl * ctrl) = 0;
};

class IAnimNodeSwitcher:public IAnimNode
{
public:
	virtual void SwitchTo(IAnimNode *an,AnimTick durBlend=ANIMTICK_FROM_SECOND(0.25f))=0;
	virtual BOOL IsStop(AnimTick t)=0;
	virtual IAnimNode *GetTop() =0;
};

