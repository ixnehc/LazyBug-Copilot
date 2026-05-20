/********************************************************************
	created:	2019/07/29 
	author:		cxi
	
	purpose:	 对threat进行包围
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_Siege.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"

#include "Log/LogDump.h"

#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_Siege
BIND_BGN_CLASS(CBgnThreat_Siege,CBgpThreat_Siege);

void CBgnThreat_Siege::Destroy()
{
	_Finish(FALSE);
}

void CBgnThreat_Siege::_Finish(BOOL bStopMove)
{
	SAFE_RELEASE(_target);
	SAFE_DESTROY(_attrnode);

	CLevelObj *lo=_GetLo();
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (bStopMove)
	{
		if (driver)
			driver->StopMove();
	}

	if (TRUE)
	{
		CUnit *unit=lo->GetUnit();
		if (unit)
			unit->ClearOverrideFace();
	}

}


BOOL CBgnThreat_Siege::_Step(BOOL bFirstStep)
{
	CUnitMgrNavMesh *unitmgr=_GetLevel()->GetUnitMgr();
	CLevelObj *lo=_GetLo();
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return FALSE;

	if (!_target)
		return FALSE;

	CBgpThreat_Siege*pad=_GetPad<CBgpThreat_Siege>();
	
	LevelPos posCur=lo->GetFramePos();
	LevelPos posTarget=_target->GetFramePos();

	if (pad->bFaceTarget)
	{
		if (!bFirstStep)
		{
			LevelPos dir=posTarget-posCur;
			if (dir.getLengthSQ()>0.01f)
			{
				CUnit *unit=lo->GetUnit();
				if (unit)
					unit->OverrideFace(LevelFaceFromDir(dir));
			}
		}
	}

	float rad=atan2f(posCur.y-posTarget.y,posCur.x-posTarget.x);//当前的角度

	extern BOOL LevelUtil_IsMoving_(CLevelObj *lo);
	if (LevelUtil_IsMoving_(lo))
	{
		if (_bSiegePos)
		{
			if (_bRight)
			{
				if (i_math::judge_rotate_dir(rad,_radSiegeTolerance))
					return TRUE;//尚未转到角度
			}
			else
			{
				if (!i_math::judge_rotate_dir(rad,_radSiegeTolerance))
					return TRUE;//尚未转到角度
			}
		}
	}

	float dist=posCur.getDistanceFrom(posTarget);

	//找到合适的siege目标点(_posSiege)
	if (TRUE)
	{
		float radOff=30.0f*i_math::GRAD_PI2;
		float radSiege;
		LevelPos posSiege;
		float radSiegeTolerance;
		if (_bRight)
		{
			radSiege=rad+radOff;
			radSiegeTolerance=rad+radOff*0.6f;
		}
		else
		{
			radSiege=rad-radOff;
			radSiegeTolerance=rad-radOff*0.6f;
		}

		float radiusStart=i_math::clamp_f(dist,pad->distMin,pad->distMax);
		float stepRadius=0.25f;

		BOOL bExceedMax=FALSE,bExceedMin=FALSE;
		int idx=0;
		while((!bExceedMax)||(!bExceedMin))
		{
			float radius=radiusStart+GenScatteringStepValue(idx,stepRadius);
			idx++;

			if (radius>pad->distMax)
			{
				bExceedMax=TRUE;
				continue;
			}
			if (radius<pad->distMin)
			{
				bExceedMin=TRUE;
				continue;
			}

			posSiege=posTarget+LevelPos(cosf(radSiege)*radius,sinf(radSiege)*radius);
			LevelPos posHit;
			if (TRUE)
			{
				float distAhead=1.0f;
				float distSiegePos=posSiege.getDistanceFrom(posCur);
				if (distAhead>distSiegePos)
					distAhead=distSiegePos;
				LevelPos posAhead=posSiege;
				if (bFirstStep)//第一步更严格一点
				{
					posAhead=posSiege+(posSiege-posCur)*1.0f/distSiegePos;
					distAhead+=1.0f;
				}

				if (TRUE==unitmgr->StaticRayCast(UnitFindPath_Walkable,posCur,posAhead,posHit))
				{
					if (posHit.getDistanceSQFrom(posCur)<distAhead)
						continue;//有阻挡
					else
					{
						posSiege=posHit;
						break;
					}
				}
				else
				{
					posSiege=posAhead;
					break;
				}
			}
		}

		if(bExceedMax&&bExceedMin)
			return FALSE;

		//找到了
		_posSiege=posSiege;
		_radSiegeTolerance=radSiegeTolerance;
		_bSiegePos=1;
	}


	if (!bFirstStep)
	{
		LevelSkillTarget target;
		target.SetPos(_posSiege);
		driver->StartFollow(target,0.1f);
	}
	else
		driver->StopMove();

	return TRUE;
}


void CBgnThreat_Siege::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpThreat_Siege*pad=_GetPad<CBgpThreat_Siege>();
	_tStart=_GetT();
	_dur=CSysRandom::RandVaryUInt(pad->dur,pad->durVary);
	CLevelObj *lo=_GetLo();

	BOOL bOk=FALSE;
	CLevelObj *loTarget=_GetThreat();
	if (loTarget)
	{
		_target=loTarget;
		SAFE_ADDREF(_target);

		LevelPos posCur=lo->GetFramePos();
		LevelPos posTarget=loTarget->GetFramePos();
		_radius=posCur.getDistanceFrom(posTarget);
		_posTargetRecent=posTarget;


		CLevelObjMove *move=lo->GetMove();
		if (move)
		{
			SpeedMod *mod=move->ObtainSpeedMod();
			if (mod)
				_attrnode=mod->speed.Add(pad->speed,100);
		}

		_bRight=CSysRandom::Roll(0.5f);

		if (_Step(TRUE))
			bOk=TRUE;
		else
		{
			_bRight=!_bRight;
			if (_Step(TRUE))
				bOk=TRUE;
		}
	}

	if (bOk)
		return;

	_Finish(TRUE);

	_OutputOk(outputs,2,"中断");
}

void CBgnThreat_Siege::Update(BGNOutputs &outputs)
{
	CBgpThreat_Siege*pad=_GetPad<CBgpThreat_Siege>();
	AnimTick t=_GetT();
	CLevelObj *lo=_GetLo();

	if (t>_tStart+_dur)
	{
		_Finish(TRUE);
		_OutputOk(outputs,1,"结束");
		return;
	}

	extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
	if ((!_target->IsAlive())||LevelUtil_CheckDead(_target))
	{
		_Finish(TRUE);
		_OutputOk(outputs,1,"结束");
		return;
	}


	LevelPos posTarget=_target->GetFramePos();
	if (!posTarget.equals(_posTargetRecent))
	{
		LevelPos posCur=lo->GetFramePos();
		_radius=posCur.getDistanceFrom(posTarget);
		_posTargetRecent=posTarget;
	}

	if (!_Step(FALSE))
	{
		_Finish(TRUE);

		_OutputOk(outputs,2,"中断");
		return;
	}
}


void CBgnThreat_Siege::Break(BGNOutputs &outputs)
{
	_Finish(FALSE);
}


