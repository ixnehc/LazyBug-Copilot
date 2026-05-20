/********************************************************************
	created:	2023/05/15 
	author:		cxi
	
	purpose:	 在EventZone里检测Threat范围
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_CheckEZone.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelUtil.h"
#include "LevelResources.h"

#include "Skill_GeneralAdvS.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_CheckEZone
BIND_BGN_CLASS(CBgnThreat_CheckEZone,CBgpThreat_CheckEZone);

void CBgnThreat_CheckEZone::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_CheckEZone*pad=_GetPad<CBgpThreat_CheckEZone>();

	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	AnimEventZone *ezone=NULL;
	AnimTick tSkillCasting=ANIMTICK_INFINITE;
	if (pad->_nmEvent!=StringID_Invalid)
	{
		SkillParam_GeneralAdvS::Stage *paramStage=NULL;
		LevelRecordSkill *recSkill=NULL;
		SkillParam_GeneralAdvS *paramSkill=NULL;
		if (!pad->_bCheckTimeRange)
		{
			if (pad->_idSkill!=RecordID_Invalid)
			{
				recSkill=level->GetRecords()->GetSkill(pad->_idSkill);
				if (recSkill)
					paramSkill=recSkill->GetParam<SkillParam_GeneralAdvS>();
				if (paramSkill)
				{
					if (pad->_nmSkillStage!=StringID_Invalid)
						paramStage=paramSkill->FindStage(pad->_nmSkillStage);
				}
			}
			else
			{
				CLevelSkill *skillCur=LevelUtil_GetCastingSkill(lo);
				if (skillCur)
					recSkill=skillCur->GetRec();
				if (recSkill)
					paramSkill=recSkill->GetParam<SkillParam_GeneralAdvS>();
				if (paramSkill)
				{
					if (pad->_nmSkillStage!=StringID_Invalid)
						paramStage=paramSkill->FindStage(pad->_nmSkillStage);
					else
					{
						paramStage=((Skill_GeneralAdvS*)skillCur)->GetStageParam();
						tSkillCasting=((Skill_GeneralAdvS*)skillCur)->GetCastingTime();
					}
				}
			}
		}
		else
		{
			CLevelSkill *skillCur=LevelUtil_GetCastingSkill(lo);
			if (skillCur)
				recSkill=skillCur->GetRec();
			if (recSkill)
				paramSkill=recSkill->GetParam<SkillParam_GeneralAdvS>();
			if (paramSkill)
			{
				paramStage=((Skill_GeneralAdvS*)skillCur)->GetStageParam();
				tSkillCasting=((Skill_GeneralAdvS*)skillCur)->GetCastingTime();
			}
		}

		if (paramStage)
		{
			if (paramStage->idPathRes!=RecordID_Invalid)
			{
				LevelPathes *pathes=level->GetResources()->FindPathes(paramStage->idPathRes);
				if (pathes)
				{
					LevelPathesEvent *e=pathes->FindEvent(pad->_nmEvent);
					if (e)
						ezone=&e->zone;
				}
			}
		}
		else
		{
			extern AnimEventZone *LevelUtil_FindEZone(CLevel *level,SkillParam_GeneralAdvS *paramSkill,SkillParam_GeneralAdvS::Stage* paramSkillStage,StringID nmEvent);

			if (paramSkill)
			{
				if (!pad->_bCheckTimeRange)
					ezone=LevelUtil_FindEZone(level,paramSkill,NULL,pad->_nmEvent);
			}
		}

	}

	if (ezone)
	{
		BOOL bInTimeRange=TRUE;
		if (pad->_bCheckTimeRange)
		{
			if (!ezone->IsIn(tSkillCasting))
				bInTimeRange=FALSE;
		}

		if (bInTimeRange)
		{
			AnimTick t=tSkillCasting;
			if (t==ANIMTICK_INFINITE)
				t=ezone->t;

			i_math::xformf xfm;
			if (TRUE)
			{
				BOOL bXfm=FALSE;
				CLevelSkill *skillCur=LevelUtil_GetCastingSkill(lo);
				if (skillCur)
				{
					extern BOOL LevelUtil_CalcSkillCastingXfm(CLevelSkill *skill,i_math::xformf &xfm);
					if (LevelUtil_CalcSkillCastingXfm(skillCur,xfm))
						bXfm=TRUE;
				}

				if (!bXfm)
				{
					xfm.pos=lo->GetFramePos3D();
					LevelFaceToQuat(lo->GetFrameFace(),xfm.rot);
					xfm.scale_=lo->GetModelScale();
				}
			}


			AnimEventZone::KeyFan k;
			if (ezone->CalcKeyFan(t,k))
			{
				k.xfmCenter.applyBase(xfm);

				level->GetDbgDraw().DrawFan(lo->GetID(),k,ColorAlpha(0x0000ff,0xff),0.5f);

				LevelPos posTarget;
				if (_GetLevelSkillTarget_Pos(pad->_target,posTarget))
				{
					CLevelObj *target=_GetLevelSkillTarget_Obj(pad->_target);
					if (k.CheckIn(posTarget))
					{
						_OutputOk(outputs,1,"是");
						return;
					}
				}
			}
		}

	}

	_OutputFail(outputs,2,"否");
}


