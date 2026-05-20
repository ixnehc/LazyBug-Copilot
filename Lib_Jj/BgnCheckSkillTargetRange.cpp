/********************************************************************
	created:	2019/10/0 1
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordItemClass.h"

#include "BgnCheckSkillTargetRange.h"
#include "Skill_GeneralAdvS.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckSkillTargetRange,CBgp_CheckSkillTargetRange);

void CBgn_CheckSkillTargetRange::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckSkillTargetRange*pad=_GetPad<CBgp_CheckSkillTargetRange>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	if (CLevelSkill *skill=LevelUtil_GetCastingSkill(lo))
	{
		LevelPos posTarget;
		extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
		LevelUtil_CalcTargetPos(level,skill->GetTarget(),posTarget);

		LevelPos pos=lo->GetFramePos();
		LevelFace face=lo->GetFrameFace();

		LevelFace faceTarget=LevelFaceFromDir(posTarget-pos);

		LevelFace faceYaw=LevelFaceCalcYaw(face,faceTarget);

		if (pad->mode==CBgp_CheckSkillTargetRange::Mode_4Dir)
		{
			if ((faceYaw>=-i_math::Pi/4.0f)&&(faceYaw<=i_math::Pi/4.0f))
			{
				_OutputOk(outputs,1,"前");
				return;
			}
			if ((faceYaw<-i_math::Pi/4.0f)&&(faceYaw>=-i_math::Pi*3.0f/4.0f))
			{
				_OutputOk(outputs,3,"左");
				return;
			}
			if ((faceYaw<=i_math::Pi*3.0f/4.0f)&&(faceYaw>=i_math::Pi/4.0f))
			{
				_OutputOk(outputs,4,"右");
				return;
			}
			_OutputOk(outputs,2,"后");
			return;				
		}

		if (pad->mode==CBgp_CheckSkillTargetRange::Mode_FacingRange)
		{
			if ((faceYaw>=pad->_rngFace.low)&&(faceYaw<=pad->_rngFace.hi))
				_OutputOk(outputs,1,"是");
			else
				_OutputFail(outputs,2,"否");
			return;
		}

		if (pad->mode==CBgp_CheckSkillTargetRange::Mode_DistRange)
		{
			float dist=pos.getDistanceFrom(posTarget);
			if ((dist>=pad->_rngDist.low)&&(dist<=pad->_rngDist.hi))
				_OutputOk(outputs,1,"是");
			else
				_OutputFail(outputs,2,"否");
			return;
		}
	}

	_SetResult(A_Fail);
}

