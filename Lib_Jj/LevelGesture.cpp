/********************************************************************
	created:	2012/11/10 
	author:		cxi
	
	purpose:	用来控制生物飞行轨迹的对象
*********************************************************************/

#include "stdh.h"

#include "LevelGesture.h"
#include "LevelRecords.h"

#include "LevelRecordGesture.h"


#include "Level.h"

////////////////////////////////////////////////////////////////////////
//CLevelGesture

//带一个引用计数
CLevelGesture *CreateGesture(CLevelObj *owner,LevelRecordGesture *recGesture,LevelSkillTarget &target)
{
	CClass *clss=recGesture->Param->GetGestureClass();
	if (!clss)
		return NULL;
	CLevelGesture *p=(CLevelGesture *)clss->New();
	p->Create(owner,recGesture->GetID(),recGesture->Param,target);
	p->AddRef();
	return p;
}

BOOL CLevelGesture::Create(CLevelObj *owner,RecordID idRec,LevelGestureParam *param,LevelSkillTarget &target)
{
	_owner=owner;
	SAFE_ADDREF(_owner);

	_core.idRec=idRec;
	if (idRec!=RecordID_Invalid)
		_core.rec=owner->GetLevel()->GetRecords()->GetGesture(idRec);
	_core.param=param;
	_core.target=target;

	_core.pos3DInitial=_owner->GetFramePos3D();
	_core.faceInitial=_owner->GetFrameFace();

	_core.radiusTarget=0.0f;
	extern CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target);
	CLevelObj *loTarget=LevelUtil_GetTargetObj(_owner->GetLevel(),_core.target);
	if (loTarget)
		_core.radiusTarget=loTarget->GetRadius_();

	_core.radiusOwner=_owner->GetRadius_();

	_core.t=0;

	_OnCreate();
	return TRUE;
}

void CLevelGesture::_Destroy()
{
	if (_owner)
	{
		_events.clear();
		_OnDestroy();
		SAFE_RELEASE(_owner);
		Zero();
	}
}


void CLevelGesture::Destroy()
{
	_Destroy();
	Release();
}

StringID CLevelGesture::FetchEvent(i_math::xformf &xfm)
{
	if (_events.size()>0)
	{
		xfm=_events[0].xfm;
		StringID nm=_events[0].nm;
		_events.pop_front();
		return nm;
	}
	return StringID_Invalid;
}

void CLevelGesture::UpdateEvent(AnimTick dt)
{
	_ApplySpeedRate(dt);
	_events.clear();
	for (int i=0;i<_core.rec->Events.size();i++)
	{
		LevelGestureEvent &e=_core.rec->Events[i];
		if ((e.t>=_tEvent)&&(e.t<_tEvent+dt))
		{
			EventEntry entry;
			entry.nm=e.nm;
			entry.xfm.pos=_owner->GetFramePos3D();
			LevelFaceToQuat(_owner->GetFrameFace(),entry.xfm.rot);

			_events.push_back(entry);
		}
	}

	_tEvent+=dt;
}

void CLevelGesture::_ApplySpeedRate(AnimTick &dt)
{
	float rate=1.0f;
	CLevelBuffs *buffs=_owner->GetBuffs();
	if (buffs)
		rate=buffs->GetIAS();

	dt=(AnimTick)(rate*(float)dt);
}

void CLevelGesture::_ApplySpeedRate(float &dt)
{
	float rate=1.0f;
	CLevelBuffs *buffs=_owner->GetBuffs();
	if (buffs)
		rate=buffs->GetIAS();

	dt=rate*dt;
}

