/********************************************************************
	created:	2010/11/09
	created:	9:11:2010   9:02
	file base:	Tuner
	author:		ixnehc
	
	purpose:	数值调控系统
*********************************************************************/

#include "stdh.h"
#include "Tuner.h"

#include "../../Interfaces/WorldSystem/IAnimNodes.h"


//////////////////////////////////////////////////////////////////////////
//CTuneNode
void CTuneNode::Clear()
{
	if (_cur)
	{
		_cur->OnCancel();
		SAFE_RELEASE(_cur);
	}
	SAFE_RELEASE(_anFloat);
}

BOOL CTuneNode::_EnsureType(TuneValueType tvt)
{
	if (_tvt!=TuneValue_None)
		return (_tvt==tvt);
	_tvt=(BYTE)tvt;
	switch(tvt)
	{
		case TuneValue_Float: 
		{
			memset(_fv,0,sizeof(_fv));
			_vel=0;
			break;
		}
		case TuneValue_Color:
		{
			memset(_col,0xff,sizeof(_col));
			break;
		}
		case TuneValue_StringID: 
		{
			_idStr=StringID_Invalid;
			break;
		}
	}
	return TRUE;
}


void CTuneNode::AddTuner(CTuner*tuner)
{
	if (!_EnsureType(tuner->GetValueType()))
		return;

	_tCurTuner=_t;

	TuneLimitType lt=tuner->GetLimitType();
	if (lt<_lt)
		_lt=lt;//突破原来的限制

	//之所以要copy到临时变量的原因是为了处理重入的情况
	CTuner*old=_cur;
	_cur=tuner;
	SAFE_ADDREF(_cur);
	_bLerpCalc=_cur->NeedLerpCalc()?1:0;
	if (old)
	{
		old->OnCancel();
		old->Release();
	}
}

void CTuneNode::SetFloat(IAnimNode *an)
{
	if (!_EnsureType(TuneValue_Float))
		return;
	SAFE_REPLACE(_anFloat,an);
}


float CTuneNode::GetFloat(AnimTick t,AnimTick tLast,float lerp)
{
	if (_tvt!=TuneValue_Float)
		return 0.0f;
	float ret;
	if (_anFloat)
	{
		AnimTick tCur=tLast+(AnimTick)(lerp*(float)(t-tLast));
		float *v=_anFloat->GetValue(tCur);
		if (v)
			ret=*v;
		else
			ret=0.0f;
	}
	else
	{
		if (_bLerpCalc)
			ret=(float)i_math::lerp(_fv[1-_flip],_fv[_flip],(double)lerp);
		else
		{
			float vel;
			double ret2;
			AnimTick tCur=tLast+(AnimTick)(lerp*(float)(t-tLast));
			AnimTick age=ANIMTICK_SAFE_MINUS(tCur,_tCurTuner);
			_cur->Update(ret2,vel,ANIMTICK_TO_SECOND(age));
			ret=(float)ret2;
		}
	}
	if (_lt==TuneLimit_Free)
		return ret;
	if ((ret>=0.0f)&&(ret<=1.0f))
		return ret;
	return i_math::wrap(ret,1.0f);
}

StringID CTuneNode::GetStringID()
{
	if (_tvt!=TuneValue_StringID)
		return StringID_Invalid;
	return _idStr;
}


void CTuneNode::Update(AnimTick t)
{
	_flip=1-_flip;

	_t=t;
	float age=ANIMTICK_TO_SECOND(_t-_tCurTuner);

	switch(_tvt)
	{
		case TuneValue_Float:
		{
			_fv[_flip]=_fv[1-_flip];

			if (_bLerpCalc)
			{
				if (_cur)
					_cur->Update(_fv[_flip],_vel,age);

				if (_lt==TuneLimit_Clamp)
					_fv[_flip]=i_math::clamp_dbl(_fv[_flip],0,1);
			}

			if (_cur)
			{
				if (_cur->CanFinish(age))
				{
					if (!_bLerpCalc)
					{
						_cur->Update(_fv[_flip],_vel,age);
						_fv[1-_flip]=_fv[_flip];
					}
					else
						_vel=_cur->GetFinishVel();

					_bLerpCalc=1;

					//之所以要copy到临时变量的原因是为了处理重入的情况
					CTuner*old=_cur;
					_cur=NULL;
					old->OnFinish();
					SAFE_RELEASE(old);
				}
			}
			break;
		}
	}
}



//////////////////////////////////////////////////////////////////////////
//CTuner


void CTuner::OnCancel()	
{
	if (_owner)
		_owner->OnTunerCancel();
}

void CTuner::OnFinish()
{
	if (_owner)
		_owner->OnTunerFinish();
}

