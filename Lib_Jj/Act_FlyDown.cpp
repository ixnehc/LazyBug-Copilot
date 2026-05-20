/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	AI Actions
*********************************************************************/
#include "stdh.h"

#include "Act_FlyDown.h"
#include "LevelRecords.h"

#include "Random/Random.h"

#include "LevelRecordGesture.h"
#include "GestureFlyDown.h"

#include "LevelObjMove.h"

#include "LevelAI.h"


#include "Level.h"

BIND_ACT_PARAM(Act_FlyDown,ActParam_FlyDown);


void Act_FlyDown::Start(AnimTick t)
{
	Act_Hover::Start(t);
}

void Act_FlyDown::Finish()	
{	
	SAFE_RELEASE(_ges);
}


BOOL Act_FlyDown::_CanDescend()
{
	LevelPos3D posCur=_owner->GetFramePos3D();
	if (posCur.getDistanceFromSQ(_posStart)<4.0f*4.0f)
		return TRUE;
	return FALSE;
}


void Act_FlyDown::_SwitchGround()
{
	if (_bSwitchGround)
		return;
	CLevelObjMove *move=_owner->GetMove();
	if (move)
	{
		LevelPos pos=_owner->GetFramePos();
		move->SwitchGround(pos,LevelTeleportID_Invalid);
	}

	_bSwitchGround=TRUE;
}


void Act_FlyDown::Update(AnimTick t)
{
	if (_result!=A_Pending)
		return;
	ActParam_FlyDown *param=(ActParam_FlyDown *)_param;
	if (_stage==Hovering)
	{
		Act_Hover::Update(t);

		LevelRecordGesture *ges=_owner->GetLevel()->GetRecords()->GetGesture(param->idGesture);
		if (ges)
		{
			GestureParam_FlyDown *paramGes=ges->GetParam<GestureParam_FlyDown>();
			if (param)
			{
				LevelPos3D posStart,posEnd;
				if (AIUtil_FindLandingSpot(_owner,2.0f,paramGes->rangeDescend,paramGes->rangeLand,paramGes->htDescend,posStart,posEnd))
				{
					_posStart=posStart;
					_posEnd=posEnd;
					if (_CanDescend())
					{
						_stage=Descend;
						_Descend();
					}
					else
					{
						_stage=Prepare;
						CUnit3D *unit=_owner->GetUnit3D();
						if (unit)
							unit->SetTarget_Pos3D(_posStart);
					}
				}
			}
		}
	}

	if (_stage==Prepare)
	{
		if (_CanDescend())
		{
			_stage=Descend;
			_Descend();
		}
	}

	if (_stage==Descend)
	{
		if (_ges)
		{
			if (!_ges->IsAlive())
				SAFE_RELEASE(_ges);
		}
		if (_ges)
		{
			if (!_bSwitchGround)
			{
				if (_ges->IsLanded())
					_SwitchGround();
			}
		}
		else
		{
			if (!_bSwitchGround)
				_SwitchGround();
			_result=A_Ok;
		}
	}

}


void Act_FlyDown::_Descend()
{
	ActParam_FlyDown *param=(ActParam_FlyDown *)_param;
	CLevelGesture *CreateGesture(CLevelObj *owner,LevelRecordGesture *recGesture);
	LevelRecordGesture *rec=_owner->GetLevel()->GetRecords()->GetGesture(param->idGesture);
	CLevelGesture *gesture=CreateGesture(_owner,rec);
	if (gesture)
	{
		if (gesture->GetClass()->IsSameWith(Class_Ptr2(Gesture_FlyDown)))
		{
			((Gesture_FlyDown *)gesture)->SetTarget(_posEnd);

			CUnit3D *unit=_owner->GetUnit3D();
			if (unit)
			{
				unit->SetGesture(gesture);
			}
		}
		else
		{
			SAFE_DESTROY(gesture);
		}
	}
	_ges=(Gesture_FlyDown *)gesture;
	if (!_ges)
		_result=A_Fail;
}
