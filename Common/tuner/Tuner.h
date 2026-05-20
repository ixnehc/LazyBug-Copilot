
#pragma once

#include "../anim/animdefines.h"
#include "../strlib/strlibdefines.h"

#include "class/class.h"
#include "gds/GObj.h"

enum TuneValueType
{
	TuneValue_None,
	TuneValue_Float,
	TuneValue_Color,
	TuneValue_StringID,
	TuneValue_Pos,
};

enum TuneLimitType
{
	//值越大,限制越大
	TuneLimit_Free=0,//没有限制
	TuneLimit_Wrap,//wrap到 0..1
	TuneLimit_Clamp,//clamp到0..1
};


class IAnimNode;
class CTuner;
class CTuneNode
{
public:
	DEFINE_CLASS(CTuneNode);
	CTuneNode()
	{
		_tvt=TuneValue_None;

		_t=ANIMTICK_INFINITE;
		_tCurTuner=ANIMTICK_INFINITE;
		_flip=0;
		_cur=NULL;
		_lt=TuneLimit_Clamp;
		_bLerpCalc=1;
		_anFloat=NULL;
	}
	void Clear();
	void SetT(AnimTick t)	{		_t=t;	}
	void Update(AnimTick t);

	void SetFloat(float v)
	{
		if (!_EnsureType(TuneValue_Float))
			return;
		if ((v>1.0f)||(v<0.0f))
			_lt=TuneLimit_Free;//突破限制
		_fv[0]=_fv[1]=(double)v;
		_vel=0.0f;
	}
	void SetFloat(IAnimNode *an);

	IAnimNode *GetFloatAnimNode()	{		return _anFloat;	}
	float GetFloat(AnimTick t,AnimTick tLast,float lerp);

	void SetStringID(StringID idStr)
	{
		if (!_EnsureType(TuneValue_StringID))
			return;

		_idStr=idStr;
	}
	StringID GetStringID();

	void AddTuner(CTuner*tuner);

protected:
	BOOL _EnsureType(TuneValueType tvt);

	BYTE _tvt;//TuneValueType 类型,决定这个tune node 的数值类型
	BYTE _flip;
	BYTE _lt;//limit type
	BYTE _bLerpCalc;//是否采用插值的方式计算TuneValue

	AnimTick _t;
	CTuner*_cur;
	AnimTick _tCurTuner;//当前的tuner被加入的起始时间

	IAnimNode *_anFloat;

	union
	{
		struct
		{
			double _fv[2];//float value
			float _vel;
		};
		DWORD _col[2];//color value
		StringID _idStr;
	};

};

struct FloatOverrider
{
	FloatOverrider()
	{
		v=-10000.0f;
	}
	BOOL IsValid()
	{
		return v>-9999.0f;
	}
	void Override(float &v_)
	{
		if (IsValid())
			v_=v;
	}
	void Set(float v_)		{			v=v_;		}
	float Get()		{			return v;		}

	float v;
};



class CTunerOwner;
class CTuner
{
public:
	IMPLEMENT_REFCOUNT_C
	CTuner()
	{
		_owner=NULL;
	}

	virtual TuneLimitType GetLimitType()=0;
	void Attach(CTunerOwner*owner)	{		_owner=owner;	}
	void Detach()	{		_owner=NULL;	}

	virtual CClass *GetClass()=0;
	virtual TuneValueType GetValueType()=0;
	virtual void OnCancel();
	virtual void OnFinish();
	virtual void Update(double &tv,float &vel,float age)=0;
	virtual BOOL CanFinish(float age)=0;
	virtual float GetFinishVel()		{		return 0.0f;	}		//返回结束后需要维持什么速度
	virtual BOOL NeedLerpCalc()		{		return TRUE;	}		//返回是否要通过插值的方式计算TuneValue

	virtual void OverrideTv(float tv)	{	}
	virtual void OverrideVel(float tv)	{	}

protected:
	CTunerOwner *_owner;
};

class CTunerOwner
{
public:
	virtual void OnTunerCancel()	{	}
	virtual void OnTunerFinish()	{	}
};


