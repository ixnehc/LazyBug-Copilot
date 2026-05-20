/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	岩蝎AI
*********************************************************************/
#include "stdh.h"

#include "Act_Stroll.h"
#include "Act_Attack.h"
#include "Act_Flee.h"
#include "Act_Reside.h"

#include "AI_RockScorpion.h"
#include "Random/Random.h"

#include "LevelEvents.h"


#include "Level.h"

BIND_AIPARAM(AI_RockScorpion,AIParam_RockScorpion);

void AI_RockScorpion::_OnCreate()
{
	_SetState(Idle);

	_mirale=CSysRandom::RandRange(0.5f,1.0f);

}

void AI_RockScorpion::_OnDestroy()
{
	ClearAct();
}


void AI_RockScorpion::_DecideBattleState()
{
	if ((CSysRandom::RandRange(0.0f,1.0f)<_mirale))
		_SetState(Aggressive);
	else
	{
		if ((CSysRandom::RandRange(0.0f,1.0f)<_mirale))
			_SetState(Flee);
		else
			_SetState(Flee);
	}
}


void AI_RockScorpion::_OnUpdate(AnimTick t)
{
	if (TRUE)
	{
		extern BOOL AIUtil_TestAnyBuff(CLevelObj *lo,DWORD flagBuff);
		if (AIUtil_TestAnyBuff(_owner,BuffFlag_Birth|BuffFlag_Dead|BuffFlag_LayDown))
			return;
	}
	
	LevelEventMapFlag flags=_owner->GetLevel()->GetEventMap()->GetEventFlags(_owner->GetFramePos());
//	DWORD c;
//	LevelEvent **events=_owner->GetLevel()->GetEventMap()->GetEvents(_owner->GetFramePos(),c);

	AIParam_RockScorpion *param=(AIParam_RockScorpion*)_param;
	switch (_state)
	{
		case Idle:
		{
			if (!_act)
				NewAct<Act_Stroll>(&param->paramStroll)->Start(t);

			if (_Is_x4())
				UpdateAct(t);

			if (_Is_x4())
			{
				extern CLevelObj *AIUtil_DetectPlayerUnit(CLevelObj *lo,float range,LevelMoveMethodMask methods);
				CLevelObj *lo=AIUtil_DetectPlayerUnit(_owner,param->rangeSight,LevelMoveMethodMask_Ground);
				if (lo)
				{//发现有敌军,进入战斗状态
					ClearAct();
					_SetState(Aggressive);
				}
			}
			break;
		}
		case Aggressive:
		{
			if (!_act)
				NewAct<Act_Attack>(&param->paramAttack)->Start(t,param->rangeSight);

			if (flags&LevelEventMapFlag_PlayerKilling)
			{
				ClearAct();
				if (AIUtil_DetectClosestResidable(_owner,param->paramReside.radius,NULL,param->paramReside.uidAgent))
					_SetState(Residing);
				else
					_SetState(Flee);
				break;
			}

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
				NewAct<Act_Flee>(&param->paramFlee)->Start(t);
			if (_Is_x4())
			{
				UpdateAct(t);

				if (((Act_Flee*)_act)->IsTimeUp())
				{
					ClearAct();
					_SetState(Idle);
				}
			}
			break;
		}

		case Residing:
		{
			if (!_act)
			{
				extern Act_Reside *AIUtil_StartResideAct(CLevelObj *lo,ActParam_Reside *param);
				_act=AIUtil_StartResideAct(_owner,&param->paramReside);
				if (!_act)
				{
					_SetState(Aggressive);
					break;
				}
			}

			if(_act)
			{
				if (_Is_x4())
				{
					UpdateAct(t);

					AResult result=((Act_Reside*)_act)->GetResult();
					if (result==A_Ok)
					{
						ClearAct();
						_SetState(Resided);
					}
					else
					{
						if (result==A_Fail)
						{
							ClearAct();
							_SetState(Flee);
						}
					}
				}
			}
			else
				_SetState(Flee);
			break;
		}

	}

}
