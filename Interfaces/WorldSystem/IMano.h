/********************************************************************
	created:	1:1:2010   20:07
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	material node
*********************************************************************/
#pragma once

#include "strlib/strlib.h"

#include "ctrlqueue/CtrlQueue.h"

class IMtrl;


#define MAOLayorFlag_NativeColMod 1//不使用MAO的colmod
#define MAOLayorFlag_Glow 2//不使用MAO的colmod

enum ManoChannel
{
	MNC_Alpha0,
	MNC_Alpha1,

	MNC_Color0,

	MNC_Illum0,

	MNC_Layor0,

	MNC_Glow0,
	MNC_Outline0,

	//只能在末尾增加
	MNC_Max,
};

struct MAOColMod
{
	BYTE bColMod:1;
	BYTE bAlpha:1;
	i_math::vector4df difmul;
	i_math::vector4df difadd;

	void Merge(MAOColMod &other)
	{
		if ((!bColMod)&&(!other.bColMod))
			return;//都没有
		if (!bColMod)
			memcpy(this,&other,sizeof(*this));//我没有,other有
		else
		{
			if (other.bColMod)
			{//都有
				bColMod=bColMod||other.bColMod;
				bAlpha=bAlpha||other.bAlpha;
				difmul*=other.difmul;
				difadd+=other.difadd;
			}
		}
	}
};

struct MAOLayor
{
	IMtrl *mtrl_;
	IMtrl *mtrlGlow;
	AnimTick age;
	MAOColMod cm;
	BYTE flags;
	StringID nmEP;
	float vEP;
	MAOLayor*next;
};

struct ManoEP
{
	StringID idEP;
	int gvt;//a GVarType value
	float buf[4];
};

struct MAOEPs
{
	MAOEPs()
	{
		count=0;
	}
	DWORD count;
	ManoEP buf[8];
};



struct MtrlAddOn
{
	MtrlAddOn()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}
	BOOL ExistNativeColModLayor()
	{
		MAOLayor *p=layors;
		while(p)
		{
			if (p->flags&MAOLayorFlag_NativeColMod)
				return TRUE;
			p=p->next;
		}
		return FALSE;
	}
	BOOL ExistGlowLayor()
	{
		MAOLayor *p=layors;
		while(p)
		{
			if (p->flags&MAOLayorFlag_Glow)
				return TRUE;
			p=p->next;
		}
		return FALSE;
	}
	MAOColMod cm;
	i_math::vector3df colGlow;
	DWORD colOutline;
	float thickOutline;
	MAOLayor*layors;
	MAOEPs *eps;
	BYTE bCover:1;
	BYTE bOutline:1;
	BYTE bOutlineIgnoreHide:1;//Draw outline even the ratom is hide
	BYTE bOutlineGlow:1;
	BYTE bGlow:1;
};

class CClass;
struct MtrlCtrl:public CCtrlQueue::Ctrl
{
	virtual CClass *GetClass()=0;
	virtual float GetAlpha(AnimTick t)	{		return 1.0f;	}
	virtual DWORD GetColor(AnimTick t)	{		return 0xffffffff;	}
	virtual DWORD GetGlow(AnimTick t)	{		return 0xffffffff;}//Glow的颜色
	virtual float GetGlowStr(AnimTick t)	{		return 1.0f;}//Glow的强度
	virtual DWORD GetOutline(AnimTick t)	{		return 0xffffffff;	}
	virtual float GetOutlineThick(AnimTick t)	{		return 0.01f;	}
	virtual BOOL GetOutlineIgnoreHide(AnimTick t)	{		return FALSE;	}
	virtual BOOL GetOutlineGlow(AnimTick t)	{		return FALSE;	}
	virtual IMtrl *GetMtrl(DWORD &flags)	{		flags=0;return NULL;	}
	virtual IMtrl *GetMtrlGlow()	{		return NULL;	}
	virtual StringID GetMtrlEP(float &v,AnimTick t)	{		v=0.0f;	return StringID_Invalid;}
};


//Mano为MAterial NOde的缩写
struct ManoEP;
class IMano
{
public:
	INTERFACE_REFCOUNT;

	virtual MtrlAddOn *GetAddOn(AnimTick t)=0;

	virtual BOOL AddCtrl(ManoChannel ch,MtrlCtrl*ctrl)=0;
	virtual void Tick(AnimTick t)=0;

	virtual BOOL AddEP(StringID idEP,i_math::vector3df &v)=0;
	virtual void RemoveEP(StringID idEP)=0;

};
