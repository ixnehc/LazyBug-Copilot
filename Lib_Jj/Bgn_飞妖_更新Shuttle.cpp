/********************************************************************
	created:	2019/12/22 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_飞妖_更新Shuttle.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"


#include "Skill_GeneralAdvS.h"

#include "Random/Random.h"



////////////////////////////////////////////////////////////////////////
//CBgn_飞妖_更新Shuttle
BIND_BGN_CLASS(CBgn_飞妖_更新Shuttle,CBgp_飞妖_更新Shuttle);

void CBgn_飞妖_更新Shuttle::_UpdateCenter(BOOL bFirst)
{
	CBgp_飞妖_更新Shuttle*pad=_GetPad<CBgp_飞妖_更新Shuttle>();
	CLevelObj *lo=_GetLo();
	if (TRUE)
	{
		CLevelObj *loTarget=_GetLoFromVar(pad->varTarget);
		if (loTarget)
			_posCenter=loTarget->GetFramePos();
		else
		{
			if (bFirst)
				_posCenter=lo->GetFramePos();
		}
	}
}

void CBgn_飞妖_更新Shuttle::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_飞妖_更新Shuttle*pad=_GetPad<CBgp_飞妖_更新Shuttle>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	_tLastUpdate=level->GetT_();
	_tNextAdjust=level->GetT_();

	_UpdateCenter(TRUE);

	Update(outputs);

	return;
}

void CBgn_飞妖_更新Shuttle::Update(BGNOutputs &outputs)
{
	CBgp_飞妖_更新Shuttle*pad=_GetPad<CBgp_飞妖_更新Shuttle>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	AnimTick t=level->GetT_();

	LevelPos posSrc=lo->GetFramePos();

	_UpdateCenter(FALSE);

	AnimTick dt=0;
	if (TRUE)
	{
		AnimTick tUpdate=t;
		dt=ANIMTICK_SAFE_MINUS(tUpdate,_tLastUpdate);
		_tLastUpdate=tUpdate;
	}

	if (TRUE)
	{
		BOOL bNeedAdjust=FALSE;
		if (t>=_tNextAdjust)
			bNeedAdjust=TRUE;

		//调整_spdRot_Target
		if (bNeedAdjust)
		{
			if (TRUE)
			{
				float dur=CSysRandom::RandRange(pad->durAdjustMin,pad->durAdjustMax);
				_tNextAdjust=t+ANIMTICK_FROM_SECOND(dur);
			}

			_spdRot_Target=CSysRandom::RandRange(-pad->spdRotMax,pad->spdRotMax)*i_math::GRAD_PI2;

			if (TRUE)
			{
				LevelFace faceToCenter=0.0f;
				faceToCenter=LevelFaceFromDir(_posCenter-posSrc);
				LevelFace off=i_math::normalize_radian(faceToCenter-lo->GetFrameFace());
				if (off>0.0f)
					_spdRot_Target=CSysRandom::RandRange(0.0f,pad->spdRotMax)*i_math::GRAD_PI2;
				else
					_spdRot_Target=CSysRandom::RandRange(-pad->spdRotMax,0.0f)*i_math::GRAD_PI2;
			}


			//Adjust
		}

		//计算_spdRot_Cur
		if (TRUE)
		{
			float acc=pad->accRot*i_math::GRAD_PI2;

			if (_spdRot_Cur<_spdRot_Target)
			{
				_spdRot_Cur+=acc*ANIMTICK_TO_SECOND(dt);
				if (_spdRot_Cur>_spdRot_Target)
					_spdRot_Cur=_spdRot_Target;
			}
			else
			{
				_spdRot_Cur-=acc*ANIMTICK_TO_SECOND(dt);
				if (_spdRot_Cur<_spdRot_Target)
					_spdRot_Cur=_spdRot_Target;
			}
		}

		if (pad->varRotSpd!=StringID_Invalid)
			_SetFloat(pad->varRotSpd,_spdRot_Cur*i_math::GRAD_PI);

		//计算Target
		LevelPos posTarget;
		if(TRUE)
		{
			LevelFace face=lo->GetFrameFace();
			face+=_spdRot_Cur*ANIMTICK_TO_SECOND(dt);
			posTarget=posSrc+LevelFaceToDir(face)*10.0f;
		}

		extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
		CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
		if (skill)
		{
			LevelPos posSrc=lo->GetFramePos();

			if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
			{
				Skill_GeneralAdvS *skillG=(Skill_GeneralAdvS *)skill;

				skillG->GetTarget().SetAim(posTarget);

				if (_spdRot_Cur<0.0f)
					skillG->SetPhysObstacleAvoidDir(SkillParam_GeneralAdvS::PhysObstacleAvoidDir_CW);
				else
					skillG->SetPhysObstacleAvoidDir(SkillParam_GeneralAdvS::PhysObstacleAvoidDir_CCW);
			}
		}

	}

}
