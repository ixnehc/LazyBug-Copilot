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

#include "Bgn_飞妖_更新FlankAttack.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"


#include "Skill_GeneralAdvS.h"



////////////////////////////////////////////////////////////////////////
//CBgn_飞妖_更新FlankAttack
BIND_BGN_CLASS(CBgn_飞妖_更新FlankAttack,CBgp_飞妖_更新FlankAttack);


void CBgn_飞妖_更新FlankAttack::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_飞妖_更新FlankAttack*pad=_GetPad<CBgp_飞妖_更新FlankAttack>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	_t=level->GetT_();
	_tStart=_t;

	Update(outputs);

	return;
}

void CBgn_飞妖_更新FlankAttack::Update(BGNOutputs &outputs)
{
	CBgp_飞妖_更新FlankAttack*pad=_GetPad<CBgp_飞妖_更新FlankAttack>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	AnimTick dt;
	if (TRUE)
	{
		AnimTick t=level->GetT_();
		dt=ANIMTICK_SAFE_MINUS(t,_t);
		_t=t;
	}


	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
	if (skill)
	{
		if (!skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
			return;
		Skill_GeneralAdvS *skillG=(Skill_GeneralAdvS *)skill;

		LevelPos posTarget;
		LevelPos posSrc=lo->GetFramePos();

		if (TRUE)
		{
			extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
			LevelSkillTarget &target=skill->GetTarget();

			if (target.tp==LevelSkillTarget::Target_DefObj)
				_targetOrg=target;

			if (_targetOrg.tp==LevelSkillTarget::Target_DefObj)
			{
				if (LevelUtil_CheckDead(level,_targetOrg.ObjID()))
				{
					_SetNumber(pad->varNeedEnd,1);
					return;
				}
			}

			if (!LevelUtil_CalcTargetPos(level,_targetOrg,posTarget))
			{
				_SetNumber(pad->varNeedEnd,1);
				return;
			}
		}

		//更新posTarget
		if (!_bTargetPos)
		{
			_posTarget=posTarget;
			_posTargetLast=_posTarget;
			_bTargetPos=TRUE;
		}
		else
		{
			_posTargetLast=_posTarget;
			float dist=posTarget.getDistanceFrom(_posTarget);
			if (dist>0.001f)
			{
				LevelPos dir=posTarget-_posTarget;
				dir.normalize();
				float distMove=pad->spdCenterFollow*ANIMTICK_TO_SECOND(dt);
				if (distMove>dist)
					distMove=dist;

				_posTarget+=dir*distMove;
			}
			else
				_posTarget=posTarget;
		}

		LevelPos posPredict;
		if (TRUE)
		{
//			float distPredict=_swept*40.0f/(i_math::Pi*2.0f);
			float distPredict=(((float)ANIMTICK_SAFE_MINUS(_t,_tStart))/(float)pad->dur)*40.0f;
			posPredict=_posTarget+(_posTarget-_posTargetLast).normalize()*distPredict*ANIMTICK_TO_SECOND(dt);
		}

		BOOL bInside=FALSE;
		if (TRUE)
		{
			if (!_bPathFace)
				bInside=TRUE;
			else
			{
				LevelFace faceToTargetPos=LevelFaceFromDir(_posTarget-posSrc);
				LevelFaceYaw yaw=LevelFaceCalcYaw(_facePath,faceToTargetPos);
				if (!pad->bCW)
				{
					if (yaw<0.0f)
						bInside=TRUE;//逆时针转时,目标在左侧表示在圈内
				}
				else
				{
					if (yaw>0.0f)
						bInside=TRUE;//顺时针转时,目标在右侧表示在圈内
				}
			}
		}

		if (!bInside)
			_SetNumber(pad->varNeedEnd,1);

		//更新_posCenter
		float distToCenter;
		if (TRUE)
		{
			float dist=posSrc.getDistanceFrom(_posTarget);
			if (!bInside)
				dist=-1000.0f;

			LevelPos posCenter=_posTarget;
			if (dist>pad->radiusMax)
				dist=pad->radiusMax;
			else
			{
				if (dist<pad->radiusMin)
				{
					if (_bCenterPos)
						posCenter=_posCenter;

					dist=pad->radiusMin;
				}
			}

			distToCenter=dist;
			_posCenter=posCenter;
			_bCenterPos=TRUE;
		}


		if (!_bPathFace)
		{
			_facePath=LevelFaceFromDir(_posCenter-posSrc);
			if (!pad->bCW)
				LevelFaceApplyYaw(_facePath,i_math::Pi/2.0f);
			else
				LevelFaceApplyYaw(_facePath,-i_math::Pi/2.0f);
			_bPathFace=TRUE;
		}

		LevelFace faceToCenter=LevelFaceFromDir(_posCenter-posSrc);
		LevelFace faceToPredictedPos=LevelFaceFromDir(posPredict-posSrc);

		LevelFace faceToTarget=LevelFaceFromDir(_posTarget-posSrc);

		if (TRUE)
		{
			LevelFaceYaw yaw=LevelFaceCalcYaw(faceToCenter,faceToTarget);
			if (fabsf(yaw)>60.0f*i_math::GRAD_PI2)
			{
				//太偏了
				_SetNumber(pad->varNeedEnd,1);
			}
		}

		if (TRUE)
		{
			LevelFaceYaw yaw;
			if (TRUE)
			{
				LevelFace face=LevelFaceFromDir(posSrc-_posCenter);

				if (!pad->bCW)
					LevelFaceApplyYaw(face,-15.0f*i_math::GRAD_PI2);
				else
					LevelFaceApplyYaw(face,15.0f*i_math::GRAD_PI2);
				LevelPos posNew=_posCenter+LevelFaceToDir(face)*distToCenter;

				yaw=LevelFaceCalcYaw(faceToCenter,LevelFaceFromDir(posNew-posSrc));
			}

			_facePath=faceToCenter;
			LevelFaceApplyYaw(_facePath,yaw);

			yaw=LevelFaceCalcYaw(faceToPredictedPos,_facePath);

			skillG->SetPathFacingYaw(yaw);

			skillG->GetTarget().SetPos(posPredict);
		}

		if (TRUE) 
		{

			LevelFace dir=LevelFaceFromDir(posSrc-_posCenter);

			if (_dirLast>-10000.0f)
			{
				LevelFaceYaw yaw=LevelFaceCalcYaw(_dirLast,dir);
				if (!pad->bCW)
				{
					if (yaw<0.0f)
						_swept+=-yaw;
				}
				else
				{
					if (yaw>0.0f)
						_swept+=yaw;
				}
			}

			_dirLast=dir;
		}

//		if (_swept>i_math::Pi*2.0f)
		if (_t>_tStart+pad->dur)
			_SetNumber(pad->varNeedEnd,1);
		else
		{
			CLevelObj *loTooth=_GetLoFromVar(pad->varTooth);
			if (loTooth)
			{
				if (loTooth->GetFramePos().getDistanceFrom(posSrc)>pad->distMaxFromTooth)
					_SetNumber(pad->varNeedEnd,1);
			}
			else
				_SetNumber(pad->varNeedEnd,1);
		}
	}
}

