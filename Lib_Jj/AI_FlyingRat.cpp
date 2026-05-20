/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	巨蝠兽AI
*********************************************************************/
#include "stdh.h"

#include "AI_FlyingRat.h"
#include "Random/Random.h"

#include "LevelEvents.h"


#include "Level.h"

#include "LoUnit.h"

BIND_AIPARAM(AI_FlyingRat,AIParam_FlyingRat);

void AI_FlyingRat::_OnCreate()
{
}

void AI_FlyingRat::_OnDestroy()
{
	ClearAct();
}

void AI_FlyingRat::_OnInitialUpdate()
{
	AIParam_FlyingRat *param=(AIParam_FlyingRat*)_param;

	_SetState(Idle);
}

void AI_FlyingRat::_OnUpdate(AnimTick t)
{
	if (TRUE)
	{
		extern BOOL AIUtil_TestAnyBuff(CLevelObj *lo,DWORD flagBuff);
		if (AIUtil_TestAnyBuff(_owner,BuffFlag_Birth|BuffFlag_Dead|BuffFlag_LayDown))
			return;
	}

	AIParam_FlyingRat *param=(AIParam_FlyingRat*)_param;

	
	LevelEventMapFlag flags=_owner->GetLevel()->GetEventMap()->GetEventFlags(_owner->GetFramePos());
//	DWORD c;
//	LevelEvent **events=_owner->GetLevel()->GetEventMap()->GetEvents(_owner->GetFramePos(),c);

	switch (_state)
	{
		case Idle:
		{
			if (!_act)
				NewAct<Act_Stroll>(&param->stroll)->Start(t);

			if (_Is_x4())
				UpdateAct(t);

			if (_Is_x4())
			{
				extern CLevelObj *AIUtil_DetectPlayerUnit(CLevelObj *lo,float range,LevelMoveMethodMask methods);
				CLevelObj *lo=AIUtil_DetectPlayerUnit(_owner,param->rangeSight,LevelMoveMethodMask_Ground);
				if (lo)
				{//发现有敌军,进入战斗状态
					ClearAct();
//					_SetState(Aggressive);
					_SetState(FlyUp);
					break;
				}
			}
			break;
		}
		case Aggressive:
		{
			if (!_act)
				NewAct<Act_Attack>(&param->attackGround)->Start(t,param->rangeSight);

			if (_Is_x4())
			{
				UpdateAct(t);

				//太长时间没有攻击对象,则回到Idle状态
				if (((Act_Attack*)_act)->GetIdleTime(t)>ANIMTICK_FROM_SECOND(4.0f))
				{
					ClearAct();
					_SetState(Idle);
					break;
				}
			}
			break;
		}
		case Flee:
		{
			if (!_act)
				NewAct<Act_Flee>(&param->flee)->Start(t);
			if (_Is_x4())
			{
				UpdateAct(t);

				if (((Act_Flee*)_act)->IsTimeUp())
				{
					ClearAct();
					_SetState(Idle);
					break;
				}
			}
			break;
		}
		case FlyUp:
		{
			if (!_act)
				NewAct<Act_FlyUp>(&param->flyup)->Start(t);

			if (_Is_x4())
			{
				UpdateAct(t);
				AResult result=((Act_FlyUp*)_act)->GetResult();
				if (result==A_Fail)
				{
					ClearAct();
					_SetState(Idle);
					break;
				}
				if (result==A_Ok)
				{
					ClearAct();
					_SetState(FlyingIdle);
					break;
				}
			}

			break;
		}
		case FlyingIdle:
		{
			if (!_act)
				NewAct<Act_Hover>(&param->hover)->Start(t);
			if (_Is_x2())
				UpdateAct(t);
			if (_Is_x4())
			{
				if (_GetStateAge()>ANIMTICK_FROM_SECOND(2.0f))
				{
					if (AIUtil_DetectClosestPlayerUnit(_owner,param->rangeFlyingSight,NULL,LevelMoveMethodMask_Ground))
					{
						ClearAct();
						_SetState(FlyingAttack);
						break;
					}
				}
			}
			if (_Is_x4())
			{
				if (((Act_Hover*)_act)->IsTimeUp())
				{
					ClearAct();
					_SetState(FlyDown);
					break;
				}
			}

			break;
		}
		case FlyingAttack:
		{
			if (!_act)
			{
				CLevelObj *obj=AIUtil_DetectClosestPlayerUnit(_owner,param->rangeFlyingSight,NULL,LevelMoveMethodMask_Ground);
				if (obj)
					NewAct<Act_AttackOnce3D>(&param->attackFlying)->Start(t,obj->GetID());
				else
				{
					_SetState(FlyingIdle);
					break;
				}
			}
			UpdateAct(t);

			if (TRUE)
			{
				if (((Act_AttackOnce3D*)_act)->GetResult()!=A_Pending)
				{
					ClearAct();
					_SetState(FlyingIdle);
					break;
				}
			}

			break;
		}
		case FlyDown:
		{
			if (!_act)
				NewAct<Act_FlyDown>(&param->flydown)->Start(t);

			if (TRUE)
			{
				UpdateAct(t);
				AResult result=((Act_FlyDown*)_act)->GetResult();
				if (result==A_Ok)
				{
					ClearAct();
					_SetState(Idle);
				}
				else
				{
					if (result==A_Fail)
					{
						ClearAct();
						_SetState(FlyingIdle);
					}
				}
			}
			break;
		}
	}

}


void AI_FlyingRat::OnEvent(LevelEvent &e)
{

}
