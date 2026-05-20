/********************************************************************
	created:	2023/07/29
	author:		cxi
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelUtil.h"

#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "Skill_GeneralAdvS.h"

#include "BgnLocoSkill.h"

IMPLEMENT_CLASS(BMO_LocoSkillState);

////////////////////////////////////////////////////////////////////////
//CBgn_LocoSkillSetup

BIND_BGN_CLASS(CBgn_LocoSkillSetup,CBgp_LocoSkillSetup);

void CBgn_LocoSkillSetup::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_LocoSkillSetup*pad=_GetPad<CBgp_LocoSkillSetup>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=NULL;

	if (pad->varState!=StringID_Invalid)
	{
		BMO_LocoSkillState *state=NULL;
		state=_GetMem()->GetObj<BMO_LocoSkillState>(pad->varState);
		if (!state)
		{
			state=Class_New(BMO_LocoSkillState);
			_GetMem()->DepositObj(pad->varState,state);
		}

		state->Reset(&pad->setting);
	}
	_OutputOk(outputs,1,"结束");
}

//////////////////////////////////////////////////////////////////////////
//CBgn_LocoSkillSwitchStage
BIND_BGN_CLASS(CBgn_LocoSkillSwitchStage,CBgp_LocoSkillSwitchStage);

BOOL FindLocoNextDir(CLevelObj *lo,LevelPos posTarget,LevelPos &dirNext)
{
	CLevel *level=lo->GetLevel();
	CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();

	LevelPos posCur=lo->GetFramePos();
	LevelFace faceCur=lo->GetFrameFace();

	CUnit *unit=unitmgr->CreateUnit(posCur,faceCur,lo->GetRadius_(),5.0f,NULL);
	unit->RequestTargetPos(posTarget);

	lo->SetCollide_Ghost(TRUE);
	unitmgr->UpdateSingle(unit,0.05f);
	lo->SetCollide_Ghost(FALSE);

	if (unit->IsEnd())
	{
		unit->Destroy();
		return FALSE;
	}
	LevelPos posNew=unit->GetPos();
	dirNext=posNew-posCur;
	dirNext.safe_normalize();
	unit->Destroy();
	return TRUE;
}

BOOL GetLocoSkillTargetPos(CLevelObj *lo,BMO_LocoSkillState *state,LevelPos &posTarget)
{
	CLevelObj *loThreat=LevelUtil_GetThreat(lo);
	if (loThreat)
	{
		posTarget=loThreat->GetFramePos();
		return TRUE;
	}
	return FALSE;
}

void CBgn_LocoSkillSwitchStage::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_LocoSkillSwitchStage*pad=_GetPad<CBgp_LocoSkillSwitchStage>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();
	CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();

	if (pad->varState!=StringID_Invalid)
	{
		BMO_LocoSkillState *state=_GetMem()->GetObj<BMO_LocoSkillState>(pad->varState);
		if (state)
		{
			extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
			CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
			if (skill)
			{
				if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
				{
					Skill_GeneralAdvS *skillG=(Skill_GeneralAdvS *)skill;
					LocoSkillSetting *setting=state->setting;

					LevelPos posCur=lo->GetFramePos();
					LevelFace faceCur=lo->GetFrameFace();

					BOOL bNextDir=FALSE;
					LevelPos dirNext;
					if (!state->bRequestStop)
					{
						LevelPos posTarget;
						if (GetLocoSkillTargetPos(lo,state,posTarget))
						{
							if (FindLocoNextDir(lo,posTarget,dirNext))
							{
								switch(state->status)
								{
									case BMO_LocoSkillState::None:
									case BMO_LocoSkillState::Stop:
										bNextDir=TRUE;
										break;
									default:
									{
										LevelFace faceNext=LevelFaceFromDir(dirNext);
										if (fabsf(LevelFaceCalcYaw(faceCur,faceNext))<i_math::deg2rad(90.0f))
											bNextDir=TRUE;
										break;
									}
								}
							}
						}
					}

					if (bNextDir)
					{
						switch(state->status)
						{
							case BMO_LocoSkillState::None:
							case BMO_LocoSkillState::Stop:
							{
								skill->GetTarget().SetPos(posCur+dirNext*10.0f);

								LevelFace faceNext=LevelFaceFromDir(dirNext);
								LevelFaceYaw yaw=LevelFaceCalcYaw(faceCur,faceNext);

								if (yaw<i_math::deg2rad(-135.0f))
								{
									skillG->DoSwitchStage(setting->stages.startL180);
									state->bStartToLeft=TRUE;
								}
								else
								{
									if (yaw<i_math::deg2rad(-45.0f))
									{
										skillG->DoSwitchStage(setting->stages.startL90);
										state->bStartToLeft=TRUE;
									}
									else
									{
										if (yaw<i_math::deg2rad(45.0f))
										{
											skillG->DoSwitchStage(setting->stages.startFwd);
											state->bStartToLeft=FALSE;
										}
										else
										{
											if (yaw<i_math::deg2rad(135.0f))
											{
												skillG->DoSwitchStage(setting->stages.startR90);
												state->bStartToLeft=TRUE;
											}
											else
											{
												skillG->DoSwitchStage(setting->stages.startR180);
												state->bStartToLeft=TRUE;
											}
										}
									}
								}

								state->status=BMO_LocoSkillState::Start;

								break;
							}
							case BMO_LocoSkillState::Start:
							{
								skill->GetTarget().SetPos(posCur+dirNext*10.0f);

								if (state->bStartToLeft)
								{
									skillG->DoSwitchStage(setting->stages.cycleLStomp);
									state->status=BMO_LocoSkillState::CycleLStomp;
								}
								else
								{
									skillG->DoSwitchStage(setting->stages.cycleRStomp);
									state->status=BMO_LocoSkillState::CycleRStomp;
								}

								break;
							}
							case BMO_LocoSkillState::CycleLStomp:
							{
								skill->GetTarget().SetPos(posCur+dirNext*10.0f);
								skillG->DoSwitchStage(setting->stages.cycleRStomp);
								state->status=BMO_LocoSkillState::CycleRStomp;
								break;
							}
							case BMO_LocoSkillState::CycleRStomp:
							{
								skill->GetTarget().SetPos(posCur+dirNext*10.0f);
								skillG->DoSwitchStage(setting->stages.cycleLStomp);
								state->status=BMO_LocoSkillState::CycleLStomp;
								break;
							}
						}
					}
					else
					{
						//Need stop
						switch(state->status)
						{
							case BMO_LocoSkillState::Start:
							{
								if (state->bStartToLeft)
								{
									skillG->DoSwitchStage(setting->stages.stopLStomp);
									state->status=BMO_LocoSkillState::Stop;
								}
								else
								{
									skillG->DoSwitchStage(setting->stages.stopRStomp);
									state->status=BMO_LocoSkillState::Stop;
								}
								break;
							}
							case BMO_LocoSkillState::CycleLStomp:
							{
								skillG->DoSwitchStage(setting->stages.stopRStomp);
								state->status=BMO_LocoSkillState::Stop;
								break;
							}
							case BMO_LocoSkillState::CycleRStomp:
							{
								skillG->DoSwitchStage(setting->stages.stopLStomp);
								state->status=BMO_LocoSkillState::Stop;
								break;
							}
							case BMO_LocoSkillState::Stop:
							{
								state->status=BMO_LocoSkillState::None;
								state->bRequestStop=FALSE;
								break;
							}
						}
					}
				}
			}
		}
	}
	_OutputOk(outputs,1,"结束");
}


//////////////////////////////////////////////////////////////////////////
//CBgn_LocoSkillStop
BIND_BGN_CLASS(CBgn_LocoSkillStop,CBgp_LocoSkillStop);

void CBgn_LocoSkillStop::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_LocoSkillStop*pad=_GetPad<CBgp_LocoSkillStop>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	if (LevelUtil_GetCastingSkill(lo))
	{
		if (pad->varState!=StringID_Invalid)
		{
			BMO_LocoSkillState *state=_GetMem()->GetObj<BMO_LocoSkillState>(pad->varState);
			if (state)
			{
				if (state->status!=BMO_LocoSkillState::None)
				{
					state->bRequestStop=TRUE;
					return;
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}

void CBgn_LocoSkillStop::Update(BGNOutputs &outputs)
{
	CBgp_LocoSkillStop*pad=_GetPad<CBgp_LocoSkillStop>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	if (LevelUtil_GetCastingSkill(lo))
	{
		if (pad->varState!=StringID_Invalid)
		{
			BMO_LocoSkillState *state=_GetMem()->GetObj<BMO_LocoSkillState>(pad->varState);
			if (state->status!=BMO_LocoSkillState::None)
				return;
		}
	}

	_OutputOk(outputs,1,"结束");

}
