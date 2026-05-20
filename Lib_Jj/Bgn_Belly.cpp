/********************************************************************
	created:	2023/09/22 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_Belly.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoBelly.h"
#include "LoMagicCircuit.h"
#include "LoUnit.h"

#include "Skill_GeneralAdvS.h"


////////////////////////////////////////////////////////////////////////
//CBgn_BellyMinion_RequestAction
BIND_BGN_CLASS(CBgn_BellyMinion_RequestAction,CBgp_BellyMinion_RequestAction);

void CBgn_BellyMinion_RequestAction::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_BellyMinion_RequestAction*pad=_GetPad<CBgp_BellyMinion_RequestAction>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	CLoBelly *loBelly=(CLoBelly *)level->GetUniqueObj(LevelUniqueObj_Belly);
	if (loBelly)
	{
		LevelPos posTarget;
		LevelObjID idTarget;
		CBellyMinionCombatState::ActionType action=loBelly->RequestMinionAction(lo->GetID(),posTarget,idTarget);
		switch(action)
		{
			case CBellyMinionCombatState::ActionType_Hop:
			{
				_SetPos(pad->nmVarPos,posTarget);
				_SetID(pad->nmVarObjID,BehaviorMemType_ObjID,idTarget);

				_OutputOk(outputs,2,"Hop");
				return;
			} 
			case CBellyMinionCombatState::ActionType_StompEgg:
			{
				_SetPos(pad->nmVarPos,posTarget);
				_SetID(pad->nmVarObjID,BehaviorMemType_ObjID,idTarget);

				_OutputOk(outputs,3,"StompEgg");
				return;
			} 
		}
	}
	_OutputOk(outputs,1,"NoAction");
}


////////////////////////////////////////////////////////////////////////
//CBgn_BellyKing_RequestPos
BIND_BGN_CLASS(CBgn_BellyKing_RequestPos,CBgp_BellyKing_RequestPos);

void CBgn_BellyKing_RequestPos::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_BellyKing_RequestPos*pad=_GetPad<CBgp_BellyKing_RequestPos>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	CLoBelly *loBelly=(CLoBelly *)level->GetUniqueObj(LevelUniqueObj_Belly);
	if (loBelly)
	{
		LevelPos posTarget;
		if (pad->tp==0)
		{
			if(loBelly->RequestKingSpawnEgg(pad->rangeMin,pad->rangeMax,posTarget))
			{
				_SetPos(pad->nmVarPos,posTarget);
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->tp==1)
		{
			if(loBelly->RequestKingEvadeJump(pad->rangeMin,pad->rangeMax,posTarget))
			{
				_SetPos(pad->nmVarPos,posTarget);
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->tp==2)
		{
			if(loBelly->RequestKingApproachPos(posTarget))
			{
				_SetPos(pad->nmVarPos,posTarget);
				_OutputOk(outputs,1,"成功");
				return;
			}
		}

	}
	_OutputFail(outputs,2,"失败");
}


////////////////////////////////////////////////////////////////////////
//CBgn_BellyKing_RequestPos
BIND_BGN_CLASS(CBgn_BellyEelOp,CBgp_BellyEelOp);

void CBgn_BellyEelOp::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_BellyEelOp*pad=_GetPad<CBgp_BellyEelOp>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	Skill_GeneralAdvS *skillG=NULL;
	if (TRUE)
	{
		extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
		CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
		if (skill)
		{
			if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
				skillG=(Skill_GeneralAdvS *)skill;
		}
	}

	if (skillG)
	{
		CLoBelly *loBelly=(CLoBelly *)level->GetUniqueObj(LevelUniqueObj_Belly);
		if (loBelly)
		{
			LevelPos posCur=lo->GetFramePos();
			if (pad->tp==0)
			{
				CLoUnit *loPlayer=LevelUtil_GetFirstPlayerLoUnit(level);
				if (loPlayer)
				{
					LevelPos posTarget=loPlayer->GetFramePos();
					if (loBelly->ValidateEelTargetPos(posCur,posTarget))
					{
						skillG->GetTarget().SetObjID(loPlayer->GetID());
						_OutputOk(outputs,1,"成功");
						return;
					}
				}
			}
			if (pad->tp==1)
			{
				LevelPos posTarget;
				if(loBelly->FindValidEelTargetPos(posCur,4.0f,posTarget))
				{
					skillG->GetTarget().SetPos(posTarget);
					_OutputOk(outputs,1,"成功");
					return;
				}
			}
			if (pad->tp==2)
			{
				CLoMagicCircuit *loMagicCircuit=(CLoMagicCircuit *)level->GetUniqueObj(LevelUniqueObj_MagicCircuit);

				if (loMagicCircuit)
				{
					LevelObjID idRelay=loMagicCircuit->FindTargetRelayForEel(posCur);
					if (idRelay)
					{
						skillG->GetTarget().SetObjID(idRelay);
						_OutputOk(outputs,1,"成功");
						return;
					}
				}
			}
			if (pad->tp == 3)
			{
				CLoMagicCircuit* loMagicCircuit = (CLoMagicCircuit*)level->GetUniqueObj(LevelUniqueObj_MagicCircuit);
				if (loMagicCircuit)
				{
					LevelObjID idNode = loMagicCircuit->FindEelNetworkNodeID(posCur);
					if (idNode != LevelObjID_Invalid)
					{
						LevelObjID idNext = loMagicCircuit->GetEelRoadNetwork().FindReparingTarget(idNode);
						if (idNext==LevelObjID_Invalid)
							idNext = loMagicCircuit->GetEelRoadNetwork().FindNextStepForRepairingRoad(idNode);
						if (idNext != LevelObjID_Invalid)
						{
							skillG->GetTarget().SetObjID(idNext);
							_OutputOk(outputs, 1, "成功");
							return;
						}
					}
				}
			}
			if (pad->tp == 4)
			{
				CLoMagicCircuit* loMagicCircuit = (CLoMagicCircuit*)level->GetUniqueObj(LevelUniqueObj_MagicCircuit);
				if (loMagicCircuit)
				{
					LevelObjID idNode = loMagicCircuit->FindEelNetworkNodeID(posCur);
					if (idNode != LevelObjID_Invalid)
					{
						LevelObjID idSkillTarget = LevelUtil_GetTargetObjID(level, skillG->GetTarget());
						if (idSkillTarget != LevelObjID_Invalid)
						{
							if (loMagicCircuit->GetEelRoadNetwork().CanRepair(idNode, idSkillTarget))
							{
								_OutputOk(outputs, 1, "成功");
								return;
							}
						}
					}
				}
			}
		}
	}

	_OutputFail(outputs,2,"失败");
}
