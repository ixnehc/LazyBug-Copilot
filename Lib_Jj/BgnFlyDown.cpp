/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	AI Actions
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "BgnFlyDown.h"
#include "LevelRecords.h"

#include "Random/Random.h"

#include "LevelRecordGesture.h"
#include "GestureFlyDown.h"

#include "LevelObjMove.h"


#include "Level.h"

BIND_BGN_CLASS(CBgn_FlyDown,CBgp_FlyDown);


void CBgn_FlyDown::Destroy()
{
	SAFE_RELEASE(_ges);
}

void CBgn_FlyDown::Break(BGNOutputs &outputs)	
{	
	Destroy();
}

void CBgn_FlyDown::Start(DWORD iStb,BGNOutputs &outputs)
{
	_owner=_GetLo();
	CBgn_FlyingHover::Start(iStb,outputs);
}

BOOL CBgn_FlyDown::_CanDescend()
{
	LevelPos3D posCur=_owner->GetFramePos3D();
	if (posCur.getDistanceFromSQ(_posStart)<4.0f*4.0f)
		return TRUE;
	return FALSE;
}


void CBgn_FlyDown::_SwitchGround()
{
	if (_bSwitchGround)
		return;
	CLevelObjMove *move=_owner->GetMove();
	if (move)
	{
		LevelPos &vel=move->GetVel();
		LevelPos pos=_owner->GetFramePos();
		move->SwitchGround(pos,atan2f(vel.y,vel.x),LevelTeleportID_Invalid);
	}

	_bSwitchGround=TRUE;
}

void CBgn_FlyDown::_FireFail(BGNOutputs &outputs)
{
	_OutputOk(outputs,2,"无法着陆");
}


void CBgn_FlyDown::Update(BGNOutputs &outputs)
{
	CBgp_FlyDown*pad=_GetPad<CBgp_FlyDown>();
	if (_stage==Hovering)
	{
		if (CBgn_FlyingHover::_Update())
		{
			_FireFail(outputs);
			return;
		}

		LevelRecordGesture *ges=_owner->GetLevel()->GetRecords()->GetGesture(pad->_idGesture);
		if (ges)
		{
			GestureParam_FlyDown *paramGes=ges->GetParam<GestureParam_FlyDown>();
			if (paramGes)
			{
				LevelPos3D posStart,posEnd;
				extern BOOL LevelUtil_FindLandingSpot(CLevelObj *lo,float fwd,float rangeDescend,float rangeLand,float height,LevelPos3D &posStart,LevelPos3D &posEnd);
				if (LevelUtil_FindLandingSpot(_owner,2.0f,paramGes->rangeDescend,paramGes->rangeLand,paramGes->htDescend,posStart,posEnd))
				{
					_posStart=posStart;
					_posEnd=posEnd;
					if (_CanDescend())
					{
						_stage=Descend;
						if (!_Descend())
						{
							_FireFail(outputs);
							return;
						}
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
			if (!_Descend())
			{
				_FireFail(outputs);
				return;
			}
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

			_OutputOk(outputs,1,"成功着陆");
		}
	}

}


BOOL CBgn_FlyDown::_Descend()
{
	CBgp_FlyDown*pad=_GetPad<CBgp_FlyDown>();
	CLevelGesture *CreateGesture(CLevelObj *owner,LevelRecordGesture *recGesture,LevelSkillTarget &target);
	LevelRecordGesture *rec=_owner->GetLevel()->GetRecords()->GetGesture(pad->_idGesture);
	CLevelGesture *gesture=CreateGesture(_owner,rec,LevelSkillTarget());
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
		return FALSE;
	return TRUE;
}
