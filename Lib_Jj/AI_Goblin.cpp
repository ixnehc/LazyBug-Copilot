/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	地精AI
*********************************************************************/
#include "stdh.h"

#include "AI_Goblin.h"
#include "Random/Random.h"

#include "LevelEvents.h"


#include "Level.h"

#include "LoUnit.h"

#include "Buff_ResideWT.h"

BIND_AIPARAM(AI_Goblin,AIParam_Goblin);

void AI_Goblin::_OnCreate()
{
}

void AI_Goblin::_OnDestroy()
{
	ClearAct();
}

void AI_Goblin::_OnInitialUpdate()
{
	AIParam_Goblin *param=(AIParam_Goblin*)_param;

	_SetState(Idle);
	if (AIUtil_FindBuff(_owner,Class_Ptr2(Buff_ResideWT)))
		_SetState(ResidedWT);

	//根据单位拿的武器决定_posture
	_posture=Unarmed;
	if (_owner->GetClass()->IsSameWith(Class_Ptr2(CLoUnit)))
	{
		CLoUnit *owner=(CLoUnit*)_owner;
		ExprEquips *equips=owner->GetExprEquips();
		if (equips)
		{
			RecordID id=equips->items[EquipPart_RHand];
			if (id==param->idRod)
				_posture=Rod;
			if (id==param->idSpear)
				_posture=Spear;
		}
	}

}



void AI_Goblin::_OnUpdate(AnimTick t)
{
	if (TRUE)
	{
		extern BOOL AIUtil_TestAnyBuff(CLevelObj *lo,DWORD flagBuff);
		if (AIUtil_TestAnyBuff(_owner,BuffFlag_Birth|BuffFlag_Dead|BuffFlag_LayDown))
			return;
	}

	AIParam_Goblin *param=(AIParam_Goblin*)_param;

	
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
				LevelMoveMethodMask methods=LevelMoveMethodMask_Ground;
				if (_posture==Spear)
					methods|=LevelMoveMethodMask_Flying;

				CLevelObj *lo=AIUtil_DetectPlayerUnit(_owner,param->rangeSight,methods);
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
			{
				extern CLevelObj *AIUtil_DetectClosestResidable(CLevelObj *lo,float range,CLevelObj *toIgnore,ClassUID uid);
				if (AIUtil_DetectClosestResidable(_owner,param->resideWT.radius,NULL,param->resideWT.uidAgent))
				{
					_SetState(ResidingWT);
					break;
				}
				switch(_posture)
				{
					case Unarmed:
						NewAct<Act_Attack>(&param->attackUnarmed)->Start(t,param->rangeSight);
						break;
					case Rod:
						NewAct<Act_Attack>(&param->attackRod)->Start(t,param->rangeSight);
						break;
					case Spear:
						NewAct<Act_Attack>(&param->attackSpear)->Start(t,param->rangeSight);
						break;
				}
			}

			if (_Is_x4())
			{
				UpdateAct(t);

				if (_Is_x8())
				{
					extern CLevelObj *AIUtil_DetectClosestResidable(CLevelObj *lo,float range,CLevelObj *toIgnore,ClassUID uid);
					if (AIUtil_DetectClosestResidable(_owner,param->resideWT.radius,NULL,param->resideWT.uidAgent))
					{
						ClearAct();
						_SetState(ResidingWT);
						break;
					}
				}

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
		case ResidingWT:
		{
			if (!_act)
			{
				extern Act_Reside *AIUtil_StartResideAct(CLevelObj *lo,ActParam_Reside *param);
				_act=AIUtil_StartResideAct(_owner,&param->resideWT);
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
						_SetState(ResidedWT);
					}
					else
					{
						if (result==A_Fail)
						{
							ClearAct();
							_SetState(Aggressive);
						}
					}
				}
			}
			else
				_SetState(Aggressive);
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
				}
			}
			break;
		}
		case ResidedWT:
		{
			if (!_act)
				NewAct<Act_Attack>(&param->attackWT)->Start(t,param->rangeSight);

			extern BOOL AIUtil_TestAnyBuff(CLevelObj *lo,DWORD flagBuff);
			if (!AIUtil_TestAnyBuff(_owner,BuffFlag_Reside))
			{
				ClearAct();
				_SetState(Aggressive);
				break;
			}

			if (_Is_x4())
			{
				UpdateAct(t);

			}

			break;
		}

	}

}


void AI_Goblin::OnEvent(LevelEvent &e)
{

}
